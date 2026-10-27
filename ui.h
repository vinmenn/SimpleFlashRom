//=============================================================================================
// User Interface functions
//=============================================================================================
#ifndef _ui_h
#define _ui_h

#include <Arduino.h>
#include "globals.h"
#include "ansi.h"


#define LOG_DEBUG ANSI::magenta
#define LOG_INFO ANSI::green
#define LOG_WARN ANSI::yellow
#define LOG_ERROR ANSI::red
#define LOG_ROWS 7  // Number of log rows


#define VERSION_X 1   // X position of version area
#define VERSION_Y 1   // Y position of version area
#define VERSION_W 34  // Width of version area
#define VERSION_H 3   // Heigth of version area

#define INFO_X 1   // X position of info area
#define INFO_Y 4   // Y position of info area
#define INFO_W 80  // Width of map area
#define INFO_H 6   // Heigth of map area

#define MAP_X 1   // X position of map area
#define MAP_Y 10  // Y position of map area
#define MAP_W 80  // Width of map area
#define MAP_H 6   // Heigth of map area

#define PROGRESS_X 36  // X position of progress bar
#define PROGRESS_Y 2   // Y position of progress bar
#define PROGRESS_W 44  // Width of progress bar


#define LOG_X 1   // X position of log area
#define LOG_Y 16  // Y position of log area
#define LOG_W 80  // Width of log area
#define LOG_H 9   // Heigth of log area

#define INPUT_X 1           // X position of input area
#define INPUT_Y 25          // Y position of input area
#define INPUT_LENGTH 24     // Max length of input area
#define LINE_LENGTH 80      // Line length in bytes
#define FILENAME_LENGTH 40  // Filename length

class Ui {
private:
  //ANSI interface
  ANSI ansi;

  int _row_level[LOG_ROWS];   //Log row level
  String _row_log[LOG_ROWS];  //Log row text
  int _log_index;
  //------------------------------------------------------------------------------
  // Common definitions
  //------------------------------------------------------------------------------
  //char _line[81];                   // 80 columns line + \0
  String _input;   // input text buffer
  char _last_key;  // last pressed key
  // int _inp_done;                 // flag input done

  //progress bar
  int _barCol;
  int _barRow;
  int _barLength;
  //map area
  uint8_t _map_index = 0;

  char _cmd;
  uint32_t _tmp_address;
  uint8_t _tmp_data;
  uint8_t _dump_row;
  //uint8_t _input_mode; //0 = normal, 1 = help, 2 = scroll
  bool _wait_return;
  bool _clear_screen;

  //---------------------------------------------------------------------------------------------
  // Parse command from serial line
  // extracted values:
  //  _cmd: selected command
  //  _tmp_address: 32bit flash address
  //  _tmp_data: 8bit user data
  // if only 1 parameter is found _tmp_address = _tmp_data
  //---------------------------------------------------------------------------------------------
  void parseCommand(String line);
  //---------------------------------------------------------------------------------------------
  // Draw top section of window
  //---------------------------------------------------------------------------------------------
  void topWindow(int col, int row, int length, const char* title);
  //---------------------------------------------------------------------------------------------
  // Draw bottom section of window
  //---------------------------------------------------------------------------------------------
  void bottomWindow(int col, int row, int length);
  //---------------------------------------------------------------------------------------------
  // Draw entire window
  //---------------------------------------------------------------------------------------------
  void drawWindow(int col, int row, int width, int height, const char* title);
  //---------------------------------------------------------------------------------------------
  // Shift last LOG_ROWS-1 rows up
  //---------------------------------------------------------------------------------------------
  void shiftLogs();
  //---------------------------------------------------------------------------------------------
  // show content of log inside log area
  //---------------------------------------------------------------------------------------------
  void showLogs();
public:
  //=============================================================================================
  // INIT FUNCTIONS
  //=============================================================================================
  //---------------------------------------------------------------------------------------------
  // init user interface
  //---------------------------------------------------------------------------------------------
  void init(void);
  //=============================================================================================
  // INPUT FUNCTIONS
  //=============================================================================================
  //---------------------------------------------------------------------------------------------
  // check user input from ANSI serial console
  //---------------------------------------------------------------------------------------------
  bool checkInput(void);
  //---------------------------------------------------------------------------------------------
  // get current user command and clear it
  //---------------------------------------------------------------------------------------------
  void getCommand(char* cmd, uint32_t* address, uint8_t* data);
  //---------------------------------------------------------------------------------------------
  // last user key pressed
  //---------------------------------------------------------------------------------------------
  char lastKey();
  //---------------------------------------------------------------------------------------------
  // clear user input
  //---------------------------------------------------------------------------------------------
  void clearInput();
  //---------------------------------------------------------------------------------------------
  // Set wait for return true/false
  //---------------------------------------------------------------------------------------------
  void setWaitForReturn(bool wait);
  //=============================================================================================
  // LOG FUNCTIONS
  //=============================================================================================
  //---------------------------------------------------------------------------------------------
  // add Log line without parameter to ansi window
  //---------------------------------------------------------------------------------------------
  void addLog(int color, char* log);
  //---------------------------------------------------------------------------------------------
  // add Log line without parameter to ansi window
  //---------------------------------------------------------------------------------------------
  void addLog(int color, const __FlashStringHelper* log);
  //---------------------------------------------------------------------------------------------
  // add Log line with int parameter to ansi window
  //---------------------------------------------------------------------------------------------
  void addLog(int color, const __FlashStringHelper* log, int parameter);
  //---------------------------------------------------------------------------------------------
  // add Log line with 2 parameter2 to ansi window
  //---------------------------------------------------------------------------------------------
  void addLog(int color, const __FlashStringHelper* log, int parameter1, int parameter2);
  //---------------------------------------------------------------------------------------------
  // add Log line with string parameter to ansi window
  //---------------------------------------------------------------------------------------------
  void addLog(int color, const __FlashStringHelper* log, const char* parameter);
  //=============================================================================================
  // HEX DUMP FUNCTIONS
  //=============================================================================================
  //---------------------------------------------------------------------------------------------
  // show single line of rom partition
  //---------------------------------------------------------------------------------------------
  void showMap(uint8_t partition, char* line);
  //---------------------------------------------------------------------------------------------
  // Show hex data in a single line
  //---------------------------------------------------------------------------------------------
  void showHexData(uint32_t address, uint8_t* data);
  //=============================================================================================
  // PROGRESS BAR FUNCTIONS
  //=============================================================================================
  //---------------------------------------------------------------------------------------------
  // show progess bar with initial value
  //---------------------------------------------------------------------------------------------
  void initProgressBar(int col, int row, uint32_t width, const char* title);
  //---------------------------------------------------------------------------------------------
  // update progess bar with current value
  //---------------------------------------------------------------------------------------------
  void updateProgressBar(uint32_t value, uint32_t max_value, const char* title);

  //=============================================================================================
  // CONSOLE SCREENS FUNCTIONS
  //=============================================================================================
  void showHelp();
  //---------------------------------------------------------------------------------------------
  // update main screen with all available data
  //---------------------------------------------------------------------------------------------
  void drawScreen(uint8_t brand, uint8_t model, uint8_t status,
                  uint32_t sd_size, uint8_t sd_type, uint8_t fat_type, const char* rom_dir,
                  uint32_t partition_size, uint8_t total_partitions,
                  String rom[], uint8_t total_roms, uint8_t program_mode);
};
#endif  //#define _ui_h