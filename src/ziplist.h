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

#define ZIPLIST_HEAD 0
#define ZIPLIST_TAIL 1

/**
 * ziplist也是一种内存数据结构。里面存放的值可以是string或者是不同类型的int。
 * 具体的解释可参照.c文件中的注释。
 */

//创建一个新的ziplist
unsigned char *ziplistNew(void);

//在ziplist的表头或者表尾（根据where）插入新的元素
unsigned char *ziplistPush(unsigned char *zl, unsigned char *s, unsigned int slen, int where);

//根据index的值取到ziplist中元素
unsigned char *ziplistIndex(unsigned char *zl, int index);

//取到p的下一个元素
unsigned char *ziplistNext(unsigned char *zl, unsigned char *p);

//取到p的上一个元素
unsigned char *ziplistPrev(unsigned char *zl, unsigned char *p);

//取到p元素中的值
unsigned int ziplistGet(unsigned char *p, unsigned char **sval, unsigned int *slen, long long *lval);

//在p元素前插入新的元素
unsigned char *ziplistInsert(unsigned char *zl, unsigned char *p, unsigned char *s, unsigned int slen);

//从ziplist中删除p元素
unsigned char *ziplistDelete(unsigned char *zl, unsigned char **p);

//删除index处的元素开始的num个元素
unsigned char *ziplistDeleteRange(unsigned char *zl, unsigned int index, unsigned int num);

//比较p元素的值与s的值
unsigned int ziplistCompare(unsigned char *p, unsigned char *s, unsigned int slen);

//找到p之后与vstr值一样的元素，每次比较后移动skip个元素
unsigned char *ziplistFind(unsigned char *p, unsigned char *vstr, unsigned int vlen, unsigned int skip);

//返回ziplist中元素个数
unsigned int ziplistLen(unsigned char *zl);

//返回ziplist占用内存大小
size_t ziplistBlobLen(unsigned char *zl);
