/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"

/* This sketch demonstrates multiple report USB HID.
 * Press button pin will move
 * - mouse toward bottom right of monitor
 * - send 'a' key
 *
 * Depending on the board, the button pin
 * and its active state (when pressed) are different
 */
//SYS Button GPIOS
#define POWER   21
#define HOME    7

// Define button GPIOs based on your configuration
#define DPAD_UP     8
#define DPAD_DOWN   9
#define DPAD_LEFT   10
#define DPAD_RIGHT  6
#define TRIGGER_L   34
#define TRIGGER_R   36
#define TRIGGER_L2  33
#define TRIGGER_R2  35
#define BUTTON_A    4
#define BUTTON_B    5
#define BUTTON_X    2
#define BUTTON_Y    3
#define BUTTON_STRT 12
#define BUTTON_SLCT 1
#define LSTICK_X    17
#define LSTICK_Y    16
#define RSTICK_X    13
#define RSTICK_Y    11

// Standard Gamepad HAT/DPAD Buttons
#define GAMEPAD_HAT_CENTERED    0  
#define GAMEPAD_HAT_UP          1  
#define GAMEPAD_HAT_UP_RIGHT    2  
#define GAMEPAD_HAT_RIGHT       3  
#define GAMEPAD_HAT_DOWN_RIGHT  4  
#define GAMEPAD_HAT_DOWN        5  
#define GAMEPAD_HAT_DOWN_LEFT   6  
#define GAMEPAD_HAT_LEFT        7  
#define GAMEPAD_HAT_UP_LEFT     8 

// Standard Gamepad Buttons Naming
#define GAMEPAD_BUTTON_A       0
#define GAMEPAD_BUTTON_B       1
#define GAMEPAD_BUTTON_X       3
#define GAMEPAD_BUTTON_Y       4
#define GAMEPAD_BUTTON_TL      6
#define GAMEPAD_BUTTON_TR      7
#define GAMEPAD_BUTTON_TL2     8
#define GAMEPAD_BUTTON_TR2     9
#define GAMEPAD_BUTTON_SELECT  10
#define GAMEPAD_BUTTON_START   11

//Stick Calibration variables
const int pins[] = {LSTICK_Y, LSTICK_X, RSTICK_Y, RSTICK_X};
int minValues[4];
int maxValues[4];

//set USB descriptors
#define USB_PRODUCT "Controller"
#define USB_MANUFACTURER "Jannis Industries"

hid_gamepad_report_t gp;
const int pin = 7;
bool activeState = false;

// Report ID
enum {
  RID_GAMEPAD = 1,
  RID_CONSUMER_CONTROL,
  RID_KEYBOARD,
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(RID_GAMEPAD)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL)),
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD))
};

// USB HID object.
Adafruit_USBD_HID usb_hid;

// the setup function runs once when you press reset or power the board
void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  Serial.begin(115200);

    // Set up button GPIOs with internal pull-ups
  pinMode(DPAD_UP, INPUT_PULLUP);
  pinMode(DPAD_DOWN, INPUT_PULLUP);
  pinMode(DPAD_LEFT, INPUT_PULLUP);
  pinMode(DPAD_RIGHT, INPUT_PULLUP);
  pinMode(TRIGGER_L, INPUT_PULLUP);
  pinMode(TRIGGER_R, INPUT_PULLUP);
  pinMode(TRIGGER_L2, INPUT_PULLUP);
  pinMode(TRIGGER_R2, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_X, INPUT_PULLUP);
  pinMode(BUTTON_Y, INPUT_PULLUP);
  pinMode(BUTTON_STRT, INPUT_PULLUP);
  pinMode(BUTTON_SLCT, INPUT_PULLUP);
  pinMode(HOME, INPUT_PULLUP);
  pinMode(POWER, INPUT_PULLUP);

  //Stickstuff
      // Initialize min and max values
    for (int i = 0; i < 4; i++) {
        minValues[i] = 2000;  // Start with a high value, expected value around 1800
        maxValues[i] = 8000;  // Start with a low value, expected value 8191
    }


  // Set up HID
  TinyUSBDevice.setManufacturerDescriptor(USB_MANUFACTURER);
  TinyUSBDevice.setProductDescriptor(USB_PRODUCT);
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB HID Composite");
  usb_hid.begin();

  // Wait for USB connection
  while (!TinyUSBDevice.mounted()) {
    delay(100);
  }

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  Serial.println("ESP32-S2 USB Gamepad Ready!");
  Serial.println("Adafruit TinyUSB HID Composite example");
}

