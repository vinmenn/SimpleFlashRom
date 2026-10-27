/*
 * SimpleFlashRom programmer:
 *
 * History:
 * 27/10/2026: 1.0.0 - Released first public version
 */

#include "globals.h"
#include "lowlevel.h"
#include "flash.h"
#include "rom.h"
#include "ui.h"

Ui ui = Ui();
LowLevel ll = LowLevel();
Flash flash = Flash(ll);
Rom rom = Rom(ROM_PATH);

//Flash settings
uint8_t _mode = 0;   // Programming mode 0: all bytes, 1 = odd bytes, 2 = even bytes
uint8_t _brand = 0;  // Flash chip brand
uint8_t _model = 0;  // Flash chip model
// uint8_t _chip_status = 0xff;    // Chip status 0 empty, 1 programmed,
//                                 //   2 verified, 4 error, 0xff unknown

//Rom settings
uint32_t _size;     // SD file size
uint8_t _sd_type;   // SD type
uint8_t _fat_type;  // SD FAT type

uint32_t _partition_size;       // Current partition size
uint8_t _total_partitions = 0;  // Total number of roms to be programmed

uint8_t _total_roms = 0;             // Found roms after SD scan
uint8_t _rom_index[MAX_PARTITIONS];  // Order of selected roms
String _rom_names[MAX_ROMS];         // Max rom images names (with MAX length for names)

void setup() {
  ui.init();
  ll.init();

  rom.init();

  // Init rom default order
  for (uint8_t i = 0; i < MAX_PARTITIONS; i++)
    _rom_index[i] = i;
}

void loop() {
  char cmd;
  uint32_t address;
  uint8_t data;

  if (ui.checkInput()) {
    ui.getCommand(&cmd, &address, &data);

    switch (cmd) {
      //-----------------------------------------------------------------------------
      // Flash functions
      //-----------------------------------------------------------------------------
      case 'i':  //Show flash info
        flash.identifyChip(&_brand, &_model, &_total_partitions, &_partition_size);
        ui.addLog(LOG_INFO, F("Chip identification done"));
        break;
      case 'c':  //Reset flash chip
        flash.resetChip();
        ui.addLog(LOG_INFO, F("Reset chip done"));
        break;
      case 'e':  //Erase flash chip
        flash.eraseChip();
        flash.resetChip();
        //_chip_status  = 0;
        ui.addLog(LOG_INFO, F("Erase chip done"));
        break;
      case 'd':  //Dump flash chip to screen
        dumpFlash(address);
        break;
      //-----------------------------------------------------------------------------
      // ROM/SD functions
      //-----------------------------------------------------------------------------
      case 'z':  //Reset SD card
        rom.resetSd();
        ui.addLog(LOG_INFO, F("SD card reset"));
        break;
      case 'I':  //Show SD info
        rom.readSdInfo(&_size, &_sd_type, &_fat_type);
        ui.addLog(LOG_INFO, F("SD card read info done"));
        break;
      case 'f':  //Search rom files
        searchRomFiles();
        break;
      case 'D':  //Dump rom file index = address
        dumpRom((uint8_t)(address & 0xff));
        break;
      //-----------------------------------------------------------------------------
      // Programming functions
      //-----------------------------------------------------------------------------
      case 'm':  //Set programming mode (0=all bytes, 1=odd bytes, 2=even bytes)
        _mode = _mode < 2 ? _mode + 1 : 0;
        ui.addLog(LOG_INFO, F("Programming mode changed: "), _mode);
        break;
      //-----------------------------------------------------------------------------
      // Store selected rom in selected partition
      // address = partition, data = rom_index
      //-----------------------------------------------------------------------------
      case 's':
        setPartition((uint8_t)(address & 0xff), data);
        break;
      case 'T':  //Test programming phase with current settings
        programFlash(true, false);
        break;
      case 'P':  //Programming phase with current settings
        programFlash(false, false);
        break;
      case 'V':  //Verification phase with current settings
        programFlash(true, true);
        break;
      case 'S':  //Save partitions to sd file
        saveFlash();
        break;
      //-----------------------------------------------------------------------------
      // program flash location with data
      //-----------------------------------------------------------------------------
      case 'p':
        flash.programData(address, data);
        flash.resetChip();
        ui.addLog(LOG_DEBUG, F("Flash program data: "), address, data);
        break;
      //-----------------------------------------------------------------------------
      // write byte to data bus
      //-----------------------------------------------------------------------------
      case 'o':
        ll.setDataBus(OUTPUT);
        ll.writeData(data);
        ui.addLog(LOG_DEBUG, F("Write data: "), data);
        break;
      //-----------------------------------------------------------------------------
      // read byte from data bus
      //-----------------------------------------------------------------------------
      case 'b':
        ll.setDataBus(INPUT);
        data = ll.readData();
        ui.addLog(LOG_DEBUG, F("Read data: "), data);
        break;
      //-----------------------------------------------------------------------------
      // read byte from address
      //-----------------------------------------------------------------------------
      case 'r':
        data = flash.readData(address);
        ui.addLog(LOG_DEBUG, F("Read flash address: "), address, data);
        break;
      //-----------------------------------------------------------------------------
      // write byte to address
      //-----------------------------------------------------------------------------
      case 'w':
        ll.writeLocation(address, data);
        ui.addLog(LOG_DEBUG, F("Write location: "), address, data);
        break;
      //-----------------------------------------------------------------------------
      //<h/H> show this help
      //-----------------------------------------------------------------------------
      case 'h':
      case 'H':
        ui.showHelp();
        break;
    }

    //Update main screen
    ui.drawScreen(
      _brand, _model, flash.getChipStatus(),
      _size, _sd_type, _fat_type, ROM_PATH,
      _partition_size, _total_partitions,
      _rom_names, _total_roms,
      _mode);

    // ui.updateProgressBar(0,100, "---");
    ui.clearInput();
  }
}

