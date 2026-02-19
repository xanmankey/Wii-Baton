// Calibrate the accelerometer on the Adafruit LSM9DS1
// Requires placing the sensor in 6 different orientations relative to gravity
// +x (sensor's x-axis points straight up), -x, +y, -y, +z, -z
// And uses a button press to confirm each orientation value

#include <Adafruit_LSM9DS1.h>

#define NEXT_ORIENTATION_BUTTON GPIO_NUM_0 // GPIO_NUM_0: Boot button on ESP32

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

// Variables to store average readings for each orientation
float accel_readings[6][3]; // [orientation_index][axis_index (0=X, 1=Y, 2=Z)]
float accel_offsets[3];     // [X_offset, Y_offset, Z_offset]

// Helper function to read and average samples for one orientation
// NOTE: this does not calculate slope, just offset since I don't need incredible accuracy, just general accuracy
void measureOrientation(int orientationIndex, const char* orientationName) {
    Serial.print(F("\nPlace sensor with ")); Serial.print(orientationName); Serial.println(F(" pointing UP."));
    Serial.println(F("Press and RELEASE the BOOT button to START measuring."));
    
    // Wait for button press to START measurement
    while (digitalRead(NEXT_ORIENTATION_BUTTON) == HIGH) {
        delay(10); // Debounce
    }
    delay(100); // Debounce
    while (digitalRead(NEXT_ORIENTATION_BUTTON) == LOW) { // Wait for button release
        delay(10); 
    }
    delay(100); // Debounce after release

    Serial.println(F("Measuring... Keep still!"));
    Serial.println(F("Press and RELEASE the BOOT button AGAIN when done."));

    long numSamples = 0;
    float sum_x = 0;
    float sum_y = 0;
    float sum_z = 0;

    // Continue measuring until the button is pressed again
    while (digitalRead(NEXT_ORIENTATION_BUTTON) == HIGH) { // Loop as long as button is NOT pressed
        lsm.read(); // Read new sensor data
        sensors_event_t a, m, g, temp;
        lsm.getEvent(&a, &m, &g, &temp); // Get event data (already scaled to m/s^2 by library)

        sum_x += a.acceleration.x;
        sum_y += a.acceleration.y;
        sum_z += a.acceleration.z;
        numSamples++;

        // No LED blinking during measurement
        if ((numSamples % 50) == 0) Serial.print('.'); // Print a dot every 50 samples for feedback
        delay(10); // Delay to avoid reading too fast
    }
    
    // Wait for button release after second press to ensure clean state for next measurement
    delay(100); 
    while (digitalRead(NEXT_ORIENTATION_BUTTON) == LOW) {
        delay(10);
    }
    delay(100); 

    if (numSamples == 0) {
        Serial.println(F("\nERROR: No samples collected! Retrying."));
        // Potentially recurse or handle error
        return; 
    }

    accel_readings[orientationIndex][0] = sum_x / numSamples;
    accel_readings[orientationIndex][1] = sum_y / numSamples;
    accel_readings[orientationIndex][2] = sum_z / numSamples;

    Serial.println(F("\nMeasurement Complete!"));
    Serial.print(orientationName); Serial.print(F(" UP -> Avg X: ")); Serial.print(accel_readings[orientationIndex][0], 3);
    Serial.print(F(", Avg Y: ")); Serial.print(accel_readings[orientationIndex][1], 3);
    Serial.print(F(", Avg Z: ")); Serial.println(accel_readings[orientationIndex][2], 3);
}


void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

    Serial.println(F("--- Accelerometer Calibration ---"));
    Serial.println(F("Place the ESP32 with the LSM9DS1 on a stable surface."));

    if (!lsm.begin()) {
        Serial.println(F("Failed to initialize IMU!"));
        while (1) delay(100); // Halt if IMU fails
    }
    pinMode(NEXT_ORIENTATION_BUTTON, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT); // Keep LED_BUILTIN pinmode for completeness, but not used for blinking during measure
    digitalWrite(LED_BUILTIN, LOW); // Ensure LED is off

    // Set desired sensor ranges BEFORE calibration
    // Adjust these if needed based on your application
    lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
    lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
    lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

    // --- Perform Measurements for all 6 Orientations ---
    measureOrientation(0, "+X"); // Index 0 for +X
    measureOrientation(1, "-X"); // Index 1 for -X
    measureOrientation(2, "+Y"); // Index 2 for +Y
    measureOrientation(3, "-Y"); // Index 3 for -Y
    measureOrientation(4, "+Z"); // Index 4 for +Z
    measureOrientation(5, "-Z"); // Index 5 for -Z

    // --- Calculate Accelerometer Offsets ---
    // offset_X = (avg_x_plus_x_up + avg_x_minus_x_up) / 2
    accel_offsets[0] = (accel_readings[0][0] + accel_readings[1][0]) / 2.0; // X offset
    accel_offsets[1] = (accel_readings[2][1] + accel_readings[3][1]) / 2.0; // Y offset
    accel_offsets[2] = (accel_readings[4][2] + accel_readings[5][2]) / 2.0; // Z offset

    Serial.println(F("\n--- Accelerometer Calibration Complete ---"));
    Serial.println(F("\nCopy and paste these values into your main sketch:"));
    Serial.print(F("const float ACCEL_OFFSET_X = ")); Serial.print(accel_offsets[0], 4); Serial.println(F(";"));
    Serial.print(F("const float ACCEL_OFFSET_Y = ")); Serial.print(accel_offsets[1], 4); Serial.println(F(";"));
    Serial.print(F("const float ACCEL_OFFSET_Z = ")); Serial.print(accel_offsets[2], 4); Serial.println(F(";"));

    Serial.println(F("\nThese values should be subtracted from your raw accelerometer readings (in m/s^2)"));
    Serial.println(F("before passing them to the sensor fusion algorithm."));
    Serial.println(F("Remember to also do Gyroscope and Magnetometer calibration!"));

    // Exit after setup
    Serial.println(F("Calibration sketch finished. Exiting."));
    exit(0);
}

void loop() {
    // This loop will not be reached due to exit(0) in setup()
}