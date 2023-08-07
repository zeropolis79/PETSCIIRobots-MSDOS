#define KEYBOARD_CONTROLLER_OUTPUT_BUFFER 0x60
#define KEYBOARD_CONTROLLER_STATUS_REGISTER 0x64
#define KEYBOARD_INTERRUPT_VECTOR 0x09

// PPI stands for Programmable Peripheral Interface (which is the Intel 8255A chip)
// The PPI ports are only for IBM PC and XT, however port A is mapped to the same
// I/O address as the Keyboard Controller's (Intel 8042 chip) output buffer for compatibility.
#define PPI_PORT_A 0x60
#define PPI_PORT_B 0x61
#define PPI_PORT_C 0x62
#define PPI_COMMAND_REGISTER 0x63

#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include "keyboard.h"

// The keyboard controller, by default, will send scan codes
// in Scan Code Set 1 (reference the IBM Technical References
// for a complete list of scan codes).
//
// Scan codes in this set come as make/break codes. The make
// code is the normal scan code of the key, and the break code
// is the make code bitwise "OR"ed with 0x80 (the high bit is set).
//
// On keyboards after the original IBM Model F 83-key, an 0xE0
// is prepended to some keys that didn't exist on the original keyboard.
//
// Some keys have their scan codes affected by the state of
// the shift, and num-lock keys. These certain
// keys have, potentially, quite long scan codes with multiple
// possible 0xE0 bytes along with other codes to indicate the
// state of the shift, and num-lock keys.
//
// There are two other Scan Code Sets, Set 2 and Set 3. Set 2
// was introduced with the IBM PC AT, and Set 3 with the IBM
// PS/2. Set 3 is by far the easiest and most simple set to work
// with, but not all keyboards support it.
//
// Note:
// The "keyboard controller" chip is different depending on
// which machine is being used. The original IBM PC uses the
// Intel 8255A-5, while the IBM PC AT uses the Intel 8042 (UPI-42AH).
// On the 8255A-5, port 0x61 can be read and written to for various
// things, one of which will clear the keyboard and disable it or
// re enable it. There is no such function on the AT and newer, but
// it is not needed anyways. The 8042 uses ports 0x60 and 0x64. Both
// the 8255A-5 and the 8042 give the scan codes from the keyboard
// through port 0x60.

// On the IBM PC and XT and compatibles, you MUST clear the keyboard
// after reading the scancode by reading the value at port 0x61,
// flipping the 7th bit to a 1, and writing that value back to port 0x61.
// After that is done, flip the 7th bit back to 0 to re-enable the keyboard.
//
// On IBM PC ATs and newer, writing and reading port 0x61 does nothing (as far
// as I know), and using it to clear the keyboard isn't necessary.

#define MAX_KEYS_HELD	5

// Keyboard variables
volatile uint16_t keyBuf[MAX_KEYBOARD_BUFFER];
volatile uint8_t bufLen = 0;
volatile uint8_t keysHeld[MAX_KEYS_HELD];
volatile uint8_t keysHeldCount = 0;
volatile uint16_t keyHeld = 0;
uint16_t modifier = 0;
uint8_t isPreviousCodeExtended = 0;
void interrupt (*oldKeyboardIsr)() = (void *)0;

// Joystick variables
volatile uint8_t joystickEnabled = 1;
volatile uint8_t keyHeldByJoystick = 0;
volatile uint8_t joystickButton1 = 0; // A Button - Fire Button
volatile uint8_t joystickButton2 = 0; // B Button - Search/Move Button
volatile uint8_t joystickButton3 = 0; // X Button - Use Button
volatile uint8_t joystickButton4 = 0; // Y Button - Pause/Cycle Weapon/Cycle Item/Map
volatile uint16_t joystickXAxis;
uint16_t joystickXAxisLow = 0xFFFF;
uint16_t joystickXAxisLeft = 0xFFFF;
uint16_t joystickXAxisCenter = 0xFFFF;
uint16_t joystickXAxisRight = 0;
uint16_t joystickXAxisHigh = 0;
volatile uint16_t joystickYAxis;
uint16_t joystickYAxisLow = 0xFFFF;
uint16_t joystickYAxisUp = 0xFFFF;
uint16_t joystickYAxisCenter = 0xFFFF;
uint16_t joystickYAxisDown = 0;
uint16_t joystickYAxisHigh = 0;
int joystickDebounce;

