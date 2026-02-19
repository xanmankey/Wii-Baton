// #include "MarioKartRun.h"
// #include <Arduino.h>
// #include <SensorFusion.h>

// #define G 9.81 // Gravitational constant in m/s^2
// #define ROTATION_THRESHOLD 245
// #define MAG_THRESHOLD 4

// // Use Madgwick sensor fusion to get the orientation relative to a global reference frame
// static MadgwickFilter sensorFusionFilter;
// float deltat;
// float prevUpdateTime = 0.0f;
// float t;
// float dt_sec;
// // float tilt;
// float intervalTime = 0.0f;
// xyz_t accelerometer, acc_t;
// xyz_t gyro, gyro_t;
// xyz_t mag;
// xyz_t grav_t;
// Quaternion orientation;
// float a_mag;
// float prev_a_mag = 0.0f;

// // RunState currentRunState = RunState::IDLE;
// float speed = 0.0f;
// float acceleration = 0.0f;
// int accelCounter = 0;
// int numAccelerations = 0;
// float alpha = 0.98;

// // Note that the magnitude of the idle states only ends up being around 9.731 due to noise
// #define ARM_IDLE_X 8.45
// #define ARM_IDLE_Y 0.5
// #define ARM_IDLE_Z -4.8
// #define ARM_IDLE_MAG 9.81

// #define MINUS_BUTTON GPIO_NUM_27
// #define HOME_BUTTON GPIO_NUM_26
// #define PLUS_BUTTON GPIO_NUM_16
// #define NUM_BUTTONS 8
// byte buttonPins2[NUM_BUTTONS] = {MINUS_BUTTON, HOME_BUTTON, PLUS_BUTTON};
// byte physicalButtons2[NUM_BUTTONS] = {11, 12, 13, 9, 4, 5, 2, 1};

// // Regardless of orientation or speed, the acceleration values still have the same SHAPE when running, just with different magnitudes for each acceleration
// // The key to comparing accelerometer values to running motion is to compare the SHAPE of the time series on a per motion basis, meaning:
// // - Check that the data follows a SINUSODAL pattern (this can be done by checking state management and tracking peaks and directional changes)
// // - Once data for a pattern has been accumulated, dynamic time warping (DTW) can be used to compare the time series to a known time series, using the difference as the amount of acceleration (accounting for a MoE)
// // - After, check that the data has a consistent rhythm (or speed; this can be tracked by scaling the time in between each motion appropriately)

// // If the data doesn't match within the defined MoE or the rhythm is not consistent between arm pumps, then it will come at acceleration and speed costs 

// // *************** TLDR: Consistent arm pump -> Consistent acceleration, Speed in between pumps -> Kart Speed *********************

// float MarioKartRun::armPumpTimeToSpeed(float time) {
//     return 0.0f;
// }

// float MarioKartRun::dtwToAcceleration() {
//     // Acceleration represents the percent of the next pump that the acceleration button will be pressed
//     return 0.0f;
// }

// void MarioKartRun::translateAnalog(GamepadDevice* gamepad, float x, float y, float z, float rx, float ry, float rz, float mx, float my, float mz) {
//     // Set the axes values in the gamepad
//     // Serial.print("X: "); Serial.print(x);
//     // Serial.print(" Y: "); Serial.print(y);
//     // Serial.print(" Z: "); Serial.print(z);
//     // Serial.print(" RX: "); Serial.print(rx);
//     // Serial.print(" RY: "); Serial.print(ry);
//     // Serial.print(" RZ: "); Serial.println(rz);
//     // Serial.print("MX: "); Serial.print(mx);
//     // Serial.print(" MY: "); Serial.print(my);
//     // Serial.print(" MZ: "); Serial.println(mz);
//     // Serial.print("\n");

//     t = millis();
//     deltat = t - prevUpdateTime; // Optionally for normalization
//     prevUpdateTime = t;
//     dt_sec = deltat / 1000.0f;

//     // Use the Madgwick sensor fusion algorithm to update the orientation
//     float acc_roll = atan2(y, z) * 180.0 / M_PI;
//     float acc_pitch = atan2(-x, sqrt(y * y + z * z)) * 180.0 / M_PI;
//     gyro = {rx, ry, rz};
//     accelerometer = {x, y, z};
//     mag = {mx, my, mz};
//     // Experiment with the beta value (larger beta values add weight to linear accelerometer and remove weight on gyroscope)
//     // sensorFusionFilter.setBeta(0.1);
//     sensorFusionFilter.setBeta(500);
//     orientation = sensorFusionFilter.update(gyro, accelerometer, mag, deltat);

//     // Use the Quaternion transformation matrix to change the basis for the acceleration and gyroscope data
//     acc_t = orientation.rotate(accelerometer);
//     // gyro_t = orientation.rotate(gyro);
//     grav_t = orientation.rotate({0.0f, 0.0f, G});

