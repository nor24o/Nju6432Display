# NJU6432 Display Library

This Arduino library drives an NJU6432-based 7-segment display with a 3-pin or 4-pin interface. It supports displaying digits, characters, temperatures, and various visual effects like scrolling, blinking, and animations. The library is designed for flexibility, allowing precise control over the display's 10 character positions (S1–S10) and special symbols, with software-based features for 3-pin setups (no inhibit pin).

## Version
5.3 (Includes `displayTemperature` with cleanup for previous temperature characters)

## Hardware Requirements
- **NJU6432-based 7-segment display** with 10 character positions and special symbols.
- **Arduino board** (e.g., Uno, Nano) with at least 3 digital pins for 3-pin mode (DATA, CLOCK, CHIP_ENABLE) or 4 pins for 4-pin mode (adds INHIBIT for brightness control).
- **Wiring**:
  - 3-pin mode: Connect DATA (DIN), CLOCK (CLK), and CHIP_ENABLE (CE) to Arduino digital pins.
  - 4-pin mode: Add INHIBIT (INH) for hardware brightness control.
  - Example for 3-pin mode:
    - DATA_PIN (e.g., 17) → NJU6432 DIN
    - CLOCK_PIN (e.g., 18) → NJU6432 CLK
    - CHIP_ENABLE_PIN (e.g., 8) → NJU6432 CE

