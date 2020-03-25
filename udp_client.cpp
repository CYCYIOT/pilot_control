#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <unistd.h>
#include <windows.h>
#include <dos.h>
#include <iostream>
#include <pthread.h>
#include "crc.h"
#include "cJSON.h"
#include "ftp_client.h"
#include "tcp_client.h"
#include <ws2tcpip.h>
#include "proto.h"
#include "udp.h"
#include<iostream>
#include<cstring>
#define TIMEOUT_T 100              //ÿ������ִ��ʱ��
#pragma comment(lib,"ws2_32.lib")
using namespace std;

SOCKET fd;

void usage_info()
{
int cap;
system("cls");
cap=batt_get_cap();
cout << "                                             ���˻�����ϵͳ    "<<endl;
cout<<"0 :У׼ 1: ��� 2: ���� 3: ǰ�� 4: �� 5: �� 6: �ҷ� 7: ���� 8: ���� 9: ���� "<<endl;
cout<<"                                                                                "<<endl;
cout<<"a :���� b: ��ǰ c: ��� d: ��ת e: ��ת f: tof����(����) g: ����  h: ��ɸ߶�(����) ����:"<<cap<<"%"<<endl;
cout<<"q : �˳�"<<endl;
if(cap <20)
  cout<<"�������ͣ���������"<<endl;

}
uint16_t crc_check(uint8_t *buf,int len)
{
 int i=0;
 uint16_t checksum=crc16_init();
 for(i=1;i<len;i++)
   checksum=crc16_update(buf[i],checksum);
return checksum;
}

void * heart_thread(void *arg)
{
    struct sockaddr_in addr;
    char buf[]={-6,8,0,2,0,0,0,0,0,0,0,0,0,-125,-51};
    int ret,len;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9696);
   // addr.sin_addr.s_addr=inet_addr("192.168.100.1");
	inet_pton(AF_INET, "192.168.100.1", &addr.sin_addr);
//    char s;
    len=(int)(sizeof(buf)/sizeof(buf[0]));
    while(1)
     {
      ret= sendto(fd,buf,len,0,(struct sockaddr *)&addr,sizeof(addr));
      if(ret < 0)
       {
        printf("send fail\n"); 
        return ((void *)-1);       
        } 
        Sleep(10);      //10ms
      }

return ((void *)0);
}

void * recv_thread(void *arg)
{
    struct sockaddr_in addr;
    uint8_t buf[]={0xfa,2,0,2,1,0,10,214,204};//�ȷ��ͻ�ȡ״̬����
    int ret,len;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9696);
	inet_pton(AF_INET, "192.168.100.1", &addr.sin_addr);
   // addr.sin_addr.s_addr=inet_addr("192.168.100.1");
//    char s;
    len=(int)(sizeof(buf)/sizeof(buf[0]));

    ret= sendto(fd,(char *)buf,len,0,(struct sockaddr *)&addr,sizeof(addr));
    if(ret < 0){
       printf("send fail\n"); 
       return ((void *)-1);       
       } 
	   
     recv_udp(fd);

return ((void *)0);
}

int pthread_create_heart()
{
 pthread_attr_t attr;
 pthread_t heart;
 pthread_attr_init(&attr);
 
 int ret;

 pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

 ret=pthread_create(&heart,&attr,heart_thread,NULL);
 if(ret < 0)
 {
  cout << "pthread_create fail"<<endl;
  return -1;
 }
 pthread_attr_destroy(&attr);
 return 0;
}

int pthread_create_recv()
{
 pthread_attr_t attr;
 pthread_t heart;
 pthread_attr_init(&attr);
 
 int ret;

 pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

 ret=pthread_create(&heart,&attr,recv_thread,NULL);
 if(ret < 0)
 {
  cout << "pthread_create recv fail"<<endl;
  return -1;
 }
 pthread_attr_destroy(&attr);
 return 0;
}


