/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief BLE Heart Rate and Running speed Relay application main file.
 *
 * @detail This application demonstrates a simple "Relay".
 * Meaning we pass on the values that we receive. By combining a collector part on
 * one end and a sensor part on the other, we show that the s130 can function
 * simultaneously as a central and a peripheral device.
 *
 * In the figure below, the sensor ble_app_hrs connects and interacts with the relay
 * in the same manner it would connect to a heart rate collector. In this case, the Relay
 * application acts as a central.
 *
 * On the other side, a collector (such as Master Control panel or ble_app_hrs_c) connects
 * and interacts with the relay the same manner it would connect to a heart rate sensor peripheral.
 *
 * Led layout:
 * LED 1: Central side is scanning       LED 2: Central side is connected to a peripheral
 * LED 3: Peripheral side is advertising LED 4: Peripheral side is connected to a central
 *
 * @note While testing, be careful that the Sensor and Collector are actually connecting to the Relay,
 *       and not directly to each other!
 *
 *    Peripheral                  Relay                    Central
 *    +--------+        +-----------|----------+        +-----------+
 *    | Heart  |        | Heart     |   Heart  |        |           |
 *    | Rate   | -----> | Rate     -|-> Rate   | -----> | Collector |
 *    | Sensor |        | Collector |   Sensor |        |           |
 *    +--------+        +-----------|   and    |        +-----------+
 *                      | Running   |   Running|
 *    +--------+        | Speed    -|-> Speed  |
 *    | Running|------> | Collector |   Sensor |
 *    | Speed  |        +-----------|----------+
 *    | Sensor |
 *    +--------+
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "peer_manager.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_rscs.h"
#include "nrf_delay.h"


#include "our_service_hrs.h"
#include "our_c_hrs.h"
//Ble services
//#include "our_service.h"

#include "ble_rscs_c.h"
#include "ble_conn_state.h"
#include "nrf_fstorage.h"
#include "fds.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_gpio.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#include "nrf_calendar.h"





#define PERIPHERAL_ADVERTISING_LED      BSP_BOARD_LED_2
#define PERIPHERAL_CONNECTED_LED        BSP_BOARD_LED_3
#define CENTRAL_SCANNING_LED            BSP_BOARD_LED_0
#define CENTRAL_CONNECTED_LED           BSP_BOARD_LED_1

#define DEVICE_NAME                     "STOP-COVID19"                              /**< Name of device used for advertising. */
#define MANUFACTURER_NAME               "CASKY"                                     /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                         /**< The advertising interval (in units of 0.625 ms). This value corresponds to 187.5 ms. */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size in octets. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size in octets. */

#define SCAN_INTERVAL                   0x00A0                                      /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                     0x0050                                      /**< Determines scan window in units of 0.625 millisecond. */

#define SCAN_DURATION                   0x0000                                      /**< Duration of the scanning in units of 10 milliseconds. If set to 0x0000, scanning will continue until it is explicitly disabled. */


#define MIN_CONNECTION_INTERVAL         (uint16_t) MSEC_TO_UNITS(7.5, UNIT_1_25_MS) /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL         (uint16_t) MSEC_TO_UNITS(30, UNIT_1_25_MS)  /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY                   0                                           /**< Determines slave latency in terms of connection events. */
#define SUPERVISION_TIMEOUT             (uint16_t) MSEC_TO_UNITS(4000, UNIT_10_MS)  /**< Determines supervision time-out in units of 10 milliseconds. */

/**@brief   Priority of the application BLE event handler.
 * @note    You shouldn't need to modify this value.
 */
#define APP_BLE_OBSERVER_PRIO           3


static ble_hrs_t m_hrs;                                             /**< Heart rate service instance. */
static ble_rscs_t m_rscs;                                           /**< Running speed and cadence service instance. */
static ble_hrs_c_t m_hrs_c;                                         /**< Heart rate service client instance. */
static ble_rscs_c_t m_rscs_c;                                       /**< Running speed and cadence service client instance. */

// Declare a service structure for our application
//ble_os_t m_our_service;   //first service name

NRF_BLE_GATT_DEF(m_gatt);                                           /**< GATT module instance. */
NRF_BLE_QWRS_DEF(m_qwr, NRF_SDH_BLE_TOTAL_LINK_COUNT);              /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                 /**< Advertising module instance. */
BLE_DB_DISCOVERY_ARRAY_DEF(m_db_discovery, 2);                      /**< Database discovery module instances. */

BLE_DB_DISCOVERY_DEF(m_db_disc);                                /**< DB discovery module instance. */

static uint16_t m_conn_handle_hrs_c  = BLE_CONN_HANDLE_INVALID;     /**< Connection handle for the HRS central application */
static uint16_t m_conn_handle_rscs_c = BLE_CONN_HANDLE_INVALID;     /**< Connection handle for the RSC central application */

// **Declare an app_timer id variable and define our timer interval and define a timer interval */
APP_TIMER_DEF(m_our_char_timer_id);

#define OUR_CHAR_TIMER_INTERVAL         APP_TIMER_TICKS(1000)   // 200 ms intervals


static bool run_time_updates = false;


/**@brief names which the central applications will scan for, and which will be advertised by the peripherals.
 *  if these are set to empty strings, the UUIDs defined below will be used
 */
static char const m_target_periph_name[] = "";

/**@brief our uuid filter . */
static ble_uuid_t const target_uuid =
{
    .uuid = HRS_UUID,
    .type = BLE_UUID_TYPE_BLE
};

/**@brief UUIDs which the central applications will scan for if the name above is set to an empty string,
 * and which will be advertised by the peripherals.
 */