## Installation
1. Place the `Nju6432Display` folder in your Arduino libraries directory (e.g., `C:\Users\<your_username>\Documents\Arduino\libraries\`).
2. Ensure the following files are included:
   - `Nju6432Display.h`
   - `Nju6432Display.cpp`
   - `DisplayConstants.h` (defines segment mappings and special symbols)
3. Include the library in your sketch:
   ```cpp
   #include "Nju6432Display.h"
   ```

## Setup
- **3-pin Constructor** (no brightness control):
  ```cpp
  Nju6432Display display(DATA_PIN, CLOCK_PIN, CHIP_ENABLE_PIN);
  ```
- **4-pin Constructor** (with brightness control):
  ```cpp
  Nju6432Display display(INHIBIT_PIN, DATA_PIN, CLOCK_PIN, CHIP_ENABLE_PIN);
  ```
- Initialize in `setup()`:
  ```cpp
  void setup() {
      display.begin();
      Serial.begin(9600); // Optional for debugging
  }
  ```

## Functions
Below is a comprehensive list of all public functions in the `Nju6432Display` class, grouped by category.

### Core Functions
- **`void begin()`**
  - Initializes the display by setting pin modes and clearing the display.
  - Configures DATA, CLOCK, and CHIP_ENABLE pins as outputs.
  - In 4-pin mode, sets INHIBIT pin as output and enables the display.
  - In 3-pin mode, logs that brightness control is unavailable.
  - Clears `videoRam` and updates the display.
  - **Usage**:
    ```cpp
    display.begin();
    ```

- **`void updateDisplay()`**
  - Updates the physical display with the contents of `videoRam`.
  - Applies bit scrambling for positions S7–S10 and sends data in two blocks (S1–S6, S7–S10) with control bits.
  - Called automatically by high-level functions after modifying `videoRam`.
  - **Usage**:
    ```cpp
    display.updateDisplay();
    ```

- **`void clear()`**
  - Clears the entire display by setting `videoRam` to zeros.
  - Resets the tracking of previous temperature lengths (`_lastTempLength`).
  - Requires `updateDisplay()` to reflect changes (unless called by other functions).
  - **Usage**:
    ```cpp
    display.clear();
    display.updateDisplay();
    ```

### High-Level "Framebuffer" Functions
- **`void setChar(byte position, char character, bool decimalPoint = false)`**
  - Sets a single character at the specified position (0–9, where 0 is leftmost, S10).
  - Supported characters: `0–9`, `a–f` (hex), `h`, `l`, `p`, `u`, `-`, `_`, `=`, `o` (degree symbol).
  - `decimalPoint`: If `true`, lights the decimal point (segment H) for that position.
  - Handles special mapping for positions 6–9 (S7–S10) due to bit scrambling.
  - **Usage**:
    ```cpp
    display.setChar(0, 'A', true); // Displays 'A' with decimal at position 0
    display.setChar(9, 'b', false); // Displays 'b' at position 9
    display.updateDisplay();
    ```

- **`void print(const char* text, byte startPosition = 0)`**
  - Displays a string starting at `startPosition` (0–9).
  - Clears the entire display before writing.
  - Supports digits, letters, and special characters (e.g., `8.8.8.8.8.` for all segments).
  - Automatically handles decimal points in the string (e.g., `12.34`).
  - Stops at 10 positions or end of string.
  - **Usage**:
    ```cpp
    display.print("0123456789"); // Displays all digits
    display.print("8.8.8.8.8.", 1); // Displays 8s with decimals at positions 1–5
    ```

- **`void displayTemperature(float temp, byte startPosition = 0)`**
  - Displays a temperature (e.g., `-23.4`, `43.5`) starting at `startPosition` (0–9).
  - Formats the temperature with one decimal place (e.g., `-99.9`, `99.9`).
  - Clears any previous temperature characters at `startPosition` to prevent leftovers (e.g., when switching from `-23.4` to `43.5`).
  - Skips display if `startPosition` is invalid or the string exceeds available positions.
  - Preserves other display content (e.g., another temperature at a different position).
  - Updates `_blinkBuffer` for software blinking compatibility.
  - **Usage**:
    ```cpp
    display.clear();
    display.displayTemperature(-23.4, 0); // Displays "-23.4" at positions 0–4
    display.displayTemperature(43.5, 5); // Displays "43.5" at positions 5–8
    display.displayTemperature(9.9, 0); // Updates position 0 to "9.9", clears previous "-23.4"
    ```

### Brightness Control
- **`void setBrightness(byte level)`**
  - Sets display brightness (0–255) in 4-pin mode using the INHIBIT pin.
  - In 3-pin mode, logs that brightness control is unavailable.
  - Deferred if blinking is active.
  - **Usage**:
    ```cpp
    display.setBrightness(128); // Half brightness (4-pin mode only)
    ```

### Blinking Mode
- **`void startBlink(unsigned int interval)`**
  - Starts software-based blinking with the specified interval (ms).
  - Stores current `videoRam` in `_blinkBuffer` and toggles between display on (content) and off (blank).
  - Works in 3-pin mode.
  - Stops other modes (scroll, typewriter, knight rider).
  - **Usage**:
    ```cpp
    display.print("BLINK");
    display.startBlink(500); // Blinks every 500ms
    ```

- **`void stopBlink()`**
  - Stops blinking and restores the original `videoRam` content.
  - **Usage**:
    ```cpp
    display.stopBlink();
    ```

- **`bool updateBlink()`**
  - Updates the blinking state (called in `loop()`).
  - Returns `true` if blinking is active, `false` otherwise.
  - Toggles display on/off based on `_blinkInterval`.
  - **Usage**:
    ```cpp
    void loop() {
        if (display.updateBlink()) {
            delay(10);
        }
    }
    ```

### Dynamic Text Modes
- **`void scrollText(const char* text, unsigned int scrollSpeed)`**
  - Scrolls the given text across the display at `scrollSpeed` (ms per shift).
  - Supports up to 64 characters (`NJU_MAX_SCROLL_TEXT_LENGTH`).
  - Clears the display and stops other modes.
  - **Usage**:
    ```cpp
    display.scrollText("HELLO WORLD", 300); // Scrolls text at 300ms per position
    ```

- **`bool updateScroll()`**
  - Updates the scrolling text position (called in `loop()`).
  - Returns `true` if scrolling is active, `false` otherwise.
  - **Usage**:
    ```cpp
    void loop() {
        if (display.updateScroll()) {
            delay(10);
        }
    }
    ```

- **`void stopScroll()`**
  - Stops scrolling text.
  - **Usage**:
    ```cpp
    display.stopScroll();
    ```

- **`void typewriter(const char* text, unsigned int charInterval)`**
  - Displays text character by character at `charInterval` (ms per character).
  - Clears the display and stops other modes.
  - **Usage**:
    ```cpp
    display.typewriter("TESTING", 500); // Types each character every 500ms
    ```

- **`bool updateTypewriter()`**
  - Updates the typewriter effect (called in `loop()`).
  - Returns `true` if typing is active, `false` if complete.
  - **Usage**:
    ```cpp
    void loop() {
        if (display.updateTypewriter()) {
            delay(10);
        }
    }
    ```

- **`void stopTypewriter()`**
  - Stops the typewriter effect.
  - **Usage**:
    ```cpp
    display.stopTypewriter();
    ```

- **`void fadeIn(const char* text, unsigned int stepDelay)`**
  - Displays text with a fade-in effect (4-pin mode only).
  - In 3-pin mode, displays text without fading and logs a message.
  - `stepDelay`: Delay (ms) per brightness step.
  - Clears the display and stops other modes.
  - **Usage**:
    ```cpp
    display.fadeIn("FADE", 10); // Displays "FADE" (no fade in 3-pin mode)
    ```

- **`void fadeOut(unsigned int stepDelay)`**
  - Fades out the display (4-pin mode only).
  - In 3-pin mode, clears the display without fading and logs a message.
  - `stepDelay`: Delay (ms) per brightness step.
  - **Usage**:
    ```cpp
    display.fadeOut(10); // Clears display (no fade in 3-pin mode)
    ```

### Visualization Modes
- **`void displayBarGraph(int value, int maxValue, bool fromCenter = false)`**
  - Displays a bar graph using `=` characters.
  - `value`: Current value (0 to `maxValue`).
  - `maxValue`: Maximum value to scale the graph.
  - `fromCenter`: If `true`, displays from the center; otherwise, from the left.
  - Clears the display and stops other modes.
  - **Usage**:
    ```cpp
    display.displayBarGraph(7, 10, false); // 7 segments from left
    display.displayBarGraph(6, 10, true); // 3 segments each side from center
    ```

- **`void knightRider(unsigned int speed)`**
  - Starts a "Knight Rider" animation (moving `8` with decimal point).
  - `speed`: Delay (ms) between position updates.
  - Clears the display and stops other modes.
  - **Usage**:
    ```cpp
    display.knightRider(200); // Moves every 200ms
    ```

- **`bool updateKnightRider()`**
  - Updates the Knight Rider animation (called in `loop()`).
  - Returns `true` if active, `false` otherwise.
  - **Usage**:
    ```cpp
    void loop() {
        if (display.updateKnightRider()) {
            delay(10);
        }
    }
    ```

- **`void stopKnightRider()`**
  - Stops the Knight Rider animation.
  - **Usage**:
    ```cpp
    display.stopKnightRider();
    ```

### Diagnostics
- **`void runDiagnostics(unsigned int delayMs = 100)`**
  - Runs a diagnostic test:
    - Lights all segments for `delayMs * 2` ms.
    - Clears the display.
    - Displays `8` with decimal point at each position for `delayMs / 2` ms.
  - Restores original brightness (4-pin mode).
  - Stops other modes.
  - **Usage**:
    ```cpp
    display.runDiagnostics(100); // Runs diagnostics with 100ms delays
    ```

### Public Data
- **`byte videoRam[14]`**
  - Public buffer for direct manipulation of display data.
  - Maps to display segments (S1–S10 and special symbols).
  - Use with caution, followed by `updateDisplay()`.
  - **Usage**:
    ```cpp
    display.videoRam[ICON_BATTERY_BYTE] |= ICON_BATTERY_SHELL; // Set battery icon
    display.updateDisplay();
    ```

## Usage Notes
- **3-pin Mode Limitations**:
  - Brightness control (`setBrightness`, `fadeIn`, `fadeOut`) is not available.
  - Software blinking (`startBlink`, `updateBlink`) is supported.
- **Temperature Display**:
  - `displayTemperature` supports negative and positive temperatures (e.g., `-23.4`, `43.5`) with one decimal place.
  - Preserves other display content, allowing multiple temperatures (e.g., `-23.4` at position 0, `43.5` at position 5).
  - Automatically clears previous temperature characters at the same `startPosition` to prevent leftovers (e.g., updating from `-23.4` to `43.5`).
  - Skips invalid inputs (e.g., `startPosition >= 10` or string too long).
- **Position Mapping**:
  - Position 0 is the leftmost digit (S10), position 9 is the rightmost (S1).
  - Ensure temperature strings fit within 10 positions (e.g., `-99.9` uses 5 positions).
- **Software Blinking**:
  - Use `startBlink`, `updateBlink`, and `stopBlink` for blinking effects in 3-pin mode.
  - Compatible with all display functions, including `displayTemperature`.
- **Other Modes**:
  - Functions like `scrollText`, `typewriter`, `displayBarGraph`, and `knightRider` clear the display and stop other modes.
  - Call `clear()` explicitly if you need to reset the display before new content.

## Example Sketch
```cpp
#include "Nju6432Display.h"

const byte DATA_PIN = 17;
const byte CLOCK_PIN = 18;
const byte CHIP_ENABLE_PIN = 8;

Nju6432Display display(DATA_PIN, CLOCK_PIN, CHIP_ENABLE_PIN);

void setup() {
    display.begin();
    Serial.begin(9600);

    // Display two temperatures
    display.clear();
    display.displayTemperature(-23.4, 0); // Positions 0–4
    display.displayTemperature(43.5, 5); // Positions 5–8
    delay(4000);

    // Update temperature at position 0
    display.displayTemperature(9.9, 0); // Updates position 0, clears previous -23.4
    delay(4000);

    // Blink both temperatures
    display.clear();
    display.displayTemperature(-23.4, 0);
    display.displayTemperature(43.5, 5);
    display.startBlink(500);
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        display.updateBlink();
        delay(10);
    }
    display.stopBlink();
}

