//=============================================================================================
// User Interface functions:
// INIT FUNCTIONS
//     init(void)
// INPUT FUNCTIONS
//     checkInput(void)
//     getCommand(char* cmd, uint32_t* address, uint8_t* data)
//     setWaitForReturn(bool wait)
//     clearInput(void)
//  P  parseCommand(String line)
// TEXT WINDOWS FUNCTIONS
//  P  drawWindow(int col, int row, int width, int height, const char* title)
//  P  topWindow(int col, int row, int length, const char* title)
//     bottomWindow(int col, int row, int length)
// LOGS FUNCTIONS
//     showLogs(void)
//  P  shiftLogs(void)
//     addLog(int color, char* log)
//     addLog(int color, const __FlashStringHelper* log)
//     addLog(int color, const __FlashStringHelper* log, int parameter)
//     addLog(int color, const __FlashStringHelper* log, const char* parameter)
// HEX DUMP FUNCTIONS
//     showHexData(uint32_t address, uint8_t* data)
// PROGRESS BAR FUNCTIONS
//     initProgressBar(int col, int row, uint32_t width, const char* title)
//     updateProgressBar(uint32_t value, uint32_t max_value, const char* title)
// MAIN SCREENS FUNCTIONS
//     showHelp(void)
//     drawScreen(uint8_t brand, uint8_t model, uint8_t status,
//                 uint32_t sd_size, uint8_t sd_type, uint8_t fat_type, const char* romdir,
//                 uint32_t partition_size, uint8_t total_partitions, uint8_t current_partition,
//                 uint8_t program_mode, const char* rom_current_file, uint8_t rom_files)
//
//=============================================================================================
#include "ui.h"

