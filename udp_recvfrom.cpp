#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
//#include <unistd.h>
#include <windows.h>
#include <iostream>
#include <pthread.h>
#include "crc.h"
#include "cJSON.h"
#include "ftp_client.h"
#include "tcp_client.h"
#include "proto.h"

#include <ws2tcpip.h>
#include<iostream>
#include<cstring>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

uint8_t cap=100;
bool calibrate;
bool status_rate;
bool control_mode;

bool get_calibrate()
{
	return calibrate;
}
bool get_status_rate()
{
	return status_rate;
}
bool get_control_mode()
{
	return control_mode;
}


uint8_t batt_get_cap()
{
  return cap;
}

void awlink_heart(awlink_msg_s * msg)
{
awlink_system_heart_s *data;
data=(awlink_system_heart_s *)msg->data;
}

void decode_base_info(awlink_msg_s *msg)
{
  awlink_status_base_info_s * data;
  data=(awlink_status_base_info_s *)msg->data;
  cap=data->capacity;
}


void subitem_id_ack(uint8_t subitem_id)
{
	switch(subitem_id){
		case AWLINK_ITEM_CONTROL_STATUS_RATE:
		calibrate=true;
		break;	
		case AWLINK_ITEM_CONTROL_CALIBRATE:
		status_rate=true;
		break;
		case AWLINK_ITEM_CONTROL_MODE:
		control_mode=true;
		break;
	}
		
}

void decode_ack(awlink_msg_s *msg)
{
  awlink_system_ack_s *data;
  data=(awlink_system_ack_s *)msg->data;
  //printf("item_id = %d  subitem_id = %d\n",data->item_id,data->subitem_id);
  switch(data->item_id){
        case AWLINK_ITEM_CONTROL:
		subitem_id_ack(data->subitem_id);
		break;
  }
}

void awlink_stream_control(awlink_msg_s *msg)
{
  switch(msg->subitem_id){
      case AWLINK_ITEM_STATUS_BASIC_INFO:
         decode_base_info(msg);
        break;
  }
}

void awlink_item_system(awlink_msg_s *msg)
{
    switch(msg->subitem_id){
      case AWLINK_ITEM_SYSTEM_ACK:
       decode_ack(msg);
      break;
      case AWLINK_ITEM_SYSTEM_HEART:
       awlink_heart(msg);
    }
}

void awlink_handle(awlink_msg_s *msg)
{
	
	switch(msg->item_id){
		case AWLINK_ITEM_SYSTEM:
           awlink_item_system(msg);
		break;
		case AWLINK_ITEM_STATUS:
           awlink_stream_control(msg);
        break;
	
	}
}

bool udp_decode_step(awlink_decode_s * decoder,uint8_t *data)
{
	bool ret=false;
	switch(decoder->awlink_parse_step){
	       case AWLINK_PARSE_STEP_MAGIC:
		        if(*data == AWLINK_MAGIC){
					decoder->awlink_parse_checksum=crc16_init();
				    decoder->awlink_parse_step=AWLINK_PARSE_STEP_LENGTH;
					decoder->awlink_package.checksum=*data;
					ret=true;
				}
                break;
           case AWLINK_PARSE_STEP_LENGTH:
		        decoder->awlink_parse_step=AWLINK_PARSE_STEP_ID_SRC;
				decoder->awlink_parse_checksum=crc16_update(*data,decoder->awlink_parse_checksum);
				decoder->awlink_package.length=*data;
				ret=true;
                break;		   
           case AWLINK_PARSE_STEP_ID_SRC:
		        decoder->awlink_parse_step=AWLINK_PARSE_STEP_I_ID;
				decoder->awlink_parse_checksum=crc16_update(*data,decoder->awlink_parse_checksum);
				decoder->awlink_package.id_src=*data;
				ret=true;
                break;		   
           case AWLINK_PARSE_STEP_I_ID:
                decoder->awlink_parse_step=AWLINK_PARSE_STEP_SI_ID;
                decoder->awlink_parse_checksum=crc16_update(*data,decoder->awlink_parse_checksum);	
				decoder->awlink_package.item_id=*data;
				ret=true;
				break;
           case AWLINK_PARSE_STEP_SI_ID:
		        decoder->awlink_parse_step=AWLINK_PARSE_STEP_DATA;
				decoder->awlink_parse_checksum=crc16_update(*data,decoder->awlink_parse_checksum);
				decoder->awlink_package.subitem_id=*data;
				if(decoder->awlink_package.length == 0){
					decoder->awlink_parse_step=AWLINK_PARSE_STEP_CHECKSUM1;
					
				}
				ret=true;
		        break;
           case AWLINK_PARSE_STEP_DATA:
                decoder->awlink_parse_checksum=crc16_update(*data,decoder->awlink_parse_checksum);	
                decoder->awlink_package.data[decoder->awlink_parse_data_count]=*data;	
                decoder->awlink_parse_data_count++;
                if(decoder->awlink_parse_data_count >= decoder->awlink_package.length){
					decoder->awlink_parse_step=AWLINK_PARSE_STEP_CHECKSUM1;
				}
				
				ret=true;
				break;
           case AWLINK_PARSE_STEP_CHECKSUM1:
                if(*data == (decoder->awlink_parse_checksum & 0xFF)){
                   decoder->awlink_parse_step=AWLINK_PARSE_STEP_CHECKSUM2;              
                   ret=true;  
				}
				break;
           case AWLINK_PARSE_STEP_CHECKSUM2:
                if(*data == (decoder->awlink_parse_checksum >> 8)& 0xFF){
					decoder->awlink_parse_step=AWLINK_PARSE_STEP_OK;
					ret=true;			
                }
                break;				
	}
	
	return ret;
}

void udp_decode(awlink_decode_s *decoder,uint8_t *data,int length)
{
	int count=0; 
	for(count =0;count < length; count++){
		if(udp_decode_step(decoder,&data[count]) == false){
		   decoder->awlink_parse_error++;
	       decoder->awlink_parse_step=AWLINK_PARSE_STEP_MAGIC;
		   decoder->awlink_parse_data_count=0;
		}
		else{
			if(decoder->awlink_parse_step == AWLINK_PARSE_STEP_OK){
			  awlink_handle(&decoder->awlink_package);
		      decoder->awlink_parse_step=AWLINK_PARSE_STEP_MAGIC;
			  decoder->awlink_parse_data_count=0;
			}
		}
		
	}
	
}

void awlink_package_init(awlink_msg_s *package)
{
	package->magic=0;
	package->length=0;
	package->id_src=0;
	package->item_id=0;
	package->subitem_id=0;
	package->data=(uint8_t *)malloc(250);
	package->checksum=0;
	
	
}

void awlink_decode_init(awlink_decode_s *decoder)
{
	decoder->type=0;
	decoder->awlink_parse_error=0;
	decoder->awlink_parse_step=AWLINK_PARSE_STEP_MAGIC;
	decoder->awlink_parse_data_count=0;
	decoder->awlink_parse_checksum=0;
	awlink_package_init(&decoder->awlink_package);

}
void recv_udp(int fd)
{
	struct sockaddr_in udp_addr_send;
    uint8_t recvbuf[1000]; //zhuyi
	int udp_addr_len=sizeof(udp_addr_send);
	int len=0;
	awlink_decode_s decoder;
	awlink_decode_init(&decoder);
	
    while(1){
	len=recvfrom(fd,(char *)recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&udp_addr_send,&udp_addr_len);
	if(len > 0){
	  udp_decode(&decoder,recvbuf,len);
	  }
	}
}



