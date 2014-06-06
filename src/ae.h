/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __AE_H__
#define __AE_H__

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)

struct aeEventLoop;

/* Types and data structures */
//将函数原型定义成类型
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/* File event structure */
//文件时间的结构体
typedef struct aeFileEvent {
    int mask; /* one of AE_(READABLE|WRITABLE) */ //掩码，表示读写
    aeFileProc *rfileProc; //函数指针，指向读文件所用的函数
    aeFileProc *wfileProc; //函数指针，指向写文件所用的函数
    void *clientData;      //客户端数据
} aeFileEvent;

/* Time event structure */
//时间事件的结构体
typedef struct aeTimeEvent {
    long long id; /* time event identifier. */ //标识符
    long when_sec; /* seconds */ //事件预期发生的时间(秒）
    long when_ms; /* milliseconds */ //事件预期发生的时间(毫秒）
    aeTimeProc *timeProc; //函数指针，指向处理函数
    aeEventFinalizerProc *finalizerProc; //函数指针，指向清除函数
    void *clientData; //客户端数据
    struct aeTimeEvent *next; //指向下一个时间事件
} aeTimeEvent;

/* A fired event */
//待续的事件的结构体
typedef struct aeFiredEvent {
    int fd;
    int mask;
} aeFiredEvent;

/* State of an event based program */
//事件驱动程序状态的结构体
typedef struct aeEventLoop {
    int maxfd;   /* highest file descriptor currently registered */ //当前存在的文件描述符中最大值
    int setsize; /* max number of file descriptors tracked */       //文件事件的容量
    long long timeEventNextId;                                      //下一个时间事件的标识符
    time_t lastTime;     /* Used to detect system clock skew */     //上一次记录的时间
    aeFileEvent *events; /* Registered events */                    //文件事件的数组
    aeFiredEvent *fired; /* Fired events */                         //就绪事件的数组  
    aeTimeEvent *timeEventHead;        //时间事件列表的表头
    int stop;  //是否停止程序
    void *apidata; /* This is used for polling API specific data */ //用于不同polling API的数据
    aeBeforeSleepProc *beforesleep;  //sleep前调用的函数
} aeEventLoop;

/* Prototypes */
//创建一个表示事件驱动程序状态的结构体
aeEventLoop *aeCreateEventLoop(int setsize);

//删除一个表示事件驱动程序状态的结构体
void aeDeleteEventLoop(aeEventLoop *eventLoop);

//将stop的值设为1
void aeStop(aeEventLoop *eventLoop);

//添加一个新的文件事件到eventLoop中
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData);
		
//从eventLoop中删除一个文件事件
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);

//根据文件描述符，从eventLoop中取到一个文件事件
int aeGetFileEvents(aeEventLoop *eventLoop, int fd);

//添加一个新的时间事件到eventLoop中
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc);
		
//从eventLoop中删除一个时间事件
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);

//处理eventLoop中注册的事件
int aeProcessEvents(aeEventLoop *eventLoop, int flags);

//等待fd对应的文件就绪
int aeWait(int fd, int mask, long long milliseconds);

//事件驱动程序的main函数，调用aeProcessEvents处理注册的事件
void aeMain(aeEventLoop *eventLoop);

//取到使用的复用层实现的名字
char *aeGetApiName(void);

//设置sleep前调用的函数
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep);

//取到当前eventLoop中事件注册的容量
int aeGetSetSize(aeEventLoop *eventLoop);

//更新eventLoop中事件注册的容量
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize);

#endif
