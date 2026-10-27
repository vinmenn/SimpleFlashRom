#include "lowlevel.h"
//=============================================================================================
// LOW LEVEL FUNCTIONS
//=============================================================================================

void LowLevel::init() {
  //GPIO init
  pinModeFast(LED_BUILTIN, OUTPUT);
  //Set ADDRESS pins
  for (int i = 0; i < 19; i++) {
    pinModeFast(A[i], OUTPUT);
    digitalWriteFast(A[i], LOW);
  }
  //Set control pins
  pinModeFast(CE, OUTPUT);
  digitalWriteFast(CE, HIGH);
  pinModeFast(WE, OUTPUT);
  digitalWriteFast(WE, HIGH);
  pinModeFast(OE, OUTPUT);
  digitalWriteFast(OE, HIGH);
}

//---------------------------------------------------------------------------------------------
// set data pins as INPUT/OUTPUT
// mode = INPUT | OUTPUT
//---------------------------------------------------------------------------------------------
void LowLevel::setDataBus(unsigned long mode, bool reset) {
  for (int i = 0; i < 8; i++) {
    pinModeFast(D[i], mode);
    if (mode == OUTPUT && reset)
      digitalWriteFast(D[i], LOW);
  }
}
//---------------------------------------------------------------------------------------------
// read data from data pins configured as input
//---------------------------------------------------------------------------------------------
uint8_t LowLevel::readData() {
  uint8_t temp_in = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalReadFast(D[i])) {
      bitSet(temp_in, i);
    }
  }
  return temp_in;
}
//---------------------------------------------------------------------------------------------
// write data to data pins configured as output
//---------------------------------------------------------------------------------------------
void LowLevel::writeData(uint8_t data) {
  for (int i = 0; i < 8; i++)
    digitalWriteFast(D[i], bitRead(data, i));
}
//---------------------------------------------------------------------------------------------
// set address lines
//---------------------------------------------------------------------------------------------
void LowLevel::writeAddress(unsigned long address) {
  for (int i = 0; i < 19; i++)
    digitalWriteFast(A[i], bitRead(address, i));
}
//---------------------------------------------------------------------------------------------
// highlevel function to read a byte from a given address
//---------------------------------------------------------------------------------------------
uint8_t LowLevel::readLocation(unsigned long address) {
  uint8_t data = 0;

  digitalWriteFast(WE, HIGH);
  digitalWriteFast(OE, HIGH);

  writeAddress(address);

  digitalWriteFast(OE, LOW);
  delayMicroseconds(10);
  data = readData();
  digitalWriteFast(OE, HIGH);

  return data;
}
//---------------------------------------------------------------------------------------------
// Send the special address/command combos. The address is written on before WE* falls
// and the command byte b is written before the WE* rises.
//---------------------------------------------------------------------------------------------
void LowLevel::writeLocation(unsigned long address, uint8_t data) {
  digitalWriteFast(OE, HIGH);
  digitalWriteFast(WE, HIGH);

  writeAddress(address);

  writeData(data);
  digitalWriteFast(WE, LOW);
  digitalWriteFast(WE, HIGH);
  delayMicroseconds(10);
}
