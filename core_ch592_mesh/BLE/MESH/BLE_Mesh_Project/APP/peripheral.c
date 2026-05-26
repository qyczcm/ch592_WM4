/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ����ӻ�������Ӧ�ó��򣬳�ʼ���㲥���Ӳ�����Ȼ��㲥������������
 *                      ����������Ӳ�����ͨ���Զ������������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "MESH_LIB.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "app.h"
#include "app_vendor_model_srv.h"
#include "user_header.h"
#include "stdbool.h"

uint8_t Stop_BLE_ADV_Flag = 0;
bool Updata_MTU_State = false;
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD              1600

// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD             1600

// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY               6400

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         80

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 6=7.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6

// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    100

// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT         500

// Company Identifier: WCH
#define WCH_COMPANY_ID                       0x07D7

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] = {
    // complete name
    0x09, // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'W','M','4','_','0','0','0','0',

    // connection interval range
    0x05, // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // Tx power level
    0x02, // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
static uint8_t advertData[] = {
    // Flags; this sets the device to use limited discoverable
    // mode (advertises for 30 seconds at a time) instead of general
    // discoverable mode (advertises indefinitely)
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // service UUID, to notify central devices what services are included
    // in this peripheral
    0x03,                  // length of this data
    GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all
    LO_UINT16(SIMPLEPROFILE_SERV_UUID),
    HI_UINT16(SIMPLEPROFILE_SERV_UUID),

    0x05,                  // length of this data
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    0xD7,
    0x07,
    0x00,
    0x00
};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple Peripheral";

// Connection item list
static peripheralConnItem_t peripheralConnList;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
static void performPeriodicTask(void);
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout);
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList);
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks
    peripheralRssiCB,              // When a valid RSSI is read from controller (not used by application)
    peripheralParamUpdateCB
};

// Broadcast Callbacks
static gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
    NULL, // Not used in peripheral role
    NULL  // Receive scan request callback
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Peripheral_BondMgrCBs = {
    NULL, // Passcode callback (not used by application)
    NULL  // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t Peripheral_SimpleProfileCBs = {
    simpleProfileChangeCB // Characteristic value change callback
};
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

