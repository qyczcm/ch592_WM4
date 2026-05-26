#ifndef USER_HEADER_H
#define USER_HEADER_H

#include "stdbool.h"

#define SoftWare_Version    0x2518

#define CMD_BLE_DATA_HEAD           0xD55D  //BLE???
#define CMD_MESH_DATA_HEAD          0xC55C  //mesh???
#define CMD_SUB_DEV_DATA_HEAD       0xB55B //?????豸????
#define CMD_GATEWAY_DATA_HEAD       0xA55A  //?????????????
#define CMD_DEBUG_MODE_HEAD         0xE55E  //??????????
#define CMD_QUERY_MESH_ADDR_HEAD    0xF55F  //???MESH???
#define CMD_QUERY_MESH_KEY_HEAD     0xF66F  //???MESH???

#define MY_CONFIG_ADDR                     0x3800  //32K???? ???7FFF
#define MY_DES_ADDR                        0x3E00  //32K???? ???7FFF
#define DEBUG_MODE_ADDR                    0x3F00  //??????????洢???

#define CRC_32_NOCHECK                    TRUE   //???????????
#define CRC_16_IBM_NOCHECK                TRUE   //???????????

#define WCH_DEFAULT_CMD                   TRUE   //???????????

// ??????????????BLE??????????????????
// ??????????????? FALSE
#define DEBUG_ALWAYS_ADV                  TRUE   //????????????

//#define GATEWAY                           TRUE   //?????????

typedef union
{
    struct
    {
        uint8_t cmd_head[3];
        uint8_t cmd_len[2];
        uint8_t timestr[4];
        uint8_t dev_addr_src[2];
        uint8_t dev_addr_des[2];
        uint8_t cmd_type[2];//01 00    02 00    03 00
        uint8_t key_value[2];
        uint8_t key_state;
        uint8_t lamp_state;
        uint8_t loop_start;//?????·
        uint8_t loop_end;//??????·
        uint8_t databuf[204];//???????
    } common_manage_cmd;
    struct
    {
        uint8_t cmd_head[3];
        uint8_t cmd_len[2];
        uint8_t timestr[4];
        uint8_t dev_addr_src[2];
        uint8_t dev_addr_des[2];
        uint8_t cmd_type[2];//04 00
        uint8_t key_value[2];
        uint8_t key_state;
        uint8_t loop_start;//?????·
        uint8_t loop_end;//??????·
        uint8_t databuf[204];//???????
    } fast_manage_cmd;
    struct
       {
           uint8_t cmd_head[3];
           uint8_t cmd_len[2];
           uint8_t timestr[4];
           uint8_t dev_addr_src[2];
           uint8_t dev_addr_des[2];
           uint8_t cmd_type[2];//05 00
           uint8_t key_value[2];
           uint8_t key_state;
           uint8_t modify_num[2];//??????  20
           uint8_t databuf[204];//???????
           //Condition *register_conditions;//????????+??
       } multi_manage_cmd;
    //
    struct
    {//80 80 EE 18 00 00 00 01 00 01 FF FF 06 00 00 01 07 00 01 01 00 00 00 01 00 00 00
        uint8_t cmd_head[3];
        uint8_t cmd_len[2];
        uint8_t timestr[4];
        uint8_t dev_addr_src[2];
        uint8_t dev_addr_des[2];
        uint8_t cmd_type[2];//06 00
        uint8_t utc_version[4];
        uint8_t data_len[4];//?????????
        uint8_t reg_addr_start[4];
        uint8_t crc32[4];
        uint8_t action;
        uint8_t crc16_ibm[2];
    } config_update_cmd;
    struct
    {//80 80 EE xx(len) 00 00 00 01    00 01 FF FF    08 00   06   01 00   00 01 02 03 04 05    00 00 00 00 00 00
     //80 80 EE 1A 00 00 00 01 00 01 FF FF 08 00 06 01 00 00 01 02 03 04 05 00 00 00 00 00 00
        uint8_t cmd_head[3];
        uint8_t cmd_len[2];
        uint8_t timestr[4];
        uint8_t dev_addr_src[2];
        uint8_t dev_addr_des[2];
        uint8_t cmd_type[2];//08 00
        uint8_t data_len[4];//???????????
        uint8_t reg_addr[4];
        uint8_t databuf[204];//???????
    } config_update_data;
    struct
    {
        uint8_t cmd_head[3];
        uint8_t cmd_len[2];
        uint8_t timestr[4];
        uint8_t dev_addr_src[2];
        uint8_t dev_addr_des[2];
        uint8_t cmd_type[2];//0A 00
        uint8_t utc_version[4];
        uint8_t data_len[4];//?????????
        uint8_t reg_addr_start[4];
        uint8_t crc32[4];
        uint8_t action;//1-??У?? 2-??У??
        uint8_t crc16_ibm[2];
    } config_crc_check_cmd;
//    struct
//    {
//        uint8_t databuf[221];
//    } lamp_manage_data;
} app_dev_manage_t;

typedef struct
{
    uint8_t state[2];
    uint8_t net_configed;//1 - ???????   0 - ???δ????
    uint8_t dev_type;//?豸????
    uint8_t net_id[2];
    uint8_t user_key[16];
    uint8_t user_data[4];//??????4?????
} user_net_config;

enum
{
  net_no_config = 0,
  net_configed,
  user_net_configed,
};

//
//extern uint8_t  UartRxBuff[200];
//extern uint8_t  UartRxCnt;
extern uint16_t my_net_addr;
/** Mesh ADC 帧内有人无人字节：app_main 中自动更新（≥2 路 ADC 大于阈值则为 1=有人，否则 0=无人） */
extern uint8_t  g_mesh_occ_byte;
extern bool  BLE_Connect_Flag;
extern bool  Mesh_is_Provisioned_Flag;
extern bool Target_reply_cmd_state;
extern bool Debug_Always_ADV_Flag;  // ????????????
extern uint8_t self_prov_net_key[16];  // ???????
//extern app_dev_manage_t app_dev_manage;
void Deal_Mesh_RX_Data(uint8_t *pValue, uint16_t len);
void Save_Debug_Mode_To_Flash(void);    // ???????????Flash
void Load_Debug_Mode_From_Flash(void);  // ??Flash?????????

#endif