//---------------------------------------------------------------------------------------------
// init user interface
//---------------------------------------------------------------------------------------------
void Ui::init(void) {

  Serial.begin(115200);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  //------------------------------------------------------------------------------------------
  //Init defaults
  //------------------------------------------------------------------------------------------
  //TODO read from .ini
  for (int i = 0; i < LOG_ROWS; i++) {
    _row_level[i] = 0;
    _row_log[i] = String();
  }

  _input = "";
  _dump_row = 0;
  _tmp_data = 0;
  _tmp_address = 0;
  _wait_return = true;

  ansi.set80columns();
  ansi.clearScreen();
  ansi.cursorHide();
  ansi.foreground(ansi.white);
}
//=============================================================================================
// INPUT FUNCTIONS
//=============================================================================================
//---------------------------------------------------------------------------------------------
// Check if user input is arrived from ANSI console
//---------------------------------------------------------------------------------------------
bool Ui::checkInput(void) {
  //int ch;
  if (ansi.available() > 0) {
    _last_key = ansi.read();

    if (_wait_return) {
      //Printable characters
      if (((_last_key == ',') || (_last_key >= '0' && _last_key <= '9') || (_last_key >= 'A' && _last_key <= 'Z') || (_last_key >= 'a' && _last_key <= 'z')) && _input.length() < INPUT_LENGTH) {
        _input.concat(_last_key);
        ansi.foreground(ansi.white);
        ansi.gotoXY(INPUT_X + 1 + _input.length(), INPUT_Y);
        ansi.print(_last_key);
      }
      //Backspace
      else if (_last_key == 8) {
        _input.remove(_input.length() - 1, 1);
        ansi.foreground(ansi.white);
        ansi.gotoXY(INPUT_X + 1, INPUT_Y);
        ansi.print(">");
        ansi.print(_input);
        ansi.print(" ");
        ansi.cursorBack(1);
      }
      //return
      else if (_last_key == '\n' || _last_key == '\r') {
        parseCommand(_input);
        ansi.foreground(ansi.white);
        ansi.gotoXY(INPUT_X + 1, INPUT_Y);
        ansi.clearLine();
        ansi.print(">");
        return true;
      }
      //beep to the others
      else {
        ansi.print('\x07');
      }
    } else  //No wait for return
    {
      _input = String(_last_key);

      return true;
    }
  }
  return false;
}
//---------------------------------------------------------------------------------------------
// get current user command and clear it
//---------------------------------------------------------------------------------------------
void Ui::getCommand(char* cmd, uint32_t* address, uint8_t* data) {
  *cmd = _cmd;
  *address = _tmp_address;
  *data = _tmp_data;
}
//---------------------------------------------------------------------------------------------
// last user input
//---------------------------------------------------------------------------------------------
char Ui::lastKey() {
  return _last_key;
}
//---------------------------------------------------------------------------------------------
// Set wait for return true/false
//---------------------------------------------------------------------------------------------
void Ui::setWaitForReturn(bool wait) {
  _wait_return = wait;
}
//---------------------------------------------------------------------------------------------
// Get input mode (0 = normal, 1 = help, 2 = map)
//---------------------------------------------------------------------------------------------
void Ui::clearInput(void) {
  _cmd = 0;
  _tmp_address = 0;
  _tmp_data = 0;
  _last_key = 0;
  _input = "";
}
//---------------------------------------------------------------------------------------------
// Parse command from serial line
// extracted values:
//  _cmd: selected command
//  _tmp_address: 32bit flash address
//  _tmp_data: 8bit user data
// if only 1 parameter is found _tmp_address = _tmp_data
//---------------------------------------------------------------------------------------------
void Ui::parseCommand(String line) {
  _cmd = line.charAt(0);

  int i = line.indexOf(',');

  if (i > 0)
    _tmp_address = line.substring(i + 1).toInt();

  i = line.lastIndexOf(',');

  if (i > 0) {
    //Safe truncate to 8 bit
    uint32_t par2 = line.substring(i + 1).toInt();
    _tmp_data = (uint8_t)(par2 & 0xff);
  }
}
//=============================================================================================
// TEXT WINDOWS FUNCTIONS
//=============================================================================================
//---------------------------------------------------------------------------------------------
// Draw entire window
//---------------------------------------------------------------------------------------------
void Ui::drawWindow(int col, int row, int width, int height, const char* title) {
  topWindow(col, row, width, title);

  for (int i = 0; i < (height - 2); i++) {
    ansi.gotoXY(col, row + i + 1);

    ansi.print(F("│"));
    for (int ii = 2; ii < width; ii++)
      ansi.print(F(" "));
    ansi.print(F("│"));
  }
  bottomWindow(col, row + height - 1, width);
}
//---------------------------------------------------------------------------------------------
// Draw top section of window
//---------------------------------------------------------------------------------------------
void Ui::topWindow(int col, int row, int length, const char* title) {
  ansi.gotoXY(col, row);
  if (title) {
    ansi.print(F("┌┤ "));
    ansi.print(title);
    ansi.print(F(" ├"));
    for (int i = 6 + strlen(title); i < length; i++)
      ansi.print(F("─"));
  } else {
    ansi.print(F("┌"));
    for (int i = 2; i < length; i++)
      ansi.print(F("─"));
  }
  ansi.print(F("┐"));
}
//---------------------------------------------------------------------------------------------
// Draw bottom section of window
//---------------------------------------------------------------------------------------------
void Ui::bottomWindow(int col, int row, int length) {
  ansi.gotoXY(col, row);
  ansi.print(F("└"));
  for (int i = 2; i < length; i++)
    ansi.print(F("─"));
  ansi.print(F("┘"));
}
//=============================================================================================
// LOGS FUNCTIONS
//=============================================================================================
//---------------------------------------------------------------------------------------------
// Shift last LOG_ROWS-1 rows up
//---------------------------------------------------------------------------------------------
void Ui::shiftLogs(void) {
  for (int i = 0; i < (LOG_ROWS - 1); i++) {
    //strcpy(_row_log[i], _row_log[i+1]);
    _row_log[i] = _row_log[i + 1];
    _row_level[i] = _row_level[i + 1];
  }
  _row_log[LOG_ROWS - 1] = String();
  ;
  _row_level[LOG_ROWS - 1] = ansi.black;
}
//---------------------------------------------------------------------------------------------
// show content of log inside log area
//---------------------------------------------------------------------------------------------
void Ui::showLogs(void) {
  //print all lines
  for (int i = 0; i < LOG_ROWS; i++) {
    ansi.gotoXY(LOG_X + 1, LOG_Y + i + 1);
    ansi.foreground(_row_level[i]);
    ansi.clearLine();
    ansi.print(_row_log[i].substring(0, 70));
    ansi.foreground(ansi.white);
    ansi.gotoXY(LOG_X + 79, LOG_Y + i + 1);
    ansi.print(F("│"));
  }
}
//---------------------------------------------------------------------------------------------
// add Log line without parameter to ansi window
//---------------------------------------------------------------------------------------------
void Ui::addLog(int color, char* log) {
  shiftLogs();

  //Copy new line
  //strcpy(_row_log[LOG_ROWS-1], log);
  _row_log[LOG_ROWS - 1] = String(log);
  //Log level (color)
  _row_level[LOG_ROWS - 1] = color;

  showLogs();
}
//---------------------------------------------------------------------------------------------
// add Log line without parameter to ansi window
//---------------------------------------------------------------------------------------------
void Ui::addLog(int color, const __FlashStringHelper* log) {

  shiftLogs();

  //Copy new line
  //strcpy(_row_log[LOG_ROWS-1], log);
  _row_log[LOG_ROWS - 1] = String(log);
  //Log level (color)
  _row_level[LOG_ROWS - 1] = color;

  showLogs();
}
//---------------------------------------------------------------------------------------------
// add Log line with int parameter to ansi window
//---------------------------------------------------------------------------------------------
void Ui::addLog(int color, const __FlashStringHelper* log, int parameter) {
  char line[81];  // 80 columns line + \0

  shiftLogs();

  char buffer[81];
  strncpy_P(buffer, (const char*)log, 80);  // _P is the version to read from program space
  //Copy new line with string parameter
  sprintf(line, "%s%d", buffer, parameter);
  //strcpy(_row_log[LOG_ROWS-1], log);
  _row_log[LOG_ROWS - 1] = String(line);
  //Log level (color)
  _row_level[LOG_ROWS - 1] = color;

  showLogs();
}
void Ui::addLog(int color, const __FlashStringHelper* log, int parameter1, int parameter2) {
  char line[81];  // 80 columns line + \0

  shiftLogs();

  char buffer[81];
  strncpy_P(buffer, (const char*)log, 80);  // _P is the version to read from program space
  //Copy new line with string parameter
  sprintf(line, "%s%d,%d", buffer, parameter1, parameter2);
  //strcpy(_row_log[LOG_ROWS-1], log);
  _row_log[LOG_ROWS - 1] = String(line);
  //Log level (color)
  _row_level[LOG_ROWS - 1] = color;

  showLogs();
}