void loop() {
    // Empty loop
}
```

## Debugging
- **Serial Output**: Enable Serial at 9600 baud to see debug messages (e.g., "Displaying temperature: -23.4 at position 0").
- **Common Issues**:
  - **Leftover Characters**: Ensure `_lastTempLength` is clearing previous positions in `displayTemperature`.
  - **Invalid Display**: Check `startPosition` and string length in `displayTemperature`.
  - **Blinking Issues**: Verify `updateBlink` is called regularly in `loop()`.
- **Verbose Compilation**: Enable in Arduino IDE (`File > Preferences`) to diagnose compilation errors.

## License
This library is provided as-is for educational and personal use. No warranty is implied.

## Contact
For issues or feature requests, please describe the problem, expected behavior, and any Serial output or display anomalies.

## Phisical segment MAP - from right to left 
```cpp
Bit Position:  0  ->  Byte[0],   Bit(0) = S1 D
Bit Position:  1  ->  Byte[0],   Bit(1) = S1 dp
Bit Position:  2  ->  Byte[0],   Bit(2) = S1 E
Bit Position:  3  ->  Byte[0],   Bit(3) = S1 C
Bit Position:  4  ->  Byte[0],   Bit(4) = S1 F
Bit Position:  5  ->  Byte[0],   Bit(5) = S1 A
Bit Position:  6  ->  Byte[0],   Bit(6) = S1 G
Bit Position:  7  ->  Byte[0],   Bit(7) = S1 B

