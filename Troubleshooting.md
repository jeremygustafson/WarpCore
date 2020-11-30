# Warp Core ESP-IDF Troubleshooting Notes

These are some troubleshooting notes I kept over several months battling the ESP-IDF and trying to compile the warp core code. If you run into issues, hopefully something in here will be helpful.

**As a general rule:** when in doubt, `make clean` followed by `make flash` again.

If that doesn't work, **as a larger hammer try:**
```
make erase_flash
make partition_table # Then copy/paste the command it gives
make flashfs
make flash monitor -j32
```




----------------------------------------
### Issue: First attempt at `make flash monitor` gives this error:
```
~/esp/warp_core/main/WebServer.cpp:11:16: fatal error: FS.h: No such file or directory
```
----------------------------------------

**To resolve:**

Download and install the [Arduino IDE](https://www.arduino.cc/en/software), as we’ll need to borrow some libraries from it.

Follow steps 1-5 from this article: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/ , also included below:

In the Arduino IDE, open Preferences.

Under “Additional Boards Manager URLs:” add “https://dl.espressif.com/dl/package_esp32_index.json”. Click OK.

Click on Tools > Board > “Boards Manager…”. Search for “esp32” and install version 1.0.4 from Espressif Systems.

Back in your terminal:
```
cp -a ~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/libraries/FS ~/esp/warp_core/components/arduino/libraries
```

Make sure this file is present: `~/esp/warp_core/components/arduino/component.mk`





----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
~/esp/warp_core/components/arduino/libraries/AsyncTCP-master/src/AsyncTCP.cpp:22:21: fatal error: Arduino.h: No such file or directory
```
----------------------------------------

**To resolve:**

```
mkdir ~/esp/warp_core/components/arduino/cores
cp -a ~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/cores/esp32 ~/esp/warp_core/components/arduino/cores
```




----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
In file included from ~/esp/warp_core/components/arduino/cores/esp32/wiring_private.h:31:0,
                 from ~/esp/warp_core/components/arduino/cores/esp32/wiring_pulse.c:18:
~/esp/warp_core/components/arduino/cores/esp32/Arduino.h:185:26: fatal error: pins_arduino.h: No such file or directory
```
----------------------------------------

**To resolve:**

```
mkdir ~/esp/warp_core/components/arduino/variants
cp -a ~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/variants/esp32 ~/esp/warp_core/components/arduino/variants
```




----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
In file included from ~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncEventSource.cpp:21:0:
~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncEventSource.h:28:25: fatal error: ESPAsyncTCP.h: No such file or directory
```
----------------------------------------

**To resolve:**

`vi ~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncEventSource.h`

```
#include <Arduino.h>
#define ESP32 // ADD THIS LINE
#ifdef ESP32
```

NOTE: The above step isn’t actually needed after everything else is done. Documenting here "just in case".





----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
In file included from ~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncEventSource.h:32:0,
                 from ~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncEventSource.cpp:21:
~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/ESPAsyncWebServer.h:32:18: fatal error: WiFi.h: No such file or directory
```
----------------------------------------

**To resolve:**

```
cp -a ~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/libraries/WiFi ~/esp/warp_core/components/arduino/libraries
```





----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
In file included from ~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncWebSocket.cpp:22:0:
~/esp/warp_core/components/arduino/libraries/ESPAsyncWebServer/src/AsyncWebSocket.h:29:25: fatal error: ESPAsyncTCP.h: No such file or directory
```
----------------------------------------

**To resolve:**

I think I did this:
```
cd ~/esp/warp_core/components/
git clone --recursive https://github.com/espressif/arduino-esp32.git
cd ~/esp/warp_core/components/arduino-esp32/
git checkout 4d98cea085d619bed7026b37071bd8402a485d95
(As of this writing, the July 18, 2020 revision, commit 4d98cea085d619bed7026b37071bd8402a485d95)
```

And then the earlier-mentioned steps in the README that modify files in that directory.





----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
~/esp/warp_core/components/arduino-esp32/libraries/AzureIoT/src/az_iot/iothub_client/src/iothubtransport_mqtt_common.c:1058:32: error: 'request_id' may be used uninitialized in this function [-Werror=maybe-uninitialized]
                             if (request_id == msg_entry->packet_id)
```
----------------------------------------

**To resolve:**

```
rm -r ~/esp/warp_core/components/arduino/libraries/AzureIoT/*
make clean
make flash monitor
```




----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
In file included from ~/esp/esp-idf/components/wifi_provisioning/include/wifi_provisioning/scheme_ble.h:18:0,
                 from ~/esp/warp_core/components/arduino/libraries/WiFi/src/WiFiProv.h:21,
                 from ~/esp/warp_core/components/arduino/libraries/WiFi/src/WiFi.h:40,
                 from ~/esp/warp_core/components/arduino/libraries/ArduinoOTA/src/ArduinoOTA.h:4,
                 from ~/esp/warp_core/components/arduino/libraries/ArduinoOTA/src/ArduinoOTA.cpp:6:
~/esp/esp-idf/components/protocomm/include/transports/protocomm_ble.h:17:29: fatal error: esp_gap_ble_api.h: No such file or directory
```
----------------------------------------

**To resolve:**

`vi ~/esp/esp-idf/components/protocomm/include/transports/protocomm_ble.h`

```
// KLUDGE: COMMENT THIS LINE OUT: #include <esp_gap_ble_api.h>
// KLUDGE: ADD THE FOLLOWING LINE
#include "../../../arduino-esp32/tools/sdk/include/bt/esp_gap_ble_api.h" // KLUDGE
```





----------------------------------------
### Issue: Next attempt at `make flash monitor` gives this error:
```
~/esp/warp_core/components/arduino-esp32/libraries/WiFi/src/ETH.cpp:196:37: error: conversion from 'const ip_addr_t* {aka const ip_addr*}' to non-scalar type 'ip_addr_t {aka ip_addr}' requested
     ip_addr_t dns_ip = dns_getserver(dns_no);
```
----------------------------------------

**To resolve:**

Manually add this fix I found from https://github.com/espressif/arduino-esp32/issues/3760 (which itself was linked from https://github.com/espressif/esp32-arduino-lib-builder/issues/10) :

`vi ~/esp/warp_core/components/arduino-esp32/libraries/WiFi/src/ETH.cpp`

```(line 196)
    //    ip_addr_t dns_ip = dns_getserver(dns_no); // KLUDGE
    //    return IPAddress(dns_ip.u_addr.ip4.addr); // KLUDGE
    const ip_addr_t * dns_ip = dns_getserver(dns_no); // KLUDGE
    return IPAddress(dns_ip->u_addr.ip4.addr); // KLUDGE
```

Make the same change in this file, too:

`vi ~/esp/warp_core/components/arduino-esp32/libraries/WiFi/src/WiFiSTA.cpp`

```(line 491)
    //    ip_addr_t dns_ip = dns_getserver(dns_no); // KLUDGE
    //    return IPAddress(dns_ip.u_addr.ip4.addr); // KLUDGE
    const ip_addr_t * dns_ip = dns_getserver(dns_no); // KLUDGE
    return IPAddress(dns_ip->u_addr.ip4.addr); // KLUDGE
```




----------------------------------------
### Issue:
```
~/esp/warp_core/components/arduino-esp32/libraries/WiFiClientSecure/src/ssl_client.cpp:23:4: error: #error "Please configure IDF framework to include mbedTLS -> Enable pre-shared-key ciphersuites and activate at least one cipher"
 #  error "Please configure IDF framework to include mbedTLS -> Enable pre-shared-key ciphersuites and activate at least one cipher"
    ^
~/esp/warp_core/components/arduino-esp32/libraries/WiFiClientSecure/src/ssl_client.cpp: In function 'int start_ssl_client(sslclient_context*, const char*, uint32_t, int, const char*, const char*, const char*, const char*, const char*)':
~/esp/warp_core/components/arduino-esp32/libraries/WiFiClientSecure/src/ssl_client.cpp:152:67: error: 'mbedtls_ssl_conf_psk' was not declared in this scope
                  (const unsigned char *)pskIdent, strlen(pskIdent));
