#include "Nju6432Display.h"

// The number of 7-segment character positions on your display.
#define NUM_CHAR_POSITIONS 10

// -- CONSTRUCTORS --
Nju6432Display::Nju6432Display(byte inhibitPin, byte dataPin, byte clockPin, byte chipEnablePin) {
    _inhibitPin = inhibitPin;
    _dataPin = dataPin;
    _clockPin = clockPin;
    _chipEnablePin = chipEnablePin;
}

Nju6432Display::Nju6432Display(byte dataPin, byte clockPin, byte chipEnablePin)
    : Nju6432Display(NJU_NO_PIN, dataPin, clockPin, chipEnablePin) {}

// -- CORE Functions --
void Nju6432Display::begin() {
    if (_inhibitPin != NJU_NO_PIN) {
        pinMode(_inhibitPin, OUTPUT);
        digitalWrite(_inhibitPin, HIGH); // Ensure display is on initially
        Serial.print("Inhibit pin set to OUTPUT: ");
        Serial.println(_inhibitPin);
    } else {
        Serial.println("Inhibit pin is NJU_NO_PIN (3-pin mode), brightness control disabled");
    }
    pinMode(_dataPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_chipEnablePin, OUTPUT);

    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, LOW);
    digitalWrite(_chipEnablePin, LOW);

    clear();
    updateDisplay();
    setBrightness(255);
}

void Nju6432Display::updateDisplay() {
    // Copy videoRam to transferBuffer
    for (int i = 0; i < 14; i++) {
        _transferBuffer[i] = videoRam[i];
    }

    // Perform bit scrambling for bytes 7–13 (S7–S10)
    byte carry = (_transferBuffer[6] >> 5) & 0b00000111;
    for (int i = 7; i < 14; i++) {
        byte currentByte = _transferBuffer[i];
        byte newData = ((currentByte << 3) & 0b11111000) | carry;
        carry = (currentByte >> 5) & 0b00000111;
        _transferBuffer[i] = newData;
    }

    applyControlBits(6, CONTROL_BLOCK_0);
    applyControlBits(13, CONTROL_BLOCK_1);

    digitalWrite(_chipEnablePin, HIGH);
    sendBlock(0, 6);
    digitalWrite(_dataPin, LOW);
    delayMicroseconds(1);
    digitalWrite(_chipEnablePin, LOW);
    delayMicroseconds(5);
    digitalWrite(_chipEnablePin, HIGH);
    delayMicroseconds(1);
    sendBlock(7, 13);
    digitalWrite(_dataPin, LOW);
    digitalWrite(_chipEnablePin, LOW);
}

void Nju6432Display::clear() {
    for (byte i = 0; i < 14; i++) {
        videoRam[i] = 0;
    }
        // Reset last temperature lengths
    for (byte i = 0; i < NUM_CHAR_POSITIONS; i++) {
        _lastTempLength[i] = 0;
    }
}

// -- HIGH-LEVEL PRINTING --
void Nju6432Display::setChar(byte position, char character, bool decimalPoint) {
    if (position >= NUM_CHAR_POSITIONS) return;
    int hw_index = NUM_CHAR_POSITIONS - 1 - position; // position 0 -> leftmost (S10), hw_index=9; position 9 -> rightmost (S1), hw_index=0

    byte fontPattern = getCharacterFont(character);
    if (decimalPoint) fontPattern |= (1 << SEG_H);

    if (hw_index <= 5) { // S1-S6 (rightmost, normal bytes)
        videoRam[hw_index] = fontPattern;
    } else { // S7-S10 (leftmost, split bytes)
        byte mainByte = 0;
        byte b_segment = (fontPattern & (1 << SEG_B)) ? 1 : 0;

        if (hw_index == 6) { // S7, special mapping with skip at bit 4
            videoRam[hw_index] = 0; // Fully clear, no previous B to preserve
            mainByte |= (fontPattern & (1 << SEG_D)) ? (1 << 0) : 0;
            mainByte |= (fontPattern & (1 << SEG_H)) ? (1 << 1) : 0;
            mainByte |= (fontPattern & (1 << SEG_E)) ? (1 << 2) : 0;
            mainByte |= (fontPattern & (1 << SEG_C)) ? (1 << 3) : 0;
            mainByte |= (fontPattern & (1 << SEG_F)) ? (1 << 5) : 0;
            mainByte |= (fontPattern & (1 << SEG_A)) ? (1 << 6) : 0;
            mainByte |= (fontPattern & (1 << SEG_G)) ? (1 << 7) : 0;
            videoRam[hw_index] = mainByte;
            videoRam[hw_index + 1] &= 0b11111110; // Clear B bit
            videoRam[hw_index + 1] |= b_segment;
        } else { // S8-S10, standard shifted mapping
            videoRam[hw_index] &= 0b00000001; // Preserve previous B in bit 0
            mainByte |= (fontPattern & (1 << SEG_D)) ? (1 << 1) : 0;
            mainByte |= (fontPattern & (1 << SEG_H)) ? (1 << 2) : 0;
            mainByte |= (fontPattern & (1 << SEG_E)) ? (1 << 3) : 0;
            mainByte |= (fontPattern & (1 << SEG_C)) ? (1 << 4) : 0;
            mainByte |= (fontPattern & (1 << SEG_F)) ? (1 << 5) : 0;
            mainByte |= (fontPattern & (1 << SEG_A)) ? (1 << 6) : 0;
            mainByte |= (fontPattern & (1 << SEG_G)) ? (1 << 7) : 0;
            videoRam[hw_index] |= mainByte;
            videoRam[hw_index + 1] &= 0b11111110; // Clear B bit
            videoRam[hw_index + 1] |= b_segment;
        }
    }
}

