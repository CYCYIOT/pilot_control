#include "tcp_client.h"
#include "ftp_client.h"
#include "cJSON.h"
#include "proto.h"
SOCKET sclient;

int tcp_client_init()
{
	
	WORD socketVersion = MAKEWORD(2,2);
    WSADATA wsaData; 
    if(WSAStartup(socketVersion, &wsaData) != 0)
    {      
           return 0;
    }
	
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(4646);
	inet_pton(AF_INET, "192.168.100.1", &serAddr.sin_addr);
	
   // serAddr.sin_addr.s_addr=inet_addr("192.168.100.1");
    cout<<"等待无线连接...."<<endl;
    sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sclient == INVALID_SOCKET){
			printf("invalid socket!");
			return 0;
		}
	
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{  //连接失败
			printf("无线未连接 !\n");
			closesocket(sclient);
			return 0;
		}
	cout<<"无线连接成功"<<endl;
	ftp_init();
	return 0;	
}

int tcp_send(int cmd,int val)
{ 
   char *out=NULL;
   int ret;
   cJSON *root = NULL;
   cJSON *param=NULL;
   cJSON *item = NULL;
   cJSON *found = NULL;
   cJSON *vel=NULL;
	switch(cmd)
	{
		case CMD_IMG_LOAD:
	    root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "CMD", CMD_IMG_LOAD);
        cJSON_AddItemToObject(root, "PARAM", param = cJSON_CreateObject());
        cJSON_AddNumberToObject(param,"num",1);
	    cJSON_AddNumberToObject(param,"delay",0);
	    out = cJSON_Print(root);
	    //char *p="{\"CMD\":87,\"PARAM\":{\"num\":1,\"delay\":0}}";
	   // printf("out:%s\n",out);
	    cJSON_Delete(root);
		break;
		case CMD_SAFE_MODE:
		root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "CMD", CMD_SAFE_MODE); 
		cJSON_AddNumberToObject(root, "PARAM", 1); 
		out = cJSON_Print(root);
		cJSON_Delete(root);
		break;
		case CMD_TAKEOFF_ALT:
		root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "CMD", CMD_TAKEOFF_ALT); 
		cJSON_AddNumberToObject(root, "PARAM", val); 
		out = cJSON_Print(root);
		cJSON_Delete(root);
		break;
		default:
		cout<<"命令错误"<<endl;
	}
	ret=send(sclient, out, strlen(out), 0);
	if(ret < 0){
	cout<<"命令发送失败"<<endl;
	return -1;
	}
	char recData[255];
    ret = recv(sclient, recData, 255, 0);
	if (ret > 0) {
      recData[ret] = 0x00;
	  switch(cmd)
	  {
		case CMD_IMG_LOAD:
        item =cJSON_Parse(recData);
        found=cJSON_GetObjectItem(item, "PARAM");
	    vel=cJSON_GetObjectItem(found,"photo");
	    ftp_main(vel->valuestring);
	    cJSON_Delete(item);
		break;
	 }	
	}
	cout<<"设置完成"<<endl;
	return 0;
	
}
void tcp_quit()
{
  closesocket(sclient);
  ftp_quit();
  WSACleanup();
}



