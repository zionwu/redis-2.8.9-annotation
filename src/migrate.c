#include "redis.h"
#include "endianconv.h"

/* -----------------------------------------------------------------------------
 * DUMP, RESTORE and MIGRATE commands
 * -------------------------------------------------------------------------- */

/* Generates a DUMP-format representation of the object 'o', adding it to the
 * io stream pointed by 'rio'. This function can't fail. */
//将给定的robj以固定格式写到rio中
void createDumpPayload(rio *payload, robj *o) {
    unsigned char buf[2];
    uint64_t crc;

    /* Serialize the object in a RDB-like format. It consist of an object type
     * byte followed by the serialized object. This is understood by RESTORE. */
    //初始化rio
    rioInitWithBuffer(payload,sdsempty());
    //保存类型
    redisAssert(rdbSaveObjectType(payload,o));
    //保存对象
    redisAssert(rdbSaveObject(payload,o));

    /* Write the footer, this is how it looks like:
     * ----------------+---------------------+---------------+
     * ... RDB payload | 2 bytes RDB version | 8 bytes CRC64 |
     * ----------------+---------------------+---------------+
     * RDB version and CRC are both in little endian.
     */

    /* RDB version */
    buf[0] = REDIS_RDB_VERSION & 0xff;
    buf[1] = (REDIS_RDB_VERSION >> 8) & 0xff;
    //保存版本
    payload->io.buffer.ptr = sdscatlen(payload->io.buffer.ptr,buf,2);

    /* CRC64 */
    crc = crc64(0,(unsigned char*)payload->io.buffer.ptr,
                sdslen(payload->io.buffer.ptr));
    memrev64ifbe(&crc);
    //保存crc
    payload->io.buffer.ptr = sdscatlen(payload->io.buffer.ptr,&crc,8);
}

/* Verify that the RDB version of the dump payload matches the one of this Redis
 * instance and that the checksum is ok.
 * If the DUMP payload looks valid REDIS_OK is returned, otherwise REDIS_ERR
 * is returned. */
//检查rbd的版本和crc64的正确性
int verifyDumpPayload(unsigned char *p, size_t len) {
    unsigned char *footer;
    uint16_t rdbver;
    uint64_t crc;

    /* At least 2 bytes of RDB version and 8 of CRC64 should be present. */
    if (len < 10) return REDIS_ERR;
    footer = p+(len-10);

    /* Verify RDB version */
    //验证rbd版本
    rdbver = (footer[1] << 8) | footer[0];
    if (rdbver != REDIS_RDB_VERSION) return REDIS_ERR;

    /* Verify CRC64 */
    //验证数据的crc
    crc = crc64(0,p,len-8);
    memrev64ifbe(&crc);
    return (memcmp(&crc,footer+2,8) == 0) ? REDIS_OK : REDIS_ERR;
}

/* DUMP keyname
 * DUMP is actually not used by Redis Cluster but it is the obvious
 * complement of RESTORE and can be useful for different applications. */
//dump命令的实现
void dumpCommand(redisClient *c) {
    robj *o, *dumpobj;
    rio payload;

    /* Check if the key is here. */
    if ((o = lookupKeyRead(c->db,c->argv[1])) == NULL) {
        addReply(c,shared.nullbulk);
        return;
    }

    /* Create the DUMP encoded representation. */
    createDumpPayload(&payload,o);

    /* Transfer to the client */
    dumpobj = createObject(REDIS_STRING,payload.io.buffer.ptr);
    addReplyBulk(c,dumpobj);
    decrRefCount(dumpobj);
    return;
}

/* RESTORE key ttl serialized-value */
//restore命令的实现
void restoreCommand(redisClient *c) {
    long long ttl;
    rio payload;
    int type;
    robj *obj;

    /* Make sure this key does not already exist here... */
    if (lookupKeyWrite(c->db,c->argv[1]) != NULL) {
        addReplyError(c,"Target key name is busy.");
        return;
    }

    /* Check if the TTL value makes sense */
    //验证ttl是否有效
    if (getLongLongFromObjectOrReply(c,c->argv[2],&ttl,NULL) != REDIS_OK) {
        return;
    } else if (ttl < 0) {
        addReplyError(c,"Invalid TTL value, must be >= 0");
        return;
    }

    /* Verify RDB version and data checksum. */
    //验证RDB版本和数据校验和有效
    if (verifyDumpPayload(c->argv[3]->ptr,sdslen(c->argv[3]->ptr)) == REDIS_ERR)
    {
        addReplyError(c,"DUMP payload version or checksum are wrong");
        return;
    }

    rioInitWithBuffer(&payload,c->argv[3]->ptr);
    //调用RDB的API从数据中恢复出robj
    if (((type = rdbLoadObjectType(&payload)) == -1) ||
        ((obj = rdbLoadObject(type,&payload)) == NULL))
    {
        addReplyError(c,"Bad data format");
        return;
    }

    /* Create the key and set the TTL if any */
    //将key添加到db中
    dbAdd(c->db,c->argv[1],obj);
    if (ttl) setExpire(c->db,c->argv[1],mstime()+ttl);
    signalModifiedKey(c->db,c->argv[1]);
    addReply(c,shared.ok);
    server.dirty++;
}