Bit Position:  8  ->  Byte[1],   Bit(0) = S2 D
Bit Position:  9  ->  Byte[1],   Bit(1) = S2 dp
Bit Position: 10  ->  Byte[1],   Bit(2) = S2 E
Bit Position: 11  ->  Byte[1],   Bit(3) = S2 C
Bit Position: 12  ->  Byte[1],   Bit(4) = S2 F
Bit Position: 13  ->  Byte[1],   Bit(5) = S2 A
Bit Position: 14  ->  Byte[1],   Bit(6) = S2 G
Bit Position: 15  ->  Byte[1],   Bit(7) = S2 B

Bit Position: 16  ->  Byte[2],   Bit(0) = S3 D
Bit Position: 17  ->  Byte[2],   Bit(1) = S3 dp
Bit Position: 18  ->  Byte[2],   Bit(2) = S3 E
Bit Position: 19  ->  Byte[2],   Bit(3) = S3 C
Bit Position: 20  ->  Byte[2],   Bit(4) = S3 F
Bit Position: 21  ->  Byte[2],   Bit(5) = S3 A
Bit Position: 22  ->  Byte[2],   Bit(6) = S3 G
Bit Position: 23  ->  Byte[2],   Bit(7) = S3 B

Bit Position: 24  ->  Byte[3],   Bit(0) = S4 D
Bit Position: 25  ->  Byte[3],   Bit(1) = S4 dp
Bit Position: 26  ->  Byte[3],   Bit(2) = S4 E
Bit Position: 27  ->  Byte[3],   Bit(3) = S4 C
Bit Position: 28  ->  Byte[3],   Bit(4) = S4 F
Bit Position: 29  ->  Byte[3],   Bit(5) = S4 A
Bit Position: 30  ->  Byte[3],   Bit(6) = S4 G
Bit Position: 31  ->  Byte[3],   Bit(7) = S4 B

