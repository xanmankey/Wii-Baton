#include "MarioKartRun.h"
#include <Arduino.h>
#include <SensorFusion.h>

#define G 9.81 // Gravitational constant in m/s^2
#define ROTATION_THRESHOLD 245
#define MAG_THRESHOLD 4

// Use Madgwick sensor fusion to get the orientation relative to a global reference frame
static MadgwickFilter sensorFusionFilter;
float deltat;
float prevUpdateTime = 0.0f;
float t;
float dt_sec;
// float tilt;
float intervalTime = 0.0f;
xyz_t accelerometer, acc_t;
xyz_t gyro, gyro_t;
xyz_t mag;
xyz_t grav_t;
Quaternion orientation;
float a_mag;
float prev_a_mag = 0.0f;
float filtered_pitch, filtered_roll = 0.0f;

// RunState currentRunState = RunState::IDLE;
float speed = 0.0f;
float acceleration = 0.0f;
int accelCounter = 0;
int numAccelerations = 0;
// Alpha represents the weight given to the gyroscope data versus the accelerometer data in the complementary filter
float alpha = 0.98;

// Note that the magnitude of the idle states only ends up being around 9.731 due to noise
#define ARM_IDLE_X 8.45
#define ARM_IDLE_Y 0.5
#define ARM_IDLE_Z -4.8
#define ARM_IDLE_MAG 9.81

#define MINUS_BUTTON GPIO_NUM_27
#define HOME_BUTTON GPIO_NUM_26
#define PLUS_BUTTON GPIO_NUM_16
#define NUM_BUTTONS 8
byte buttonPins2[NUM_BUTTONS] = {MINUS_BUTTON, HOME_BUTTON, PLUS_BUTTON};
byte physicalButtons2[NUM_BUTTONS] = {11, 12, 13, 9, 4, 5, 2, 1};

// Regardless of orientation or speed, the acceleration values still have the same SHAPE when running, just with different magnitudes for each acceleration
// The key to comparing accelerometer values to running motion is to compare the SHAPE of the time series on a per motion basis, meaning:
// - Check that the data follows a SINUSODAL pattern (this can be done by checking state management and tracking peaks and directional changes)
// - Once data for a pattern has been accumulated, dynamic time warping (DTW) can be used to compare the time series to a known time series, using the difference as the amount of acceleration (accounting for a MoE)
// - After, check that the data has a consistent rhythm (or speed; this can be tracked by scaling the time in between each motion appropriately)

// If the data doesn't match within the defined MoE or the rhythm is not consistent between arm pumps, then it will come at acceleration and speed costs 

// *************** TLDR: Consistent arm pump -> Consistent acceleration, Speed in between pumps -> Kart Speed *********************

float MarioKartRun::armPumpTimeToSpeed(float time) {
    return 0.0f;
}

float MarioKartRun::dtwToAcceleration() {
    // Acceleration represents the percent of the next pump that the acceleration button will be pressed
    return 0.0f;
}

long MarioKartRun::applyExpoCurve(long inputValue, float expoFactor) {
    // 1. Normalize the input to a -1.0 to 1.0 range
    float normalizedInput = inputValue / 32768.0f;

    // 2. Apply the exponential curve
    // We use the absolute value, apply the power, then restore the original sign.
    float curvedOutput = 0.0;
    if (normalizedInput >= 0) {
        curvedOutput = pow(normalizedInput, expoFactor);
    } else {
        curvedOutput = -pow(-normalizedInput, expoFactor);
    }

    // 3. Scale the output back up to the gamepad's range
    return (long)(curvedOutput * 32767.0f);
}

// TODO: figure out a way to go from turning to side to side, ignoring the central deadzone
// TODO: figure out how to make the speed adjustments work properly
// Goal is to get it to a place where I can consistently win races, FAST

void MarioKartRun::translateAnalog(GamepadDevice* gamepad, float x, float y, float z, float rx, float ry, float rz, float mx, float my, float mz) {
    t = millis();
    deltat = t - prevUpdateTime;
    prevUpdateTime = t;
    dt_sec = deltat / 1000.0f;

    // Convert from rad to degrees
    rx = rx * 180.0 / M_PI;
    ry = ry * 180.0 / M_PI;
    rz = rz * 180.0 / M_PI;

    a_mag = sqrt(x * x + y * y + z * z);

    if (!gamepad->isPressed(physicalButtons2[1])) {
        if (abs(a_mag - prev_a_mag) > 0.5f) {
            gamepad->press(BUTTON_3);
        }
        else {
            gamepad->release(BUTTON_3);
        }
        prev_a_mag = a_mag;

        // Use a complementary sensor fusion algorithm to update the orientation
        float acc_roll = atan2(y, z) * 180.0 / M_PI;
        float acc_pitch = atan2(-x, sqrt(y * y + z * z)) * 180.0 / M_PI;
        
        // Pitch should use the Y-axis gyroscope (ry)
        // Roll should use the X-axis gyroscope (rx)
        filtered_pitch = alpha * (filtered_pitch + ry * dt_sec) + (1.0 - alpha) * acc_pitch;
        filtered_roll  = alpha * (filtered_roll  + rx * dt_sec) + (1.0 - alpha) * acc_roll;

        float steer_angle = filtered_roll;

        const float deadzone = 7.5f;
        if (abs(steer_angle) < deadzone) {
            steer_angle = 0;
        }

        long linear_output = map(steer_angle, -45, 45, -32768, 32767);
        linear_output = constrain(linear_output, -32768, 32767);

        const float expoFactor = 1.8f; 
        long final_output = applyExpoCurve(linear_output, expoFactor);

        speed = map(abs(a_mag - prev_a_mag), 0, 8, -32768, 32767);
        gamepad->setAxes(final_output, 0, 0, speed, speed);
    }
    else {
        byte state = digitalRead(buttonPins2[0]);
        // Serial.print("State: "); Serial.print(state);
        if (state == LOW) {
            // In menu mode, the L button is used to navigate forward (like A)
            gamepad->press(BUTTON_3);
        }
        else {
            gamepad->release(BUTTON_3);
        }
        // In menu mode, set axes to be neutral
        gamepad->setAxes(0, 0, 0, 0);
    }
}