#ifndef _PROTO_H__
#define _PROTO_H__


#define PACKED __attribute__((__packed__))
#define AWLINK_MAGIC 0xFA

#define AWLINK_IDSRC_MASTER 0
#define AWLINK_IDSRC_CLIENT 1

#define AWLINK_PARSE_STEP_MAGIC      1
#define AWLINK_PARSE_STEP_LENGTH     2
#define AWLINK_PARSE_STEP_ID_SRC     3
#define AWLINK_PARSE_STEP_I_ID       4
#define AWLINK_PARSE_STEP_SI_ID      5
#define AWLINK_PARSE_STEP_DATA       6
#define AWLINK_PARSE_STEP_CHECKSUM1  7
#define AWLINK_PARSE_STEP_CHECKSUM2  8
#define AWLINK_PARSE_STEP_OK         9
#define AWLINK_PARSE_STEP_FAIL       10

#define AWLINK_ITEM_SYSTEM            0
#define AWLINK_ITEM_STATUS            1
#define AWLINK_ITEM_CONTROL           2
#define AWLINK_ITEM_MISSION           3
#define AWLINK_ITEM_PARAM             4
#define AWLINK_ITEM_LOG               5
#define AWLINK_ITEM_FILE              6

#define AWLINK_ITEM_SYSTEM_ACK          0
#define AWLINK_ITEM_SYSTEM_AWLINK_VER   1
#define AWLINK_ITEM_SYSTEM_AWPILOT_VER  2
#define AWLINK_ITEM_SYSTEM_HEART        3
#define AWLINK_ITEM_SYSTEM_MSG          4


#define AWLINK_ITEM_STATUS_BASIC_INFO         0
#define AWLINK_ITEM_STATUS_MISSION_INFO       1
#define AWLINK_ITEM_STATUS_GPS_INFO           2
#define AWLINK_ITEM_STATUS_SENSOR_INFO        3
#define AWLINK_ITEM_STATUS_SENSOR_CALIB_INFO  4
#define AWLINK_ITEM_STATUS_USER1_INFO         5


#define AWLINK_ITEM_CONTROL_JOYSTICK       0
#define AWLINK_ITEM_CONTROL_STATUS_RATE    1
#define AWLINK_ITEM_CONTROL_CALIBRATE      2
#define AWLINK_ITEM_CONTROL_MODE           3


typedef struct {
	uint8_t heart;
}awlink_system_heart_s;


typedef struct {
	uint8_t ack;
	uint8_t item_id;
	uint8_t subitem_id;
}awlink_system_ack_s;

typedef struct{
	float att[3];
	float vel_ned[3];
	float pos_ned[3];
	uint8_t status;
	uint8_t mode;
	uint8_t capacity;
	uint8_t voltage;
	bool charge;
	bool headfree;
	bool armed;
}awlink_status_base_info_s;


typedef struct {
	uint8_t magic;
	uint8_t length;
	uint8_t id_src;
	uint8_t item_id;
	uint8_t subitem_id;
	uint8_t *data;
	uint16_t checksum;

}awlink_msg_s;

typedef struct {
	uint8_t type;
	uint16_t awlink_parse_error;
	uint8_t  awlink_parse_step;
	uint8_t  awlink_parse_data_count;
	uint16_t awlink_parse_checksum;
	awlink_msg_s awlink_package;

}awlink_decode_s;

enum cmd {
	CMD_SAFE_MODE = 83,
	CMD_WING_PROTECTION,
	CMD_CONTROL_YAW,
	CMD_CONTROL_ALT,
	CMD_IMG_LOAD,
	CMD_TOF_ALTHOLD,//88
	CMD_TAKEOFF_ALT,
	CMD_INFRARED_SHOOTING,
};

void recv_udp(int fd);
uint8_t batt_get_cap();

#endif