//---------------------------------------------------------------------------------------------
// add Log line with string parameter to ansi window
//---------------------------------------------------------------------------------------------
void Ui::addLog(int color, const __FlashStringHelper* log, const char* parameter) {
  char line[81];  // 80 columns line + \0

  shiftLogs();
  char buffer[81];
  strncpy_P(buffer, (const char*)log, 80);  // _P is the version to read from program space
  //Copy new line with string parameter
  sprintf(line, "%s%s", buffer, parameter);
  //strcpy(_row_log[LOG_ROWS-1], log);
  _row_log[LOG_ROWS - 1] = String(line);
  //Log level (color)
  _row_level[LOG_ROWS - 1] = color;

  showLogs();
}
//=============================================================================================
// HEX DUMP FUNCTIONS
//=============================================================================================
//---------------------------------------------------------------------------------------------
// Show hex data in a single line
//  address = start adress of memory dump
//  data    = 16 bytes dumped data array
//  index   = first byte to be shown
//---------------------------------------------------------------------------------------------
void Ui::showHexData(uint32_t address, uint8_t* data) {
  char line[81];  // 80 columns line + \0

  ansi.gotoXY(MAP_X + 3, MAP_Y + 1 + _map_index);
  _map_index = _map_index < (MAP_H - 3) ? _map_index + 1 : 0;


  sprintf(line, "[%#08x] %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x",
          address,
          data[0x00], data[0x01], data[0x02], data[0x03],
          data[0x04], data[0x05], data[0x06], data[0x07],
          data[0x08], data[0x09], data[0x0a], data[0x0b],
          data[0x0c], data[0x0d], data[0x0e], data[0x0f]);
  ansi.println(line);
}
//=============================================================================================
// PROGRESS BAR FUNCTIONS
//=============================================================================================
//---------------------------------------------------------------------------------------------
// show progess bar with value
//---------------------------------------------------------------------------------------------
void Ui::initProgressBar(int col, int row, uint32_t width, const char* title) {
  //Update title
  drawWindow(col - 1, row - 1, width + 2, 3, title);
  //drawWindow(35, 1, 46, 3, "Action");

  char str_val[10];
  sprintf(str_val, "000");

  _barCol = col;
  _barRow = row;
  _barLength = width;

  ansi.gotoXY(_barCol, _barRow);
  ansi.print(("["));
  for (uint32_t i = 2; i < width / 2 - 1; i++)
    ansi.print(("░"));

  ansi.print(str_val);

  for (uint32_t i = width / 2 + 3; i < width; i++)
    ansi.print(("░"));
  ansi.print(("]"));
}
//---------------------------------------------------------------------------------------------
// show progess bar with value
//---------------------------------------------------------------------------------------------
void Ui::updateProgressBar(uint32_t value, uint32_t max_value, const char* title) {
  drawWindow(_barCol - 1, _barRow - 1, _barLength + 2, 3, title);

  float val = (float)value / (float)max_value;
  char str_val[10];

  uint32_t w_val = (uint32_t)(val * _barLength);
  sprintf(str_val, "%03d%%", (uint32_t)(val * 100));

  ansi.gotoXY(_barCol, _barRow);
  ansi.print(("["));
  for (uint32_t i = 2; i < _barLength / 2 - 1; i++)
    if ((i - 1) <= w_val)
      ansi.print(("█"));
    else
      ansi.print(("░"));

  ansi.print(str_val);

  for (uint32_t i = _barLength / 2 + 3; i < _barLength; i++)
    if ((i - 1) <= w_val)
      ansi.print(("█"));
    else
      ansi.print(("░"));
  ansi.print(("]"));
}

