# SimpleFlashRom
An ANSI terminal based flash programmer, use to program flash memories used as ROMs in old computers.

## Description
For many people is fun to experiment with old computers and explore possibilities that were impratical in the past days.

One of these is rom upgrade, an example is the [Atari ST](https://en.wikipedia.org/wiki/Atari_ST).

This computer has roms chips installed through sockets and is possibile to easily exchange with different versions of [TOS](https://en.wikipedia.org/wiki/Atari_TOS).

In the process of these experiments I developed this little tool to program FLASH chips that in modern days replace the old write once ROM chips.

I used the [SST39SF040](https://www.microchip.com/en-us/product/SST39SF040), a 4Mbit Flash rom, these chips operates with 5V and are compatible with old computers

## Use 
The use should be straightforward: copy your rom files in a sd card , insert a flash chip onto breadboard and connect the board to computer.

An ANSI terminal is used as user interface:
<img width="946" height="584" alt="screen_01" src="https://github.com/user-attachments/assets/f551b4f4-3e9d-466a-9618-f34d869d15d4" />

Through Ui you can read/program/verifiy flash chips and also splitting memory in odd/even sections (as used by Atari).
* A progress bar show execution for long commands (Programming entire chip requires some minutes)
* Setting area shows flash and sd info 
* Map area indcates how chip memory is used (max 4 partitions can be programmed into F040 chip)
* To execute a command press key + return and see results into log section.
  
---
### Main commands
* __h/H__: show help on screen
* __i__: read chip information (to determine chip size)
* __c__: reset chip (not delete data)
* __e__: erase chip. delete all data
* __z__: reset sd card
* __I__: read sd card info
* __f__: scan sd card for rom files (.bin, .img)
* __m__: set programming mode: 0 = all bytes, 1 = odd bytes, 2 = even bytes
* __P__: program entire chip
* __V__: verify chip after programming
  
### other commands
* __d__ , address: dump chips on screen
* __D__ , address: dump rom file on screen
* __s__ , partition , rom_index: store selected rom in selected partition
* __T__: simulate programming chip
* __S__: save chip data to rom file (file is named "flash_<partition>.img" inside rom path

### low level/test commands
* __p__, address, data: program flash location with data
* __w__, address, write byte at address
* __r__, address:  read byte from address
* __o__
* 
---
## Bill of materials
* Arduino Mega
* SD flash reader
* A small sd card
* 8x 10/12k resistors
* Breadboard
* many wires!

In my case I used a board that I already had similar to Arduino Mega a [Fishino Mega](https://www.fishino.it/fishino-mega.html)
This board includes a mini sd card reader besides other things. Functionally is equivalent to a plain Mega so it's not necessary for this project.

## Schematic
I used a breadboard and many wires to connect board to flash chip that i simply inserted into breadboard 

<img width="946" height="710" alt="breadboard" src="https://github.com/user-attachments/assets/a9cdcde7-ae2f-4b8c-a9be-9e271c26bedd" />

This is the schematic:

<img width="946" height="770" alt="SimpleFlashRom schematic" src="https://github.com/user-attachments/assets/87ceb7d7-d549-4783-a59f-2e90f1ce3203" />

Pullup resistors are used only for data bus (D0/7).

## Software
The project is compiled with Arduino IDE (any recent version should work).  
If you want to use Fishino board you have to install specific board variant,  
instructions could be found at [Fishino website](https://www.fishino.it/arduino-ide-packages.html).
You also need specific library for internal SD card reader see [here](https://www.fishino.it/download-libraries.html)
If you use normal Mega + external card sd standard libraries should work (not tested by me).

## Installation
* Download project or clone it, load into arduino ide and compile as usual.
* Copy your rom files into sd with specific path (default is /rom).
* Insert card into reader and power up board
* After connection you could connect through serial port with a terminal software compatible with ANSI codes (putty or similar)

##  Example of use
With a sd inserted with 4 files saved in /rom folder use these steps to program flash chip.
Arduino is connected to PC and terminal as putty is connected on serial port with parameters 115200 1N8
All commands are followed by <return>
1. press return to show main screen
2. press __i__ to have flash info. If flash is connected correctly screen should show chip brand and type
3. press __f__ to scan rom files into sd
4. press __m__ to select program mode (default = all bytes, 1 = odd bytes, 2 even bytes)
5. if flash chip is not empty press __e__ to erase the chip
6. press __P__ to program entire chip (several minutes to finish program)
7. press __V__ to verify chip (if any location differ from corrispondent byte, log will show in red)
8. if you need differentiation for odd/even bytes, repeat steps 4-7 with another chip

9. enjoy your brand new programmed flash chips !