//---------------------------------------------------------------------------------------------
// set rom index into partition
// parameters:
//  partition: 0..3 position of rom into flash memory (0 free position)
//  rom_index: 0..x selected rom (index of found rom during sd scan)
//---------------------------------------------------------------------------------------------
void setPartition(uint8_t partition, uint8_t rom_index) {
  char line[LINE_LENGTH];  // sprintf line

  if (rom_index >= _total_roms) rom_index = _total_roms - 1;
  if (partition >= _total_partitions) partition = _total_partitions - 1;

  //bind rom file with partition
  _rom_index[partition] = rom_index;
  _rom_names[partition] = String(rom.getRomFilename(rom_index));

  sprintf(line, "Saved rom %s in partition %d", _rom_names[rom_index].c_str(), partition);
  ui.addLog(LOG_INFO, line);
}
//---------------------------------------------------------------------------------------------
// Set rom file index
//
// Parameters:
//  partition: 0..3 position of rom into flash memory (0 free position)
//  rom_index: 0..x selected rom (index of found rom during sd scan)
//---------------------------------------------------------------------------------------------
void setRomFile(uint8_t partition, uint8_t rom_index) {
  //Sanitize parameters
  if (rom_index >= _total_roms) rom_index = _total_roms - 1;
  if (partition >= _total_partitions) partition = _total_partitions - 1;

  _rom_index[partition] = rom_index;
}

