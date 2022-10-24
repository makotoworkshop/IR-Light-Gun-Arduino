#include "MouseAbs.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  
  //  Mouse
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)  // 54
    0x09, 0x02,                    // USAGE (Mouse)
    0xA1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x85, 0x01,                    //     REPORT_ID (1)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
	/////////////////////////////////
    //0x09, 0x38,                  //     USAGE (Wheel)
    0x16, 0x00,0x00,               //     LOGICAL_MINIMUM (0) 0x00 -> minimum pour x et y (attention, caractère signée)
    0x26, 0xFF,0x7F,               //     LOGICAL_MAXIMUM (32767) 0x7F -> maximum pour x et y (attention, caractère signée)
    0x75, 0x10,                    //     REPORT_SIZE (16) -> Taille des valeurs en bits
    0x95, 0x02,                    //     REPORT_COUNT (2) -> nombre de valeurs (X et Y = 2)
    0x81, 0x02,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
};

Mouse_::Mouse_(void) : _buttons(0)
{
    static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
    HID().AppendDescriptor(&node);
}

void Mouse_::init(void) 
{
}

void Mouse_::report(void)
{
  uint8_t m[5];
  m[0] = _buttons;
  m[1] = Actual_X & 0xFF;
  m[2] = (Actual_X >> 8) & 0xFF;
  m[3] = Actual_Y & 0xFF;
  m[4] = (Actual_Y >> 8) & 0xFF;
  HID().SendReport(1, m, 5);
}

void Mouse_::press(uint8_t button) 
{
  _buttons |= button;
}

void Mouse_::release(uint8_t button)
{
  _buttons &= ~button;
}

Mouse_ Mouse;
