
#ifndef OUR_SERVICE_H__
#define OUR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

//  Defining 16-bit service and 128-bit base UUIDs
#define BLE_UUID_OUR_BASE_UUID              {{0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}}// 128-bit base UUID
#define BLE_UUID_OUR_SERVICE_UUID_1                0xCA11 //  Main service
//#define BLE_UUID_OUR_SERVICE_UUID_2                0xCA22 // in the futur

//  Defining 16-bit characteristic UUID
#define BLE_UUID_CHARACTERISTIC_LED           		 0xCA12 // LED Just a random, but recognizable value
#define BLE_UUID_CHARACTERISTIC 				 0xCA13 // AX Mpu6050 Accelerometer
#define BLE_UUID_CHARACTERISTIC_CRASH				 0xCA14 // AX Mpu6050 Accelerometer
// #define BLE_UUID_CHARACTERISTIC_A_Y 				 0xCAB1 // AY Mpu6050 Accelerometer
// #define BLE_UUID_CHARACTERISTIC_A_Z 				 0xCAC1 // AZ Mpu6050 Accelerometer
// #define BLE_UUID_CHARACTERISTIC_G_X 				 0xCA1A // GX Mpu6050 gyroscope
// #define BLE_UUID_CHARACTERISTIC_G_Y 				 0xCA1B // GY Mpu6050 gyroscope
// #define BLE_UUID_CHARACTERISTIC_G_Z 				 0xCA1C // GZ Mpu6050 gyroscope
// #define BLE_UUID_CHARACTERISTIC_F_F 				 0xCAFF // freefall Mpu6050
// #define BLE_UUID_CHARACTERISTIC_B_L 				 0xCABB // Battery level
// #define BLE_UUID_CHARACTERISTIC_P_T 				 0xCADD // Processeur Temperature


// This structure contains various status information for our service.
// The name is based on the naming convention used in Nordics SDKs.
// 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and
// ‘os’ is short for Our Service).
typedef struct
{
    uint16_t                    conn_handle;
    uint16_t                    service_handle_1; //led Service
    // uint16_t                    service_handle_2; //mpu6050 service
    // uint16_t                    service_handle_3; //battery service
    // OUR_JOB: Step 2.D, Add handles for our characteristic

	ble_gatts_char_handles_t    char_handles_led; // LED Just a random, but recognizable value
	ble_gatts_char_handles_t    char_handles_ax; // AX Mpu6050 Accelerometer
	ble_gatts_char_handles_t    char_handles_crash; // AX Mpu6050 Accelerometer
	// ble_gatts_char_handles_t    char_handles_ay; // AY Mpu6050 Accelerometer
	// ble_gatts_char_handles_t    char_handles_az; // AZ Mpu6050 Accelerometer
	// ble_gatts_char_handles_t    char_handles_gx; // GX Mpu6050 gyroscope
	// ble_gatts_char_handles_t    char_handles_gy; // GY Mpu6050 gyroscope
	// ble_gatts_char_handles_t    char_handles_gz; // GZ Mpu6050 gyroscope
	// ble_gatts_char_handles_t    char_handles_ff; // freefall Mpu6050
	// ble_gatts_char_handles_t    char_handles_bl; // Battery level
	// ble_gatts_char_handles_t    char_handles_pt; // Processeur Temperature
}ble_os_t;

/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_our_service       Our Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */

void our_service_init_1(ble_os_t * p_our_service); //led Service
// void our_service_init_2(ble_os_t * p_our_service); //Mpu6050 Service
// void our_service_init_3(ble_os_t * p_our_service); //Battery Service

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever our timer_timeout_handler triggers
 *
 * @param[in]   p_our_service                     Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void our_led_characteristic_update(ble_os_t *p_our_service, int32_t *led_value); //LED Just a random, but recognizable value
void our_characteristic_update(ble_os_t *p_our_service, int32_t *ax_value, uint16_t len); // buffer
void our_crash_update(ble_os_t *p_our_service, int32_t *crash_value, uint16_t len); // buffer

// void our_mpu_ay_characteristic_update(ble_os_t *p_our_service, int32_t *ay_value); // AY Mpu6050 Accelerometer
// void our_mpu_az_characteristic_update(ble_os_t *p_our_service, int32_t *az_value); // AZ Mpu6050 Accelerometer
// void our_mpu_gx_characteristic_update(ble_os_t *p_our_service, int32_t *gx_value); // GX Mpu6050 gyroscope
// void our_mpu_gy_characteristic_update(ble_os_t *p_our_service, int32_t *gy_value); // GY Mpu6050 gyroscope
// void our_mpu_gz_characteristic_update(ble_os_t *p_our_service, int32_t *gz_value); // GZ Mpu6050 gyroscope
// void our_mpu_ff_characteristic_update(ble_os_t *p_our_service, int32_t *ff_value); // freefall Mpu6050
// void our_battery_characteristic_update(ble_os_t *p_our_service, int32_t *bl_value); // Battery level
// void our_temperature_characteristic_update(ble_os_t *p_our_service, int32_t *pt_value); // Processeur Temperature

#endif  /* _ OUR_SERVICE_H__ */
