# STOP-COVID19 IoT device.


This example is meant to be used togheter with the examples of the NORDIC SDK15. 
and modified to make it well and customizebl to our needs

Why using beacons?
- No gps location tracking, the main factor in citizen acceptance and the success of covid-19 contact tracing.
- RSSI (Received Signal Strength Indicator) is a measure specific to each manufacturer and therefore not standard.
- Radio waves, such as absorption, interference or diffraction, strongly influence the RSSI signal.
- In Singapore as an example, only 1/7 citizen had installed the TraceTogether mobile application.
- No smartphone or mobile data are required to track contacts tracing.
- Beacons battery duration can more than 45 days.

## Features
- Both Central and peripherl at the same time, which means the beacon can be both a server and client.
- Central:
	- Scan for the STOP-COVID19 nearby devices. 1 meter - 1.5 meters. to be adjusted.
	- Save the Mac Adresse of the specific UUID after 10 minutes to 15 minutes of 2 beacons contact.
	- Save to Flash memory. conditions 1 and 2 are required.

- Peripheral:
	- Advertise the UUID and service
	- Get connected and read the flash memory
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
* nRF Connect Bluetooth low energy app for Desktop. 
You can also use nRF Connect for mobile found in both Google Play on Android and App Store for iPhone.
* Keil uVision v5.xx or SEGGER Embedded Studio (SES)
* SDK v15.0.0.
* SoftDevice S132 V6.x.x
* Example files.

Other kits, dongles and software versions might work as well, but this will not be covered here.

To compile it download the project files and copy the folder "Custom_beacon_date" to "your_SDK_folder/examples/ble_central_and_peripheral/experimental/Custom_beacon_date".

Please post any questions about this project on https://devzone.nordicsemi.com/questions/.

## About this project
This application is one of several applications that has been built by the support team at Nordic Semiconductor, and our team Casky.io

for any technical information, please do contact: ao@casky.io
for business information, partnership or support please do contact: ak@casky.io


##### Version 0.7.0
