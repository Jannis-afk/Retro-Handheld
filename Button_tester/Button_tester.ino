// List of button input pins on ESP32-S2
const int inputPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 21, 40, 33, 34, 35, 36, 37, 38, 39};  
const int numPins = sizeof(inputPins) / sizeof(inputPins[0]);

// ADC Pins to read voltage (0-256)
const int adcPins[] = {16, 17, 11, 13};
const int numAdcPins = sizeof(adcPins) / sizeof(adcPins[0]);

void setup() {
    Serial.begin(115200);

    // Set all defined pins as INPUT_PULLUP
    for (int i = 0; i < numPins; i++) {
        pinMode(inputPins[i], INPUT_PULLUP);
    }

    // ADC Pins (No need to set pinMode, as they default to ADC input)
}

void loop() {
    // Check button presses
    for (int i = 0; i < numPins; i++) {
        if (digitalRead(inputPins[i]) == LOW) {  // Button pressed (active low)
            Serial.print("Button pressed on pin: ");
            Serial.println(inputPins[i]);
            delay(300); // Debounce delay
        }
    }

    // Read and print ADC values (scaled from 0 to 256)
    Serial.print("ADC Values -> ");
    for (int i = 0; i < numAdcPins; i++) {
        int rawValue = analogRead(adcPins[i]);  // Read ADC (0-4095)
        int scaledValue = map(rawValue, 0, 8192, 0, 256);  // Scale to 0-256





        
        Serial.print("Pin ");
        Serial.print(adcPins[i]);
        Serial.print(": ");
        Serial.print(scaledValue -127);
        Serial.print("  ");
    }
    Serial.println();  // New line for readability

    delay(100);  // Small delay to avoid serial flooding
}
