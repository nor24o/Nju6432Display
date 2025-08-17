// Include the modernized display library headers.
#include "Nju6432Display.h"
#include "DisplayConstants.h" // This will now include your custom map

// -- Hardware Pin Definitions --
const byte INHIBIT_PIN = 9;
const byte DATA_PIN = 17;
const byte CLOCK_PIN = 18;
const byte CHIP_ENABLE_PIN = 8;

// Create an instance of the display driver class.
Nju6432Display display(INHIBIT_PIN, DATA_PIN, CLOCK_PIN, CHIP_ENABLE_PIN);

void setup() {
    display.begin();
    Serial.begin(9600);
    Serial.println("--- Custom Display Test ---");

    // --- Test 1: Print numbers and text ---
    // The print function should now work correctly with your display's wiring.
    // Position 0 is the leftmost character (S1 on your map).
    Serial.println("Displaying '8.8.8.8.8.' - All segments should be on.");
    display.print("8.8.8.8.8.", 0);
    display.updateDisplay();
    delay(4000);

    display.clear();
    Serial.println("Displaying '0123456789'");
    display.print("0123456789", 0);
    display.updateDisplay();
    delay(4000);

    // --- Test 2: Activate special symbols ---
    // We do this by directly manipulating the videoRam array using the new constants.
    display.clear();
    Serial.println("Activating special symbols...");

    // Turn on the 'Enter' and 'Minus' symbols
    display.videoRam[ICON_ENTER_BYTE] |= ICON_ENTER_BIT;
    display.videoRam[ICON_MINUS_BYTE] |= ICON_MINUS_BIT;

    // Turn on the 'Tilded M' symbol
    display.videoRam[ICON_TILDED_M_BYTE] |= ICON_TILDED_M_BIT;
    
    display.updateDisplay();
    delay(4000);

    // --- Test 3: Battery Indicator Animation ---
    Serial.println("Running battery animation...");
    for (int i = 0; i < 5; i++) {
        // Clear only the battery bits, leaving other icons on if they were.
        display.videoRam[ICON_BATTERY_BYTE] &= ~ICON_BATTERY_SHELL;
        display.videoRam[ICON_BATTERY_BYTE] &= ~ICON_BATTERY_SEG_1;
        display.videoRam[ICON_BATTERY_BYTE] &= ~ICON_BATTERY_SEG_2;
        display.videoRam[ICON_BATTERY_BYTE] &= ~ICON_BATTERY_SEG_3;
        display.videoRam[ICON_BATTERY_SEG_4_BYTE] &= ~ICON_BATTERY_SEG_4_BIT;

        // Turn on shell
        display.videoRam[ICON_BATTERY_BYTE] |= ICON_BATTERY_SHELL;
        display.updateDisplay();
        delay(500);

        // Animate segments
        display.videoRam[ICON_BATTERY_BYTE] |= ICON_BATTERY_SEG_1;
        display.updateDisplay();
        delay(500);
        display.videoRam[ICON_BATTERY_BYTE] |= ICON_BATTERY_SEG_2;
        display.updateDisplay();
        delay(500);
        display.videoRam[ICON_BATTERY_BYTE] |= ICON_BATTERY_SEG_3;
        display.updateDisplay();
        delay(500);
        display.videoRam[ICON_BATTERY_SEG_4_BYTE] |= ICON_BATTERY_SEG_4_BIT;
        display.updateDisplay();
        delay(1000);
    }
    
    Serial.println("Test complete. Restarting...");
    delay(2000);
}

void loop() {
  // The setup function runs the whole demo, so the loop can be empty.
  // Or you can add your own code here.
}