static ble_uuid_t m_adv_uuids[] =
{
    //{BLE_UUID_OUR_BASE_UUID,        BLE_UUID_TYPE_BLE},
    {HRS_UUID,        BLE_UUID_TYPE_BLE},
    //{BLE_UUID_RUNNING_SPEED_AND_CADENCE, BLE_UUID_TYPE_BLE}
};

/**@brief Parameters used when scanning. */
static ble_gap_scan_params_t const m_scan_params =
{
    .extended      = 1,
    .active        = 1,
    .interval      = SCAN_INTERVAL,
    .window        = SCAN_WINDOW,
    .timeout       = SCAN_DURATION,
    .scan_phys     = BLE_GAP_PHY_1MBPS,
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
};

static uint8_t m_scan_buffer_data[BLE_GAP_SCAN_BUFFER_EXTENDED_MIN]; /**< buffer where advertising reports will be stored by the SoftDevice. */

/**@brief Pointer to the buffer where advertising reports will be stored by the SoftDevice. */
static ble_data_t m_scan_buffer =
{
    m_scan_buffer_data,
    BLE_GAP_SCAN_BUFFER_EXTENDED_MIN
};

/**@brief Connection parameters requested for connection. */
static ble_gap_conn_params_t const m_connection_param =
{
    MIN_CONNECTION_INTERVAL,
    MAX_CONNECTION_INTERVAL,
    SLAVE_LATENCY,
    SUPERVISION_TIMEOUT
};


/***  Used for testing FDS ***/
static volatile uint8_t write_flag_fds_test = 0;
#define FILE_ID_FDS_TEST     0x0001
#define REC_KEY_FDS_TEST     0x2222
/*****************************/


/**@brief Function to handle asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initiating scanning.
 */
static void scan_start(void)
{
    ret_code_t err_code;

    (void) sd_ble_gap_scan_stop();

    err_code = sd_ble_gap_scan_start(&m_scan_params, &m_scan_buffer);
    // It is okay to ignore this error since we are stopping the scan anyway.
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for initiating advertising and scanning.
 */
static void adv_scan_start(void)
{
    ret_code_t err_code;

    //check if there are no flash operations in progress
    if (!nrf_fstorage_is_busy(NULL))
    {
        // Start scanning for peripherals and initiate connection to devices which
        // advertise Heart Rate or Running speed and cadence UUIDs.
        scan_start();

        // Turn on the LED to signal scanning.
        bsp_board_led_on(CENTRAL_SCANNING_LED);

        // Start advertising.
        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.\n");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d. \n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            adv_scan_start();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // This can happen when the local DB has changed.
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

//****************************FDS flash sauvgarde*********
static void my_fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result != FDS_SUCCESS)
            {
                // Initialization failed.
            }
            break;
                case FDS_EVT_WRITE:
                        if (p_fds_evt->result == FDS_SUCCESS)
                        {
                            write_flag_fds_test=1;
                        }
                        break;
        default:
            break;
    }
}

static ret_code_t fds_test_write(void)
{

        uint8_t  m_deadbeef[9] = {0xf0,0x18,0x98,0xcf,0xd1,0x7d,0xfe,0xff,0xef};
        fds_record_t        record;
        fds_record_desc_t   record_desc;

        // Set up data.

        // Set up record.
        record.file_id              = FILE_ID_FDS_TEST;
        record.key                      = REC_KEY_FDS_TEST;
        record.data.p_data       = &m_deadbeef;
        //record.data.length_words   = sizeof(m_deadbeef)/sizeof(uint32_t);
        record.data.length_words   = sizeof(m_deadbeef)/sizeof(uint8_t);

        printf("lentgh : %d\n", record.data.length_words  );
        ret_code_t ret = fds_record_write(&record_desc, &record);
        // if (ret != FDS_SUCCESS)
        // {
        //         return ret;
        // }
        if(ret == FDS_SUCCESS){ printf("If the operation was queued successfully.\n");}
        if(ret == FDS_ERR_NOT_INITIALIZED){ printf("If the module is not initialized\n");}
        if(ret == FDS_ERR_NULL_ARG){ printf(" If p_record is NULL.\n");}
        if(ret == FDS_ERR_INVALID_ARG){ printf("If the file ID or the record key is invalid.\n");}
        if(ret == FDS_ERR_UNALIGNED_ADDR){ printf(" If the record data is not aligned to a 4 byte boundary.\n");}
        if(ret == FDS_ERR_RECORD_TOO_LARGE){ printf(" If the record data exceeds the maximum length.\n");}
        if(ret == FDS_ERR_NO_SPACE_IN_QUEUES){ printf("  If the operation queue is full or there are more record chunks than can be buffered.\n");}
        if(ret == FDS_ERR_NO_SPACE_IN_FLASH ){ printf(" If there is not enough free space in flash to store the record.\n");}



         printf("Writing Record ID = %d \r\n",record_desc.record_id);
        return NRF_SUCCESS;
}

void printBuf(uint8_t* b, int n){
  for(int i=0; i<n; i++)
  {
    printf("%d ",b[i]);
    printf(" ");
  }
  printf("\n");
}
static ret_code_t fds_read(void)
{

        fds_flash_record_t  flash_record;
        fds_record_desc_t   record_desc;
        fds_find_token_t    ftok ={0};//Important, make sure you zero init the ftok token
        //uint32_t *data;
        uint8_t *data;
        uint32_t err_code;

        printf("Start searching... \r\n");
        // Loop until all records with the given key and file ID have been found.
        while (fds_record_find(FILE_ID_FDS_TEST, REC_KEY_FDS_TEST, &record_desc, &ftok) == FDS_SUCCESS)
        {
                err_code = fds_record_open(&record_desc, &flash_record);
                if ( err_code != FDS_SUCCESS)
                {
                    return err_code;
                }

                printf("Found Record ID = %d\r\n",record_desc.record_id);
                printf("Data = ");
                //data = (uint32_t *) flash_record.p_data;
                data = (uint8_t *) flash_record.p_data;
                for (uint8_t i=0;i<flash_record.p_header->length_words;i++)
                {
                    printf("0x%8x ",data[i]);
                }
                printf("\r\n");

                //printBuf(data,flash_record.p_header->length_words);

                // Access the record through the flash_record structure.
                // Close the record when done.
                err_code = fds_record_close(&record_desc);
                if (err_code != FDS_SUCCESS)
                {
                    return err_code;
                }
        }
        return NRF_SUCCESS;

}

