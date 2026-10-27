#include "Arduino.h"
#include "flash.h"
//=============================================================================================
//FLASH FUNCTIONS
//=============================================================================================
Flash::Flash(LowLevel ll) {
  _ll = ll;
  _chip_status = 0xff;
}
//---------------------------------------------------------------------------------------------
// Reset chip command , return to normal mode
//---------------------------------------------------------------------------------------------
void Flash::resetChip(void) {
  _ll.setDataBus(OUTPUT);
  //Init to read mode
  _ll.writeLocation(0x0000, 0xf0);

  //TODO: Check chip status
  //  0 empty, 1 programmed, 2 verified, 4 error, 0xff unknown
  _chip_status = readData(0) == 0xff ? 0 : 1;
}
//---------------------------------------------------------------------------------------------
// Special command to identify chip
//---------------------------------------------------------------------------------------------
void Flash::identifyChip(uint8_t* brand, uint8_t* model, uint8_t* partitions, uint32_t* size) {
  _ll.setDataBus(OUTPUT);
  // special mode sequence
  _ll.writeLocation(0x5555, 0xaa);
  _ll.writeLocation(0x2aaa, 0x55);
  _ll.writeLocation(0x5555, 0x90);

  //Read codes
  _ll.setDataBus(INPUT);

  *brand = _ll.readLocation(0x0000);
  *model = _ll.readLocation(0x0001);

  switch (*model) {
    case 0xb5:  // SST39SF010A = 1Mbit
      *partitions = 1;
      *size = MAX_PARTITION_SIZE >> 2;
      break;
    case 0xb6:  // SST39SF020A = 2Mbit
      *partitions = 2;
      *size = MAX_PARTITION_SIZE >> 1;
      break;
    case 0xb7:  // SST39SF040 =  4Mbit
      *partitions = 4;
      *size = MAX_PARTITION_SIZE;
      break;
    default:
      *partitions = 0;
      *size = 0;
      break;
  }

  //return to read mode
  _ll.setDataBus(OUTPUT);
  _ll.writeLocation(0x5555, 0xaa);
  _ll.writeLocation(0x2aaa, 0x55);
  _ll.writeLocation(0x5555, 0xf0);
}
//---------------------------------------------------------------------------------------------
// Special command to erase entire chip
//---------------------------------------------------------------------------------------------
void Flash::eraseChip(void) {
  // special mode sequence
  _ll.setDataBus(OUTPUT);
  _ll.writeLocation(0x5555, 0xaa);
  _ll.writeLocation(0x2aaa, 0x55);
  _ll.writeLocation(0x5555, 0x80);
  _ll.writeLocation(0x5555, 0xaa);
  _ll.writeLocation(0x2aaa, 0x55);
  _ll.writeLocation(0x5555, 0x10);
  delay(100);

  _chip_status = 0;
}
//---------------------------------------------------------------------------------------------
// Program single location (after erasing)
//---------------------------------------------------------------------------------------------
void Flash::programData(unsigned long address, uint8_t data) {
  // special mode sequence
  _ll.setDataBus(OUTPUT);
  _ll.writeLocation(0x5555, 0xaa);
  _ll.writeLocation(0x2aaa, 0x55);
  _ll.writeLocation(0x5555, 0xa0);
  _ll.writeLocation(address, data);
  delayMicroseconds(10);
  _chip_status = 1;
}
uint8_t Flash::readData(unsigned long address) {
  _ll.setDataBus(INPUT);
  return _ll.readLocation(address);
}

//---------------------------------------------------------------------------------------------
// Get flash chip information
//---------------------------------------------------------------------------------------------
uint8_t Flash::getChipStatus(void) {
  return _chip_status;
}