```
----------------------------------------

**To resolve:**

See also https://github.com/espressif/arduino-esp32/issues/3457
```
make menuconfig
Component config -> mbedTLS -> TLS Key Exchange Methods -> [*] Enable pre-shared-key ciphersuites
```





----------------------------------------
### Issue: After `make flashfs` and `make flash monitor`, the warp_core web page GUI doesn't load in web browser
----------------------------------------

**To resolve:**

This seemed to be caused by missing an extra flag when compiling mkspiffs. Be sure to use this command line:

`make BUILD_CONFIG_NAME="-esp-idf" CPPFLAGS="-DSPIFFS_OBJ_META_LEN=4"`





--------------------
### Issue: When trying to flash, you get this (or similar) error:
```
[Errno 16] could not open port /dev/tty.SLAB_USBtoUART
```
--------------------

**To resolve:**

Reboot your computer. I couldn't find any other way around it.





--------------------
### Issue: When trying to flash, you get this error:
```
A fatal error occurred: Timed out waiting for packet header
make: *** [flash] Error 2
```
--------------------

**To resolve:**

Try the `make flash monitor` again. If it fails again, you might need to use the "big hammer" mentioned at the top of this document.





--------------------
### Issue: When doing a `make flash`, you see an error scroll by like below, and the compile aborts before flashing the ESP chip.
```
make[1]: *** No rule to make target `cores/esp32/libb64/cencode.o', needed by `libarduino-esp32.a'.  Stop.
make[1]: *** Waiting for unfinished jobs....
make: *** [component-arduino-esp32-build] Error 2
make: *** Waiting for unfinished jobs....
```
--------------------

**To resolve:**

The specific example copied above appeared to be a weird race condition due to the `-j32` flag, and I wasn't able to reproduce it. The solution was simply run another `make flash monitor -j32`. In general, I just keep running that command until the only output remaining is obviously error output. If it ends up succeeding with no errors, great.





--------------------
### Issue: ESP monitoring shows this error:
```
E (586) SPIFFS: mount failed, -10025
[E][SPIFFS.cpp:89] begin(): Mounting SPIFFS failed! Error: -1
[E][vfs_api.cpp:22] open(): File system is not mounted
```
--------------------

**To resolve:**

Assuming this is happening with the warp_core_audio code:

`vi ~/esp/warp_core_audio/main/main.cpp`

Uncomment the section that says "Uncomment this section for debug:"

Save, then also double check this value is 4096: `make menuconfig` > "SPIffs Example Configuration  --->" > "SPIFFS Logical block size" > 4096

`make flashfs monitor`





--------------------
### Issue: How to unpack a spiffs image locally
--------------------

**To resolve:**

```
~/esp/warp_core_audio $ ./mkspiffs -u ~/spiffs_unpacked_dir build/spiffs_image.img
```





--------------------
### Issue: SPIFFS files not showing up, or, getting error "Wav: File too small (riffLen)"
--------------------

**To resolve:**

As a first step, add this code block to the bottom of ~/esp/warp_core_audio/main/AudioHandler.cpp:

```
// For debugging
// Based on https://www.esp32.com/viewtopic.php?t=12557
void listDir(fs::FS &fs, const char * dirname, uint8_t levels=0) {
	ESP_LOGI("Wav","Listing directory: %s\r\n", dirname);

	File root = fs.open(dirname);
	if (!root) {
		ESP_LOGE("Wav","- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		ESP_LOGI("Wav"," - not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			ESP_LOGI("Wav", "DIR : %s",file.name());
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		} else {
			ESP_LOGI("Wav","  FILE: %s",file.name());
			ESP_LOGI("Wav","\tSIZE: %d",file.size());
		}
		file = root.openNextFile();
	}
}
listDir(SPIFFS, "/");
```

Then `make flash monitor` and examine the output.





--------------------
### Issue: ESP monitoring shows this error:
```
:E (3742) Audio: wavFile is NULL. Error code: 2 : No such file or directory
```
--------------------

**To resolve:**

Edit ~/esp/warp_core_audio/main/main.cpp and uncomment this debug section:
```
		// Uncomment this section for debug:
        /*
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while(file){
                ESP_LOGI(MAIN_TAG,"FILE: %s", file.name());
                file = root.openNextFile();
        }
        */