static ret_code_t fds_test_find_and_delete (void)
{

        fds_record_desc_t   record_desc;
        fds_find_token_t    ftok;

        ftok.page=0;
        ftok.p_addr=NULL;
        // Loop and find records with same ID and rec key and mark them as deleted.
        while (fds_record_find(FILE_ID_FDS_TEST, REC_KEY_FDS_TEST, &record_desc, &ftok) == FDS_SUCCESS)
        {
            fds_record_delete(&record_desc);
            NRF_LOG_INFO("Deleted record ID: %d \r\n",record_desc.record_id);
        }
        // call the garbage collector to empty them, don't need to do this all the time, this is just for demonstration
        ret_code_t ret = fds_gc();
        if (ret != FDS_SUCCESS)
        {
                return ret;
        }
        return NRF_SUCCESS;
}

static ret_code_t fds_test_init (void)
{

        ret_code_t ret = fds_register(my_fds_evt_handler);
        if (ret != FDS_SUCCESS)
        {
                    return ret;

        }
        ret = fds_init();
        if (ret != FDS_SUCCESS)
        {
                return ret;
        }

        return NRF_SUCCESS;

}





//****************************FDS flash memory saving*********



/**@brief Handles events coming from the Heart Rate central module.
 */
static void hrs_c_evt_handler(ble_hrs_c_t * p_hrs_c, ble_hrs_c_evt_t * p_hrs_c_evt)
{
    switch (p_hrs_c_evt->evt_type)
    {
        case BLE_HRS_C_EVT_DISCOVERY_COMPLETE:
        {
            if (m_conn_handle_hrs_c == BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;

                m_conn_handle_hrs_c = p_hrs_c_evt->conn_handle;
                NRF_LOG_INFO("HRS discovered on conn_handle 0x%x \n", m_conn_handle_hrs_c);

                err_code = ble_hrs_c_handles_assign(p_hrs_c,
                                                    m_conn_handle_hrs_c,
                                                    &p_hrs_c_evt->params.peer_db);
                APP_ERROR_CHECK(err_code);
                // Initiate bonding.
                err_code = pm_conn_secure(m_conn_handle_hrs_c, false);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }

                // Heart rate service discovered. Enable notification of Heart Rate Measurement.
                err_code = ble_hrs_c_hrm_notif_enable(p_hrs_c);
                APP_ERROR_CHECK(err_code);
            }
        } break; // BLE_HRS_C_EVT_DISCOVERY_COMPLETE

        case BLE_HRS_C_EVT_HRM_NOTIFICATION:
        {
            ret_code_t err_code;

            //NRF_LOG_INFO("Heart Rate = %d \n", p_hrs_c_evt->params.hrm.hr_value);
            //printBuff(p_hrs_c_evt->params.hrm.hr_value,9);

        uint8_t* Charbuffer = getbuffer();

            printBuff(Charbuffer,9);
        //our_characteristic_update(&m_our_service, Charbuffer,9); // buffer charachtere



            err_code = ble_hrs_heart_rate_measurement_send_1(&m_hrs, Charbuffer);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != NRF_ERROR_RESOURCES) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                )
            {
                APP_ERROR_HANDLER(err_code);
            }
        } break; // BLE_HRS_C_EVT_HRM_NOTIFICATION

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Handles events coming from  Running Speed and Cadence central module.
 */
static void rscs_c_evt_handler(ble_rscs_c_t * p_rscs_c, ble_rscs_c_evt_t * p_rscs_c_evt)
{
    switch (p_rscs_c_evt->evt_type)
    {
        case BLE_RSCS_C_EVT_DISCOVERY_COMPLETE:
        {
            if (m_conn_handle_rscs_c == BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;

                m_conn_handle_rscs_c = p_rscs_c_evt->conn_handle;
                NRF_LOG_INFO("Running Speed and Cadence service discovered on conn_handle 0x%x \n",
                             m_conn_handle_rscs_c);

                err_code = ble_rscs_c_handles_assign(p_rscs_c,
                                                    m_conn_handle_rscs_c,
                                                    &p_rscs_c_evt->params.rscs_db);
                APP_ERROR_CHECK(err_code);

                // Initiate bonding.
                err_code = pm_conn_secure(m_conn_handle_rscs_c, false);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }

                // Running speed cadence service discovered. Enable notifications.
                err_code = ble_rscs_c_rsc_notif_enable(p_rscs_c);
                APP_ERROR_CHECK(err_code);
            }
        } break; // BLE_RSCS_C_EVT_DISCOVERY_COMPLETE:

        case BLE_RSCS_C_EVT_RSC_NOTIFICATION:
        {
            ret_code_t      err_code;
            ble_rscs_meas_t rscs_measurment;

            NRF_LOG_INFO("Speed      = %d \n", p_rscs_c_evt->params.rsc.inst_speed);

            rscs_measurment.is_running                  = p_rscs_c_evt->params.rsc.is_running;
            rscs_measurment.is_inst_stride_len_present  = p_rscs_c_evt->params.rsc.is_inst_stride_len_present;
            rscs_measurment.is_total_distance_present   = p_rscs_c_evt->params.rsc.is_total_distance_present;

            rscs_measurment.inst_stride_length = p_rscs_c_evt->params.rsc.inst_stride_length;
            rscs_measurment.inst_cadence       = p_rscs_c_evt->params.rsc.inst_cadence;
            rscs_measurment.inst_speed         = p_rscs_c_evt->params.rsc.inst_speed;
            rscs_measurment.total_distance     = p_rscs_c_evt->params.rsc.total_distance;

            err_code = ble_rscs_measurement_send(&m_rscs, &rscs_measurment);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != NRF_ERROR_RESOURCES) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                )
            {
                APP_ERROR_HANDLER(err_code);
            }
        } break; // BLE_RSCS_C_EVT_RSC_NOTIFICATION

        default:
            // No implementation needed.
            break;
    }
}