uint8_t color = 0;

void joystickRead(){
	
	_asm {
		// Disable interrupts
		cli
		// Init SI (X axis value), DI (Y axis value) to 0, and BX (button values)
		xor bx, bx
		mov si, bx
		mov di, bx
		// Set a mask to indicate what bits we still need to read
		mov ah, 3
		// Write a byte to the joystick port (0x0201) to get timing values for X/Y axis
		mov dx, 0x0201
		out dx, al
		// Set how many times to loop
		mov cx, 0xFFFF
	read_loop:
		// Read from joystick
		in al, dx
		mov bh, al
		not bh
		or bl, bh
		and ah, al
		jz done
		cmp ah, 2
		jl x_axis
		inc di
		cmp ah, 2
		je check_done
	x_axis:
		inc si
	check_done:
		loop read_loop
		mov SS:joystickEnabled, 0
	done:
		// Enable interrupts
		sti
		// Save joystick axis values
		mov al, bl
		and al, 16
		mov SS:joystickButton1, al
		mov al, bl
		and al, 32
		mov SS:joystickButton2, al
		mov al, bl
		and al, 64
		mov SS:joystickButton3, al
		and bl, 128
		mov SS:joystickButton4, bl
		mov SS:joystickXAxis, si
		mov SS:joystickYAxis, di
	}
	if (joystickEnabled){
		if (joystickXAxisCenter == 0xFFFF){
			// Set initial centered values for axis
			joystickXAxisCenter = joystickXAxis;
			joystickYAxisCenter = joystickYAxis;
			joystickXAxisLow = joystickXAxis >> 1;
			joystickXAxisHigh = joystickXAxis + joystickXAxisLow;
			joystickXAxisLeft = joystickXAxisLow;
			joystickXAxisRight = joystickXAxisHigh;
			joystickYAxisLow = joystickYAxis >> 1;
			joystickYAxisHigh = joystickYAxis + joystickYAxisLow;
			joystickYAxisUp = joystickYAxisLow;
			joystickYAxisDown = joystickYAxisHigh;
		} else{
			if (joystickXAxis < joystickXAxisLow && joystickXAxis < joystickXAxisCenter - 10){
				joystickXAxisLow = joystickXAxis;
				joystickXAxisLeft = (joystickXAxisCenter - joystickXAxis) >> 2;
			}
			if (joystickXAxis > joystickXAxisHigh && joystickXAxis > joystickXAxisCenter + 10){
				joystickXAxisHigh = joystickXAxis;
				joystickXAxisRight = joystickXAxisHigh - ((joystickXAxis - joystickXAxisCenter) >> 2);
			}
			if (joystickYAxis < joystickYAxisLow && joystickYAxis < joystickYAxisCenter - 10){
				joystickYAxisLow = joystickYAxis;
				joystickYAxisUp = (joystickYAxisCenter - joystickYAxis) >> 2;
			}
			if (joystickYAxis > joystickYAxisHigh && joystickYAxis > joystickYAxisCenter + 10){
				joystickYAxisHigh = joystickYAxis;
				joystickYAxisDown = joystickYAxisHigh - ((joystickYAxis - joystickYAxisCenter) >> 2);
			}
		}
	}
}

