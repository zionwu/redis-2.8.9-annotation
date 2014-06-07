/* anet.c -- Basic TCP socket stuff made a bit less boring
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

#ifndef ANET_H
#define ANET_H

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

/* Flags used with certain functions. */
#define ANET_NONE 0
#define ANET_IP_ONLY (1<<0)

#if defined(__sun)
#define AF_LOCAL AF_UNIX
#endif

//创建一个新的TCP连接并连接到给定地址端口
int anetTcpConnect(char *err, char *addr, int port);

//创建一个新的非阻塞的TCP连接并连接到给定地址端口
int anetTcpNonBlockConnect(char *err, char *addr, int port);

//创建一个unix socket连接并连接到给定路径
int anetUnixConnect(char *err, char *path);

//创建一个非阻塞的unix socket连接并连接到给定路径
int anetUnixNonBlockConnect(char *err, char *path);

//从socket读到count个字节后返回
int anetRead(int fd, char *buf, int count);

//解析host并返回IP地址
int anetResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);

//验证host是否是IP地址的格式
int anetResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);

//创建一个基于IPv4的TCP服务器socket
int anetTcpServer(char *err, int port, char *bindaddr, int backlog);

//创建一个基于IPv6的TCP服务器socket
int anetTcp6Server(char *err, int port, char *bindaddr, int backlog);

//创建一个unix服务器socket
int anetUnixServer(char *err, char *path, mode_t perm, int backlog);

//接受新的TPC连接
int anetTcpAccept(char *err, int serversock, char *ip, size_t ip_len, int *port);

////接受新的unix连接
int anetUnixAccept(char *err, int serversock);

//往socket中写进count个字节后返回
int anetWrite(int fd, char *buf, int count);

//将socket的O_NONBLOCK设置
int anetNonBlock(char *err, int fd);

//使TCP_NODELAY选项生效
int anetEnableTcpNoDelay(char *err, int fd);

//使TCP_NODELAY选项失效
int anetDisableTcpNoDelay(char *err, int fd);

//使SO_KEEPALIVE选项生效
int anetTcpKeepAlive(char *err, int fd);

//取到连接另一方的ip地址和端口
int anetPeerToString(int fd, char *ip, size_t ip_len, int *port);

//将socket描述符fd的keep alive选项设上
int anetKeepAlive(char *err, int fd, int interval);

//取到连接的本地ip地址和端口
int anetSockName(int fd, char *ip, size_t ip_len, int *port);

#endif