bool Read =false;

uint8_t  macbeef[8] ;
uint8_t  macbeef2[8] = {0xf0,0x18,0x98,0xcf,0xd1,0x7d,0x05,0x03};
int in;

int pro1=0;int cnt1=0;
int pro2=0;int cnt2=0;
int pro3=0;int cnt3=0;
int pro4=0;int cnt4=0;
int pro5=0;int cnt5=0;
int pro6=0;int cnt6=0;
int pro7=0;int cnt7=0;
int pro8=0;int cnt8=0;
int pro9=0;int cnt9=0;
int pro10=0;int cnt10=0;

// This is a timer event handler this one we make it only for sendning values of the mpu
static void timer_timeout_handler(void * p_context)
{
        int* reading = getreading();
        fds_flash_record_t  flash_record;
        fds_record_desc_t   record_desc;
        fds_find_token_t    ftok ={0};//Important, make sure you zero init the ftok token
        //uint32_t *data;
        uint8_t *data;
        ret_code_t err_code;


    // struct tm * now = nrf_cal_get_time();

    // printf( "%4d-%02d-%02d %02d:%02d:%02d\n",
    //     now->tm_year+1900, now->tm_mon+1, now->tm_mday,
    //     now->tm_hour, now->tm_min, now->tm_sec );
//print_current_time();

        macbeef2[5]= ++macbeef2[5];
        macbeef2[7]= ++macbeef2[7];

        if (macbeef2[7]>29)
        {
            // printf("done\n");
             macbeef2[7]=1;
        }
        if (macbeef2[5]>250 )
        {
            //printf("done\n");
             macbeef2[5]=0;
        }


            // printf("Start searching... \r\n");
        if(*reading==1){

            // Loop until all records with the given key and file ID have been found.
            in=0;
            while (fds_record_find(FILE_ID_FDS_TEST, REC_KEY_FDS_TEST, &record_desc, &ftok) == FDS_SUCCESS)
            {

                    err_code = fds_record_open(&record_desc, &flash_record);
                    if ( err_code != FDS_SUCCESS)
                    {
                        return err_code;
                    }

                        nrf_delay_ms(1000);

                        printf("Found Record ID = %d\r\n",record_desc.record_id);

                        //printf("Data = ");
                        //data = (uint32_t *) flash_record.p_data;
                        data = (uint8_t *) flash_record.p_data;
                        for (uint8_t i=0;i<flash_record.p_header->length_words;i++)
                        {
                           // printf("0x%8x ",data[i]);

                            macbeef[i]=data[i];

                        }
                        //printf("\r\n");
                        if (record_desc.record_id==1)
                        {
                            in++;
                            printf("in: %d\n",in );
                            if (in==1)
                            {
                                *reading=0;
                            }
                        }

                        printBuf(macbeef,8);

                            err_code = ble_hrs_heart_rate_measurement_send_1(&m_hrs, macbeef);
                            if ((err_code != NRF_SUCCESS) &&
                                (err_code != NRF_ERROR_INVALID_STATE) &&
                                (err_code != NRF_ERROR_RESOURCES) &&
                                (err_code != NRF_ERROR_BUSY) &&
                                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                                )
                            {
                                APP_ERROR_HANDLER(err_code);
                            }

                    // Access the record through the flash_record structure.
                    // Close the record when done.
                    err_code = fds_record_close(&record_desc);
                    if (err_code != FDS_SUCCESS)
                    {
                        return err_code;
                        //Read==false;
                    }
            }
            return NRF_SUCCESS;

        }


            if (pro1==10)
            {
                cnt1++;
                printf("%d\n",cnt1 );
            }
            if (pro2==20)
            {
                cnt2++;
                printf("%d\n",cnt2 );
            }
            if (pro3==30)
            {
                cnt3++;
                printf("%d\n",cnt3 );
            }
            if (pro4==40)
            {
                cnt4++;
                printf("%d\n",cnt4 );
            }
            if (pro5==50)
            {
                cnt5++;
                printf("%d\n",cnt5 );
            }


}

uint8_t MacAdrsse[8];
uint8_t  m_deadbeef[8];
bool kayn;