void Nju6432Display::print(const char* text, byte startPosition) {
    if (startPosition >= NUM_CHAR_POSITIONS) return;
    clear();

    byte currentPos = startPosition;
    for (int i = 0; text[i] != '\0' && currentPos < NUM_CHAR_POSITIONS; i++) {
        bool dp = (text[i + 1] == '.');
        setChar(currentPos, text[i], dp);
        if (dp) i++;
        currentPos++;
    }
}

void Nju6432Display::displayTemperature(float temp, byte startPosition) {
    if (startPosition >= NUM_CHAR_POSITIONS) {
        Serial.println("displayTemperature: Invalid start position, skipping");
        return;
    }

    // Format temperature as a string (e.g., "-23.4", "43.5")
    char tempStr[8]; // Enough for "-99.9" or "99.9"
    dtostrf(temp, 5, 1, tempStr); // 5 chars total, 1 decimal place

    // Trim leading/trailing spaces and count length
    char formattedStr[8];
    int len = 0;
    bool isNegative = (tempStr[0] == '-');
    int i = isNegative ? 1 : 0; // Skip minus sign initially
    bool decimalFound = false;

    // Build formatted string
    if (isNegative) {
        formattedStr[len++] = '-'; // Add minus sign
    }
    for (; tempStr[i] != '\0' && len < 7; i++) {
        if (tempStr[i] == ' ') continue; // Skip spaces
        if (tempStr[i] == '.') decimalFound = true;
        formattedStr[len++] = tempStr[i];
    }
    formattedStr[len] = '\0';

    // Check if temperature fits in remaining positions
    if (startPosition + len > NUM_CHAR_POSITIONS) {
        Serial.println("displayTemperature: Temperature string too long for display, skipping");
        return;
    }

    // Clear previous temperature characters at startPosition
    byte prevLength = _lastTempLength[startPosition];
    for (byte pos = startPosition; pos < startPosition + prevLength && pos < NUM_CHAR_POSITIONS; pos++) {
        setChar(pos, ' ', false); // Clear with blank character
    }

    // Display the temperature
    Serial.print("Displaying temperature: ");
    Serial.print(tempStr);
    Serial.print(" at position ");
    Serial.println(startPosition);

    byte currentPos = startPosition;
    for (int i = 0; formattedStr[i] != '\0' && currentPos < NUM_CHAR_POSITIONS; i++) {
        bool dp = (formattedStr[i + 1] == '.');
        setChar(currentPos, formattedStr[i], dp);
        if (dp) i++; // Skip the decimal point
        currentPos++;
    }

    // Store the length of the new temperature string
    _lastTempLength[startPosition] = len;

    updateDisplay();

    // Update _blinkBuffer for software blinking compatibility
    for (int i = 0; i < 14; i++) {
        _blinkBuffer[i] = videoRam[i];
    }
}

// -- ALL OTHER MODES --
void Nju6432Display::setBrightness(byte level) {
    if (_inhibitPin == NJU_NO_PIN) {
        Serial.println("setBrightness: 3-pin mode, brightness control not available");
        return;
    }
    _brightness = level;
    if (!_isBlinking) {
        Serial.print("Setting brightness to ");
        Serial.print(level);
        Serial.print(" on pin ");
        Serial.println(_inhibitPin);
        analogWrite(_inhibitPin, level);
    } else {
        Serial.println("setBrightness: Blinking is active, brightness change deferred");
    }
}

