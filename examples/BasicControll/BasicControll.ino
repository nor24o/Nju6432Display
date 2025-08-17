// Include the modernized display library headers.
#include "Nju6432Display.h"
#include "DisplayConstants.h"

// -- Hardware Pin Definitions --
// Define the Arduino pins connected to the display.
const byte INHIBIT_PIN = 9;    // Display blanking/enable pin
const byte DATA_PIN = 17;      // Serial data pin
const byte CLOCK_PIN = 18;     // Serial clock pin
const byte CHIP_ENABLE_PIN = 8;  // Chip enable/select pin

// Create an instance of the display driver class.
Nju6432Display display(INHIBIT_PIN, DATA_PIN, CLOCK_PIN, CHIP_ENABLE_PIN);

// Buffer to hold incoming serial commands.
char command_buffer[32];
byte buffer_pos = 0;

//================================================================
void setup() {
    // Initialize the display driver.
    display.begin();
    // Start serial communication for debugging output.
    Serial.begin(9600);
    
    // Print instructions to the user.
    Serial.println("--- NJU6432 Segment Mapper (Binary Mode) ---");
    Serial.println("Send 'r' to turn all segments off.");
    Serial.println("Send 'B<index>,B<binary_value>' to set a byte.");
    Serial.println("Example: B5,B11110000");
    Serial.println("---------------------------------");

    // Start with a completely blank display.
    display.clear();
    display.updateDisplay();
}
//================================================================
void loop() {
    // Check if there's any incoming data from the Serial Monitor.
    if (Serial.available() > 0) {
        char incoming_char = Serial.read();

        // If a newline character is received, the command is complete.
        if (incoming_char == '\n' || incoming_char == '\r') {
            if (buffer_pos > 0) { // Ensure the command is not empty
                command_buffer[buffer_pos] = '\0'; // Null-terminate the string
                processCommand(command_buffer);    // Process the received command
                buffer_pos = 0; // Reset buffer for the next command
            }
        } else {
            // Add the character to the buffer if there's space.
            if (buffer_pos < sizeof(command_buffer) - 1) {
                command_buffer[buffer_pos++] = incoming_char;
            }
        }
    }
}

//================================================================
// This function parses and executes the command received from the serial port.
//================================================================
void processCommand(char* command) {
    // --- Handle single-character 'reset' command ---
    if (strlen(command) == 1 && (command[0] == 'r' || command[0] == 'R')) {
        display.clear();
        display.updateDisplay();
        Serial.println("--- Display Cleared ---");
        printRamState();
        return;
    }

    // --- Handle multi-character 'B' command for setting a byte with a binary string ---
    if (command[0] == 'B' || command[0] == 'b') {
        // Find the comma that separates the index and the binary value.
        char* comma = strchr(command, ',');
        if (comma == NULL) {
            Serial.println("Error: Command must contain a comma. Format: B<index>,B<binary>");
            return;
        }

        // Temporarily split the string at the comma to parse the index.
        *comma = '\0';
        int byteIndex = atoi(command + 1); // +1 to skip the initial 'B'
        
        // Point to the start of the binary part.
        char* binaryPart = comma + 1;
        
        // Restore the comma in the original string (good practice).
        *comma = ',';

        // Validate the binary part of the command.
        if (binaryPart[0] != 'B' && binaryPart[0] != 'b') {
            Serial.println("Error: Binary value must start with 'B'.");
            return;
        }
        
        char* binaryStr = binaryPart + 1; // Get the actual binary digits.
        if (strlen(binaryStr) != 8) {
            Serial.println("Error: Binary value must be 8 digits long.");
            return;
        }

        // Convert the binary string to a byte value.
        byte byteValue = 0;
        for (int i = 0; i < 8; i++) {
            if (binaryStr[i] == '1') {
                // Set the corresponding bit. (7-i) converts from left-to-right to standard bit order.
                byteValue |= (1 << (7 - i));
            } else if (binaryStr[i] != '0') {
                Serial.println("Error: Binary value can only contain '0' and '1'.");
                return;
            }
        }

        // Final validation of the byte index.
        if (byteIndex >= 0 && byteIndex < 14) {
            display.videoRam[byteIndex] = byteValue;
            display.updateDisplay();
            Serial.print("OK: Set Byte[");
            Serial.print(byteIndex);
            Serial.print("] = B");
            Serial.println(binaryStr);
            printRamState(); // Show the full state of the display memory
        } else {
            Serial.println("Error: Index must be between 0 and 13.");
        }
        return;
    }

    Serial.println("Error: Unknown command.");
}

//================================================================
// Helper function to print the current state of the entire videoRam array.
//================================================================
void printRamState() {
    Serial.print("videoRam: [");
    for(int i = 0; i < 14; i++) {
        Serial.print(display.videoRam[i]);
        if (i < 13) Serial.print(", ");
    }
    Serial.println("]");
}



