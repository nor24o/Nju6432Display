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

// -- CORE FUNCTIONS --
void Nju6432Display::begin() {
    if (_inhibitPin != NJU_NO_PIN) {
        pinMode(_inhibitPin, OUTPUT);
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
}

// -- HIGH-LEVEL PRINTING --
void Nju6432Display::setChar(byte position, char character, bool decimalPoint) {
    // Map logical position (0 = leftmost) to hardware position (0 = S1, rightmost)
    if (position >= NUM_CHAR_POSITIONS) return;
    int hw_index = NUM_CHAR_POSITIONS - 1 - position; // Reverse: position 0 -> S1, position 9 -> S10

    // Get the font pattern
    byte fontPattern = getCharacterFont(character);
    if (decimalPoint) fontPattern |= (1 << SEG_H);

    // Clear the space for the new character
    if (hw_index <= 5) { // S1–S6 (Bytes 0–5)
        videoRam[hw_index] = 0;
    } else if (hw_index >= 6 && hw_index < 10) { // S7–S10 (Bytes 6–9)
        videoRam[hw_index] &= B00000001; // Preserve B segment of previous char
        videoRam[hw_index + 1] &= B11111110; // Clear B segment of this char
    }

    // Write the character's data
    if (hw_index <= 5) { // S1–S6
        videoRam[hw_index] = fontPattern;
    } else if (hw_index >= 6 && hw_index < 10) { // S7–S10
        byte mainByte = 0;
        byte b_segment_byte = 0;

        // Map font pattern to hardware layout
        if (fontPattern & (1 << SEG_D)) mainByte |= (1 << 1); // D -> bit 1
        if (fontPattern & (1 << SEG_H)) mainByte |= (1 << 2); // dp -> bit 2
        if (fontPattern & (1 << SEG_E)) mainByte |= (1 << 3); // E -> bit 3
        if (fontPattern & (1 << SEG_C)) mainByte |= (1 << 4); // C -> bit 4
        if (fontPattern & (1 << SEG_F)) mainByte |= (1 << 5); // F -> bit 5
        if (fontPattern & (1 << SEG_A)) mainByte |= (1 << 6); // A -> bit 6
        if (fontPattern & (1 << SEG_G)) mainByte |= (1 << 7); // G -> bit 7
        if (fontPattern & (1 << SEG_B)) b_segment_byte |= (1 << 0); // B -> bit 0 of next byte

        videoRam[hw_index] |= mainByte;
        videoRam[hw_index + 1] |= b_segment_byte;
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

// -- ALL OTHER MODES --
void Nju6432Display::setBrightness(byte level) {
    if (_inhibitPin == NJU_NO_PIN) return;
    _brightness = level;
    if (!_isBlinking) {
        analogWrite(_inhibitPin, _brightness);
    }
}

void Nju6432Display::startBlink(unsigned int interval) {
    if (_inhibitPin == NJU_NO_PIN) return;
    stopAllModes();
    _blinkInterval = interval;
    _isBlinking = true;
    _lastBlinkTime = millis();
    _blinkStateOn = true;
}

void Nju6432Display::stopBlink() {
    _isBlinking = false;
    if (_inhibitPin != NJU_NO_PIN) {
        setBrightness(_brightness);
    }
}

bool Nju6432Display::updateBlink() {
    if (!_isBlinking) return false;
    if (millis() - _lastBlinkTime >= _blinkInterval) {
        _lastBlinkTime = millis();
        _blinkStateOn = !_blinkStateOn;
        if (_inhibitPin != NJU_NO_PIN) {
            analogWrite(_inhibitPin, _blinkStateOn ? _brightness : 0);
        }
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
    if (_inhibitPin == NJU_NO_PIN) return;
    stopAllModes();
    print(text, 0);
    updateDisplay();
    for (int i = 0; i <= _brightness; i++) {
        analogWrite(_inhibitPin, i);
        delay(stepDelay);
    }
}

void Nju6432Display::fadeOut(unsigned int stepDelay) {
    if (_inhibitPin == NJU_NO_PIN) return;
    stopAllModes();
    for (int i = _brightness; i >= 0; i--) {
        analogWrite(_inhibitPin, i);
        delay(stepDelay);
    }
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
    for (int i = 0; i < 14; i++) videoRam[i] = 0xFF;
    updateDisplay();
    delay(delayMs * 2);
    clear();
    updateDisplay();
    delay(delayMs);
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