uint8_t attDeviceName2[16] = "WM4_0000";  // ȫ�ִ洢�豸��
void BuildScanRspData(uint16_t mesh_addr)
{
    // 1. �����豸�������� "BED_0312"
    snprintf(attDeviceName2, sizeof(attDeviceName2), "WM4_%04X", mesh_addr);
    // ���������� �豸���ֶ� ����������
    scanRspData[6] =attDeviceName2[4];
    scanRspData[7] =attDeviceName2[5];
    scanRspData[8] =attDeviceName2[6];
    scanRspData[9] =attDeviceName2[7];
}
/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Peripheral_Init()
{
    Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t  initial_advertising_enable = TRUE;
        uint16_t desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16_t desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;

        APP_DBG("my_net_addr:%d", my_net_addr);
        BuildScanRspData(my_net_addr);
        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t), &desired_min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t), &desired_max_interval);
    }

    // Set advertising interval
    {
        uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);
    }

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = 0; // passkey "000000"
        uint8_t  pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        uint8_t  mitm = TRUE;
        uint8_t  bonding = TRUE;
        uint8_t  ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }

    // Initialize GATT attributes
    GGS_AddService(GATT_ALL_SERVICES);           // GAP
    GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes
    SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile
    GATT_InitClient();//added by cuixh
    DevInfo_AddService();

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName2);

    // Setup the SimpleProfile Characteristic Values
    {
        uint8_t charValue1[SIMPLEPROFILE_CHAR1_LEN] = {1};
        uint8_t charValue2[SIMPLEPROFILE_CHAR2_LEN] = {2};
        uint8_t charValue3[SIMPLEPROFILE_CHAR3_LEN] = {3};
        uint8_t charValue4[SIMPLEPROFILE_CHAR4_LEN] = {4};
        uint8_t charValue5[SIMPLEPROFILE_CHAR5_LEN] = {1, 2, 3, 4, 5};

        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, SIMPLEPROFILE_CHAR1_LEN, charValue1);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, SIMPLEPROFILE_CHAR2_LEN, charValue2);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, SIMPLEPROFILE_CHAR3_LEN, charValue3);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, SIMPLEPROFILE_CHAR4_LEN, charValue4);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5);
    }

    // Init Connection Item
    peripheralInitConnItem(&peripheralConnList);

    // Register callback with SimpleGATTprofile
    SimpleProfile_RegisterAppCBs(&Peripheral_SimpleProfileCBs);

    // Register receive scan request callback
    GAPRole_BroadcasterSetCB(&Broadcaster_BroadcasterCBs);

    // Setup a delayed profile startup
    tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      peripheralInitConnItem
 *
 * @brief   Init Connection Item
 *
 * @param   peripheralConnList -
 *
 * @return  NULL
 */
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList)
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
        {
            Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & SBP_START_DEVICE_EVT)
    {
        // Start the Device
        GAPRole_PeripheralStartDevice(Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs);
        return (events ^ SBP_START_DEVICE_EVT);
    }

    if(events & SBP_PERIODIC_EVT)
    {
        // Restart timer
        if(SBP_PERIODIC_EVT_PERIOD)
        {
            tmos_start_task(Peripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD);
        }
        // Perform periodic application task
        performPeriodicTask();
        return (events ^ SBP_PERIODIC_EVT);
    }

    if(events & SBP_PARAM_UPDATE_EVT)
    {
        // Send connect param update request
        GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             Peripheral_TaskID);

        return (events ^ SBP_PARAM_UPDATE_EVT);
    }

    if(events & SBP_READ_RSSI_EVT)
    {
        GAPRole_ReadRssiCmd(peripheralConnList.connHandle);
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
        return (events ^ SBP_READ_RSSI_EVT);
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkEstablished
 *
 * @brief   Process link established.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

    // See if already connected
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        PRINT("Connection max...\n");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;

        // Set timer for periodic event
        //    tmos_start_task( Peripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );

        // Set timer for param update event
        tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);

        // Start read rssi
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);

        PRINT("Conn %x - Int %x \n", event->connectionHandle, event->connInterval);
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkTerminated
 *
 * @brief   Process link terminated.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)
{
    gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *)pEvent;

    if(event->connectionHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;
        tmos_stop_task(Peripheral_TaskID, SBP_PERIODIC_EVT);
        tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);

        // ���ݵ���ģʽ��־�����Ƿ������㲥
        if(Debug_Always_ADV_Flag)
        {
            // ����ģʽ��ʼ�����¿����㲥�����㷴�����Ӳ��ԣ�
            uint8_t advertising_enable = TRUE;
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
            PRINT("Restart advertising (DEBUG mode)\n");
        }
        else
        {
            // ��ʽģʽ��ֻ����δ����������²����¿����㲥
            // ���������豸�Ͽ����Ӻ��ٹ㲥���͹����Ż���
            if(!Mesh_is_Provisioned_Flag)
            {
                uint8_t advertising_enable = TRUE;
                GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
                PRINT("Restart advertising (not provisioned)\n");
            }
            else
            {
                PRINT("Stop advertising (already provisioned)\n");
            }
        }
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralRssiCB
 *
 * @brief   RSSI callback.
 *
 * @param   connHandle - connection handle
 * @param   rssi - RSSI
 *
 * @return  none
 */
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi)
{
    //PRINT("RSSI -%d dB Conn  %x \n", -rssi, connHandle);
    attExchangeMTUReq_t req;//added by cuixh
    req.clientRxMTU = 247;
    if(!Updata_MTU_State)
    Updata_MTU_State = GATT_ExchangeMTU(peripheralConnList.connHandle, &req, Peripheral_TaskID);
}