/* MIGRATE host port key dbid timeout */
//migrate命令的实现
void migrateCommand(redisClient *c) {
    int fd;
    long timeout;
    long dbid;
    long long ttl = 0, expireat;
    robj *o;
    rio cmd, payload;

    /* Sanity check */
    if (getLongFromObjectOrReply(c,c->argv[5],&timeout,NULL) != REDIS_OK)
        return;
    if (getLongFromObjectOrReply(c,c->argv[4],&dbid,NULL) != REDIS_OK)
        return;
    if (timeout <= 0) timeout = 1;

    /* Check if the key is here. If not we reply with success as there is
     * nothing to migrate (for instance the key expired in the meantime), but
     * we include such information in the reply string. */
    if ((o = lookupKeyRead(c->db,c->argv[3])) == NULL) {
        addReplySds(c,sdsnew("+NOKEY\r\n"));
        return;
    }
    
    /* Connect */
    //连接到给定host的端口
    fd = anetTcpNonBlockConnect(server.neterr,c->argv[1]->ptr,
                atoi(c->argv[2]->ptr));
    if (fd == -1) {
        addReplyErrorFormat(c,"Can't connect to target node: %s",
            server.neterr);
        return;
    }

    //等待该套接字可写
    if ((aeWait(fd,AE_WRITABLE,timeout*1000) & AE_WRITABLE) == 0) {
        addReplySds(c,sdsnew("-IOERR error or timeout connecting to the client\r\n"));
        return;
    }

    /* Create RESTORE payload and generate the protocol to call the command. */
    //将恢复该key的restore命令的形式写到rio中
    rioInitWithBuffer(&cmd,sdsempty());
    //写进 "*2\r\n$6\r\nSELECT\r\n$len\r\ndbid\r\n"
    redisAssertWithInfo(c,NULL,rioWriteBulkCount(&cmd,'*',2));
    redisAssertWithInfo(c,NULL,rioWriteBulkString(&cmd,"SELECT",6));
    redisAssertWithInfo(c,NULL,rioWriteBulkLongLong(&cmd,dbid));

    expireat = getExpire(c->db,c->argv[3]);
    if (expireat != -1) {
        ttl = expireat-mstime();
        if (ttl < 1) ttl = 1;
    }

    //写进“*4\r\n$7\r\nRESTORE\r\n$kenlen\r\nkey\r\n$ttllen\r\nttl\r\n”
    redisAssertWithInfo(c,NULL,rioWriteBulkCount(&cmd,'*',4));
    redisAssertWithInfo(c,NULL,rioWriteBulkString(&cmd,"RESTORE",7));
    redisAssertWithInfo(c,NULL,c->argv[3]->encoding == REDIS_ENCODING_RAW);
    redisAssertWithInfo(c,NULL,rioWriteBulkString(&cmd,c->argv[3]->ptr,sdslen(c->argv[3]->ptr)));
    redisAssertWithInfo(c,NULL,rioWriteBulkLongLong(&cmd,ttl));

    /* Finally the last argument that is the serailized object payload
     * in the DUMP format. */
    //将key的数据以RDB形式存到缓冲区中
    createDumpPayload(&payload,o);
    //将缓冲区写到rio
    redisAssertWithInfo(c,NULL,rioWriteBulkString(&cmd,payload.io.buffer.ptr,
                                sdslen(payload.io.buffer.ptr)));
    sdsfree(payload.io.buffer.ptr);

    /* Tranfer the query to the other node in 64K chunks. */
    //将rio缓冲区数据写到socket中
    {
        sds buf = cmd.io.buffer.ptr;
        size_t pos = 0, towrite;
        int nwritten = 0;

        while ((towrite = sdslen(buf)-pos) > 0) {
        	//每次最多写64KB
            towrite = (towrite > (64*1024) ? (64*1024) : towrite);
            nwritten = syncWrite(fd,buf+pos,towrite,timeout);
            if (nwritten != (signed)towrite) goto socket_wr_err;
            pos += nwritten;
        }
    }

    /* Read back the reply. */
    //从套接字中读出响应
    {
        char buf1[1024];
        char buf2[1024];

        /* Read the two replies */
        if (syncReadLine(fd, buf1, sizeof(buf1), timeout) <= 0)
            goto socket_rd_err;
        if (syncReadLine(fd, buf2, sizeof(buf2), timeout) <= 0)
            goto socket_rd_err;
        if (buf1[0] == '-' || buf2[0] == '-') {
            addReplyErrorFormat(c,"Target instance replied with error: %s",
                (buf1[0] == '-') ? buf1+1 : buf2+1);
        } else {
            robj *aux;

            dbDelete(c->db,c->argv[3]);
            signalModifiedKey(c->db,c->argv[3]);
            addReply(c,shared.ok);
            server.dirty++;

            /* Translate MIGRATE as DEL for replication/AOF. */
            aux = createStringObject("DEL",3);
            rewriteClientCommandVector(c,2,aux,c->argv[3]);
            decrRefCount(aux);
        }
    }

    sdsfree(cmd.io.buffer.ptr);
    close(fd);
    return;

socket_wr_err:
    addReplySds(c,sdsnew("-IOERR error or timeout writing to target instance\r\n"));
    sdsfree(cmd.io.buffer.ptr);
    close(fd);
    return;

socket_rd_err:
    addReplySds(c,sdsnew("-IOERR error or timeout reading from target node\r\n"));
    sdsfree(cmd.io.buffer.ptr);
    close(fd);
    return;
}
