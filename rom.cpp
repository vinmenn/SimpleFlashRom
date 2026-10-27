#include "rom.h"
//=============================================================================================
//ROM / SD FUNCTIONS
//=============================================================================================
Rom::Rom(const char *rom_path) {
  _rom_path = rom_path;
}
//---------------------------------------------------------------------------------------------
// Init sd and rom data during setup phase
//---------------------------------------------------------------------------------------------
void Rom::init(uint8_t partitions) {
  //Safe check on max partitions
  _total_partitions = partitions <= MAX_PARTITIONS ? partitions : MAX_PARTITIONS;

  //Reset SD card
  resetSd();
}
//---------------------------------------------------------------------------------------------
// Reset SD card
//---------------------------------------------------------------------------------------------
bool Rom::resetSd() {
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!_sd.begin(SD_CHIP_SELECT, SPI_SPEED)) {
#ifdef DEBUG
    Serial.println("SD begin failed!");
#endif
    //Error code
    _error_code = _sd.card()->errorCode();
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------------------------
// Show Sd information
//---------------------------------------------------------------------------------------------
//_sd_size, _sd.card()->type(), fat_type, ROM_PATH
bool Rom::readSdInfo(uint32_t *size, uint8_t *sd_type, uint8_t *fat_type) {
  csd_t csd;

  //---------------------------------------
  // CARD SIZE
  //---------------------------------------
  uint32_t card_size = _sd.card()->cardSize();
  //  uint32_t card_size = _sd.card()->sectorCount();
  _sd.card()->readCSD(&csd);

  if (card_size == 0) {
#ifdef DEBUG
    Serial.println("Card size is zero!");
#endif
    return false;
  }

  *size = 0.000512 * card_size + 0.5;

  //---------------------------------------
  // SD TYPE
  //---------------------------------------
  *sd_type = _sd.card()->type();

  //---------------------------------------
  // FAT TYPE
  //---------------------------------------
  *fat_type = _sd.vol()->fatType();

  return true;
}
//---------------------------------------------------------------------------------------------
// Search rom images into specific folder
// Memorize filenames available for FLASH programming
//  returns number of found roms (0..MAX_ROMS)
//---------------------------------------------------------------------------------------------
uint8_t Rom::searchRomFiles() {
  char name[MAX_LENGTH];
  File dir;
  File file;
  int len;

  //Reset rom array
  _total_roms = 0;

  if (!_sd.exists(_rom_path)) {
#ifdef DEBUG
    Serial.println("SD not found!");
#endif
    return 0;
  }

  if (!dir.open(_rom_path)) {
#ifdef DEBUG
    Serial.println("path not found!");
#endif
    return 0;
  }

  while (_total_roms < MAX_ROMS) {
    file.close();
    if (!file.openNext(&dir, O_RDONLY)) {
#ifdef DEBUG
      Serial.print("_total_roms ");
      Serial.println(_total_roms);
#endif
      return _total_roms;
    }

    //No subdir
    if (!file.isDirectory() && !file.isHidden()) {
      len = sizeof(name);
      file.getName(name, len);

      if (strstr(name, ".img") || strstr(name, ".bin")) {
        //strncpy(_rom[_total_roms], name, len <= MAX_LENGTH ? len : MAX_LENGTH);
        _rom[_total_roms++] = String(name);
      }
    }
  }
  return _total_roms;
}
//---------------------------------------------------------------------------------------------
// Open rom file correspondant to given partition
//---------------------------------------------------------------------------------------------
File Rom::openRomFile(uint8_t rom_index) {
  String filename;

  //Set filepath
  filename = String(_rom_path) + "/" + _rom[rom_index];

  _sd.chdir(_rom_path);
  File file = _sd.open(_rom[rom_index]);

  return file;
}
//---------------------------------------------------------------------------------------------
// Create rom file correspondant to given partition
//---------------------------------------------------------------------------------------------
bool Rom::createPartitionFile(uint8_t partition, File *fileptr) {
  String filename;
  File file;

  //Set filepath
  filename = String(_rom_path) + "/flash_" + partition + ".img";

  // open the file for reading
  if (!file.open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) {
#ifdef DEBUG
    Serial.println("File open failed!");
#endif
    return false;
  }
  //Set new opened file
  fileptr = &file;
  return true;
}
//---------------------------------------------------------------------------------------------
// Return rom filename correspondant to rom index
//---------------------------------------------------------------------------------------------
String Rom::getRomFilename(uint8_t rom_index) {
  if (rom_index >= _total_roms)
    //No rom file selected for partition
    return String();

  return _rom[rom_index];
}
//---------------------------------------------------------------------------------------------
// Return number of found rom files (to MAX_ROMS)
//---------------------------------------------------------------------------------------------
uint8_t Rom::getRomFiles() {
  return _total_roms;
}
//---------------------------------------------------------------------------------------------
// Return last error code from SD library and clear it
//---------------------------------------------------------------------------------------------
uint8_t Rom::getErrorCode() {
  uint8_t error = _error_code;
  _error_code = 0;
  return error;
}
