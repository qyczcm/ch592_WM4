/******************************************************************************/
/* ???????? */
#include "CONFIG.h"
#include "MESH_LIB.h"
#include "HAL.h"
#include "app_mesh_config.h"
#include "app.h"
#include "stdbool.h"
#include <stdio.h>
#include <malloc.h>
#include "user_header.h"
#include "devinfoservice.h"
#include "peripheral.h"
#include "app_mux_adc.h"
#include <stdlib.h>   // 需要包含这个头文件来使用 rand()
extern void Mesh_Send_Data(uint16_t addr, uint8_t *pValue, uint16_t len);

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

bool  BLE_Connect_Flag = false;
bool  Mesh_is_Provisioned_Flag = false;
bool  Need_Stop_Adv_Flag = false;
uint32_t MyTimestr = 0;
uint16_t my_net_addr = 0;         // cuixh

// ???????????????????BLE???????
// ???????Flash???????????DEBUG_ALWAYS_ADV?????
bool Debug_Always_ADV_Flag = false;  // ????Load_Debug_Mode_From_Flash()??????

uint8_t g_mesh_occ_byte = 0;

static uint8_t s_mesh_adc_tx_seq;

uint8_t UartRxBuff[256];
uint8_t UartRxCnt;
uint8_t trigB;
blePaControlConfig_t  blePaControlConfigIO; //????????????
uint8_t Main_TaskID = INVALID_TASK_ID;

#define Ticks_Update_EVT                  (0x0001 << 0)
#define UART_CHECK_EVT                    (0x0001 << 1)
#define Start_Stop_Adv_EVT                (0x0001 << 2)
#define MESH_ADC_REPORT_EVT               (0x0001 << 3)
#ifndef MESH_ADC_SAME_STATE_SEND_WINDOWS
#define MESH_ADC_SAME_STATE_SEND_WINDOWS  3
#endif
/* Periodic Mesh publish of ASCII ADC line (same format as before); ms */
#ifndef MESH_ADC_REPORT_PERIOD_MS
#define MESH_ADC_REPORT_PERIOD_MS         800
#endif
/* ?????? [3..4]????????? mesh ?????????????????????????????????? */
#ifndef MESH_ADC_GATEWAY_ADDR
#define MESH_ADC_GATEWAY_ADDR             0x0102u  //这里改网关
#endif
/* ??????????????��???��???��???????????????????? MIN_OVER ��???????? */
#ifndef MESH_ADC_OCC_THRESHOLD
#define MESH_ADC_OCC_THRESHOLD            2200u
#endif
#ifndef MESH_ADC_OCC_MIN_OVER
#define MESH_ADC_OCC_MIN_OVER             1u
#endif

/*
 * Mesh ADC frame 19 bytes:
 * [0..1] C5 5C | [2] len 14 | [3..4] gateway | [5..6] self addr |
 * [7..14] 4x uint16 ADC LE | [15] occ | [16] seq++ | [17..18] CRC16 LE (IBM), covers [0..16]
 */
#define MESH_ADC_FRAME_LEN                19
#define MESH_ADC_FRAME_BODY_AFTER_LEN     14

#define  FEED_IWDG()   {R32_IWDG_KR=0xAAAA;} //??????????????

static uint16_t mesh_adc_crc16_ibm(const uint8_t *data, unsigned len)
{
    uint16_t crc = 0xFFFFu;
    unsigned i, j;

    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(j = 0; j < 8; j++)
            crc = (crc & 1u) ? (crc >> 1) ^ 0xA001u : (crc >> 1);
    }
    return crc;
}
    static uint8_t run_count = 0;
    static uint8_t person_count = 0;
    static uint8_t last_sent_occ_byte = 0xFF;
    static uint8_t same_state_window_count = 0;