void joystickTranslate(){
	int key = 0, xdist, ydist;
	
	if (bufLen == 0 || keyHeldByJoystick == 1){
		if (joystickButton1 && joystickButton4){
			// Combo to exit game
			key = (SCAN_ESC | MODIFIER_SHIFT);
		} else if (joystickButton2 && joystickButton3){
			// Combo to toggle music
			key = (SCAN_M | MODIFIER_SHIFT);
		} else if (joystickButton3){
			// Use key for Gravis controller
			key = (SCAN_NUMPAD_ENTER | MODIFIER_SHIFT);
		} else if (joystickButton2){
			// Search (1 click) or Move (2 clicks) button
			key = (SCAN_TILDE | MODIFIER_SHIFT);
		} else if (joystickXAxis < joystickXAxisLeft){
			if (joystickYAxis < joystickXAxis){
				key = SCAN_UP_ARROW;
			} else if (joystickYAxis > joystickYAxisDown && joystickYAxis - joystickYAxisDown > joystickXAxisLeft - joystickXAxis){
				key = SCAN_DOWN_ARROW;
			} else{
				key = SCAN_LEFT_ARROW;
			}
		} else if (joystickXAxis > joystickXAxisRight){
			if (joystickYAxis < joystickYAxisUp && joystickYAxisUp - joystickYAxis > joystickXAxis - joystickXAxisRight){
				key = SCAN_UP_ARROW;
			} else if (joystickYAxis > joystickYAxisDown && joystickYAxis - joystickYAxisDown > joystickXAxis - joystickXAxisRight){
				key = SCAN_DOWN_ARROW;
			} else{
				key = SCAN_RIGHT_ARROW;
			}
		} else if (joystickYAxis < joystickYAxisUp){
			key = SCAN_UP_ARROW;
		} else if (joystickYAxis > joystickYAxisDown){
			key = SCAN_DOWN_ARROW;
		}
		if (joystickButton1){
			switch (key){
				case SCAN_UP_ARROW:{
					key = CUSTOM_KEYS[KEYS_FIRE_UP];
					break;
				}
				case SCAN_DOWN_ARROW:{
					key = CUSTOM_KEYS[KEYS_FIRE_DOWN];
					break;
				}
				case SCAN_LEFT_ARROW:{
					key = CUSTOM_KEYS[KEYS_FIRE_LEFT];
					break;
				}
				case SCAN_RIGHT_ARROW:{
					key = CUSTOM_KEYS[KEYS_FIRE_RIGHT];
					break;
				}
			}
		} else if (joystickButton4){
			switch (key){
				case SCAN_UP_ARROW:{
					key = (SCAN_P | MODIFIER_SHIFT);
					break;
				}
				case SCAN_DOWN_ARROW:{
					key = SCAN_TAB | MODIFIER_CTRL;
					break;
				}
				case SCAN_LEFT_ARROW:{
					key = CUSTOM_KEYS[KEYS_CYCLE_WEAPONS];
					break;
				}
				case SCAN_RIGHT_ARROW:{
					key = CUSTOM_KEYS[KEYS_CYCLE_ITEMS];
					break;
				}
			}
		}
		if (key != 0 && keyHeldByJoystick == 0){
			keyHeldByJoystick = 1;
		} else{
			if (key != 0){
				keyBuf[0] = key;
				bufLen = 1;
				keyHeldByJoystick = 1;
			} else{
				keyHeldByJoystick = 0;
				bufLen = 0;
				keyHeld = 0;
			}
		}
	}
}

