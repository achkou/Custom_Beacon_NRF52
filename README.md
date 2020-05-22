# CASKY Application
This example is meant to be used togheter with the examples of the NORDIC SDK15. and modified to make it well and customizebl to our needs

## Features
- Both Central and peripherl at the same time
- Central:
	- Scan for the STOP_COVID19
	- Save the Mac Adresse of the specific UUID
	- Save to Flash memory

- Peripheral:
	- Advertise the UUID and service
	- get connected and read the flash memory
- Connecting to the peripheral:
	- Reading the FLash memory via BLE with a specefic frequency

- Specefication of Saving the Mac:
	- Wait for a specefic periode
	- Near to the device  with specefic distance
- Date counter:
	- a year counter of the date
	- to Add it the MAc adresse

## Requirements

* nRF52832
* Segger Dongle  jlink
* nRF Connect Bluetooth low energy app for Desktop. You can also use nRF Connect for mobile found in both Google Play on Android and App Store for iPhone.
* Keil uVision v5.xx or SEGGER Embedded Studio (SES)
* SDK v15.0.0.
* SoftDevice S132 V6.x.x
* Example files.

Other kits, dongles and software versions might work as well, but this will not be covered here.

To compile it download the project files and copy the folder "Custom_beacon_date" to "your_SDK_folder/examples/ble_central_and_peripheral/experimental/Custom_beacon_date".

Please post any questions about this project on https://devzone.nordicsemi.com/questions/.

## About this project
This application is one of several applications that has been built by the support team at Nordic Semiconductor, and our team Casky.io

for more information contact: ao@casky.io


##### Version 0.7.0