/*********************************************************************
 * @fn      peripheralParamUpdateCB
 *
 * @brief   Parameter update complete callback
 *
 * @param   connHandle - connect handle
 *          connInterval - connect interval
 *          connSlaveLatency - connect slave latency
 *          connTimeout - connect timeout
 *
 * @return  none
 */
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout)
{
    if(connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        PRINT("Update %x - Int %x \n", connHandle, connInterval);
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState)
    {
        case GAPROLE_STARTED:
            PRINT("Initialized..\n");
            break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
            }
            BLE_Connect_Flag = false;
            PRINT("Advertising..\n");
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                Peripheral_LinkEstablished(pEvent);
            }
            BLE_Connect_Flag = true;
            PRINT("Connected..\n");
            break;

        case GAPROLE_CONNECTED_ADV:
            PRINT("Connected Advertising..\n");
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                uint8_t  advertising_enable = TRUE;
                PRINT("Waiting for advertising..\n");
                GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                if(pEvent->gap.hdr.status != SUCCESS)
                {
                    PRINT("Waiting for advertising..\n");
                }
                else
                {
                    PRINT("Error..\n");
                }
            }
            else
            {
                PRINT("Error..%x\n", pEvent->gap.opcode);
            }
            break;

        case GAPROLE_ERROR:
            PRINT("Error..\n");
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          TMOS event.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask(void)
{

}

/*********************************************************************
 * @fn      peripheralChar4Notify
 *
 * @brief   Prepare and send simpleProfileChar4 notification
 *
 * @param   pValue - data to notify
 *          len - length of data
 *
 * @return  none
 */
