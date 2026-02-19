// Strapping pins on an ESP32 are GPIO0, GPIO2, GPIO4, GPIO5, GPIO12, and GPIO15.
// DON'T use pin 15 because of boot
// DON'T use RX0 or TX0 if you want to use flash
// VN and VP pins DO NOT have pullup resistors; neither do 35 and 34 since they are input only
// Press and hold EN to restart the program
// If you make changes to the bluetooth code, make sure to FORGET THE BLUETOOTH DEVICE, as it caches on the host end and does not update
// try 0x400805e4 -> second stage of bootloader; right now it's hanging on the bootloader
// If the setup function calls multiple times, that means there's a crash in the setup function
// If you get hardware failures, one thing to check is to make sure that you have the right board selected
// That means check against HARDWARE (i.e. does your board contain PSRAM? Ethernet?)
// Pressing the upload button will also build

// Hold Boot button press EN and then release Boot button to reset (in case code upload to flash isn't working)

/*
// __attribute__((packed)) means no padding between struct members
struct __attribute__((packed)) gamepad_report_t {
  uint8_t buttons; // B, HOME, PLUS, MINUS, UP, DOWN, LEFT, RIGHT
  // acc
  int8_t x_axis;
  int8_t y_axis;
  int8_t z_axis;
  // gyro
  int8_t rx_axis;
  int8_t ry_axis;
  int8_t rz_axis;
};

// HID report descriptor
uint8_t gamepad_report_descriptor[] = {
  0x05, 0x01, // USAGE_PAGE (Generic Desktop)
  0x09, 0x05, // Usage (Game Pad)
  0xA1, 0x01, // COLLECTION (Application)
    0xA1, 0x00, //   COLLECTION (Physical)
      0x05, 0x09, //     USAGE_PAGE (Button)
      0x19, 0x01, //     USAGE_MINIMUM (Button 1)
      0x29, 0x08, //     USAGE_MAXIMUM (Button 8) // Defines 8 buttons
      0x15, 0x00, //     LOGICAL_MINIMUM (0)
      0x25, 0x01, //     LOGICAL_MAXIMUM (1) // Button values are 0 or 1
      0x75, 0x01, //     REPORT_SIZE (1) // Each button is 1 bit
      0x95, 0x08, //     REPORT_COUNT (8) // We have 8 buttons
      0x81, 0x02, //     INPUT (Data,Var,Abs) // Pack 8 1-bit values into 1 byte

      0x05, 0x01, //     USAGE_PAGE (Generic Desktop) // Switch back to Generic Desktop page for axes
      0x09, 0x30, //     USAGE (X)
      0x09, 0x31, //     USAGE (Y)
      0x09, 0x32, //     USAGE (Z) // Defines X, Y, Z axes
      0x15, 0x81, //     LOGICAL_MINIMUM (-127) // Signed 8-bit minimum
      0x25, 0x7F, //     LOGICAL_MAXIMUM (127) // Signed 8-bit maximum
      0x75, 0x08, //     REPORT_SIZE (8) // Each axis is 8 bits
      0x95, 0x03, //     REPORT_COUNT (3) // We have 3 axes
      0x81, 0x02, //     INPUT (Data,Var,Abs) // Send 3 8-bit values

      0x09, 0x33, //     USAGE (Rx)
      0x09, 0x34, //     USAGE (Ry)
      0x09, 0x35, //     USAGE (Rz) // Defines Rx, Ry, Rz axes
      0x15, 0x81, //     LOGICAL_MINIMUM (-127) // Signed 8-bit minimum
      0x25, 0x7F, //     LOGICAL_MAXIMUM (127) // Signed 8-bit maximum
      0x75, 0x08, //     REPORT_SIZE (8) // Each axis is 8 bits
      0x95, 0x03, //     REPORT_COUNT (3) // We have 3 axes
      0x81, 0x02, //     INPUT (Data,Var,Abs) // Send 3 8-bit values

    0xC0,       //   END_COLLECTION (Physical)
  0xC0        // END_COLLECTION (Application)
};
*/

// *************** CALIBRATION NOTES ****************
// To calibrate, run the following calibration examples and follow the instructions in the serial monitor one at a time for each sensor
// I just converted the examples to headers and included them in the main file and built the main file
// NOTE: calibration needs to be done PER SENSOR

// #include "AccelCal.h"
const float ACCEL_OFFSET_X = -0.0275;
const float ACCEL_OFFSET_Y = -0.0177;
const float ACCEL_OFFSET_Z = -0.1620;

// #include "GyroCal.h"
const float GYRO_OFFSET_X = 0.0184;
const float GYRO_OFFSET_Y = 0.0242;
const float GYRO_OFFSET_Z = 0.0215;

