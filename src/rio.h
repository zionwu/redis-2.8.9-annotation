/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __REDIS_RIO_H
#define __REDIS_RIO_H

#include <stdio.h>
#include <stdint.h>
#include "sds.h"


//redis io使用的结构体
struct _rio {
    /* Backend functions.
     * Since this functions do not tolerate short writes or reads the return
     * value is simplified to: zero on error, non zero on complete success. */
	//函数指针，指向读函数
    size_t (*read)(struct _rio *, void *buf, size_t len);
    //函数指针，指向写函数
    size_t (*write)(struct _rio *, const void *buf, size_t len);
    //函数指针
    off_t (*tell)(struct _rio *);
    /* The update_cksum method if not NULL is used to compute the checksum of
     * all the data that was read or written so far. The method should be
     * designed so that can be called with the current checksum, and the buf
     * and len fields pointing to the new block of data to add to the checksum
     * computation. */
    //函数指针，指向更新所有读过或写过数据的checksum的函数
    void (*update_cksum)(struct _rio *, const void *buf, size_t len);

    /* The current checksum */
    //当前处理过数据的checksum
    uint64_t cksum;

    /* number of bytes read or written */
    //读或写过的字节数
    size_t processed_bytes;

    /* maximum single read or write chunk size */
    //每次读写最大的字节数
    size_t max_processing_chunk;

    /* Backend-specific vars. */
    union {
        struct {
            sds ptr; //作为缓存区的sds
            off_t pos; //表示当前操作的位置
        } buffer;
        struct {
            FILE *fp; //文件指针
            off_t buffered; /* Bytes written since last fsync. */ //当前缓存(未写到设备)的字节数
            off_t autosync; /* fsync after 'autosync' bytes written. */ //写了autosync字节后，要将数据写到设备
        } file;
    } io;
};

typedef struct _rio rio;

/* The following functions are our interface with the stream. They'll call the
 * actual implementation of read / write / tell, and will update the checksum
 * if needed. */
//基于redis io写函数
static inline size_t rioWrite(rio *r, const void *buf, size_t len) {
    while (len) {
    	//获得这次写的字节数。最大不能超过max_processing_chunk
        size_t bytes_to_write = (r->max_processing_chunk && r->max_processing_chunk < len) ? r->max_processing_chunk : len;
        //更新checksum
        if (r->update_cksum) r->update_cksum(r,buf,bytes_to_write);
        if (r->write(r,buf,bytes_to_write) == 0)
            return 0;
        buf = (char*)buf + bytes_to_write;
        len -= bytes_to_write;
        r->processed_bytes += bytes_to_write;
    }
    return 1;
}

//基于redis io的读函数
static inline size_t rioRead(rio *r, void *buf, size_t len) {
    while (len) {
    	//获得这次读的字节数。最大不能超过max_processing_chunk
        size_t bytes_to_read = (r->max_processing_chunk && r->max_processing_chunk < len) ? r->max_processing_chunk : len;
        if (r->read(r,buf,bytes_to_read) == 0)
            return 0;
        //更新checksum
        if (r->update_cksum) r->update_cksum(r,buf,bytes_to_read);
        buf = (char*)buf + bytes_to_read;
        len -= bytes_to_read;
        r->processed_bytes += bytes_to_read;
    }
    return 1;
}

//tell函数，返回当前操作的位置
static inline off_t rioTell(rio *r) {
    return r->tell(r);
}

//设置基于文件的io的文件指针
void rioInitWithFile(rio *r, FILE *fp);

//设置基于缓存的io的缓存区
void rioInitWithBuffer(rio *r, sds s);

//将参数以"*<count>\r\n"形式写到rio中
size_t rioWriteBulkCount(rio *r, char prefix, int count);

//将buf内的内容以"$<count>\r\n<payload>\r\n"形式写到rio中. count是参数len的字符表示，payload是buf中内容
size_t rioWriteBulkString(rio *r, const char *buf, size_t len);

//将longlong型的l以"$<count>\r\n<payload>\r\n"写进rio。
//count是l字符串表示的长度，payload是l的字符串表示
size_t rioWriteBulkLongLong(rio *r, long long l);

//将double型的l以"$<count>\r\n<payload>\r\n"写进rio。
//count是d字符串表示的长度，payload是d的字符串表示
size_t rioWriteBulkDouble(rio *r, double d);

//更新redis io内的处理过数据的checksum
void rioGenericUpdateChecksum(rio *r, const void *buf, size_t len);

//设置autosync值，当写的数据字节数超过autosync时，缓存区内数据将被写到文件并同步到设备
void rioSetAutoSync(rio *r, off_t bytes);

#endif