//     x = acc_t.x - grav_t.x;
//     y = acc_t.y - grav_t.y;
//     z = acc_t.z - grav_t.z;
//     a_mag = sqrt(x * x + y * y + z * z);

//     // Check difference to ensure constant motion
//     // If START button is being pressed (held), you're in menu mode
//     // No A button presses or tilts will occur, use the DPAD and the L button to navigate
//     if (!gamepad->isPressed(physicalButtons2[1])) {
//         if (abs(a_mag - prev_a_mag) > 0.5f) {
//             // The A button is BUTTON_3 in the HID report descriptor
//             gamepad->press(BUTTON_3);
//         }
//         else {
//             gamepad->release(BUTTON_3);
//         }

//         // Calculate speed based on the magnitude of the difference between the current and previous accelerometer values
//         speed = abs(a_mag - prev_a_mag);
//         prev_a_mag = a_mag;

//         // Print out the time series
//         // Serial.print("X: "); Serial.print(x);
//         // Serial.print(", Y: "); Serial.print(y);
//         // Serial.print(", Z: "); Serial.print(z);
//         // Serial.print(", A: "); Serial.print(a_mag);
//         // Serial.print(", t: "); Serial.print(t);
//         // Serial.print(", speed: "); Serial.print(speed);
//         // Serial.print("\n");

//         // rx = gyro_t.x;
//         // ry = gyro_t.y;
//         // rz = gyro_t.z;

//         // rx = map(fmod(orientation.calculateRollDegrees(), 90.0), -90, 90, -32768, 32768);
//         rx = orientation.calculateRollDegrees();
//         // Apply a low-pass filter
//         rx = (low_pass_alpha * rx) + (1.0 - low_pass_alpha) * prev_rx;

//         // Huge values for pitch -> gimbal lock, when 2 of 3 axes align so you get huge unstable values
//         ry = orientation.calculatePitchDegrees();
//         ry = (low_pass_alpha * ry) + (1.0 - low_pass_alpha) * prev_ry;
//         // prev_rx = rx;
//         // prev_ry = ry;

//         rz = orientation.calculateYawDegrees();
//         // ry = map(fmod(orientation.calculatePitchDegrees(), 90.0), -90, 90, -32768, 32768);
//         // rz = map(fmod(orientation.calculateYawDegrees(), 90.0), -90, 90, -32768, 32768);

//         Serial.print("X: "); Serial.print(x);
//         Serial.print(" Y: "); Serial.print(y);
//         Serial.print(" Z: "); Serial.print(z);
//         Serial.print(" RX: "); Serial.print(rx);
//         Serial.print(" RY: "); Serial.print(ry);
//         Serial.print(" RZ: "); Serial.print(rz);
//         Serial.print("\n");

//         // Translate the axes values to the range of -1 to 1
//         // SDL is from -32768 to 32767, but is mapped by Dolphin to 0-255 for the gamepad
//         // My hunch is that each direction has an on/off state, and that the Z axis corresponds to the trigger.
//         x = map(x, -G * 2, G * 2, -32768, 32768);
//         // Y axis is inverted for SDL controllers; if you don't want this inverted, remove the negative sign
//         y = -map(y, -G * 2, G * 2, -32768, 32768);
//         // z = map(z, -G * 2, G * 2, -32768, 32768);

//         // rx = 32767;
//         // Serial.print("RX: "); Serial.print(rx);
//         // Serial.print(", RY: "); Serial.print(y);
//         // Serial.print(", RZ: "); Serial.print(z);
//         // Serial.print("\n");
        
//         ry = 32767;

//         // Send the tilt data to the gamepad
//         // float rx = map(rx, -90, 90, -32768, 32767);
//         // float ry = map(ry, -90, 90, -32768, 32767);
//         // if (abs(rx) < 500) {
//         //     rx = 0;
//         // }
//         // if (abs(ry) < 500) {
//         //     ry = 0;
//         // }
//         // Goal is to be able to detect the orientation of the LSM9DS1 relative to flat
//         // With right correlating with right and left correlating with left 
//         speed = map(speed, 0, 8, 0, 255); // Map the speed to the range of 0 to 255
//         // gamepad->setAxes(x, y, 0, rx);
//         gamepad->setAxes(rx, ry, 0, 32767);
//     }
//     else {
//         byte state = digitalRead(buttonPins2[0]);
//         // Serial.print("State: "); Serial.print(state);
//         if (state == LOW) {
//             // In menu mode, the L button is used to navigate forward (like A)
//             gamepad->press(BUTTON_3);
//         }
//         else {
//             gamepad->release(BUTTON_3);
//         }
//         // In menu mode, set axes to be neutral
//         gamepad->setAxes(0, 0, 0, 0);
//     }
// }