void peripheralChar4Notify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        noti.len = len;
        noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(simpleProfile_Notify(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            PRINT("Notify ERR \n");
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *          pValue - pointer to data that was changed
 *          len - length of data
 *
 * @return  none
 */
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len)
{
    switch(paramID)
    {
        case SIMPLEPROFILE_CHAR1:
        {
            PRINT("Rev CHAR1\n");
            App_peripheral_reveived(pValue, len);
            break;
        }

        case SIMPLEPROFILE_CHAR3://FFE3
        {
            uint8_t newValue[SIMPLEPROFILE_CHAR3_LEN];
            uint16_t CMD_HEAD = 0;
            uint16_t target_dev = 0;
            tmos_memcpy(newValue, pValue, len);
            CMD_HEAD = newValue[0]<<8 | newValue[1];
            PRINT("Rev CHAR3..\n");
//            for(uint8_t i=0;i<len;i++)
//            {
//                PRINT("%02X ",newValue[i]);
//            }
//            PRINT("\r\n");
//            PRINT("No use\n");

            if(CMD_HEAD == CMD_MESH_DATA_HEAD)//C5C5͸��
            {
                if(Mesh_is_Provisioned_Flag)
                {//C5  5C  xx(����)  XX  XX(ID) ...
                    target_dev = newValue[4]<<8 | newValue[3];
                    Mesh_Send_Data(target_dev,newValue,len);//����͸��
            //                    UART1_SendString(reply1,strlen(reply1));
            //                    UART0_SendString(reply1,strlen(reply1));
                }
                else
                {
                    PRINT("No Mesh\r\n");
            //                    UART1_SendString(reply3,strlen(reply3));
//                    UART0_SendString(reply3,strlen(reply3));
                }
            }
            else if(CMD_HEAD == CMD_SUB_DEV_DATA_HEAD)//B55B�����豸����
            {//B5 5B 15 F1 01 00
                    target_dev = newValue[5]<<8 | newValue[4];
                    PRINT("target_dev %04X\r\n",target_dev);
                    #ifdef GATEWAY
                        Mesh_Send_Data(target_dev,newValue,len);
                    #else
//                        Deal_User_Config_Data(newValue,len);
                    #endif
            }
            else if(CMD_HEAD == CMD_BLE_DATA_HEAD)//D55D BLE͸��
            {
//                UART0_SendString(newValue,len);
            }
            else if(CMD_HEAD == CMD_GATEWAY_DATA_HEAD)//���������� ������Կ
            {
                PRINT("Config Gateway\r\n");
                #ifdef GATEWAY
                    Deal_User_Config_Data(newValue,len);
                #endif
            }
            else if(CMD_HEAD == CMD_DEBUG_MODE_HEAD)//E55E ����ģʽ����
            {
                // ��ʽ: E5 5E 01 [0x00�ر�/0x01����]
                if(len >= 4)
                {
                    uint8_t mode = newValue[3];
                    bool old_mode = Debug_Always_ADV_Flag;
                    Debug_Always_ADV_Flag = (mode != 0);

                    PRINT("Debug mode %s\r\n", Debug_Always_ADV_Flag ? "ENABLED" : "DISABLED");

                    // ���浽Flash������ģʽ�����仯ʱ��
                    if(old_mode != Debug_Always_ADV_Flag)
                    {
                        Save_Debug_Mode_To_Flash();
                    }

                    // ����Ӧ�������ã�����л�������ģʽ�ҵ�ǰδ�㲥�������㲥
                    if(Debug_Always_ADV_Flag)
                    {
                        uint8_t advertising_enable = TRUE;
                        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
                        PRINT("Advertising enabled\r\n");
                    }

                    // ����ȷ��
                    uint8_t response[4] = {0xE5, 0x5E, 0x01, mode};
                    peripheralChar4Notify(response, 4);
                }
            }
            else if(CMD_HEAD == CMD_QUERY_MESH_ADDR_HEAD)//F55F ��ѯMESH��ַ
            {
                // ��ʽ: F5 5F (��ѯ�����2�ֽ�)
                // ��Ӧ: F5 5F [����״̬] [��ַL] [��ַH]
                uint8_t response[5];
                response[0] = 0xF5;
                response[1] = 0x5F;
                response[2] = Mesh_is_Provisioned_Flag ? 0x01 : 0x00;
                response[3] = my_net_addr & 0xFF;
                response[4] = (my_net_addr >> 8) & 0xFF;

                peripheralChar4Notify(response, 5);
                PRINT("Query Mesh Addr - Provisioned:%d, Addr:0x%04X\r\n",
                      Mesh_is_Provisioned_Flag, my_net_addr);
            }
            else if(CMD_HEAD == CMD_QUERY_MESH_KEY_HEAD)//F66F ��ѯMESH��Կ
            {
                // ��ʽ: F6 6F (��ѯ�����2�ֽ�)
                // ��Ӧ: F6 6F [����״̬] [16�ֽ�������Կ]
                uint8_t response[19];
                response[0] = 0xF6;
                response[1] = 0x6F;
                response[2] = Mesh_is_Provisioned_Flag ? 0x01 : 0x00;

                // ������Կ������user_header.h��������
                if(Mesh_is_Provisioned_Flag)
                {
                    tmos_memcpy(&response[3], self_prov_net_key, 16);
                }
                else
                {
                    tmos_memset(&response[3], 0, 16);
                }

                peripheralChar4Notify(response, 19);
                PRINT("Query Mesh Key - Provisioned:%d\r\n", Mesh_is_Provisioned_Flag);
            }
            else
            {
//                Mesh_Send_Data(0xffff,newValue,len);
                App_peripheral_reveived(newValue,len);//����
            }
            break;
        }

        default:
            // should not reach here!
            break;
    }
}

/*********************************************************************
 * @fn      Peripheral_AdvertData_Privisioned
 *
 * @brief   �޸Ĺ㲥������������־.
 *
 * @param   privisioned - �Ƿ�������
 *
 * @return  none
 */
void Peripheral_AdvertData_Privisioned(uint8_t privisioned)
{
    uint8_t  advertising_enable;
    if(privisioned)
    {
        advertData[11]=0x01;
        advertData[12]=0x00;
    }
    else
    {
        advertData[11]=0x00;
        advertData[12]=0x00;
    }
    GAPRole_GetParameter(GAPROLE_ADVERT_ENABLED, &advertising_enable);
    if(advertising_enable)
    {
        advertising_enable = FALSE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        advertising_enable = TRUE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
    }
    else
    {
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
    }
}

/*********************************************************************
 * @fn      Peripheral_TerminateLink
 *
 * @brief   �Ͽ�����.
 *
 * @return  none
 */
void Peripheral_TerminateLink(void)
{
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(peripheralConnList.connHandle);
    }
}
void Stop_Adv(void)
{
    uint8_t initial_advertising_enable = FALSE;
    Stop_BLE_ADV_Flag = 1;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);//�رչ㲥
}
void Start_Adv(void)
{
    uint8_t initial_advertising_enable = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);//�����㲥
}
/*********************************************************************
*********************************************************************/