uint16_t Main_ProcessEvent(uint8_t task_id, uint16_t events)
{
    static uint32_t Start = 0;
    uint8_t Test_Data[10] = {0x01};
    if(events & Ticks_Update_EVT)
    {
        FEED_IWDG(); //????
        tmos_start_task(Main_TaskID,Ticks_Update_EVT,400);//2000ms
        return (events ^ Ticks_Update_EVT);
    }
    if(events & MESH_ADC_REPORT_EVT)
    {
        uint8_t meshpkt[MESH_ADC_FRAME_LEN];
        uint16_t crc;
        uint8_t i;
        uint8_t over_cnt = 0;


        for(i = 0; i < MUX_ADC_CHANNEL_NUM; i++)
        {
            if(g_mux_adc_raw[i] > MESH_ADC_OCC_THRESHOLD)
                over_cnt++;
        }
        g_mesh_occ_byte = (over_cnt >= MESH_ADC_OCC_MIN_OVER) ? 1u : 0u;
        if(g_mesh_occ_byte == 1){
            person_count++;
        }
        uint8_t should_send_mesh = 0;
        run_count++;
        
        if(run_count >= 4)
        {
            g_mesh_occ_byte = (person_count >= 1) ? 1u : 0u;
            person_count = 0;
            run_count = 0;
            if(Mesh_is_Provisioned_Flag)
            {
                if(g_mesh_occ_byte != last_sent_occ_byte)
                {
                    last_sent_occ_byte = g_mesh_occ_byte;
                    same_state_window_count = 0;
                    should_send_mesh = 1;
                }
                else
                {
                    same_state_window_count++;
                    if(same_state_window_count >= MESH_ADC_SAME_STATE_SEND_WINDOWS)
                    {
                        same_state_window_count = 0;
                        should_send_mesh = 1;
                    }
                }
            }
        }
        meshpkt[0] = 0xC5;
        meshpkt[1] = 0x5C;
        meshpkt[2] = MESH_ADC_FRAME_BODY_AFTER_LEN;
        meshpkt[3] = (uint8_t)(MESH_ADC_GATEWAY_ADDR & 0xFF);
        meshpkt[4] = (uint8_t)((MESH_ADC_GATEWAY_ADDR >> 8) & 0xFF);
        meshpkt[5] = (uint8_t)(my_net_addr & 0xFF);
        meshpkt[6] = (uint8_t)((my_net_addr >> 8) & 0xFF);
        for(i = 0; i < MUX_ADC_CHANNEL_NUM; i++)
        {
            meshpkt[7 + i * 2]     = (uint8_t)(g_mux_adc_raw[i] & 0xFF);
            meshpkt[7 + i * 2 + 1] = (uint8_t)((g_mux_adc_raw[i] >> 8) & 0xFF);
        }
        meshpkt[15] = g_mesh_occ_byte;
        meshpkt[16] = s_mesh_adc_tx_seq;
        crc = mesh_adc_crc16_ibm(meshpkt, 17);
        meshpkt[17] = (uint8_t)(crc & 0xFF);
        meshpkt[18] = (uint8_t)((crc >> 8) & 0xFF);



        if(should_send_mesh)
        {
            s_mesh_adc_tx_seq++;
            Mesh_Send_Data(MESH_ADC_GATEWAY_ADDR, meshpkt, MESH_ADC_FRAME_LEN);
        }

        if(BLE_Connect_Flag)
        {
            char adc_ascii[48];
            int n;

            n = snprintf(adc_ascii, sizeof(adc_ascii), "%u,%u,%u,%u\r\n",
                         (unsigned)g_mux_adc_raw[0], (unsigned)g_mux_adc_raw[1],
                         (unsigned)g_mux_adc_raw[2], (unsigned)g_mux_adc_raw[3]);
            if(n > 0 && n <= (int)sizeof(adc_ascii))
                peripheralChar4Notify((uint8_t *)adc_ascii, (uint16_t)n);
        }
        int random_offset = (rand() % 101) - 50;
        tmos_start_task(Main_TaskID, MESH_ADC_REPORT_EVT, MESH_ADC_REPORT_PERIOD_MS+random_offset);

        return (events ^ MESH_ADC_REPORT_EVT);
    }
    if(events & UART_CHECK_EVT)
    {
        const uint8_t reply1[10]="OK\r\n";
        const uint8_t reply2[10]="CRC Err\r\n";
        const uint8_t reply3[15]="Mesh Err\r\n";
        uint16_t CMD_HEAD = 0;
        uint16_t target_dev = 0;
        CMD_HEAD = UartRxBuff[0]<<8 | UartRxBuff[1];
        {
            if(CMD_HEAD == CMD_MESH_DATA_HEAD)//C55C Mesh?????????
            {
                if(Mesh_is_Provisioned_Flag)
                {//C5  5C  xx(????)  XX  XX(ID) ...
                    target_dev = UartRxBuff[4]<<8 | UartRxBuff[3];
                    PRINT("mesh: ");
                    Mesh_Send_Data(target_dev,UartRxBuff,UartRxCnt);//???????
                }
                else
                {
                    PRINT("No Mesh\r\n");
//                    UART0_SendString(reply3,strlen(reply3));
                }
            }
            else if(CMD_HEAD == CMD_BLE_DATA_HEAD)//D55D ???? - BLE ????????? ??????????????
            {
                peripheralChar4Notify(UartRxBuff,UartRxCnt);
            }
            else if(CMD_HEAD == CMD_SUB_DEV_DATA_HEAD)//B55B???????????
            {//B5 5B 15 F1 01 00
                    target_dev = UartRxBuff[5]<<8 | UartRxBuff[4];
                    PRINT("target_dev %04X\r\n",target_dev);
                    #ifdef GATEWAY
                        Mesh_Send_Data(target_dev,UartRxBuff,UartRxCnt);
                    #else
//                            Deal_User_Config_Data(UartRxBuff,UartRxCnt);
                    #endif
            }
            else if(CMD_HEAD == CMD_GATEWAY_DATA_HEAD)//?????????? ???????
            {
                PRINT("Config Gateway\r\n");
                #ifdef GATEWAY
                    Deal_User_Config_Data(UartRxBuff,UartRxCnt);
                #endif
                //A5 5A 15(????) A1 02 F1 00 23 45 67 89 ab cd ef 00 23 45 67 89 ab cd ef 10 00
//                    App_peripheral_reveived(&UartRxBuff[7], data_len - 2 - 4);
//                Deal_User_Config_Data(data_t,UartRxCnt + PROVISION_NET_KEY_LEN);
            }
            else
            {
                Mesh_Send_Data(0xffff,UartRxBuff,UartRxCnt); //??????? ???mesh??
            }
        }
        PRINT("CMD_HEAD:0x%04X\r\n",CMD_HEAD);
//        for (int i = 0; i < UartRxCnt; i++) {  // UartRxLen ?????????????????
//            PRINT("%02X ", UartRxBuff[i]);
//        }
//        PRINT("\r\n");
        UartRxCnt = 0;

        return (events ^ UART_CHECK_EVT);
    }
    if(events & Start_Stop_Adv_EVT)
    {
        PRINT("Stop_Advr\n");
        Stop_Adv();
        return (events ^ Start_Stop_Adv_EVT);
    }
    return 0;
}
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      bt_mesh_lib_init
 *
 * @brief   mesh ??????
 *
 * @return  state
 */
