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
// float filtered_pitch, filtered_roll = 0.0f;

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

// // Gyro threshold in degrees per second. If the rotation speed is below this,
// // we consider the controller to be "steady". TUNE THIS VALUE.
// const float gyro_threshold = 1.0f;

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

// long MarioKartRun::applyExpoCurve(long inputValue, float expoFactor) {
//     // 1. Normalize the input to a -1.0 to 1.0 range
//     float normalizedInput = inputValue / 32768.0f;

//     // 2. Apply the exponential curve
//     // We use the absolute value, apply the power, then restore the original sign.
//     float curvedOutput = 0.0;
//     if (normalizedInput >= 0) {
//         curvedOutput = pow(normalizedInput, expoFactor);
//     } else {
//         curvedOutput = -pow(-normalizedInput, expoFactor);
//     }

//     // 3. Scale the output back up to the gamepad's range
//     return (long)(curvedOutput * 32767.0f);
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

//         // Use a complementary sensor fusion algorithm to update the orientation
//         float acc_roll = atan2(y, z) * 180.0 / M_PI;
//         float acc_pitch = atan2(-x, sqrt(y * y + z * z)) * 180.0 / M_PI;
//         filtered_pitch = alpha * (filtered_pitch + rx * dt_sec) + (1.0 - alpha) * acc_pitch;
//         filtered_roll  = alpha * (filtered_roll  + ry * dt_sec) + (1.0 - alpha) * acc_roll;

//         float steer_angle = filtered_roll;

//         const float deadzone = 5.0f;
//         long final_output = 0;

//         if (abs(ry) < gyro_threshold) {
//             // The gyro is quiet, so you're likely holding a line.
//             // Apply the deadzone for straight-line stability.
//             if (abs(steer_angle) < deadzone) {
//                 steer_angle = 0;
//             }
//             else {
//                 long linear_output = map(steer_angle, -45, 45, -32768, 32767);
//                 linear_output = constrain(linear_output, -32768, 32767);

//                 long final_output = linear_output;
//                 const float expoFactor = 1.5f;
//                 final_output = applyExpoCurve(linear_output, expoFactor);
//             }
//         }
//         else {
//             // Ignore inertia due to snap turns
//             // Map ry (gyro Y) to the full range of final_output
//             final_output = map(ry, -3, 3, -32768, 32767);
//             final_output = constrain(final_output, -32768, 32767);
//         }

//         // speed = map(speed, 0, 8, 0, 255); // Map the speed to the range of 0 to 255
//         speed = map(speed, 0, 8, -32768, 32767); // Map the speed to the range of 0 to 255
//         // gamepad->setAxes(x, y, 0, rx);
//         gamepad->setAxes(final_output, 0, 0, speed, speed);
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