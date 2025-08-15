/*
 * Library for driving an NJU6432-based display.
 * File: Nju6432Display.h
 * Version: 5.0 (Final, Complete)
 */
#ifndef NJU6432_DISPLAY_H
#define NJU6432_DISPLAY_H

#include "Arduino.h"
#include "DisplayConstants.h"

// A special value to indicate an unused pin (for 3-pin setup)
static const byte NJU_NO_PIN = 255; 
#define NJU_MAX_SCROLL_TEXT_LENGTH 64

class Nju6432Display {
public:
    // -- Constructors --
    // For 4-pin setup (with inhibit/brightness control)
    Nju6432Display(byte inhibitPin, byte dataPin, byte clockPin, byte chipEnablePin);
    // For 3-pin setup (no brightness/blink control)
    Nju6432Display(byte dataPin, byte clockPin, byte chipEnablePin);

    // -- Core Functions --
    void begin();
    void updateDisplay();
    void clear();

    // -- High-Level "Framebuffer" Functions --
    void setChar(byte position, char character, bool decimalPoint = false);
    void print(const char* text, byte startPosition = 0);

    // -- Brightness Control --
    void setBrightness(byte level); // 0 (off) to 255 (max)

    // -- Blinking Mode --
    void startBlink(unsigned int interval);
    void stopBlink();
    bool updateBlink();

    // -- Dynamic Text Modes --
    void scrollText(const char* text, unsigned int scrollSpeed);
    bool updateScroll();
    void stopScroll();
    
    void typewriter(const char* text, unsigned int charInterval);
    bool updateTypewriter();
    void stopTypewriter();
    
    void fadeIn(const char* text, unsigned int stepDelay);
    void fadeOut(unsigned int stepDelay);

    // -- Visualization Modes --
    void displayBarGraph(int value, int maxValue, bool fromCenter = false);
    void knightRider(unsigned int speed);
    bool updateKnightRider();
    void stopKnightRider();

    // -- Diagnostics --
    void runDiagnostics(unsigned int delayMs = 100);

    // Public buffer for direct manipulation if needed
    byte videoRam[14] = {0};

private:
    // -- Internal Members & Functions --
    void sendBlock(byte start, byte end);
    void applyControlBits(byte bufferIndex, byte controlWord);
    byte getCharacterFont(char c);
    void stopAllModes();

    // Pin assignments
    byte _inhibitPin, _dataPin, _clockPin, _chipEnablePin;

    // Display Protocol Internals
    static const byte CONTROL_BLOCK_0 = 0b00100000;
    static const byte CONTROL_BLOCK_1 = 0b10000000;
    byte _transferBuffer[14] = {0};
    
    // State management for all non-blocking modes
    byte _brightness = 255;

    bool _isBlinking = false, _blinkStateOn = true;
    unsigned int _blinkInterval = 500;
    unsigned long _lastBlinkTime = 0;

    bool _isScrolling = false;
    char _scrollTextBuffer[NJU_MAX_SCROLL_TEXT_LENGTH];
    int _scrollTextLen = 0, _scrollPosition = 0;
    unsigned int _scrollSpeed = 200;
    unsigned long _lastScrollTime = 0;
    
    bool _isTyping = false;
    char _typewriterBuffer[NJU_MAX_SCROLL_TEXT_LENGTH];
    int _typewriterLen = 0, _typewriterPos = 0;
    unsigned int _typewriterInterval = 150;
    unsigned long _lastTypeTime = 0;

    bool _isScanning = false;
    int _scannerPos = 0, _scannerDir = 1;
    unsigned int _scannerSpeed = 50;
    unsigned long _lastScanTime = 0;
};

#endif // NJU6432_DISPLAY_H