int main_control()
{
   struct sockaddr_in addr;
 
   unsigned char acc[]={0xfa,2,0,2,2,0,0,232,140};

  
   char takeoff[]={-6,9,0,2,3,3,0,0,-128,63,0,0,-128,63,-86,-17};
   char land[]={-6,9,0,2,3,4,0,0,0,0,0,0,0,0,127,-8};
   char forward[]={-6,9,0,2,3,8,0,0,0,0,0,0,0,0,80,-72};
   char back[]={-6,9,0,2,3,8,0,0,-128,63,0,0,0,0,-33,3};
   char left[]={-6,9,0,2,3,8,0,0,0,64,0,0,0,0,114,121};
   char right[]={-6,9,0,2,3,8,0,0,64,64,0,0,0,0,-93,123};

 
  unsigned char up[]={0xfa,8,0,2,0,0,0,0,0,96,0,0xe3,3,0xd,0xa5};
  unsigned char down[]={0xfa,8,0,2,0,0,0,0,0,6,0,25,252,0x73,0xf9};
  unsigned char forward_t[]={0xfa,8,0,2,0,238,255,232,3,0,0,0,0,0x4f,0xd5};
  unsigned char back_t[]={0xfa,8,0,2,0,63,0,27,252,0,0,0,0,0xba,0x16};
  unsigned char left_t[]={0xfa,8,0,2,0,26,252,51,0,0,0,0,0,0x4e,0x5d};
  unsigned char right_t[]={0xfa,8,0,2,0,232,3,31,0,0,0,0,0,0x8b,0x2f};
  unsigned char left_yaw[]={0xfa,8,0,2,0,0,0,0,0,25,252,233,255,0x2f,0xef};
  unsigned char right_yaw[]={0xfa,8,0,2,0,0,0,0,0,231,3,39,0,0x77,0xaf};
  uint8_t tof_alt[]={0xfa,1,0,2,9,150,0xd3,0xa8};
  uint8_t get_status[]={0xfa,2,0,2,1,0,10,214,204};

  char buf[16]={0};

  int ret=0,len=0,count=0;
  uint16_t checksum=0;

   WORD socketVersion = MAKEWORD(2,2);
   WSADATA wsaData; 
   if(WSAStartup(socketVersion, &wsaData) != 0){      
           return 0;
   }
    
   fd=socket(AF_INET,SOCK_DGRAM,0);
   if(fd < 0){
     printf("socket error\n");
     return -1;
   }
                        
	usage_info();             //��ʾ��Ϣ     
	tcp_client_init();        //��ʼ��tcp����
	 
    pthread_create_heart();     //�����̣߳�����������
	pthread_create_recv();      //�����̣߳��������˻����ص�����
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9696);
  //  addr.sin_addr.s_addr=inet_addr("192.168.100.1");
	inet_pton(AF_INET, "192.168.100.1", &addr.sin_addr);
	
    char s;
    int val;
	double t;
    while(1){
loop: 
     usage_info();  
     count=0; 
     cout <<"������ָ�";
     cin >> s;

     switch (s)
        {
        case '0':  //У׼
         ret= sendto(fd,(char *)acc,9,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 9){
           cout <<"acc ok"<<endl;
           goto loop;
          }
          break;
        case '1': //���
          memcpy(buf,takeoff,16);
          break;
        case '2': //����
          memcpy(buf,land,16);
          break;
        case '3': // ǰ��
          memcpy(buf,forward,16);
          break;
        case '4': //��
          memcpy(buf,back,16);
          break;
        case '5': //��
          memcpy(buf,left,16);
          break;
        case '6': //�ҷ�
          memcpy(buf,right,16);
          break;
        case '7': //����
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
         ret= sendto(fd,(char *)up,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
    //       cout <<"up ok"<<endl;
          }
         }
           goto loop;
          break;
        case '8': //����
		t = (double)GetTickCount();
         while((double)GetTickCount() - t < TIMEOUT_T){
         ret= sendto(fd,(char *)down,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
      //     cout <<"down ok"<<endl;
          }
         }
           goto loop;
          break;
        case '9': //����
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
        ret= sendto(fd,(char *)left_t,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
        //   cout <<"left ok"<<endl;
          }
         }
           goto loop;
          break;
        case 'a': //����
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
        ret= sendto(fd,(char *)right_t,15,0,(struct sockaddr *)&addr,sizeof(addr));
        if(ret == 15){
          // cout <<"right ok"<<endl;
          }
         }
           goto loop;
         case 'b': //��ǰ
		 t = (double)GetTickCount();
		 while ((double)GetTickCount() - t < TIMEOUT_T) {
         ret= sendto(fd,(char *)forward_t,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
         //  cout <<"forward_t ok"<<endl;
          }
         }
           goto loop;
         break;
        case 'c': //���
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
         ret= sendto(fd,(char *)back_t,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
         //  cout <<"back_t ok"<<endl;
          }
         }
           goto loop;
         break;
        case 'd': //����
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
         ret= sendto(fd,(char *)right_yaw,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
        //   cout <<"right_yaw ok"<<endl;
          }
         }
           goto loop;
         break;
        case 'e': //����
		t = (double)GetTickCount();
		while ((double)GetTickCount() - t < TIMEOUT_T) {
         ret= sendto(fd,(char *)left_yaw,15,0,(struct sockaddr *)&addr,sizeof(addr));
         if(ret == 15){
         //  cout <<"left_yaw ok"<<endl;
          }
         }
           goto loop;
         break;     
	  case 'f': //tof����
         cout <<"�����붨�߸߶�(0 ~ 255):";
         cin >>val;
        //  scanf("%d",&val);
         if(val >255 || val <= 0)
           {
           cout<<"������Ч"<<endl;
           cin.clear();     //c++ ��Ҫ�������
           goto loop;
             }
         tof_alt[5]=val;
         checksum = crc_check(tof_alt,6);  //��̬crc16 У��
         tof_alt[6]=(checksum & 0xff);
         tof_alt[7]=(checksum >> 8) & 0xff;
         ret= sendto(fd,(char *)tof_alt,8,0,(struct sockaddr *)&addr,sizeof(addr));
         goto loop;
         break;
	  case 'g': //����
         tcp_send(CMD_IMG_LOAD,0);	
         goto loop;
         break;	
	  case 'h':  //��ɸ߶�����
	     cout <<"��������ɸ߶�(0 ~ 255):";
         cin >>val;
        //  scanf("%d",&val);
         if(val >255 || val <= 0)
           {
           cout<<"������Ч"<<endl;
           cin.clear();     //c++ ��Ҫ���
           goto loop;
             }
		 tcp_send(CMD_TAKEOFF_ALT,val);	 
         goto loop;
         break;		 
	  case 'j':   //ģʽ�л�
	     tcp_send(CMD_SAFE_MODE,0);
		 goto loop;
		 break;
	  case 'l':  //�������ͻ�ȡ״̬��Ϣ
         ret= sendto(fd,(char *)get_status,9,0,(struct sockaddr *)&addr,sizeof(addr));  
		 goto loop;
		 break;
#if 0          
      case 'm':    //crc У�����
           checksum = crc_check(get_status,7);	
           printf("1= %d 2=%d\n",(checksum & 0xff),(checksum >> 8) & 0xff);
		   goto loop;
#endif		       
      case 'q':
        goto quit;
		break;
        default:
        cout <<"��ֵ��Ч"<<endl;
        goto loop;
        break;
       }
 
      len=(int)(sizeof(buf)/sizeof(buf[0]));
      ret= sendto(fd,buf,len,0,(struct sockaddr *)&addr,sizeof(addr));
      if(ret < 0){
        printf("send fail\n"); 
        return -1;       
        } 
      
      }
quit:
    tcp_quit();
    closesocket(fd);
	WSACleanup();  

  return 0;

}