Bit Position: 32  ->  Byte[4],   Bit(0) = S5 D
Bit Position: 33  ->  Byte[4],   Bit(1) = S5 dp
Bit Position: 34  ->  Byte[4],   Bit(2) = S5 E
Bit Position: 35  ->  Byte[4],   Bit(3) = S5 C
Bit Position: 36  ->  Byte[4],   Bit(4) = S5 F
Bit Position: 37  ->  Byte[4],   Bit(5) = S5 A
Bit Position: 38  ->  Byte[4],   Bit(6) = S5 G
Bit Position: 39  ->  Byte[4],   Bit(7) = S5 B

Bit Position: 40  ->  Byte[5],   Bit(0) = S6 D
Bit Position: 41  ->  Byte[5],   Bit(1) = S6 dp
Bit Position: 42  ->  Byte[5],   Bit(2) = S6 E
Bit Position: 43  ->  Byte[5],   Bit(3) = S6 C
Bit Position: 44  ->  Byte[5],   Bit(4) = S6 F
Bit Position: 45  ->  Byte[5],   Bit(5) = S6 A
Bit Position: 46  ->  Byte[5],   Bit(6) = S6 G
Bit Position: 47  ->  Byte[5],   Bit(7) = S6 B

Bit Position: 48  ->  Byte[6],   Bit(0) = S7 D   
Bit Position: 49  ->  Byte[6],   Bit(1) = S7 dp  
Bit Position: 50  ->  Byte[6],   Bit(2) = S7 E   
Bit Position: 51  ->  Byte[6],   Bit(3) = S7 C  
Bit Position: 52  ->  Byte[6],   Bit(4) = ---NOT USED--
Bit Position: 53  ->  Byte[6],   Bit(5) = S7 F
Bit Position: 54  ->  Byte[6],   Bit(6) = S7 A   
Bit Position: 55  ->  Byte[6],   Bit(7) = S7 G   
Bit Position: 56  ->  Byte[7],   Bit(0) = S7 B  
 
Bit Position: 57  ->  Byte[7],   Bit(1) = S8 D   
Bit Position: 58  ->  Byte[7],   Bit(2) = S8 dp  
Bit Position: 59  ->  Byte[7],   Bit(3) = S8 E   
Bit Position: 60  ->  Byte[7],   Bit(4) = S8 C   
Bit Position: 61  ->  Byte[7],   Bit(5) = S8 F   
Bit Position: 62  ->  Byte[7],   Bit(6) = S8 A   
Bit Position: 63  ->  Byte[7],   Bit(7) = S8 G   
Bit Position: 64  ->  Byte[8],   Bit(0) = S8 B
   
