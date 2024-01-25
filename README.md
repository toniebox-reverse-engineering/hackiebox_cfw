# hackiebox_cfw
Custom firmware for the Toniebox WIP

**This firmware is dropped for favour of the new [HackieboxNG Custom Firmware project](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng)**

***

## State of development
The custom bootloader is fully functional. The hackiebox custom firmware itself isn't ready as daily driver yet. 
### Implemented features:
- Custom (sd) bootloader
- Hardware drivers (WiFi, AMP/DAC, RFID reader, accelleration sensor, battery/charger, SD, LEDs)
- Webinterface with basic CLI and file upload/download
- JSON configuration file 
- Battery stamina test with WAV-playback with csv log
- Tonie file header decoder
- WAV player (with cli + by wav file in /rCONTENT/<UIDREV>/)
### Todo
- [Configuration of the WiFi credentials via a WiFi Hotspot](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/issues/10)
- [Optimize "Threading" to play WAV without stuttering](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/issues/14)
- [OPUS decoding / playing tonie files](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/issues/12)
- MP3 decoding (or other formats)
- Remote WiFi speaker (chromecast or similar)
- Custom (sd) bootloader that allows context sensitive firmware image switching for more complex features that don't fit into a single image
- [Bootloader: Allow dynamic target address to allow direct starting of the OFW.](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng)
- [Bootloader: Dynamic patching of loaded firmwares by an addtitional file.](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng)
- [Overcome limited include path of Energia](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/issues/11)
- [Set up buildchain without Energia / micropython dependencies](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/issues/13)
### Known bugs
- WiFi configuration is cleared and replaced with the credentials from the json config## Installation
### Preface
It is recommended to have a second copy of the cfw to be able to load the working image and update a broken cfw image over your backup cfw.
### Copy to sd
First of all you need to create "/revvox/web" on your sd card (subdir revvox should be already there if you have successfully installed the sd bootloader) and copy over the content of the /web/ directory of this repository. The same applies to the "/revvox/audio" directory for WAV-playback during the battery test. In addition you have to copy your cfw image to your selected slot(s) on the sd card. (ex. "/revvox/boot/ng-cfw1.bin")
### First boot
Reinsert the sd card and run the cfw once. Then shutdown the box again (put the box onto the front where the speaker/tonie logo is and press both ears for 10s). Then remove the sd card again and add your wifi credentials to the created "/revvox/hackiebox.config.json" config file. 
## Firmware updates
To update the firmware ota you can just use the hackiebox website und the box' ip address. (Expert->File Upload).
## Additional information for developers
Keep in mind that connecting the RX Pin of the box to a serial interface blocks the big ear. For reading the log messages it is enough to have the TX Pin only connected (beside GND). 
In general there is something wrong with the firmware. If you start to refactor the firmware or make bigger changes, it may crash on startup. The reason is currently unknown. Could be a linker problem or some kind of overflow.

## Compiling
### Preface
Building works on Windows, macOS and Linux.
You should prepare the toniebox with the [sd bootloader](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/wiki/Custom-bootloader) to load the cfw from your sd card.
### Prerequisite
#### Energia
Please install the latest [Energia release](https://energia.nu/download/) (1.8.10E23 tested).
#### CC3200 board library
In addition you need to install the cc3200 board library. 
#### Toniebox board library
Replace the cc3200 board library files with the [toniebox-cc3200-core](https://github.com/toniebox-reverse-engineering/toniebox-cc3200-core) and restart energia.
The folder with the boards packages are located at:
##### Windows
`%LOCALAPPDATA%\Energia15\packages\hardware\`
##### Linux
`~/.energia15/packages/energia/hardware/`
##### macOS
`~/Library/Energia15/packages/hardware/`
#### Additional libraries (Install lib via ZIP)
[SimpleCLI](https://github.com/toniebox-reverse-engineering/SimpleCLI)
[ESP8266Audio](https://github.com/toniebox-reverse-engineering/ESP8266Audio)
[ESP8266SAM](https://github.com/toniebox-reverse-engineering/ESP8266SAM)
### Build
Open hackiebox_cfw.ino with energia and build the cfw. Remember the path where the hackiebox.ino.bin is saved. Usally you find it at `C:\Users\<User>\AppData\Local\Temp\arduino_build_XXXXXX`.

