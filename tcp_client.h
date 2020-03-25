#ifndef _TCP_H__
#define _TCP_H__


#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
//#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "crc.h"
#include "cJSON.h"
#include "ftp_client.h"
#include <ws2tcpip.h>
#include<iostream>
#include<cstring>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

int tcp_client_init();
void tcp_quit();
int tcp_send(int cmd,int vel);
#endif
