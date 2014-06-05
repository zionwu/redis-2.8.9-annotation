/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds;


//sds的结构体
struct sdshdr {
    int len;    //sds的长度
    int free;   //空闲的长度
    char buf[]; //保存string的指针
};

//取到sds的长度。
static inline size_t sdslen(const sds s) {
	//s是string开始的地方，s减去sizeof(struct sdshdr)后sh指向string对应sdshdr结构体
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    //返回sdshdr->len
    return sh->len;
}

//取到sds中多余的空间大小
static inline size_t sdsavail(const sds s) {
	//指向sds对应的sdshdr
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    //返回free字段
    return sh->free;
}

//创建一个长度为initlen，string为init的的sds
sds sdsnewlen(const void *init, size_t initlen);

//创建一个指向init的sds
sds sdsnew(const char *init);

//创建一个空的sds
sds sdsempty(void);

//取到sds中string的长度
size_t sdslen(const sds s);

//复制一个s
sds sdsdup(const sds s);

//释放sds s的内存
void sdsfree(sds s);

//取到sds s中的空余的内存大小
size_t sdsavail(const sds s);

//将sds s的长度扩展值len,额外的空间用0填充
sds sdsgrowzero(sds s, size_t len);

//将长度为len的string t加到sds s的后面
sds sdscatlen(sds s, const void *t, size_t len);

//将string t加到sds s的后面
sds sdscat(sds s, const char *t);

//将sds t添加到sds s后面
sds sdscatsds(sds s, const sds t);

//将sds s的string内容变成t所指stirng
sds sdscpylen(sds s, const char *t, size_t len);

//将sds s的string内容变成t所指stirng
sds sdscpy(sds s, const char *t);

//将ap内参数填入fmt模式后添加到sds s后面
sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

//从sds s的左右两边移除cset中字符
sds sdstrim(sds s, const char *cset);

//将sds s剪切成从start 到end的string
void sdsrange(sds s, int start, int end);

//更新sds的长度
void sdsupdatelen(sds s);

//将sds s的string清除
void sdsclear(sds s);

//比较sds s1与s2
int sdscmp(const sds s1, const sds s2);

//根据sep将s分隔，返回分隔后每一子串对应的sds
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);

//对tokens内的sds调用sdsfree释放内存
void sdsfreesplitres(sds *tokens, int count);

//将sds内的string变成小写
void sdstolower(sds s);

//将sds内的string变成大写
void sdstoupper(sds s);

//从longlong型的value构造sds
sds sdsfromlonglong(long long value);

//
sds sdscatrepr(sds s, const char *p, size_t len);

//
sds *sdssplitargs(const char *line, int *argc);

//
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);

//将argv的字符串以sep连接起来，返回对应的sds
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
//为sds s的内存增加addlen字节
sds sdsMakeRoomFor(sds s, size_t addlen);

//将sds的长度增长incr
void sdsIncrLen(sds s, int incr);

//移除sds中的多余的内存
sds sdsRemoveFreeSpace(sds s);

//返回sds占用的总内存大小
size_t sdsAllocSize(sds s);

#endif