void writeFDS(uint8_t* b){

         // uint8_t  m_deadbeef[6] = {b[0],b[1],b[2],b[3],b[4],b[5]};
            m_deadbeef[0]=b[0];
            m_deadbeef[1]=b[1];
            m_deadbeef[2]=b[2];
            m_deadbeef[3]=b[3];
            m_deadbeef[4]=b[4];
            m_deadbeef[5]=b[5];
            m_deadbeef[6]=b[6];
            m_deadbeef[7]=b[7];

         printBuff(m_deadbeef,8);
         // uint8_t  m_deadbeef[6] = {0xf0,0x18,0x98,0xcf,0xd1,0x7d};
        // static uint8_t const m_deadbeef[6] = {0xf0,0x18,0x98,0xcf,0xd1,0x7d};
        // static uint32_t const m_deadbeef[2] = {0xDEADBEEF,0xBAADF00D};
        //static uint8_t const m_deadbeef[8] = {0x1,0x2,0x3,0x4,0x51,0x62,0x73,0x84};

        fds_record_t        record;
        fds_record_desc_t   record_desc;

        // Set up data.

        // Set up record.
        record.file_id              = FILE_ID_FDS_TEST;
        record.key                      = REC_KEY_FDS_TEST;
        record.data.p_data       = &m_deadbeef;
        //record.data.length_words   = sizeof(m_deadbeef)/sizeof(uint32_t);
        record.data.length_words   = sizeof(m_deadbeef)/sizeof(uint8_t);

        printf("lentgh : %d\n", record.data.length_words  );
        ret_code_t ret = fds_record_write(&record_desc, &record);
        // if (ret != FDS_SUCCESS)
        // {
        //         return ret;
        // }
        if(ret == FDS_SUCCESS){ printf("If the operation was queued successfully.\n");}
        if(ret == FDS_ERR_NOT_INITIALIZED){ printf("If the module is not initialized\n");}
        if(ret == FDS_ERR_NULL_ARG){ printf(" If p_record is NULL.\n");}
        if(ret == FDS_ERR_INVALID_ARG){ printf("If the file ID or the record key is invalid.\n");}
        if(ret == FDS_ERR_UNALIGNED_ADDR){ printf(" If the record data is not aligned to a 4 byte boundary.\n");}
        if(ret == FDS_ERR_RECORD_TOO_LARGE){ printf(" If the record data exceeds the maximum length.\n");}
        if(ret == FDS_ERR_NO_SPACE_IN_QUEUES){ printf("  If the operation queue is full or there are more record chunks than can be buffered.\n");}
        if(ret == FDS_ERR_NO_SPACE_IN_FLASH ){ printf(" If there is not enough free space in flash to store the record.\n");}



         printf("Writing Record ID = %d \r\n",record_desc.record_id);
        return NRF_SUCCESS;

}

uint8_t  macfound[6] ;

bool verifyRead(uint8_t* b){
        bool found;

        fds_flash_record_t  flash_record;
        fds_record_desc_t   record_desc;
        fds_find_token_t    ftok ={0};//Important, make sure you zero init the ftok token
        //uint32_t *data;
        uint8_t *data;
        ret_code_t err_code;

        //printBuff(b,6);
           // printf("Start searching... \r\n");

            // Loop until all records with the given key and file ID have been found.
            while (fds_record_find(FILE_ID_FDS_TEST, REC_KEY_FDS_TEST, &record_desc, &ftok) == FDS_SUCCESS)
            {

                    err_code = fds_record_open(&record_desc, &flash_record);
                    if ( err_code != FDS_SUCCESS)
                    {
                        return err_code;
                    }

                        // nrf_delay_ms(1000);
                        // printf("Found Record ID = %d\r\n",record_desc.record_id);

                        //printf("Data = ");
                        //data = (uint32_t *) flash_record.p_data;
                        data = (uint8_t *) flash_record.p_data;
                        for (uint8_t i=0;i<flash_record.p_header->length_words;i++)
                        {
                            // printf("0x%8x ",data[i]);

                            macfound[i]=data[i];

                        }
                        // printf("\r\n");

                        if(b[0]==macfound[0] && b[1]==macfound[1] && b[2]==macfound[2] && b[3]==macfound[3] && b[4]==macfound[4] && b[5]==macfound[5]){

                            found =true;
                            //printf("mujuuuuuuuud\n");
                        }else{
                            found =false;

                        }


                    // Access the record through the flash_record structure.
                    // Close the record when done.
                    err_code = fds_record_close(&record_desc);
                    if (err_code != FDS_SUCCESS)
                    {
                        return err_code;
                    }
            }



        return found;
}

bool verifyMac(uint8_t* MacAdrsse,uint8_t*  Mac){

    if(Mac[0]==MacAdrsse[0] && Mac[1]==MacAdrsse[1] && Mac[2]==MacAdrsse[2] && Mac[3]==MacAdrsse[3] && Mac[4]==MacAdrsse[4] && Mac[5]==MacAdrsse[5]){

    return true;
    //printf("mujuuuuuuuud\n");
    }else{
    return false;

    }

}
uint8_t Mac1[8],Mac2[8],Mac3[8],Mac4[8],Mac5[8];
bool occupe=false;
bool prev1,prev2,prev3,prev4,prev5;
int baraka=0;
/**@brief Function for handling the advertising report BLE event.
 *
 * @param[in] p_adv_report  Advertising report from the SoftDevice.
 */
