#ifndef _rom_h
#define _rom_h

#include <Arduino.h>
#include <SPI.h>

#include "globals.h"

#if defined(FISHINO_MEGA_R2)
#include "FishinoSdFat.h"
#else
#define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
#include "SdFat.h"
#include "sdios.h"
#define File FsFile
#endif

//------------------------------------------------------------------------------
// * Set DISABLE_CHIP_SELECT to disable a second SPI device.
// * For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// * to 10 to disable the Ethernet controller.
//------------------------------------------------------------------------------
#ifdef FISHINO_MEGA_R2
#define SPI_SPEED SD_SCK_MHZ(15)
#else
#define SPI_SPEED SD_SCK_MHZ(4)
#endif

#define DISABLE_CHIP_SELECT -1
//SD Buffer length
#define SD_BUFFER_LENGTH 64

//=============================================================================================
//FLASH FUNCTIONS
//=============================================================================================
class Rom {
private:
  //------------------------------------------------------------------------------
  // SDCARD section
  //------------------------------------------------------------------------------
#ifdef SDCS
  const uint8_t SD_CHIP_SELECT = SDCS;
#else
  const uint8_t SD_CHIP_SELECT = SS;
#endif
#ifdef FISHINO_MEGA_R2
  SdFat _sd;
#else
  SdFs _sd;
#endif
  uint32_t _sd_size = 0;
  const char *_rom_path;  // Rom path (default ROM_PATH)
  String _rom[MAX_ROMS];  // Max rom images names (with MAX length for names)

  uint8_t _total_partitions = 0;  // Total number of partitions available inside flash memory
  uint8_t _total_roms = 0;        // Found roms after SD scan
  uint8_t _error_code = 0;        // Last error code from SD library

public:
  //---------------------------------------------------------------------------------------------
  // Default constructor
  //---------------------------------------------------------------------------------------------
  Rom(const char *rom_path = ROM_PATH);
  //---------------------------------------------------------------------------------------------
  // Init sd rom data during setup phase
  //---------------------------------------------------------------------------------------------
  void init(uint8_t partitions = MAX_PARTITIONS);
  //---------------------------------------------------------------------------------------------
  // Reset SD card
  //---------------------------------------------------------------------------------------------
  bool resetSd();
  //---------------------------------------------------------------------------------------------
  // Read Sd information
  //---------------------------------------------------------------------------------------------
  bool readSdInfo(uint32_t *size, uint8_t *sd_type, uint8_t *fat_type);
  //---------------------------------------------------------------------------------------------
  // Search rom images into specific folder
  // Memorize filenames available for FLASH programming
  //---------------------------------------------------------------------------------------------
  uint8_t searchRomFiles();
  //---------------------------------------------------------------------------------------------
  // Open rom file correspondant to given index
  //---------------------------------------------------------------------------------------------
  File openRomFile(uint8_t rom_index);
  //---------------------------------------------------------------------------------------------
  // Create rom file correspondant to given partition
  //---------------------------------------------------------------------------------------------
  bool createPartitionFile(uint8_t partition, File *fileptr);
  //---------------------------------------------------------------------------------------------
  // Return rom filename correspondant to rom index
  //---------------------------------------------------------------------------------------------
  String getRomFilename(uint8_t rom_index);
  //---------------------------------------------------------------------------------------------
  // Return number of found rom files (to MAX_ROMS)
  //---------------------------------------------------------------------------------------------
  uint8_t getRomFiles();
  //---------------------------------------------------------------------------------------------
  // Return last error code from SD library and clear it
  //---------------------------------------------------------------------------------------------
  uint8_t getErrorCode();
};
#endif  //#define _rom_h