uint8_t bt_mesh_lib_init(void)
{
    uint8_t ret;

    if(tmos_memcmp(VER_MESH_LIB, VER_MESH_FILE, strlen(VER_MESH_FILE)) == FALSE)
    {
        PRINT("mesh head file error...\n");
        while(1);
    }

    ret = RF_RoleInit();
    hal_rf_tx_wait_enable(ENABLE);

#if((CONFIG_BLE_MESH_PROXY) ||   \
    (CONFIG_BLE_MESH_PB_GATT) || \
    (CONFIG_BLE_MESH_OTA))
    ret = GAPRole_PeripheralInit();
#endif /* PROXY || PB-GATT || OTA */

#if(CONFIG_BLE_MESH_PROXY_CLI)
    ret = GAPRole_CentralInit();
#endif /* CONFIG_BLE_MESH_PROXY_CLI */

    MeshTimer_Init();
    MeshDeamon_Init();
    ble_sm_alg_ecc_init();

#if(CONFIG_BLE_MESH_IV_UPDATE_TEST)
    bt_mesh_iv_update_test(TRUE);
#endif
    return ret;
}

//?????????65S
void IWDG_Enable()
{
    R32_IWDG_KR=0x5555;    //???IWDG????
    R32_IWDG_CFG |= (7<<12);   //32K???512?????62.5Hz
    R32_IWDG_CFG &=0xFFFFF000;
    R32_IWDG_CFG |=0x0FF;   //?????????65s??0xFFF/(32K/512))
    R32_IWDG_KR=0xCCCC;    //????IWDG????
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ??????
 *
 * @return  none
 */
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#ifdef CONFIG_BLE_MESH_GBO_POWER
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOB_SetBits(bTXD0);
    GPIOB_ModeCfg(bTXD0, GPIO_ModeOut_PP_5mA);
    UART0_DefInit();
#endif
    CH59x_BLEInit();
    HAL_Init();
    bt_mesh_lib_init();
    // ??Flash????????????????????App_Init??????????????????
    Load_Debug_Mode_From_Flash();

    App_Init();//??????????

    //BLE????????????????????Io??????  PA?????IO
    GPIOB_ModeCfg(GPIO_Pin_12|GPIO_Pin_13, GPIO_ModeOut_PP_5mA) ;//????????GPIO??????????????????//?????pin??bit
    blePaControlConfigIO.txEnableGPIO = (uint32_t)&R32_PB_OUT;
    blePaControlConfigIO.txDisableGPIO = (uint32_t)&R32_PB_CLR;
    blePaControlConfigIO.tx_pin = GPIO_Pin_12;//TX

    blePaControlConfigIO.rxEnableGPIO = (uint32_t)&R32_PB_OUT;
    blePaControlConfigIO.rxDisableGPIO = (uint32_t)&R32_PB_CLR;
    blePaControlConfigIO.rx_pin = GPIO_Pin_13;//RX
    BLE_PAControlInit(&blePaControlConfigIO);
    /* Mux ADC uses PB13/PB14/PB15; RF PA code above also toggles PB13 for RX ?? avoid sharing on one PCB */
    MuxADC_Init();
    MuxADC_TaskStart();

    Main_TaskID = TMOS_ProcessEventRegister(Main_ProcessEvent);
    tmos_start_task(Main_TaskID,Ticks_Update_EVT,1600);
    tmos_start_task(Main_TaskID, MESH_ADC_REPORT_EVT, MESH_ADC_REPORT_PERIOD_MS);
    IWDG_Enable();
    Main_Circulation();
}