void Nju6432Display::startBlink(unsigned int interval) {
    stopAllModes();
    _blinkInterval = interval;
    _isBlinking = true;
    _lastBlinkTime = millis();
    _blinkStateOn = true;
    // Store current videoRam state for blinking
    for (int i = 0; i < 14; i++) {
        _blinkBuffer[i] = videoRam[i];
    }
    Serial.print("Starting software blink with interval ");
    Serial.print(interval);
    Serial.println("ms (3-pin mode)");
    updateDisplay(); // Ensure current content is displayed
}

void Nju6432Display::stopBlink() {
    if (_isBlinking) {
        Serial.println("Stopping software blink");
        _isBlinking = false;
        // Restore original videoRam state
        for (int i = 0; i < 14; i++) {
            videoRam[i] = _blinkBuffer[i];
        }
        updateDisplay();
    }
}

bool Nju6432Display::updateBlink() {
    if (!_isBlinking) return false;
    if (millis() - _lastBlinkTime >= _blinkInterval) {
        _lastBlinkTime = millis();
        _blinkStateOn = !_blinkStateOn;
        Serial.print("Software Blink: Display ");
        Serial.println(_blinkStateOn ? "ON" : "OFF");
        if (_blinkStateOn) {
            // Restore original content
            for (int i = 0; i < 14; i++) {
                videoRam[i] = _blinkBuffer[i];
            }
        } else {
            // Clear display
            for (int i = 0; i < 14; i++) {
                videoRam[i] = 0;
            }
        }
        updateDisplay();
    }
    return true;
}

void Nju6432Display::scrollText(const char* text, unsigned int scrollSpeed) {
    stopAllModes();
    strncpy(_scrollTextBuffer, text, NJU_MAX_SCROLL_TEXT_LENGTH - 1);
    _scrollTextBuffer[NJU_MAX_SCROLL_TEXT_LENGTH - 1] = '\0';
    _scrollTextLen = strlen(_scrollTextBuffer);
    _scrollSpeed = scrollSpeed;
    _scrollPosition = -NUM_CHAR_POSITIONS;
    _isScrolling = true;
    _lastScrollTime = millis();
}

bool Nju6432Display::updateScroll() {
    if (!_isScrolling) return false;
    if (millis() - _lastScrollTime >= _scrollSpeed) {
        _lastScrollTime = millis();
        char displaySegment[NUM_CHAR_POSITIONS + 1];
        for (int i = 0; i < NUM_CHAR_POSITIONS; i++) {
            int textIndex = _scrollPosition + i;
            if (textIndex < 0 || textIndex >= _scrollTextLen) {
                displaySegment[i] = ' ';
            } else {
                displaySegment[i] = _scrollTextBuffer[textIndex];
            }
        }
        displaySegment[NUM_CHAR_POSITIONS] = '\0';
        print(displaySegment);
        updateDisplay();
        _scrollPosition++;
        if (_scrollPosition > _scrollTextLen) {
            _scrollPosition = -NUM_CHAR_POSITIONS;
        }
    }
    return true;
}

void Nju6432Display::stopScroll() { _isScrolling = false; }

void Nju6432Display::typewriter(const char* text, unsigned int charInterval) {
    stopAllModes();
    strncpy(_typewriterBuffer, text, NJU_MAX_SCROLL_TEXT_LENGTH - 1);
    _typewriterBuffer[NJU_MAX_SCROLL_TEXT_LENGTH - 1] = '\0';
    _typewriterLen = strlen(_typewriterBuffer);
    _typewriterInterval = charInterval;
    _typewriterPos = 0;
    _isTyping = true;
    _lastTypeTime = millis();
    clear();
    updateDisplay();
}

bool Nju6432Display::updateTypewriter() {
    if (!_isTyping) return false;
    if (millis() - _lastTypeTime >= _typewriterInterval) {
        if (_typewriterPos >= _typewriterLen) {
            stopTypewriter();
            return false;
        }
        _lastTypeTime = millis();
        setChar(_typewriterPos, _typewriterBuffer[_typewriterPos], false);
        updateDisplay();
        _typewriterPos++;
    }
    return true;
}

void Nju6432Display::stopTypewriter() { _isTyping = false; }

void Nju6432Display::fadeIn(const char* text, unsigned int stepDelay) {
    if (_inhibitPin == NJU_NO_PIN) {
        Serial.println("fadeIn: 3-pin mode, displaying text without fade");
        print(text, 0);
        updateDisplay();
        return;
    }
    stopAllModes();
    print(text, 0);
    updateDisplay();
    Serial.println("Fading in text");
    for (int i = 0; i <= _brightness; i++) {
        Serial.print("FadeIn: Setting brightness to ");
        Serial.print(i);
        Serial.print(" on pin ");
        Serial.println(_inhibitPin);
        analogWrite(_inhibitPin, i);
        delay(stepDelay);
    }
}