static void on_adv_report(ble_gap_evt_adv_report_t const * p_adv_report)
{
    baraka ++;
    ret_code_t err_code;
    nrf_gpio_cfg_output(29);


        if (ble_advdata_uuid_find(p_adv_report->data.p_data, p_adv_report->data.len, &target_uuid))
        {
            //NRF_LOG_INFO("RSSI: %d", p_adv_report->rssi);
            if(p_adv_report->rssi > (-68)){
            nrf_gpio_pin_set(29);
            struct tm * now = nrf_cal_get_time();

            MacAdrsse[0]=p_adv_report->peer_addr.addr[0];
            MacAdrsse[1]=p_adv_report->peer_addr.addr[1];
            MacAdrsse[2]=p_adv_report->peer_addr.addr[2];
            MacAdrsse[3]=p_adv_report->peer_addr.addr[3];
            MacAdrsse[4]=p_adv_report->peer_addr.addr[4];
            MacAdrsse[5]=p_adv_report->peer_addr.addr[5];
            MacAdrsse[6]=now->tm_mon+1;
            MacAdrsse[7]=now->tm_mday;

                    if (verifyMac(MacAdrsse,Mac1)==false&&verifyMac(MacAdrsse,Mac2)==false&& verifyMac(MacAdrsse,Mac3)==false&& cnt1==0 && prev1==false)
                    {
                        Mac1[0]=MacAdrsse[0];
                        Mac1[1]=MacAdrsse[1];
                        Mac1[2]=MacAdrsse[2];
                        Mac1[3]=MacAdrsse[3];
                        Mac1[4]=MacAdrsse[4];
                        Mac1[5]=MacAdrsse[5];
                        printf("mashi b7al b7al1\n");
                        // occupe=true;
                        pro1=10;
                        prev1=true;
                        // prev2=false;
                        // prev3=true;
                        // prev4=true;
                        // prev5=true;


                    }else if (verifyMac(MacAdrsse,Mac1)==false&&verifyMac(MacAdrsse,Mac2)==false&& verifyMac(MacAdrsse,Mac3)==false&& cnt2==0 && prev2==false)
                    {
                        Mac2[0]=MacAdrsse[0];
                        Mac2[1]=MacAdrsse[1];
                        Mac2[2]=MacAdrsse[2];
                        Mac2[3]=MacAdrsse[3];
                        Mac2[4]=MacAdrsse[4];
                        Mac2[5]=MacAdrsse[5];
                        printf("mashi b7al b7al2\n");
                        pro2=20;
                        prev2=true;
                        // prev3=false;
                        // prev4=true;
                        // prev5=true;

                    }else if (verifyMac(MacAdrsse,Mac1)==false&&verifyMac(MacAdrsse,Mac2)==false&& verifyMac(MacAdrsse,Mac3)==false&& cnt3==0&& prev3==false)
                    {
                        Mac3[0]=MacAdrsse[0];
                        Mac3[1]=MacAdrsse[1];
                        Mac3[2]=MacAdrsse[2];
                        Mac3[3]=MacAdrsse[3];
                        Mac3[4]=MacAdrsse[4];
                        Mac3[5]=MacAdrsse[5];
                        printf("mashi b7al b7al3\n");
                        pro3=30;
                        prev3=true;
                        // prev4=true;
                        // prev5=true;

                    }
                        if (cnt1>10 && verifyMac(MacAdrsse,Mac1)==true )
                        {
                            printf("ha da9i9a lewl dazt\n");
                            pro1=0;
                            cnt1=0;
                                kayn = verifyRead(Mac1);
                                if (kayn==true)
                                {
                                    printf("kayn\n");
                                    printf("..");
                                    prev1=false;
                                    Mac1[0]=0;
                                }else{
                                    Mac1[6]=now->tm_mon+1;
                                    Mac1[7]=now->tm_mday;
                                    writeFDS(Mac1);
                                    prev1=false;
                                    Mac1[0]=0;



                                }
                        }
                        if (cnt2>10 && verifyMac(MacAdrsse,Mac2)==true)
                        {
                            printf("ha da9i9a 2 dazt\n");
                            pro2=0;
                            cnt2=0;
                                kayn = verifyRead(Mac4);
                                if (kayn==true)
                                {
                                    printf("kayn\n");
                                    printf("..");
                                    prev2=false;
                                    Mac2[0]=0;


                                }else{
                                    Mac2[6]=now->tm_mon+1;
                                    Mac2[7]=now->tm_mday;
                                    writeFDS(Mac2);
                                    prev2=false;
                                    Mac3[0]=0;



                                }
                        }
                        if (cnt3>10 && verifyMac(MacAdrsse,Mac3)==true)
                        {
                            printf("ha da9i9a 3 dazt\n");
                            pro3=0;
                            cnt3=0;

                                kayn = verifyRead(Mac3);
                                if (kayn==true)
                                {
                                    printf("kayn\n");
                                    printf("..");
                                    prev3=false;
                                    Mac3[0]=0;


                                }else{
                                    Mac3[6]=now->tm_mon+1;
                                    Mac3[7]=now->tm_mday;
                                    writeFDS(Mac2);
                                    prev3=false;
                                    Mac3[0]=0;



                                }
                        }

            }else if(p_adv_report->rssi < (-75)){
                nrf_gpio_pin_clear(29);
                // pro=0;
                // cnt1=0;

                        if (cnt1>15 && verifyMac(MacAdrsse,Mac1)==true || cnt1>20)
                        {
                            printf("ha da9i9a lewl saalat\n");
                            pro1=0;
                            cnt1=0;

                                    prev1=false;
                                    Mac1[0]=0;

                        }
                        if (cnt2>15 && verifyMac(MacAdrsse,Mac2)==true || cnt2>20)
                        {
                            printf("ha da9i9a 2 saalat\n");
                            pro2=0;
                            cnt2=0;

                                    prev2=false;
                                    Mac2[0]=0;


                        }
                        if (cnt3>15 && verifyMac(MacAdrsse,Mac3)==true || cnt3>20)
                        {
                            printf("ha da9i9a 3 saalat\n");
                            pro3=0;
                            cnt3=0;

                                    prev3=false;
                                    Mac3[0]=0;

                        }

            }else{
                nrf_gpio_pin_clear(29);
                // pro=0;
                // cnt1=0;

            }



            err_code = sd_ble_gap_scan_start(NULL, &m_scan_buffer);
            APP_ERROR_CHECK(err_code);

        }
        else
        {
            err_code = sd_ble_gap_scan_start(NULL, &m_scan_buffer);
            APP_ERROR_CHECK(err_code);
            nrf_gpio_pin_clear(29);


        }
}


