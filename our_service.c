
#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "our_service.h"
#include "ble_srv_common.h"
#include "app_error.h"

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_our_service       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

int status=0;
int* getstatus(){
    return &status;
}
int speed=0;
int* getspeed(){
    return &speed;
}

int timer=10;
int* gettimer(){
    return &timer;
}
static void on_write(ble_os_t * p_our_service, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    // Custom Value Characteristic Written to.
    if (p_evt_write->handle == p_our_service->char_handles_led.value_handle)
    {



            switch (*p_evt_write->data)
            {
                case 0x01: //turn OFF ALL of modes

                    status=-1;

                    break;
                case 0x11: //LEFT SIGNALS

                   // left_led(1);
                    //Double_tape_vibration();
                    break;
                case 0x21: //RIGHT SIGNALS

                    //right_led(1);
                    // Double_tape_vibration();

                    break;
                case 0x31: //BRAKE SIGNALS

                    //brake_ble(1);
                    break;

                /*******MOBILE*******/
                case 0x02: //turnoff

                    status=-1;
                    break;
                case 0x12: //LEFT SIGNALS

                    status=4;
                    // Long_vibration();
                    break;
                case 0x22: //RIGHT SIGNALS

                    status=5;
                    // Long_vibration();

                    break;
                case 0x32: //enable WARNING
                    status=2;
                    // Triple_tape_vibration();
                    break;
                case 0x42: //enable WARNING
                    status=1;
                    // Triple_tape_vibration();
                    break;

                case 0x52: //disable WARNING

                    status=-1;
                    // Triple_tape_vibration();
                    break;
                case 0x62: //notification

                    // Double_tape_vibration();
                    break;

                case 0x72: //notification

                    // Triple_tape_vibration();
                    break;
                case 0x82: //notification

                if(timer >=1)
                    timer += 1;

                    break;

                case 0x92: //notification

                if(timer >=1)
                    timer -= 1;

                    break;

                case 0x99: //turnOFF
                    status=-1;
                    // Off_vibration();
                    sd_power_system_off();
                    break;

            }

    }

    if (p_evt_write->handle == p_our_service->char_handles_ax.value_handle)
    {
        // printf("data:%d\n", *p_evt_write->data);
        speed = *p_evt_write->data;

    }

}

// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
  	ble_os_t * p_our_service =(ble_os_t *) p_context;
		// OUR_JOB: Step 3.D Implement switch case handling BLE events related to our service.
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        case BLE_GATTS_EVT_WRITE:
        on_write(p_our_service, p_ble_evt);
        break;

        default:
            // No implementation needed.
            break;
    }

}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
static uint32_t our_char_led(ble_os_t * p_our_service)
{
    // OUR_JOB: Step 2.A, Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid      = BLE_UUID_CHARACTERISTIC_LED;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Step 2.F Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 0;
    char_md.char_props.write = 1;

    // OUR_JOB: Step 3.A, Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 0;

    // OUR_JOB: Step 2.B, Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // OUR_JOB: Step 2.G, Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // OUR_JOB: Step 2.C, Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // OUR_JOB: Step 2.H, Set characteristic length in number of bytes
    attr_char_value.max_len     = 1;
    attr_char_value.init_len    = 1;
    uint8_t value[1]            = {0x12};
    attr_char_value.p_value     = value;

    // OUR_JOB: Step 2.E, Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle_1,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->char_handles_led);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
static uint32_t our_char_mpu(ble_os_t * p_our_service)
{
    // OUR_JOB: Step 2.A, Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid      = BLE_UUID_CHARACTERISTIC;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Step 2.F Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // OUR_JOB: Step 3.A, Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;

    // OUR_JOB: Step 2.B, Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // OUR_JOB: Step 2.G, Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // OUR_JOB: Step 2.C, Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // OUR_JOB: Step 2.H, Set characteristic length in number of bytes
    attr_char_value.max_len     = 9;
    attr_char_value.init_len    = 9;
    uint8_t value[9];
    memset(value,0,sizeof(value));
    attr_char_value.p_value     = value;

    // OUR_JOB: Step 2.E, Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle_1,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->char_handles_ax);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

static uint32_t our_char_crash(ble_os_t * p_our_service)
{
    // OUR_JOB: Step 2.A, Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid      = BLE_UUID_CHARACTERISTIC_CRASH;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Step 2.F Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // OUR_JOB: Step 3.A, Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;

    // OUR_JOB: Step 2.B, Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // OUR_JOB: Step 2.G, Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // OUR_JOB: Step 2.C, Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // OUR_JOB: Step 2.H, Set characteristic length in number of bytes
    attr_char_value.max_len     = 9;
    attr_char_value.init_len    = 9;
    uint8_t value[9];
    memset(value,0,sizeof(value));
    attr_char_value.p_value     = value;

    // OUR_JOB: Step 2.E, Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle_1,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->char_handles_crash);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *  LED SERVICE
 */
void our_service_init_1(ble_os_t * p_our_service)
{
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    // FROM_SERVICE_TUTORIAL: Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_OUR_BASE_UUID;
    service_uuid.uuid = BLE_UUID_OUR_SERVICE_UUID_1;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Step 3.B, Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    //  Add our service for LED
		err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_our_service->service_handle_1);
    APP_ERROR_CHECK(err_code);

    // OUR_JOB: Call the function our_char_add() to add our new characteristic to the service.
    our_char_led(p_our_service);
    our_char_mpu(p_our_service);
    our_char_crash(p_our_service);


}
void our_led_characteristic_update(ble_os_t *p_our_service, int32_t *led_value)
{
    // OUR_JOB: Step 3.E, Update characteristic value
        if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {

        uint16_t               len = 2;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->char_handles_led.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)led_value;

        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);

    }

}

void our_characteristic_update(ble_os_t *p_our_service, int32_t *ax_value, uint16_t len)
{
    // OUR_JOB: Step 3.E, Update characteristic value
        if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {


        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->char_handles_ax.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)ax_value;

        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);

    }

}

void our_crash_update(ble_os_t *p_our_service, int32_t *crash_value, uint16_t len)
{
    // OUR_JOB: Step 3.E, Update characteristic value
        if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {


        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->char_handles_crash.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)crash_value;

        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);

    }

}