void Nju6432Display::fadeOut(unsigned int stepDelay) {
    if (_inhibitPin == NJU_NO_PIN) {
        Serial.println("fadeOut: 3-pin mode, clearing display without fade");
        clear();
        updateDisplay();
        return;
    }
    stopAllModes();
    Serial.println("Fading out display");
    for (int i = _brightness; i >= 0; i--) {
        Serial.print("FadeOut: Setting brightness to ");
        Serial.print(i);
        Serial.print(" on pin ");
        Serial.println(_inhibitPin);
        analogWrite(_inhibitPin, i);
        delay(stepDelay);
    }
    clear();
    updateDisplay();
}

void Nju6432Display::displayBarGraph(int value, int maxValue, bool fromCenter) {
    stopAllModes();
    clear();
    if (maxValue == 0) return;
    int segments = map(value, 0, maxValue, 0, NUM_CHAR_POSITIONS);
    if (fromCenter) {
        int halfSegments = segments / 2;
        for (int i = 0; i < halfSegments; i++) {
            setChar((NUM_CHAR_POSITIONS/2 - 1) - i, '-', false);
            setChar((NUM_CHAR_POSITIONS/2) + i, '-', false);
        }
    } else {
        for (int i = 0; i < segments; i++) {
            setChar(i, '=', false);
        }
    }
    updateDisplay();
}

void Nju6432Display::knightRider(unsigned int speed) {
    stopAllModes();
    _scannerSpeed = speed;
    _scannerPos = 0;
    _scannerDir = 1;
    _isScanning = true;
    _lastScanTime = millis();
}

bool Nju6432Display::updateKnightRider() {
    if (!_isScanning) return false;
    if (millis() - _lastScanTime >= _scannerSpeed) {
        _lastScanTime = millis();
        clear();
        setChar(_scannerPos, 8, true);
        updateDisplay();
        _scannerPos += _scannerDir;
        if (_scannerPos >= (NUM_CHAR_POSITIONS - 1) || _scannerPos <= 0) {
            _scannerDir *= -1;
        }
    }
    return true;
}

void Nju6432Display::stopKnightRider() { _isScanning = false; }

void Nju6432Display::runDiagnostics(unsigned int delayMs) {
    stopAllModes();
    byte oldBrightness = _brightness;
    setBrightness(255);
    Serial.println("Diagnostics: Lighting all segments");
    for (int i = 0; i < 14; i++) videoRam[i] = 0xFF;
    updateDisplay();
    delay(delayMs * 2);
    clear();
    updateDisplay();
    delay(delayMs);
    Serial.println("Diagnostics: Testing each position with '8'");
    for (int i = 0; i < NUM_CHAR_POSITIONS; i++) {
        clear();
        setChar(i, 8, true);
        updateDisplay();
        delay(delayMs / 2);
    }
    clear();
    updateDisplay();
    setBrightness(oldBrightness);
}

// -- PRIVATE HELPER FUNCTIONS --
void Nju6432Display::sendBlock(byte start, byte end) {
    for (byte i = start; i <= end; i++) {
        for (byte bitPos = 0; bitPos < 8; bitPos++) {
            digitalWrite(_dataPin, bitRead(_transferBuffer[i], bitPos));
            delayMicroseconds(1);
            digitalWrite(_clockPin, HIGH);
            delayMicroseconds(1);
            digitalWrite(_clockPin, LOW);
            delayMicroseconds(1);
        }
    }
}

void Nju6432Display::applyControlBits(byte bufferIndex, byte controlWord) {
    _transferBuffer[bufferIndex] = (_transferBuffer[bufferIndex] & 0b00011111) | controlWord;
}

byte Nju6432Display::getCharacterFont(char c) {
    if (c >= '0' && c <= '9') return sevenSegmentFont[c - '0'];
    if (c >= 'a' && c <= 'f') return sevenSegmentFont[c - 'a' + 10];
    if (c >= 'A' && c <= 'F') return sevenSegmentFont[c - 'A' + 10];
    switch (c) {
        case 'h': case 'H': return FONT_H;
        case 'l': case 'L': return FONT_L;
        case 'p': case 'P': return FONT_P;
        case 'u': case 'U': return FONT_U;
        case '-': return FONT_MINUS;
        case '_': return FONT_UNDER;
        case '=': return FONT_BAR_FULL;
        case 'o': case 'O': case '`': case '\'': return FONT_DEGREE;
        default: return 0;
    }
}

void Nju6432Display::stopAllModes() {
    stopBlink();
    stopScroll();
    stopTypewriter();
    stopKnightRider();
}