/**@brief Function for assigning new connection handle to available instance of QWR module.
 *
 * @param[in] conn_handle New connection handle.
 */
static void multi_qwr_conn_handle_assign(uint16_t conn_handle)
{
    for (uint32_t i = 0; i < NRF_SDH_BLE_TOTAL_LINK_COUNT; i++)
    {
        if (m_qwr[i].conn_handle == BLE_CONN_HANDLE_INVALID)
        {
            ret_code_t err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr[i], conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
        }
    }
}


/**@brief   Function for handling BLE events from central applications.
 *
 * @details This function parses scanning reports and initiates a connection to peripherals when a
 *          target UUID is found. It updates the status of LEDs used to report central applications
 *          activity.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_central_evt(ble_evt_t const * p_ble_evt)
{
    ret_code_t            err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        // Upon connection, check which peripheral has connected (HR or RSC), initiate DB
        // discovery, update LEDs status and resume scanning if necessary.
        case BLE_GAP_EVT_CONNECTED:
        {
            NRF_LOG_INFO("Central connected \n");
            // If no Heart Rate sensor or RSC sensor is currently connected, try to find them on this peripheral.
            if (   (m_conn_handle_hrs_c  == BLE_CONN_HANDLE_INVALID)
                || (m_conn_handle_rscs_c == BLE_CONN_HANDLE_INVALID))
            {
                NRF_LOG_INFO("Attempt to find HRS or RSC on conn_handle 0x%x \n", p_gap_evt->conn_handle);

                err_code = ble_db_discovery_start(&m_db_discovery[0], p_gap_evt->conn_handle);
                if (err_code == NRF_ERROR_BUSY)
                {
                    err_code = ble_db_discovery_start(&m_db_discovery[1], p_gap_evt->conn_handle);
                    APP_ERROR_CHECK(err_code);
                }
                else
                {
                    APP_ERROR_CHECK(err_code);
                }
            }

            // Assing connection handle to the QWR module.
            multi_qwr_conn_handle_assign(p_gap_evt->conn_handle);

            // Update LEDs status, and check if we should be looking for more peripherals to connect to.
            bsp_board_led_on(CENTRAL_CONNECTED_LED);
            if (ble_conn_state_central_conn_count() == NRF_SDH_BLE_CENTRAL_LINK_COUNT)
            {
                bsp_board_led_off(CENTRAL_SCANNING_LED);
            }
            else
            {
                // Resume scanning.
                bsp_board_led_on(CENTRAL_SCANNING_LED);
                scan_start();
            }
        } break; // BLE_GAP_EVT_CONNECTED

        // Upon disconnection, reset the connection handle of the peer which disconnected,
        // update the LEDs status and start scanning again.
        case BLE_GAP_EVT_DISCONNECTED:
        {
            if (p_gap_evt->conn_handle == m_conn_handle_hrs_c)
            {
                NRF_LOG_INFO("HRS central disconnected (reason: %d) \n",
                             p_gap_evt->params.disconnected.reason);

                m_conn_handle_hrs_c = BLE_CONN_HANDLE_INVALID;
            }
            if (p_gap_evt->conn_handle == m_conn_handle_rscs_c)
            {
                NRF_LOG_INFO("RSC central disconnected (reason: %d) \n",
                             p_gap_evt->params.disconnected.reason);

                m_conn_handle_rscs_c = BLE_CONN_HANDLE_INVALID;
            }

            if (   (m_conn_handle_rscs_c == BLE_CONN_HANDLE_INVALID)
                || (m_conn_handle_hrs_c  == BLE_CONN_HANDLE_INVALID))
            {
                // Start scanning
                scan_start();

                // Update LEDs status.
                bsp_board_led_on(CENTRAL_SCANNING_LED);
            }

            if (ble_conn_state_central_conn_count() == 0)
            {
                bsp_board_led_off(CENTRAL_CONNECTED_LED);
            }
        } break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_ADV_REPORT:
        {
            on_adv_report(&p_gap_evt->params.adv_report);
        } break; // BLE_GAP_ADV_REPORT

        case BLE_GAP_EVT_TIMEOUT:
        {
            // We have not specified a timeout for scanning, so only connection attemps can timeout.
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                NRF_LOG_INFO("Connection Request timed out. \n");
            }
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        {
            // Accept parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                        &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request. ");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief   Function for handling BLE events from peripheral applications.
 * @details Updates the status LEDs used to report the activity of the peripheral applications.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_peripheral_evt(ble_evt_t const * p_ble_evt)
{
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Peripheral connected \n");
            bsp_board_led_off(PERIPHERAL_ADVERTISING_LED);
            bsp_board_led_on(PERIPHERAL_CONNECTED_LED);

            // Assing connection handle to the QWR module.
            multi_qwr_conn_handle_assign(p_ble_evt->evt.gap_evt.conn_handle);

            // Read=true;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Peripheral disconnected\n");
            bsp_board_led_off(PERIPHERAL_CONNECTED_LED);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling advertising events.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
        {
            NRF_LOG_INFO("Fast advertising. \n");
            bsp_board_led_on(PERIPHERAL_ADVERTISING_LED);
        } break;

        case BLE_ADV_EVT_IDLE:
        {
            ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
        } break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for checking if a bluetooth stack event is an advertising timeout.
 *
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static bool ble_evt_is_advertising_timeout(ble_evt_t const * p_ble_evt)
{
    return (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_SET_TERMINATED);
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    uint16_t role        = ble_conn_state_role(conn_handle);

    // Based on the role this device plays in the connection, dispatch to the right handler.
    if (role == BLE_GAP_ROLE_PERIPH || ble_evt_is_advertising_timeout(p_ble_evt))
    {
        ble_hrs_on_ble_evt(p_ble_evt, &m_hrs);
        ble_rscs_on_ble_evt(p_ble_evt, &m_rscs);
        on_ble_peripheral_evt(p_ble_evt);
    }
    else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
    {
        ble_hrs_c_on_ble_evt(p_ble_evt, &m_hrs_c);
        ble_rscs_c_on_ble_evt(p_ble_evt, &m_rscs_c);
        on_ble_central_evt(p_ble_evt);
    }
}


/**@brief Heart rate collector initialization.
 */
