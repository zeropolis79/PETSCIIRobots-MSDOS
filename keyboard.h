// Original 83 Keys from the IBM 83-key Model F keyboard
#define SCAN_NONE              0x00
#define SCAN_ESC               0x01
#define SCAN_1                 0x02
#define SCAN_2                 0x03
#define SCAN_3                 0x04
#define SCAN_4                 0x05
#define SCAN_5                 0x06
#define SCAN_6                 0x07
#define SCAN_7                 0x08
#define SCAN_8                 0x09
#define SCAN_9                 0x0A
#define SCAN_0                 0x0B
#define SCAN_MINUS             0x0C
#define SCAN_EQUALS            0x0D
#define SCAN_BACKSPACE         0x0E
#define SCAN_TAB               0x0F
#define SCAN_Q                 0x10
#define SCAN_W                 0x11
#define SCAN_E                 0x12
#define SCAN_R                 0x13
#define SCAN_T                 0x14
#define SCAN_Y                 0x15
#define SCAN_U                 0x16
#define SCAN_I                 0x17
#define SCAN_O                 0x18
#define SCAN_P                 0x19
#define SCAN_LEFT_BRACE        0x1A
#define SCAN_RIGHT_BRACE       0x1B
#define SCAN_ENTER             0x1C
#define SCAN_LEFT_CONTROL      0x1D
#define SCAN_A                 0x1E
#define SCAN_S                 0x1F
#define SCAN_D                 0x20
#define SCAN_F                 0x21
#define SCAN_G                 0x22
#define SCAN_H                 0x23
#define SCAN_J                 0x24
#define SCAN_K                 0x25
#define SCAN_L                 0x26
#define SCAN_SEMICOLON         0x27
#define SCAN_APOSTROPHE        0x28
#define SCAN_ACCENT            0x29
#define SCAN_TILDE             0x29 // Duplicate of SCAN_ACCENT with popular Tilde name.
#define SCAN_LEFT_SHIFT        0x2A
#define SCAN_BACK_SLASH        0x2B
#define SCAN_Z                 0x2C
#define SCAN_X                 0x2D
#define SCAN_C                 0x2E
#define SCAN_V                 0x2F
#define SCAN_B                 0x30
#define SCAN_N                 0x31
#define SCAN_M                 0x32
#define SCAN_COMMA             0x33
#define SCAN_PERIOD            0x34
#define SCAN_FORWARD_SLASH     0x35
#define SCAN_RIGHT_SHIFT       0x36
#define SCAN_KP_STAR           0x37
#define SCAN_KP_MULTIPLY       0x37 // Duplicate of SCAN_KP_STAR
#define SCAN_LEFT_ALT          0x38
#define SCAN_SPACE             0x39
#define SCAN_CAPS_LOCK         0x3A
#define SCAN_F1                0x3B
#define SCAN_F2                0x3C
#define SCAN_F3                0x3D
#define SCAN_F4                0x3E
#define SCAN_F5                0x3F
#define SCAN_F6                0x40
#define SCAN_F7                0x41
#define SCAN_F8                0x42
#define SCAN_F9                0x43
#define SCAN_F10               0x44
#define SCAN_NUM_LOCK          0x45
#define SCAN_SCROLL_LOCK       0x46
#define SCAN_KP_7              0x47
#define SCAN_KP_8              0x48
#define SCAN_KP_9              0x49
#define SCAN_KP_MINUS          0x4A
#define SCAN_KP_4              0x4B
#define SCAN_KP_5              0x4C
#define SCAN_KP_6              0x4D
#define SCAN_KP_PLUS           0x4E
#define SCAN_KP_1              0x4F
#define SCAN_KP_2              0x50
#define SCAN_KP_3              0x51
#define SCAN_KP_0              0x52
#define SCAN_KP_PERIOD         0x53

// Extended keys for the IBM 101-key Model M keyboard.
#define SCAN_RIGHT_ALT         0xB7
#define SCAN_RIGHT_CONTROL     0x9C
#define SCAN_LEFT_ARROW        0xCA
#define SCAN_RIGHT_ARROW       0xCC
#define SCAN_UP_ARROW          0xC7
#define SCAN_DOWN_ARROW        0xCF
#define SCAN_NUMPAD_ENTER      0x9B
#define SCAN_INSERT            0xD1
#define SCAN_DELETE            0xD2
#define SCAN_HOME              0xC6
#define SCAN_END               0xCE
#define SCAN_PAGE_UP           0xC8
#define SCAN_PAGE_DOWN         0xD0
#define SCAN_KP_FORWARD_SLASH  0xB4
#define SCAN_PRINT_SCREEN      0xA9

// Arrow keys for Tandy
#define SCAN_UP_ARROW_TANDY    0x29
#define SCAN_DOWN_ARROW_TANDY  0x4A
#define SCAN_LEFT_ARROW_TANDY  0x2B
#define SCAN_RIGHT_ARROW_TANDY 0x4E

#define MODIFIER_SHIFT         0x100
#define MODIFIER_CTRL          0x200
#define MODIFIER_ALT           0x400

#ifndef __KEYBOARD_H_INCLUDED__
#define __KEYBOARD_H_INCLUDED__

#include "globals.h"

#define MAX_KEYBOARD_BUFFER	1

extern volatile uint16_t keyBuf[MAX_KEYBOARD_BUFFER];
extern volatile uint8_t bufLen;
extern volatile uint16_t keyHeld;

void keyboardInstallInterrupt();
void keyboardUninstallInterrupt();
int16_t keyGet();
uint16_t keyWait();

#endif //__KEYBOARD_H_INCLUDED__
