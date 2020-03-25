#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "crc.h"
#include <windows.h>
#include <ws2tcpip.h>
#include<iostream>
#include<cstring>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

int socket_cmd_fd;
int socket_data_fd;
static char m_send_buffer[1024];
static char m_recv_buffer[1024];

int socket_create() //创建socket
{
	WORD socketVersion = MAKEWORD(2,2);
    WSADATA wsaData; 
    if(WSAStartup(socketVersion, &wsaData) != 0)
    {      
           return 0;
    }
	
   SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sclient == INVALID_SOCKET)
	{
		printf("invalid socket!");
		return 0;
	 }
	
	return sclient;
}
void socket_close(int fd) //关闭socket
{
	closesocket(fd);
	
}

int socket_connect(SOCKET sclient, const char *ip,int port) //连接
{
	
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serAddr.sin_addr);
	
  // serAddr.sin_addr.s_addr=inet_addr(ip);

   
   if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
			printf("connect error !\n");
			closesocket(sclient);
			return -1;
	 }
	//cout<<"data connect 0k"<<endl;
	return 0;
}
static int ftp_send_command(const CHAR *cmd)//发送命令
{
	int ret;
	//printf("send command: %s\r\n", cmd);
	ret = send(socket_cmd_fd, cmd, (int)strlen(cmd),0);
	if(ret < 0)
	{
		printf("failed to send command: %s",cmd);
		return -1;
	}
	return 0;
}

static int ftp_recv_respond(char *resp, int len)//接收响应
{
	int ret;
	int off;
	len -= 1;
	for(off=0; off<len; off+=ret)
	{
		ret = recv(socket_cmd_fd, &resp[off], 1,0);
		if(ret < 0)
		{
			printf("recv respond error(ret=%d)!\r\n", ret);
			return 0;
		}
		if(resp[off] == '\n')
		{
			break;
		}
	}
	resp[off+1] = 0;
	//printf("respond:%s", resp);
	return atoi(resp);
}
static int ftp_enter_pasv( char *ipaddr, int *port)//进入被动模式，解析数据地址
{
	int ret;
	char *find;
	int a,b,c,d;
	int pa,pb;
	ret = ftp_send_command("PASV\r\n");
	if(ret != 0)
	{
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 227)
	{
		return -1;
	}
	find = strrchr(m_recv_buffer, '(');
	sscanf_s(find, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &pa, &pb);
	sprintf(ipaddr, "%d.%d.%d.%d", a, b, c, d);
	*port = pa * 256 + pb;
	return 0;
}

int  ftp_filesize(const char *name)//获取文件大小
{
	int ret;
	int size;
	sprintf_s(m_send_buffer,"SIZE %s\r\n",name);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 0)
	{
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 213)
	{
		return -1;
	}
	size = atoi(m_recv_buffer + 4);
	return size;
}


int  ftp_upload(const char *name, void *buf, int len)//上传文件
{
	int  ret;
	char ipaddr[32];
	int  port;
	
	//查询数据地址
	ret=ftp_enter_pasv(ipaddr, &port);
	if(ret != 0)
	{
		return -1;
	}
	ret=socket_connect(socket_data_fd, ipaddr, port);
	if(ret != 0)
	{
		return -1;
	}
	//准备上传
	sprintf_s(m_send_buffer, "STOR %s\r\n", name);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 0)
	{
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 150)
	{
		socket_close(socket_data_fd);
		return -1;
	}
	
	//开始上传
	ret = send(socket_data_fd, (char *)buf, len,0);
	if(ret != len)
	{	
		printf("send data error!\r\n");
		socket_close(socket_data_fd);
		return -1;
	}
	socket_close(socket_data_fd);
 
	//上传完成，等待回应
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	return (ret==226);
}
 
//下载文件
int  ftp_download(const char *name, void *buf, int len)//下载文件
{
	int   i;
	int   ret;
	char  ipaddr[32];
	int   port;
    
	//查询数据地址
	ret = ftp_enter_pasv(ipaddr, &port);
	if(ret != 0)
	{
		return -1;
	}
	//连接数据端口
	ret = socket_connect(socket_data_fd, ipaddr, port);
	if(ret != 0)
	{
		printf("failed to connect data port\r\n");
		return -1;
	}
 
	//准备下载
	sprintf_s(m_send_buffer, "RETR %s\r\n", name);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 0)
	{
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 150)
	{
		socket_close(socket_data_fd);
		return -1;
	}
	
	//开始下载,读取完数据后，服务器会自动关闭连接
	for(i=0; i<len; i+=ret)
	{
		ret = recv(socket_data_fd, ((char *)buf) + i, len,0);
		//printf("download %d/%d.\r\n", i + ret, len);
		if(ret < 0)
		{
			printf("download was interrupted.\r\n");
			break;
		}
	}
	//下载完成
	//printf("download %d/%d bytes complete.\r\n", i, len);
	socket_close(socket_data_fd);
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	return (ret==226);
}

int login_ftp( const char *ip,int port,const char * username,const char * password)//登录服务器
{
	int ret;
	ret=socket_connect(socket_cmd_fd,ip,21);
	if(ret < 0){
		cout<<"ftp服务连接失败"<<endl;
		return -1;
	}
	//cout<<"ftp服务连接成功"<<endl;
	
	ret=ftp_recv_respond(m_recv_buffer,1024);
	if(ret != 220){
	    cout<<"错误的服务器，不欢迎"<<endl;
		return -1;
	}
	
	sprintf_s(m_send_buffer, "USER %s\r\n", username);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 0)
	{
		socket_close(socket_cmd_fd);
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 331)
	{
		cout<<ret<<endl;
		socket_close(socket_cmd_fd);
		return -1;
	}
	//cout<<"pass"<<endl;
    //发送PASS
	sprintf_s(m_send_buffer, "PASS %s\r\n", password);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 0)
	{
		socket_close(socket_cmd_fd);
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 230)
	{
		socket_close(socket_cmd_fd);
		return -1;
	}
	//printf("login success.\r\n");
	
    //设置为二进制模式
	ret = ftp_send_command("TYPE I\r\n");
	if(ret != 0)
	{
		socket_close(socket_cmd_fd);
		return -1;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 200)
	{
		socket_close(socket_cmd_fd);
		return -1;
	}
	return 0;
}

int ftp_write(const char *buf,int len,const char *filename)//存取到本地
{
	int ret;
	const char *find;
	find=strrchr(filename, '/');  //解析图片名称(不能带路径)
//	printf("w_f=%s\n",find);
	FILE *fp = NULL;
	ret=fopen_s(&fp,find+1,"wb");
	if(fp==NULL)
       cout<<"打开文件失败"<<endl;		
	ret=fwrite(buf,1,len,fp);
	cout<<"拍照成功"<<endl;
	
	return 0;
}

void ftp_quit(void)
{
	ftp_send_command("QUIT\r\n");
	socket_close(socket_cmd_fd);
}
 
void ftp_init(void)
{
	socket_cmd_fd = socket_create();
	socket_data_fd= socket_create();
	login_ftp("192.168.100.1",21,"AW819","1663819");
}
int ftp_main(char *filename)
{
	
	CHAR *buf=NULL;
	//printf("filename = %s\n",filename);
	int len=ftp_filesize(filename);
	if(len < 0){
		cout<<"文件不存在"<<endl;
		goto loop;
	}
	buf=new CHAR[len*10];
	ftp_download(filename,buf,len);
	ftp_write(buf,len,filename);
	delete []buf;
	
loop:		
	return 0;
}