static void hrs_c_init(void)
{
    ret_code_t       err_code;
    ble_hrs_c_init_t hrs_c_init_obj;

    hrs_c_init_obj.evt_handler = hrs_c_evt_handler;

    err_code = ble_hrs_c_init(&m_hrs_c, &hrs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief RSC collector initialization.
 */
static void rscs_c_init(void)
{
    ret_code_t        err_code;
    ble_rscs_c_init_t rscs_c_init_obj;

    rscs_c_init_obj.evt_handler = rscs_c_evt_handler;

    err_code = ble_rscs_c_init(&m_rscs_c, &rscs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!\n");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to
 *                            wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
    // Start our timer
    app_timer_start(m_our_char_timer_id, OUR_CHAR_TIMER_INTERVAL, NULL);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONNECTION_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONNECTION_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = SUPERVISION_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_CONN_HANDLE_INVALID; // Start upon connection.
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;  // Ignore events.
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_hrs_on_db_disc_evt(&m_hrs_c, p_evt);
    ble_rscs_on_db_disc_evt(&m_rscs_c, p_evt);
}


/**
 * @brief Database discovery initialization.
 */
static void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    ble_hrs_init_t     hrs_init;
    ble_rscs_init_t    rscs_init;
    nrf_ble_qwr_init_t qwr_init = {0};
    uint8_t            body_sensor_location;

    // Initialize Queued Write Module instances.
    qwr_init.error_handler = nrf_qwr_error_handler;

    for (uint32_t i = 0; i < NRF_SDH_BLE_TOTAL_LINK_COUNT; i++)
    {
        err_code = nrf_ble_qwr_init(&m_qwr[i], &qwr_init);
        APP_ERROR_CHECK(err_code);
    }

    // Initialize the Heart Rate Service.
    body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

    memset(&hrs_init, 0, sizeof(hrs_init));

    hrs_init.evt_handler                 = NULL;
    hrs_init.is_sensor_contact_supported = true;
    hrs_init.p_body_sensor_location      = &body_sensor_location;

    // Here the sec level for the Heart Rate Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.write_perm);

    err_code = ble_hrs_init(&m_hrs, &hrs_init);
    APP_ERROR_CHECK(err_code);

    // // Initialize the Running Speed and Cadence Service.
    // memset(&rscs_init, 0, sizeof(rscs_init));

    // rscs_init.evt_handler = NULL;
    // rscs_init.feature     = BLE_RSCS_FEATURE_INSTANT_STRIDE_LEN_BIT |
    //                         BLE_RSCS_FEATURE_WALKING_OR_RUNNING_STATUS_BIT;

    // // Here the sec level for the Running Speed and Cadence Service can be changed/increased.
    // BLE_GAP_CONN_SEC_MODE_SET_OPEN(&rscs_init.rsc_meas_attr_md.cccd_write_perm);
    // BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_meas_attr_md.read_perm);
    // BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_meas_attr_md.write_perm);

    // BLE_GAP_CONN_SEC_MODE_SET_OPEN(&rscs_init.rsc_feature_attr_md.read_perm);
    // BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_feature_attr_md.write_perm);

    // err_code = ble_rscs_init(&m_rscs, &rscs_init);
    // APP_ERROR_CHECK(err_code);
//ourservice init
    //our_service_init_1(&m_our_service);

}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);



    NRF_LOG_DEFAULT_BACKENDS_INIT();

}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop). If there is no pending log operation,
          then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for initializing the timer.
 */
static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);


    // Initiate our timers
    app_timer_create(&m_our_char_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
}

void print_current_time()
{
   // printf("Uncalibrated time:\t%s\r\n", nrf_cal_get_time_string(false));
    printf("Calibrated time:\t%s\r\n", nrf_cal_get_time_string(true));

    // printing();

    //printf("liyum : %s\n",nrf_cal_get_time_string(true));


}

void calendar_updated()
{
    if(run_time_updates)
    {
        print_current_time();
    }
}

void set_timing()
{

    uint32_t year, month, day, hour, minute, second;

   // NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
   // NRF_CLOCK->TASKS_HFCLKSTART = 1;
   // while(NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);


    run_time_updates = false;

    year = 2020;
    month = 12;
    day = 27;
    hour =23;
    minute = 59;
    second = 50;

    nrf_cal_set_time(year, month, day, hour, minute, second);

    NRF_LOG_INFO("Time set: ");
    NRF_LOG_INFO("%s", nrf_cal_get_time_string(false));
    NRF_LOG_INFO("\r\n\n");

}

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;



    // Initialize.
    log_init();

    nrf_cal_init();
    nrf_cal_set_callback(calendar_updated, 4);

    timer_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    conn_params_init();
    db_discovery_init();
    peer_manager_init();
    hrs_c_init();
    rscs_c_init();
    services_init();
    advertising_init();
    fds_test_init();
    application_timers_start();

    set_timing();


    // Start execution.
    NRF_LOG_INFO("Relay example started.\n");

    if (erase_bonds == true)
    {
        // Scanning and advertising is done upon PM_EVT_PEERS_DELETE_SUCCEEDED event.
        delete_bonds();
    }
    else
    {
        adv_scan_start();
    }

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}