//=============================================================================================
// CONSOLE SCREENS FUNCTIONS
//=============================================================================================
//-----------------------------------------------------------------------------
// Show memory map
//-----------------------------------------------------------------------------
void Ui::showMap(uint8_t partition, char* line) {
  ansi.gotoXY(MAP_X + 2, MAP_Y + 1 + partition);
  ansi.foreground(ansi.white);
  ansi.print(line);
}
//---------------------------------------------------------------------------------------------
// Show commands help
//---------------------------------------------------------------------------------------------
void Ui::showHelp(void) {
  drawWindow(1, 1, 80, 23, "Help");

  ansi.foreground(ansi.white);
  ansi.gotoXY(3, 3);
  ansi.bold();
  ansi.println(F("Command input format"));
  ansi.gotoXY(4, 4);
  ansi.normal();
  ansi.println(F("<CMD>[,PARAMETER1][,PARAMETER2] <enter>"));

  ansi.gotoXY(3, 6);
  ansi.bold();
  ansi.println(F("Chip commands:"));
  ansi.gotoXY(4, 7);
  ansi.normal();
  ansi.println(F("<c>: reset chip - <i>: read chip info - <e>: erase chip"));
  ansi.gotoXY(4, 8);
  ansi.normal();
  ansi.println(F("<p>, address, data: program flash address with data"));
  ansi.gotoXY(4, 9);
  ansi.normal();
  ansi.println(F("<r>, address: read flash address"));

  ansi.gotoXY(3, 10);
  ansi.bold();
  ansi.println(F("SD commands:"));
  ansi.gotoXY(4, 11);
  ansi.normal();
  ansi.println(F("<z>: reset sd card - <I>: read sd card info - <f>: search rom files"));

  ansi.gotoXY(3, 12);
  ansi.bold();
  ansi.println(F("Partition commands:"));
  ansi.gotoXY(4, 13);
  ansi.normal();
  ansi.println(F("<T>: test partition - <P>: program partition - <V>: verify partition"));
  ansi.gotoXY(4, 14);
  ansi.println(F("<s> part, rom: set partition = rom - <m>: program mode"));
  ansi.gotoXY(4, 15);
  ansi.println(F("<S>: save all partitions to sd"));

  ansi.gotoXY(3, 16);
  ansi.bold();
  ansi.println(F("Dump commands:"));
  ansi.gotoXY(4, 17);
  ansi.normal();
  ansi.println(F("<d>, address: dump chip from address - <D>, rom: dump rom file"));

  ansi.gotoXY(3, 18);
  ansi.bold();
  ansi.println(F("Low level commands:"));
  ansi.gotoXY(4, 19);
  ansi.normal();
  ansi.println(F("<o>, data: write byte to data bus - <b>: read byte from data bus"));
  ansi.gotoXY(4, 20);
  ansi.normal();
  ansi.println(F("<r>, address: read location - <w>, address, data: write location"));

  ansi.gotoXY(4, 22);
  ansi.normal();
  ansi.println(F("<h/H> show this help"));
  //Wait key pressed
  while (ansi.available() == 0) {
    ;
  }
  ansi.clearScreen();
}
//---------------------------------------------------------------------------------------------
// Draw main screen
//---------------------------------------------------------------------------------------------
void Ui::drawScreen(uint8_t brand, uint8_t model, uint8_t status,
                    uint32_t sd_size, uint8_t sd_type, uint8_t fat_type, const char* rom_dir,
                    uint32_t partition_size, uint8_t total_partitions,
                    String rom[], uint8_t total_roms, uint8_t program_mode) {
  char line[80];  //ERROR IF USE GLOBAL VARIABLE _line
  char file_name[FILENAME_LENGTH];
  const char* tbrand = (brand == 0xBF ? "  SST  " : "unknown");
  const char* tmodel = (model == 0xB5 ? "SST39SF010A" : (model == 0xB6 ? "SST39SF020A" : (model == 0xB7 ? "SST39SF040 " : "unknown    ")));
  const char* tstats = (status == 0x0 ? "empty   " : (status == 0x1 ? "programd" : (status == 0x2 ? "verified" : (status == 0x4 ? "error   " : "unknown."))));
  // 1 - SD_CARD_TYPE_SD1, 2 - SD_CARD_TYPE_SD2, or 3 - SD_CARD_TYPE_SDHC.
  const char* stype = (sd_type == 1 ? "SD-V1" : (sd_type == 2 ? "SD-V2" : (sd_type == 3 ? (sd_size < 70000000 ? "SD-HC" : "SD-XC") : "unkwn")));
  // FAT TYPE
  const char* ftype = (fat_type == 12 ? "FAT12" : (fat_type == 16 ? "FAT16" : (fat_type == 32 ? "exFAT" : "unkwn")));
  //Program mode
  const char* tmode = (program_mode == 0 ? "all  bytes" : (program_mode == 1 ? "odd  bytes" : (program_mode == 2 ? "even bytes" : "error     ")));

  ansi.cursorHide();
  
  if (!_clear_screen)
    ansi.clearScreen();
  _clear_screen = true;

  ansi.foreground(ansi.white);

  // Program version
  drawWindow(VERSION_X, VERSION_Y, VERSION_W, VERSION_H, "Info");
  sprintf(line, "SimpleFlash: v.%s", APP_VERSION);
  ansi.gotoXY(VERSION_X + 2, VERSION_X + 1);
  ansi.print(line);

  drawWindow(INFO_X, INFO_Y, INFO_W, INFO_H, "Settings");
  sprintf(line, "Flash brand: %s     - Flash type: %s   - Flash state: %s",
          tbrand,
          tmodel,
          tstats);
  ansi.gotoXY(INFO_X + 2, INFO_Y + 1);
  ansi.print(line);

  sprintf(line, "Sd type: %s   - Sd size: %ld  - Fat type: %s - Rom folder: %s",
          stype,
          sd_size,
          ftype,
          rom_dir);
  ansi.gotoXY(INFO_X + 2, INFO_Y + 2);
  ansi.print(line);

  sprintf(line, "Partition size: %lu bytes - Partitions: %d", partition_size, total_partitions);
  ansi.gotoXY(INFO_X + 2, INFO_Y + 3);
  ansi.print(line);

  sprintf(line, "Program mode: %s - total roms %d", tmode, total_roms);
  ansi.gotoXY(INFO_X + 2, INFO_Y + 4);
  ansi.print(line);

  //TODO MAP DATA
  drawWindow(MAP_X, MAP_Y, MAP_W, MAP_H, "Map");

  if (total_partitions == 0) {
    //First line
    sprintf(line, "No partition defined                                       .");
    for (int i = 0; i < 4; i++) {
      showMap(i, line);
    }
  } else {
    for (int i = 0; i < total_partitions; i++) {
      uint32_t start_addr = i * partition_size;
      uint32_t end_addr = (i + 1) * partition_size - 1;
      sprintf(line, "Partition %d : address [0x%08lx - 0x%08lx] - Rom file = %s", i, start_addr, end_addr, rom[i].c_str());

      showMap(i, line);
    }
  }

  drawWindow(LOG_X, LOG_Y, LOG_W, LOG_H, "Log");
  showLogs();

  //Redraw progress bar
  initProgressBar(PROGRESS_X, PROGRESS_Y, PROGRESS_W, "---");

  ansi.gotoXY(INPUT_X + 1, INPUT_Y);
  ansi.print(">");
  ansi.cursorShow();
}