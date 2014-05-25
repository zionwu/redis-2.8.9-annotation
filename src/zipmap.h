/* String -> String Map data structure optimized for size.
 *
 * See zipmap.c for more info.
 *
 * --------------------------------------------------------------------------
 *
 * Copyright (c) 2009-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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

//zipmap也是一种内存数据结构，存放的是key和value。详细解释参照.c文件中注释。

#ifndef _ZIPMAP_H
#define _ZIPMAP_H

//构造一个新的zipmap
unsigned char *zipmapNew(void);

//在zipmap中添加一个新的key/value, 如果key存在则更新旧的值
unsigned char *zipmapSet(unsigned char *zm, unsigned char *key, unsigned int klen, unsigned char *val, unsigned int vlen, int *update);

//从zipmap中删除给定key对应的entry
unsigned char *zipmapDel(unsigned char *zm, unsigned char *key, unsigned int klen, int *deleted);

//回到zipmap的开头
unsigned char *zipmapRewind(unsigned char *zm);

//将zm当前指向的entry的值取出，并且返回指向下一个entry的指针
unsigned char *zipmapNext(unsigned char *zm, unsigned char **key, unsigned int *klen, unsigned char **value, unsigned int *vlen);

//取到key所指entry的值
int zipmapGet(unsigned char *zm, unsigned char *key, unsigned int klen, unsigned char **value, unsigned int *vlen);

//检查zipmap中是否存在key
int zipmapExists(unsigned char *zm, unsigned char *key, unsigned int klen);

//zipmap中entry的数量
unsigned int zipmapLen(unsigned char *zm);

//zipmap所占用内存字节数
size_t zipmapBlobLen(unsigned char *zm);

//打印处zipmap中所存数据
void zipmapRepr(unsigned char *p);

#endif