//---------------------------------------------------------------------------------------------
// Scan sd card for rom files (.bin)
//---------------------------------------------------------------------------------------------
void searchRomFiles() {
  char line[LINE_LENGTH];  // sprintf line
  uint8_t last_roms = 0;

  _total_roms = rom.searchRomFiles();
  for (int i = 0; i < _total_roms; i++) {
    _rom_names[i] = String(rom.getRomFilename(i));
    if ((i - last_roms) > 2) {
      sprintf(line, "\t[%d-%d] - %s - %s - %s - %s",
              last_roms, i,
              _rom_names[last_roms].c_str(),
              _rom_names[last_roms + 1].c_str(),
              _rom_names[last_roms + 2].c_str(),
              _rom_names[last_roms + 3].c_str());
      last_roms = i + 1;
      ui.addLog(LOG_DEBUG, line);
    }
  }
  if (last_roms < _total_roms) {
    //Max 3 roms to show
    sprintf(line, "\t[%d-%d] - %s - %s - %s",
            last_roms, _total_roms - 1,
            _rom_names[last_roms].c_str(),
            last_roms + 1 < _total_roms ? _rom_names[last_roms + 1].c_str() : "",
            last_roms + 2 < _total_roms ? _rom_names[last_roms + 2].c_str() : "");
    ui.addLog(LOG_DEBUG, line);
  }
  ui.addLog(LOG_INFO, F("SD card rom scan done, roms found: "), _total_roms);
}
//---------------------------------------------------------------------------------------------
// Program partition with selected rom file and mode
// simulate: doesn't burn flash only simulate process
// verify: verify after programming or verify only
//---------------------------------------------------------------------------------------------
void programFlash(bool simulate, bool verify) {
  char line[LINE_LENGTH];  // sprintf line
  bool write;              // Decide to store or not
  uint32_t address;        // Current memory address
  uint8_t data[16];        // Array of  byte read from SD
  uint8_t b = 0;           // Index of current 16 bytes data buffer
  String title;            // Progress bar title

  if (_total_partitions == 0 || _partition_size == 0) {
    ui.addLog(LOG_WARN, F("No partitions defined: read chip info before programming"));
    return;
  }
  if (_total_roms == 0) {
    ui.addLog(LOG_WARN, F("No roms defined: scan SD for rom images before programming"));
    return;
  }

  flash.resetChip();
  ui.addLog(LOG_INFO, F("press x to exit"));

  ui.setWaitForReturn(false);
  char ch = (char)0;
  uint32_t total_size = _partition_size * _total_partitions;
  //Scan all partitions
  for (int p = 0; p < _total_partitions; p++) {
    address = _partition_size * p;  //0x00000, 0x20000, 0x40000, 0x60000

    //open rom file correspondent to partition index
    File file = rom.openRomFile(_rom_index[p]);
    if (!file) {
      ui.addLog(LOG_ERROR, F("Open rom file failed for partition"), p);
      return;
    }
    //length of data is full file if mode == 0 or file size / 2 if modes 1 or 2
    uint32_t total_size = file.size();
    uint32_t size = _mode > 0 ? total_size >> 1 : total_size;
    uint32_t read_size = 0;

    //Check size
    if (size > _partition_size) {
      sprintf(line, "rom size %lu exceed partition size %lu", size, _partition_size);
      //ui.addLog(LOG_ERROR, F("rom size exceed partition size"));
      ui.addLog(LOG_ERROR, line);

      ui.setWaitForReturn(true);
      file.close();
      return;
    }

    if (!simulate) {
      title = "program";
      if (verify)
        title += " & verify";
    } else
      title = verify ? "verify" : "simulate";

    ui.updateProgressBar(read_size, total_size, title.c_str());

    sprintf(line, "Reading rom file %s size %lu", _rom_names[_rom_index[p]].c_str(), total_size);
    ui.addLog(LOG_INFO, line);

    uint32_t start_time = millis();
    // simulate/burn/verification sequence
    while (file.available()) {
      if (ui.checkInput()) {
        ui.setWaitForReturn(true);
        file.close();
        return;
      }

      uint8_t bytes_read = file.read(data, sizeof(data));
      read_size += bytes_read;
      //Check current address
      if (address >= (_partition_size * (p + 1) - 1)) {
        ui.addLog(LOG_ERROR, F("length of read data buffer exceed partition size"));
        ui.setWaitForReturn(true);
        file.close();
        return;
      }
      // Write buffer to flash
      for (uint32_t a = 0; a < bytes_read; a++) {
        //Decide if use current byte
        switch (_mode) {
          case 1:  //odd bytes
            write = ((a & 0x01) != 0);
            break;
          case 2:  //even bytes
            write = ((a & 0x01) == 0);
            break;
          default:
            write = true;
            break;
        }
        //Write if enabled
        if (write) {
          if (!simulate)
            flash.programData(address, data[a]);

          if (verify) {
            uint8_t read_data = flash.readData(address);
            if (data[a] != read_data) {
              ui.addLog(LOG_ERROR, F("read data differ from read data!"), read_data, data[a]);
              ui.setWaitForReturn(true);
              file.close();
              return;
            }
          }
          //Next address
          address++;

          //toggle led every x bytes written
          if ((address % 1024) == 0) {
            ui.updateProgressBar(read_size, total_size, title.c_str());
            //Toggle led
            digitalToggleFast(LED_BUILTIN);  // turn the LED on (HIGH is the voltage level)
          }
        }
      }
    }  // end of file.available()
    file.close();
    flash.resetChip();

    uint32_t end_time = millis();
    sprintf(line, "Action done in %d s", p, (int)((end_time - start_time) / 1000));
    ui.addLog(LOG_INFO, line);
    ui.setWaitForReturn(true);
  }
}
//---------------------------------------------------------------------------------------------
// Read partition and save to file
//---------------------------------------------------------------------------------------------
void saveFlash(void) {
  char line[LINE_LENGTH];  // sprintf line
  //https://forum.arduino.cc/t/faster-way-to-read-sd-cards/307850/2
  uint8_t buf[SD_BUFFER_LENGTH];  // SD buffer
  uint8_t b = 0;                  // buffer index
  uint32_t address;               // Current memory address
  uint32_t start_time;            // Start time of backup sequence
  uint32_t end_time;              // End time of backup sequence

  if (_total_partitions == 0) {
    ui.addLog(LOG_WARN, F("No partitions defined: read chip info before save partition"));
    return;
  }

#ifdef FISHINO_MEGA_R2
  File file;
#else
  //FsFile file;
  File file;
#endif

  flash.resetChip();
  //Scan all partitions
  for (int p = 0; p < _total_partitions; p++) {
    address = _partition_size * p;

    if (!rom.createPartitionFile(p, &file)) {
      ui.addLog(LOG_ERROR, F("Create rom file failed"));
      return;
    }
    sprintf(line, "Start save flash memory partition %i", p);
    ui.addLog(LOG_INFO, line);
    ui.updateProgressBar(0, _partition_size, "save");

    start_time = millis();
    // simulate/burn/verification sequence
    for (uint32_t a = 0; a < _partition_size; a++) {
      buf[b++] = flash.readData(address + a);
      if (b == SD_BUFFER_LENGTH) {
        file.write(buf, b);
        ui.updateProgressBar(a, _partition_size, "save");
        b = 0;
      }
    }
    //Last block
    if (b > 0 && b < SD_BUFFER_LENGTH)
      file.write(buf, b);
    file.close();
    end_time = millis();
    sprintf(line, "Partition %d saved in %d seconds", p, (end_time - start_time) / 1000);
    ui.addLog(LOG_INFO, line);
  }
  //End of all partitions
  ui.addLog(LOG_INFO, F("Flash partitions saved to sd"));
}
//---------------------------------------------------------------------------------------------
// Dump flash data read from chip starting from <address> for 1 block of 16 bytes each
//
// Parameters:
//  start_address: starting flash memory address
//---------------------------------------------------------------------------------------------
void dumpFlash(uint32_t start_address) {
  char line[LINE_LENGTH];  // sprintf line
  uint8_t data[16];
  bool showData = true;

  int32_t addr = start_address;

  if (_total_partitions == 0) {
    ui.addLog(LOG_WARN, F("Read chip info before dump memory"));
    return;
  }

  //Max end addres with total partitions
  uint32_t end_address = _partition_size * _total_partitions - 0x41;  //subtract 64 bytes to max address to show last window correctly
  uint8_t curr_part = 0;
  if (start_address >= end_address) {
    ui.addLog(LOG_WARN, F("Start address after end of flash memory: "), end_address);
    return;
  }

  sprintf(line, "Dump from address %#06X to %#06X", start_address, end_address);
  ui.addLog(LOG_INFO, line);
  ui.addLog(LOG_INFO, F("Use u=up, d=down, h=home, e=end to move dump area, x to exit"));
  //TODO: add progress bar
  ui.updateProgressBar(start_address, end_address, "Dump flash");

  ui.setWaitForReturn(false);
  char ch = (char)0;

  flash.resetChip();
  while (ch != 'x' && ch != 'X') {
    //Check received input from serial to move dump area or exit
    if (ui.checkInput()) {
      //Move dumped data window
      ch = ui.lastKey();
      if (ch == 'u' || ch == 'U')
        addr = (addr - 0x10) >= start_address && (addr - 0x10) < 0xfffffff0 ? addr - 0x10 : start_address;
      if (ch == 'd' || ch == 'D')
        addr = (addr + 0x10) <= end_address ? addr + 0x10 : end_address;
      if (ch == 'h' || ch == 'H')
        addr = start_address;
      if (ch == 'e' || ch == 'E')
        addr = end_address;  // 4 rows of 16 bytes

      if (ch == 'n' || ch == 'N')  //Next partition
        if (curr_part < (_total_partitions + 1)) {
          curr_part++;
          addr = curr_part * _partition_size;
        }
      if (ch == 'p' || ch == 'P')  //Previous partition
        if (curr_part > 0) {
          curr_part--;
          addr = curr_part * _partition_size;
        }
      //addr = end_address; // 4 rows of 16 bytes

      //show for any key oressed
      showData = true;
    }
    if (showData) {
      //Show 4 rows of dumped data
      for (uint8_t r = 0; r < 4; r++) {
        uint32_t a = addr + r * 16;  //(r*16)
        //end index
        for (uint8_t c = 0; c < 16; c++) {
          //data[c] = c; // _ll.readLocation(a+c);
          data[c] = flash.readData(a + c);
        }
        //Show hex codes
        //https://stackoverflow.com/questions/8513169/why-is-0-zero-printed-without-leading-0x-with-c-printf-format-x
        sprintf(line, "[0x%08lx]  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X",
                a,
                data[0x00], data[0x01], data[0x02], data[0x03],
                data[0x04], data[0x05], data[0x06], data[0x07],
                data[0x08], data[0x09], data[0x0a], data[0x0b],
                data[0x0c], data[0x0d], data[0x0e], data[0x0f]);
        //Show hex at row
        ui.showMap(r, line);
      }
      ui.updateProgressBar(addr, end_address, "Dump flash");
      showData = false;
    }
  }
  ui.setWaitForReturn(true);
  ui.addLog(LOG_INFO, F("Dump flash data done"));
}