// Magnetometer calibration uses the https://github.com/PaulStoffregen/MotionCal calibration software
// In addition to an adafruit script
// Mine was compiled from source at /home/xanmankey/Programming/Hardware/GameController/MotionCal
// #include "MagCal.h"
const float MAG_OFFSET_X = 24.32;
const float MAG_OFFSET_Y = 17.67;
const float MAG_OFFSET_Z = -17.91;
const float MAG_SOFTIRON_MATRIX[3][3] = {{1.004, 0.044, 0.010},
                    {0.045, 0.972, 0.003},
                    {0.010, 0.003, 1.026}};
const float MAG_FIELD = 52.19;

#include <Arduino.h>
#include <Adafruit_LSM9DS1.h>
#include <GamepadDevice.h>
#include <BleCompositeHID.h>
#include "MarioKartRun.h"
#include <WiFi.h>

#define NUM_BUTTONS 16 // 8 buttons that the user can actually press, but there are 8 more for flexibility in motion control mapping
// #define LSM on

#define DEBUG_BUTTON GPIO_NUM_0 // Boot

// I2C
#define SDA GPIO_NUM_21 // SDA
#define SCL GPIO_NUM_22 // SCL

// LEDs
// Left
#define LED_1 GPIO_NUM_13
#define LED_2 GPIO_NUM_12
// Right
#define LED_3 GPIO_NUM_2
#define LED_4 GPIO_NUM_4 // LED 4

#define POWER GPIO_NUM_34 // Closest pin to power

// Doing one empty pin in between for spacing
#define MINUS GPIO_NUM_27
#define HOME GPIO_NUM_26
#define PLUS GPIO_NUM_16

#define B GPIO_NUM_17

// Left
#define UP GPIO_NUM_33
#define LEFT GPIO_NUM_32
// Right
#define RIGHT GPIO_NUM_18
#define DOWN GPIO_NUM_23

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
BleCompositeHID compositeHID("Wii Baton", "plugNplay", 100);
GamepadDevice* gamepad;
GamepadConfiguration bleGamepadConfig;

byte previousButtonStates[NUM_BUTTONS];
byte currentButtonStates[NUM_BUTTONS];
byte buttonPins[NUM_BUTTONS] = {MINUS, HOME, PLUS, B, UP, LEFT, RIGHT, DOWN};
// byte physicalButtons[NUM_BUTTONS] = {1, 2, 3, 4, 5, 6, 7, 8};
// Ordered by Dolphin's naming conventions
// UP = Button N = 6, LEFT = Button W = 3, RIGHT = Button E = 2, DOWN = Button S = 1, B = 9,
// MINUS = Button 1 = ?, HOME = Button 2 = 3, PLUS = Button 3, B = Button 4
byte physicalButtons[NUM_BUTTONS] = {11, 12, 13, 9, 4, 5, 2, 1};

float x, y, z, rx, ry, rz, mx, my, mz, tmp_mx, tmp_my, tmp_mz;
float test_mag;

void OnPlayerIndicatorChanged(uint8_t playerIndicator)
{
  Serial.println("Player indicator changed to: " + String(playerIndicator));
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, HIGH);
  // if (playerIndicator == 0) {
  //   digitalWrite(LED_1, HIGH);
  // }
  // else if (playerIndicator == 1) {
  //   digitalWrite(LED_1, HIGH);
  //   digitalWrite(LED_2, HIGH);
  // }
  // else if (playerIndicator == 2) {
  //   digitalWrite(LED_1, HIGH);
  //   digitalWrite(LED_2, HIGH);
  //   digitalWrite(LED_3, HIGH);
  // }
  // else if (playerIndicator == 3) {
  //   digitalWrite(LED_1, HIGH);
  //   digitalWrite(LED_2, HIGH);
  //   digitalWrite(LED_3, HIGH);
  //   digitalWrite(LED_4, HIGH);
  // }
}