/*********************************************************************
 * @fn      Save_Debug_Mode_To_Flash
 *
 * @brief   ??????????????Flash
 *
 * @return  none
 */
void Save_Debug_Mode_To_Flash(void)
{
    __attribute__((aligned(4))) uint32_t flash_data[2];

    // ????? + ?????????
    flash_data[0] = 0x5AA5C33C;  // ???????????????????????
    flash_data[1] = Debug_Always_ADV_Flag ? 1 : 0;

    EEPROM_ERASE(DEBUG_MODE_ADDR, EEPROM_PAGE_SIZE);
    EEPROM_WRITE(DEBUG_MODE_ADDR, flash_data, sizeof(flash_data));

    PRINT("Debug mode saved to flash: %d\n", Debug_Always_ADV_Flag);
}

/*********************************************************************
 * @fn      Load_Debug_Mode_From_Flash
 *
 * @brief   ??Flash????????????
 *
 * @return  none
 */
void Load_Debug_Mode_From_Flash(void)
{
    __attribute__((aligned(4))) uint32_t flash_data[2];

    EEPROM_READ(DEBUG_MODE_ADDR, flash_data, sizeof(flash_data));

    // ??????????????
    if(flash_data[0] == 0x5AA5C33C)
    {
        // Flash????????????????Flash?????
        Debug_Always_ADV_Flag = (flash_data[1] != 0);
        PRINT("Debug mode loaded from flash: %d\n", Debug_Always_ADV_Flag);
    }
    else
    {
        // Flash?????????????????????????
        #if(DEBUG_ALWAYS_ADV)
        Debug_Always_ADV_Flag = true;
        #else
        Debug_Always_ADV_Flag = false;
        #endif

        PRINT("Debug mode using default: %d\n", Debug_Always_ADV_Flag);

        // ??????????Flash
        Save_Debug_Mode_To_Flash();
    }
}

/******************************** endfile @ main ******************************/