void interrupt far keyboardIsr(){
    uint8_t scanCode, ppiPortB, key_pressed, i, found;
	
	_asm {
		cli // disable interrupts
	};

	ppiPortB = inp(PPI_PORT_B); // get the current settings in PPI port B
	scanCode = inp(KEYBOARD_CONTROLLER_OUTPUT_BUFFER); // get the scancode waiting in the output buffer
	outp(PPI_PORT_B, ppiPortB | 0x80); // set the 7th bit of PPI port B (clear keyboard)
	outp(PPI_PORT_B, ppiPortB); // clear the 7th bit of the PPI (enable keyboard)
	
	// Check to see what the code was.
	// Note that we have to process the scan code one byte at a time.
	// This is because we can't get another scan code until the current
	// interrupt is finished.
	if (scanCode == 0xE0){
		// Extended scancode
		isPreviousCodeExtended = 1;
	} else{
		// Regular scancode
		// Check the high bit, if set, then it's a break code.
		if (scanCode & 0x80){
			key_pressed = 0;
			scanCode &= 0x7F;
		} else{
			key_pressed = 1;
		}
		if(isPreviousCodeExtended){
			isPreviousCodeExtended = 0;
			scanCode += 0x7F;
		}
		if (key_pressed){
			switch (scanCode){
				case SCAN_LEFT_SHIFT:
				case SCAN_RIGHT_SHIFT:{
					modifier |= MODIFIER_SHIFT;
					break;
				}
				case SCAN_LEFT_CONTROL:
				case SCAN_RIGHT_CONTROL:{
					modifier |= MODIFIER_CTRL;
					break;
				}
				case SCAN_LEFT_ALT:
				case SCAN_RIGHT_ALT:{
					modifier |= MODIFIER_ALT;
					break;
				}
				default:{
					// If the key pressed is not in the list of held keys, then reset keyHeld flag
					found = 0;
					if (scanCode != keyHeld){
						for (i = 0; i < keysHeldCount; i++){
							if (keysHeld[i] == scanCode){
								found = 1;
								break;
							}
						}
						if (!found){
							keyHeld = 0;
							// Add to the list of held keys
							if (keysHeldCount < MAX_KEYS_HELD){
								keysHeld[keysHeldCount] = scanCode;
								keysHeldCount++;
							} else{
								// Too many keys held, don't take any new keypresses
								found = 1;
								bufLen = 0;
							}
						}
					}
					if (!found){
						// Set the key being pressed in the buffer
						keyBuf[0] = scanCode | modifier;
						// Set buffer length
						bufLen = 1;
					}
				}
			}
		} else{
			switch (scanCode){
				case SCAN_LEFT_SHIFT:
				case SCAN_RIGHT_SHIFT:{
					modifier &= (MODIFIER_CTRL | MODIFIER_ALT);
					break;
				}
				case SCAN_LEFT_CONTROL:
				case SCAN_RIGHT_CONTROL:{
					modifier &= (MODIFIER_SHIFT | MODIFIER_ALT);
					break;
				}
				case SCAN_LEFT_ALT:
				case SCAN_RIGHT_ALT:{
					modifier &= (MODIFIER_SHIFT | MODIFIER_CTRL);
					break;
				}
				default:{
					// Remove key from buffer if it matches the current key in the buffer
					if (bufLen != 0 && (keyBuf[0] & 0xFF) == scanCode){
						bufLen = 0;
						keyHeld = 0;
					}
					// Remove from list of keys being held
					found = 0;
					for (i = 0; i < keysHeldCount; i++){
						if (keysHeld[i] == scanCode){
							found = 1;
						}
						if (found){
							keysHeld[i] = keysHeld[i + 1];
						}
					}
					if (found){
						keysHeldCount--;
					}
				}
			}
		}
	}

    // Send a "Non Specific End of Interrupt" command to the PIC.
    // See Intel 8259A datasheet for details.
    outp(0x20, 0x20);

    _asm{
        sti // enable interrupts
    };
}

void keyboardInstallInterrupt(){
    // Make sure the new ISR isn't already in use.
    if(oldKeyboardIsr == (void *)0){
        oldKeyboardIsr = _dos_getvect(KEYBOARD_INTERRUPT_VECTOR);
        _dos_setvect(KEYBOARD_INTERRUPT_VECTOR, keyboardIsr);
    }
}

void keyboardUninstallInterrupt(){
    // Make sure the new ISR is in use.
    if(oldKeyboardIsr != (void *)0){
        _dos_setvect(KEYBOARD_INTERRUPT_VECTOR, oldKeyboardIsr);
        oldKeyboardIsr = (void *)0;
    }
}

int16_t keyGet(){
	uint16_t key;
	
	// If the keyboard has been hit, return the key code. Otherwise return 0.

	#ifndef SHAREWARE
		if (JOYTIMER == 0 && joystickEnabled){
			joystickRead();
			joystickTranslate();
			JOYTIMER = 1;
		}
	#endif

	key = 0;
	if (KEYTIMER == 0 || keyHeld == 0){
		// Disable interrupts
		_asm{
			cli
		}
		// Fetch key in buffer. Update keyHeld for the key that was fetched
		if (bufLen != 0){
			key = keyBuf[0];
			if (KEY_NOREPEAT == 1 && keyHeld == (key & 0xFF)){
				key = 0;
			} else{
				keyHeld = key & 0xFF;
				KEYTIMER = 7;
				KEY_NOREPEAT = 0;
			}
		} else{
			KEYFAST = 0;
			KEY_NOREPEAT = 0;
		}
		// Enable interrupts
		_asm{
			sti
		}
	}
	return key;
}

uint16_t keyWait(){
	uint16_t key;
	
	// Wait for a new keypress. Once we get one, return it.
	key = 0;
	while(key == 0){
		while (KEYTIMER != 0 || keyHeld != 0) {}
		// Disable interrupts
		_asm{
			cli
		}
		// Check for new keypress. Update keyHeld for new key press
		if (bufLen != 0 && keyHeld == 0){
			key = keyBuf[0];
			keyHeld = key & 0xFF;
		}
		// Enable interrupts
		_asm{
			sti
		}
	}
	return key;
}


