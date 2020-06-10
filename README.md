# hackiebox_cfw
Custom firmware for the Toniebox WIP

## Compiling
### Preface
Currently building only works with Windows. Linux doesn't work on a x64 only system.
You should prepare the toniebox with the [sd bootloader](https://github.com/toniebox-reverse-engineering/hackiebox_cfw/wiki/Custom-bootloader) to load the cfw from your sd card.
### Prerequisite
#### Energia
Please install the latest [Energia release](https://energia.nu/download/) (1.8.10E23 tested).
#### CC3200 board library
In addition you need to install the cc3200 board library. 
#### Toniebox board library
Replace the cc3200 board library files with the [toniebox-cc3200-core](https://github.com/toniebox-reverse-engineering/toniebox-cc3200-core) and restart energia.
### Build
Open hackiebox_cfw.ino with energia and build the cfw. Remember the path where the hackiebox.ino.bin is saved. Usally you find it at `C:\Users\<User>\AppData\Local\Temp\arduino_build_XXXXXX`.
## Installation
### Preface
It is recommended to have a second copy of the cfw to be able to load the working image and update a broken cfw image over your backup cfw.
### Copy to sd
First of all you need to create "/revvox/web" on your sd card (subdir revvox should be already there if you have successfully installed the sd bootloader) and copy over the content of the /web/ directory of this repository. In addition you have to copy your cfw image to your selected slot(s) on the sd card. (ex. "/revvox/boot/pre-img1.bin")
### First boot
Reinsert the sd card and run the cfw once. Then shutdown the box again (put the box onto the front where the speaker/tonie logo is and press both ears for 10s). Then remove the sd card again and add your wifi credentials to the created "/revvox/hackiebox.config.json" config file. 
## Firmware updates
To update the firmware ota you can just use the hackiebox website und the box' ip address. (Expert->File Upload).
## Additional information for developers
Keep in mind that connecting the RX Pin of the box to a serial interface blocks the big ear. For reading the log messages it is enough to have the TX Pin only connected (beside GND).
