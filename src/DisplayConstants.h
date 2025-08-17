/*
 * File: DisplayConstants.h
 * Contains hardware and software constants for the display library.
 * CUSTOMIZED based on user-provided hardware map.
 */
#ifndef DISPLAY_CONSTANTS_H
#define DISPLAY_CONSTANTS_H

// -- Custom Segment-to-Bit Mapping for 7-Segment Digits --
constexpr byte SEG_D = 0;
constexpr byte SEG_H = 1; // Decimal Point
constexpr byte SEG_E = 2;
constexpr byte SEG_C = 3;
constexpr byte SEG_F = 4;
constexpr byte SEG_A = 5;
constexpr byte SEG_G = 6;
constexpr byte SEG_B = 7;

// -- Standard 7-Segment Font Table --
constexpr byte FONT_BLANK = 0;
constexpr byte FONT_TABLE_SIZE = 16;

constexpr byte sevenSegmentFont[FONT_TABLE_SIZE] = {
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F), // 0
    (1 << SEG_B | 1 << SEG_C),                                                    // 1
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_D | 1 << SEG_E | 1 << SEG_G),              // 2
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_G),              // 3
    (1 << SEG_B | 1 << SEG_C | 1 << SEG_F | 1 << SEG_G),                           // 4
    (1 << SEG_A | 1 << SEG_C | 1 << SEG_D | 1 << SEG_F | 1 << SEG_G),              // 5
    (1 << SEG_A | 1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G), // 6
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C),                                       // 7
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G), // 8
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_F | 1 << SEG_G), // 9
    (1 << SEG_A | 1 << SEG_B | 1 << SEG_C | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G), // A
    (1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G),              // b
    (1 << SEG_A | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F),                           // C
    (1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_G),              // d
    (1 << SEG_A | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G),              // E
    (1 << SEG_A | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G)                            // F
};

// -- Additional Character Fonts --
constexpr byte FONT_H      = (1 << SEG_B | 1 << SEG_C | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G);
constexpr byte FONT_L      = (1 << SEG_D | 1 << SEG_E | 1 << SEG_F);
constexpr byte FONT_P      = (1 << SEG_A | 1 << SEG_B | 1 << SEG_E | 1 << SEG_F | 1 << SEG_G);
constexpr byte FONT_U      = (1 << SEG_B | 1 << SEG_C | 1 << SEG_D | 1 << SEG_E | 1 << SEG_F);
constexpr byte FONT_MINUS  = (1 << SEG_G);
constexpr byte FONT_UNDER  = (1 << SEG_D);
constexpr byte FONT_DEGREE = (1 << SEG_A | 1 << SEG_B | 1 << SEG_F | 1 << SEG_G);

// -- Fonts for Bar Graph --
constexpr byte FONT_BAR_MID  = (1 << SEG_G);
constexpr byte FONT_BAR_FULL = (1 << SEG_A | 1 << SEG_D | 1 << SEG_G);

// -- Special Symbol Definitions --
constexpr byte ICON_ENTER_BYTE = 10;
constexpr byte ICON_ENTER_BIT  = (1 << 1);
constexpr byte ICON_MINUS_BYTE = 10;
constexpr byte ICON_MINUS_BIT  = (1 << 2);
constexpr byte ICON_UPPER_MINUS_BYTE = 10;
constexpr byte ICON_UPPER_MINUS_BIT  = (1 << 3);
constexpr byte ICON_UPPER_PLUS_BYTE = 10;
constexpr byte ICON_UPPER_PLUS_BIT  = (1 << 4);
constexpr byte ICON_UPPER_SUB_BYTE = 10;
constexpr byte ICON_UPPER_SUB_BIT  = (1 << 5);
constexpr byte ICON_UPPER_PROD_BYTE = 10;
constexpr byte ICON_UPPER_PROD_BIT  = (1 << 6);
constexpr byte ICON_UPPER_P_BYTE = 10;
constexpr byte ICON_UPPER_P_BIT  = (1 << 7);

constexpr byte ICON_UPPER_EQUAL_BYTE = 11;
constexpr byte ICON_UPPER_EQUAL_BIT  = (1 << 0);
constexpr byte ICON_UPPER_PROD2_BYTE = 11;
constexpr byte ICON_UPPER_PROD2_BIT  = (1 << 1);
constexpr byte ICON_UPPER_Z_BYTE = 11;
constexpr byte ICON_UPPER_Z_BIT  = (1 << 2);
constexpr byte ICON_UPPER_ECR_BYTE = 11;
constexpr byte ICON_UPPER_ECR_BIT  = (1 << 3);
constexpr byte ICON_UPPER_R_BYTE = 11;
constexpr byte ICON_UPPER_R_BIT  = (1 << 4);
constexpr byte ICON_TILDED_M_BYTE = 11;
constexpr byte ICON_TILDED_M_BIT  = (1 << 5);
constexpr byte ICON_UPPER_STL_BYTE = 11;
constexpr byte ICON_UPPER_STL_BIT  = (1 << 6);

constexpr byte ICON_BATTERY_BYTE   = 12;
constexpr byte ICON_BATTERY_SHELL  = (1 << 0);
constexpr byte ICON_BATTERY_SEG_3 = (1 << 1);
constexpr byte ICON_BATTERY_SEG_2 = (1 << 3);
constexpr byte ICON_BATTERY_SEG_1 = (1 << 5);
constexpr byte ICON_BATTERY_SEG_4_BYTE = 11;
constexpr byte ICON_BATTERY_SEG_4_BIT = (1 << 7);

#endif // DISPLAY_CONSTANTS_H