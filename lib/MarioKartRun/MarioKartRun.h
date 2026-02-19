#include "GamepadDevice.h"

namespace MarioKartRun {
    void translateAnalog(GamepadDevice* gamepad, float x, float y, float z, float rx, float ry, float rz, float mx, float my, float mz);
    float armPumpTimeToSpeed(float time);
    float dtwToAcceleration();
    long applyExpoCurve(long inputValue, float expoFactor);
    // bool isRunning();
}