```

Run `make flash monitor -j32` and note the file names that show up, for instance:
```
I (1466) MainLoop: FILE: /wav/breach.wav
I (1533) MainLoop: FILE: /wav/core.wav
I (1548) MainLoop: FILE: /wav/core_max.wav
I (1611) MainLoop: FILE: /wav/redalert.wav
I (1686) MainLoop: FILE: /wav/redalert_vgr.wav
I (1712) MainLoop: Debug: Playing core.wav
E (1877) Audio: wavFile is NULL. Error code: 2 : No such file or directory
```

Then edit the 5 file paths inside `~esp/warp_core_audio/main/AudioHandler.cpp` accordingly to match. I can't explain why, but the first time I got this to work the paths were of the form "/core.wav" and the second time they needed to be "/wav/core.wav".





--------------------
### Issue: ESP monitoring shows this error:
```
E (1621) BT-Audio: initBtAudio enable controller failed: 258,
```
--------------------

**To resolve:**

Error code 258 translates to hex 0x102, which according to this page:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html
is "ESP_ERR_INVALID_ARG (0x102): Invalid argument"

This seems to come from main/BtAudio.c, this line:
```
ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
```

That function, esp_bt_controller_enable, comes from: ~/esp/esp-idf/components/bt/bt.c
This is the relevant code:
```
//As the history reason, mode should be equal to the mode which set in esp_bt_controller_init()
    if (mode != btdm_controller_get_mode()) {
        return ESP_ERR_INVALID_ARG;
    }
```

I think esp_bt_controller_init is getting a different value sent to it. ESP_BT_MODE_BTDM must stand for "Bluetooth Dual Mode", and since it's hard-coded it also needs to be set in `make menuconfig`:

```
Component config  --->
	Bluetooth  --->
		Bluetooth controller  --->
			Bluetooth controller mode (BR/EDR/BLE/DUALMODE) --->
				(X) Bluetooth Dual Mode
```