Bit Position: 65  ->  Byte[8],   Bit(1) = S9 D   
Bit Position: 66  ->  Byte[8],   Bit(2) = S9 dp  
Bit Position: 67  ->  Byte[8],   Bit(3) = S9 E   
Bit Position: 68  ->  Byte[8],   Bit(4) = S9 C   
Bit Position: 69  ->  Byte[8],   Bit(5) = S9 F   
Bit Position: 70  ->  Byte[8],   Bit(6) = S9 A   
Bit Position: 71  ->  Byte[8],   Bit(7) = S9 G   
Bit Position: 72  ->  Byte[9],   Bit(0) = S9 B 
  
Bit Position: 73  ->  Byte[9],   Bit(1) = S10 D   
Bit Position: 74  ->  Byte[9],   Bit(2) = S10 dp 
Bit Position: 75  ->  Byte[9],   Bit(3) = S10 E  
Bit Position: 76  ->  Byte[9],   Bit(4) = S10 C  
Bit Position: 77  ->  Byte[9],   Bit(5) = S10 F  
Bit Position: 78  ->  Byte[9],   Bit(6) = S10 A  
Bit Position: 79  ->  Byte[9],   Bit(7) = S10 G  
Bit Position: 80  ->  Byte[10],  Bit(0) = S10 B 

Bit Position: 81  ->  Byte[10],  Bit(1) = simbol enter
Bit Position: 82  ->  Byte[10],  Bit(2) = simbol minus
Bit Position: 83  ->  Byte[10],  Bit(3) = simbol upper line minus
Bit Position: 84  ->  Byte[10],  Bit(4) = simbol upper line plus
Bit Position: 85  ->  Byte[10],  Bit(5) = simbol upper line substract
Bit Position: 86  ->  Byte[10],  Bit(6) = simbol upper line prod
Bit Position: 87  ->  Byte[10],  Bit(7) = simbol upper line P simbol
Bit Position: 88  ->  Byte[11],  Bit(0) = simbol upper line equal
Bit Position: 89  ->  Byte[11],  Bit(1) = simbol upper line produs simbol2
Bit Position: 90  ->  Byte[11],  Bit(2) = simbol upper line Z simbol
Bit Position: 91  ->  Byte[11],  Bit(3) = simbol upper line ECR simbol
Bit Position: 92  ->  Byte[11],  Bit(4) = simbol upper line R simbol
Bit Position: 93  ->  Byte[11],  Bit(5) = simbol Tilded M simbol
Bit Position: 94  ->  Byte[11],  Bit(6) = simbol upper line STL simbol
Bit Position: 95  ->  Byte[11],  Bit(7) = simbol upper line battery segment 4
Bit Position: 96  ->  Byte[12],  Bit(0) = simbol upper line battery shell
Bit Position: 97  ->  Byte[12],  Bit(1) = simbol upper line battery segment 3
Bit Position: 98  ->  Byte[12],  Bit(2) = simbol ---NOT USED--
Bit Position: 99  ->  Byte[12],  Bit(3) = simbol upper line battery segment 2
Bit Position: 100  ->  Byte[12], Bit(4) = simbol ---NOT USED--
Bit Position: 101  ->  Byte[12], Bit(5) = simbol upper line battery segment 1
Bit Position: 102  ->  Byte[12], Bit(6) = simbol ---NOT USED--
Bit Position: 103  ->  Byte[12], Bit(7) = simbol ---NOT USED--
Bit Position: 104  ->  Byte[13], Bit(0) = simbol ---NOT USED--
Bit Position: 105  ->  Byte[13], Bit(1) = simbol ---NOT USED--
Bit Position: 106  ->  Byte[13], Bit(2) = simbol ---NOT USED--
Bit Position: 107  ->  Byte[13], Bit(3) = simbol ---NOT USED--
Bit Position: 108  ->  Byte[13], Bit(4) = simbol ---NOT USED--
Bit Position: 109  ->  Byte[13], Bit(5) = simbol ---NOT USED--
Bit Position: 110  ->  Byte[13], Bit(6) = simbol ---NOT USED--
Bit Position: 111  ->  Byte[13], Bit(7) = simbol ---NOT USED--
```