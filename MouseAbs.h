#include "HID.h"
#define MOUSE_LEFT 0x01
#define MOUSE_RIGHT 0x02
#define MOUSE_MIDDLE 0x04

class Mouse_
{
  private:
    uint8_t _buttons;

  public:
    unsigned int Actual_X = 0; // Position du curseur sur X
    unsigned int Actual_Y = 0; // Position du curseur sur X
    Mouse_(void);
    void init(void);
    void report(void);
    void press(uint8_t b = MOUSE_LEFT);   // press LEFT by default
    void release(uint8_t b = MOUSE_LEFT); // release LEFT by default
};
extern Mouse_ Mouse;