int valparser(int val){ //parses the input value for the sticks to not exceed -127 or 127 and adds a deadzone to the middle
  if (val <= -127) {return -127;}
  else if (val >= 127) {return 127;}
  else if (val <= 20 && val >= -20) {return 0;}
  else {return val;}
}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's HOMEground
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }
  /*-----Gamepad-----*/
   // Read buttons and update bitmask
  uint16_t buttons = 0;
  buttons |= (!digitalRead(BUTTON_A)   << GAMEPAD_BUTTON_A);
  buttons |= (!digitalRead(BUTTON_B)   << GAMEPAD_BUTTON_B);
  buttons |= (!digitalRead(BUTTON_X)   << GAMEPAD_BUTTON_X);
  buttons |= (!digitalRead(BUTTON_Y)   << GAMEPAD_BUTTON_Y);
  buttons |= (!digitalRead(TRIGGER_L)  << GAMEPAD_BUTTON_TL);
  buttons |= (!digitalRead(TRIGGER_R)  << GAMEPAD_BUTTON_TR);
  buttons |= (!digitalRead(TRIGGER_L2)  << GAMEPAD_BUTTON_TL2);
  buttons |= (!digitalRead(TRIGGER_R2)  << GAMEPAD_BUTTON_TR2);
  buttons |= (!digitalRead(BUTTON_STRT)  << GAMEPAD_BUTTON_START);
  buttons |= (!digitalRead(BUTTON_SLCT)  << GAMEPAD_BUTTON_SELECT);

  // Determine D-pad hat switch position
  uint8_t hat_switch = GAMEPAD_HAT_CENTERED;
  
  bool up = !digitalRead(DPAD_UP);
  bool down = !digitalRead(DPAD_DOWN);
  bool left = !digitalRead(DPAD_LEFT);
  bool right = !digitalRead(DPAD_RIGHT);

  if (up && left)       hat_switch = GAMEPAD_HAT_UP_LEFT;
  else if (up && right) hat_switch = GAMEPAD_HAT_UP_RIGHT;
  else if (down && left) hat_switch = GAMEPAD_HAT_DOWN_LEFT;
  else if (down && right) hat_switch = GAMEPAD_HAT_DOWN_RIGHT;
  else if (up)          hat_switch = GAMEPAD_HAT_UP;
  else if (down)        hat_switch = GAMEPAD_HAT_DOWN;
  else if (left)        hat_switch = GAMEPAD_HAT_LEFT;
  else if (right)       hat_switch = GAMEPAD_HAT_RIGHT;

  //Gamepad Sticks read and calibrate
  for (int i = 0; i < 4; i++) {
        int value = analogRead(pins[i]);  // Read 13-bit analog value
        
        // Update min
        if (value < minValues[i]) minValues[i] = value;
        if (value > maxValues[i]) maxValues[i] = value;
  }

   // Map min and max to -127 to 128
  int mappedLY = map(analogRead(LSTICK_Y), 620+minValues[0], maxValues[0], -127, 127);
  int mappedLX = map(analogRead(LSTICK_X), 620+minValues[1], maxValues[1], -127, 127);
  int mappedRY = map(analogRead(RSTICK_Y), 620+minValues[2], maxValues[2], -127, 127);
  int mappedRX = map(analogRead(RSTICK_X), 620+minValues[3], maxValues[3], -127, 127);

  gp.x = valparser(-mappedLX); //X LSTICK 
  gp.y = valparser(-mappedLY);  //Y LSTICK
  gp.z = valparser(mappedRX);   //X RSTICK
  gp.rz = valparser(mappedRY);  //Y RSTICK
  gp.rx = 0;  //R TRIGGER 
  gp.ry = 0;  //L TRIGGER

  // Update the gamepad report
  gp.hat = hat_switch;
  gp.buttons = buttons;

  // Send USB HID report
  usb_hid.sendReport(RID_GAMEPAD, &gp, sizeof(gp));

  // Debug Output
  Serial.print("Buttons: ");
  Serial.print(buttons, BIN);
  Serial.print(" | Hat: ");
  Serial.println(hat_switch);

  /*-----Controlls-----*/
  bool pwr = !digitalRead(POWER);
  int ctrl = 0;

      if (pwr) {
       //send power button down (0x0030)
      ctrl = 0x0030;
    }

  usb_hid.sendReport16(RID_CONSUMER_CONTROL, ctrl);

  /*-----KEYBOARD-----*/
  bool home = !digitalRead(HOME);
  

  if (home) {
    // Send the HOME keycode
    uint8_t keycode[6] = {HID_KEY_GUI_LEFT, HID_KEY_BACKSPACE};//Backspace  KEYBOARD_MODIFIER_RIGHTGUI
    usb_hid.keyboardReport(RID_KEYBOARD,0, keycode);
    //delay(500);
    //uint8_t keycode1[6] = {HID_KEY_BACKSPACE};//Backspace
    //usb_hid.keyboardReport(RID_KEYBOARD,0, keycode1);
  } else usb_hid.keyboardRelease(RID_KEYBOARD);

  /*-----GENERAL-----*/
  // delay for stability
  static uint32_t ms = 0;
  if (millis() - ms > 5) {
    ms = millis();
  }
}