void setup2()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  Serial.println("The device started, now you can pair it with your phone!");


  Serial.begin(115200);
  // Disable wifi as a power optimization
  WiFi.mode(WIFI_OFF);

  pinMode(DEBUG_BUTTON, INPUT_PULLUP);

  // I2C
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

  // Set up accelerometer/gyroscope
  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  Serial.println("Starting...");


  // DISABLED LSM FOR TESTING PCB; MAKE SURE TO REENABLE
  /*
  // if (!lsm.begin())
  // {
  //   Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
  //   exit(1);
  // }

  // If not getting good accelerometer data, try changing the range and data rate
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G, lsm.LSM9DS1_ACCELDATARATE_10HZ);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  */

  // INPUT_PULLUP means off by default and on when pressed
  pinMode(POWER, INPUT_PULLUP);
  // Power button is in series with battery power supply, and so if you press the button it will disconnect the battery
  pinMode(MINUS, INPUT_PULLUP);
  pinMode(HOME, INPUT_PULLUP);
  pinMode(PLUS, INPUT_PULLUP);
  
  pinMode(UP, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);

  pinMode(B, INPUT_PULLUP);

  // Lights
  // OUTPUT mode, since we are sending voltage to the LEDs dependent on the signal
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  // Initialize the gamepad
  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepadConfig.setButtonCount(NUM_BUTTONS);
  // None of the rotational axes are used for mario kart wii; if you want to use them (i.e. for a different game) you can enable them
  // The rx axis is used for a gecko code analog value
  bleGamepadConfig.setWhichAxes(true, true, false, true, false, false, false, false);
  bleGamepadConfig.setIncludePlayerIndicators(true);
  // bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
  // Some non-Windows operating systems and web based gamepad testers don't like min axis set below 0, so 0 is set by default
  bleGamepadConfig.setAxesMin(-32768); // -32768 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
  // bleGamepadConfig.setAxesMin(0x00); // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
  bleGamepadConfig.setAxesMax(32767); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal 
  
  gamepad = new GamepadDevice(bleGamepadConfig); // Simulation controls, special buttons and hats 2/3/4 are disabled by default
  // Set LEDs based on controller index
  FunctionSlot<uint8_t> playerIndicatorChangedSlot(OnPlayerIndicatorChanged);
  gamepad->onPlayerIndicatorChanged.attach(playerIndicatorChangedSlot);

  compositeHID.addDevice(gamepad);
  compositeHID.begin();
  gamepad->setAxes(); // Reset all axes to zero before reading
  // NOTE: setting player indicator to 0 (Player 1) by default
  OnPlayerIndicatorChanged(0);
}

void loop2()
{
    if (compositeHID.isConnected())
    {
      // Buttons
      for (byte currentIndex = 0; currentIndex < NUM_BUTTONS; currentIndex++)
      {
          currentButtonStates[currentIndex] = digitalRead(buttonPins[currentIndex]);
          if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex])
          {
              if (currentButtonStates[currentIndex] == LOW)
              {
                  gamepad->press(physicalButtons[currentIndex]);
              }
              else
              {
                  gamepad->release(physicalButtons[currentIndex]);
              }
          }
      }

      if (currentButtonStates != previousButtonStates)
      {
          for (byte currentIndex = 0; currentIndex < NUM_BUTTONS; currentIndex++)
          {
              previousButtonStates[currentIndex] = currentButtonStates[currentIndex];
          }

      }

      // Axes
      // Get the accelerometer and gyroscope data
      // Use calibration offsets
      lsm.read();
      sensors_event_t a, m, g, temp;
      lsm.getEvent(&a, &m, &g, &temp);
      x = a.acceleration.x - ACCEL_OFFSET_X;
      y = a.acceleration.y - ACCEL_OFFSET_Y;
      z = a.acceleration.z - ACCEL_OFFSET_Z;
      test_mag = sqrt(x * x + y * y + z * z);
      // Serial.print("X: "); Serial.print(x);
      // Serial.print(", Y: "); Serial.print(y);
      // Serial.print(", Z: "); Serial.print(z);
      rx = g.gyro.x - GYRO_OFFSET_X;
      ry = g.gyro.y - GYRO_OFFSET_Y;
      rz = g.gyro.z - GYRO_OFFSET_Z;

      // For magnetometer, perform matrix multiplication
      tmp_mx = m.magnetic.x - MAG_OFFSET_X;
      tmp_my = m.magnetic.y - MAG_OFFSET_Y;
      tmp_mz = m.magnetic.z - MAG_OFFSET_Z;

      mx = MAG_SOFTIRON_MATRIX[0][0] * tmp_mx + MAG_SOFTIRON_MATRIX[0][1] * tmp_my + MAG_SOFTIRON_MATRIX[0][2] * tmp_mz + MAG_OFFSET_X;
      my = MAG_SOFTIRON_MATRIX[1][0] * tmp_mx + MAG_SOFTIRON_MATRIX[1][1] * tmp_my + MAG_SOFTIRON_MATRIX[1][2] * tmp_mz + MAG_OFFSET_Y;
      mz = MAG_SOFTIRON_MATRIX[2][0] * tmp_mx + MAG_SOFTIRON_MATRIX[2][1] * tmp_my + MAG_SOFTIRON_MATRIX[2][2] * tmp_mz + MAG_OFFSET_Z;

      // Serial.print(", RX: "); Serial.print(rx);
      // Serial.print(", RY: "); Serial.print(ry);
      // Serial.print(", RZ: "); Serial.print(rz);
      // Serial.print(", MX: "); Serial.print(mx);
      // Serial.print(", MY: "); Serial.print(my);
      // Serial.print(", MZ: "); Serial.print(mz);
      // Serial.print(", Mag: "); Serial.println(test_mag);

      // Send the data to the gamepad
      // gamepad->setAxes(x, y, z, rx, ry, rz);
      MarioKartRun::translateAnalog(gamepad, x, y, z, rx, ry, rz, mx, my, mz);
      gamepad->sendGamepadReport();
  }
  delay(20);
}