//---------------------------------------------------------------------------------------------
// Dump rom file by index
//---------------------------------------------------------------------------------------------
void dumpRom(uint8_t rom_index) {
  char line[LINE_LENGTH];  // sprintf line
  uint8_t data[16];
  bool showData = true;

  if (_total_roms == 0) {
  }
  if (rom_index >= _total_roms) {
    ui.addLog(LOG_WARN, F("Rom index greater than total roms: "), rom_index);
    return;
  }

  sprintf(line, "Dump rom [%s]", rom.getRomFilename(rom_index).c_str());
  ui.addLog(LOG_INFO, line);
  ui.addLog(LOG_INFO, F("Use u=up, d=down, h=home, e=end to move dump area, x to exit"));

  ui.setWaitForReturn(false);
  char ch = (char)0;

  File file = rom.openRomFile(rom_index);
  if (!file) {
    ui.setWaitForReturn(true);
    ui.addLog(LOG_ERROR, F("Dump rom file data failed"));
    return;
  }

  if (!file.seekSet(0)) {
    ui.setWaitForReturn(true);
    ui.addLog(LOG_ERROR, F("File seek error"));
    return;
  }
  while (ch != 'x' && ch != 'X') {
    //Check received input from serial to move dump area or exit
    if (ui.checkInput()) {
      //Move dumped data window
      ch = ui.lastKey();
      if (ch == 'u' || ch == 'U')
        if (file.curPosition() > 128) {
          // Serial.print("\n\n\nfile.curPosition()");
          // Serial.println(file.curPosition());

          file.seekSet(file.curPosition() - 128);  // 2*64

          // Serial.print("\n\n\nfile.curPosition()");
          // Serial.println(file.curPosition());
        } else
          file.seekSet(0);
      if (ch == 'd' || ch == 'D')
        if (file.curPosition() <= (file.fileSize() - 64)) {
          file.seekSet(file.curPosition());
        } else {
          file.seekSet(file.fileSize() - 64);
          //file.seekEnd();
        }
      if (ch == 'h' || ch == 'H') {
        file.seekSet(0);
      }
      if (ch == 'e' || ch == 'E') {
        file.seekSet(file.fileSize() - 64);
        // file.seekEnd();
      }
      //show for any key oressed
      showData = true;
    }
    if (showData) {
      //Show 4 rows of dumped data
      for (uint8_t r = 0; r < 4; r++) {
        uint32_t pos = file.curPosition();

        if (!file.available() || !file.read(data, 16)) {
          ui.setWaitForReturn(true);
          ui.addLog(LOG_ERROR, F("Error reading from sd"));
          return;
        }
        sprintf(line, "[0x%08lx]  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X",
                pos,
                data[0x00], data[0x01], data[0x02], data[0x03],
                data[0x04], data[0x05], data[0x06], data[0x07],
                data[0x08], data[0x09], data[0x0a], data[0x0b],
                data[0x0c], data[0x0d], data[0x0e], data[0x0f]);
        //Show hex at row
        ui.showMap(r, line);
      }
      showData = false;
    }
  }
  file.close();
  ui.setWaitForReturn(true);
  ui.addLog(LOG_INFO, F("Dump rom file data done"));
}
