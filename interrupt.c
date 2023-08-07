#include <string.h>
#include <i86.h>
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include "game.h"
#include "globals.h"
#include "video.h"
#include "audio.h"
#include "keyboard.h"
#include "vga.h"

// 8253 Programmable Interval Timer ports and registers
// The clock input is a 1.193181 MHz signal
#define TIMER_INTERRUPT_VECTOR  0x08
#define tickdiv					16571 // (1193181 / 16571) = 72.004Hz
#define tickdiv60hz				19886 // (1193181 / 19886) = 60.001Hz
#define tickpersec				1193181L / tickdiv

void interrupt (far *oldIntVector)() = 0L;
uint16_t oldIntTick, tick60hz;
uint8_t animate_timer = 10;

void interrupt far interruptRoutine(){
    _asm {
        cli // disable interrupts
    };

	// Play sound/music at 72Hz
	if (SOUND_ENABLED){
		ssy_timer_isr();
	}
	
	// Increment 60Hz tick counter
	tick60hz += tickdiv;
	if (tick60hz > tickdiv60hz){
		tick60hz -= tickdiv60hz;
		// Update game clock
		if (CLOCK.ACTIVE){
			CLOCK.CYCLES++;
			if (CLOCK.CYCLES == 60){
				CLOCK.CYCLES = 0;
				CLOCK.SECONDS++;
				// Set flag to update the time on the mini-map
				CLOCK.UPDATE = 1;
				if (CLOCK.SECONDS == 60){
					CLOCK.SECONDS = 0;
					CLOCK.MINUTES++;
					if (CLOCK.MINUTES == 60){
						CLOCK.MINUTES = 0;
						CLOCK.HOURS++;
					}
				}
			}
			// If BGTIMER has been set to 0 by backgroundtasks, then set to 1
			if (BGTIMER1 == 0){
				BGTIMER1 = 1;
				// Decrease the time until the next key repeat
				if (KEYTIMER != 0){
					KEYTIMER--;
				}
				if (JOYTIMER != 0){
					JOYTIMER--;
				}
				// Increase animation frame for water, HVAC, etc tiles at 6Hz
				animate_timer--;
				if (animate_timer == 0){
					animate_timer = 10;
					// Increase animation frame for water, HVAC, etc tiles
					TILE_ANIMATION_FRAME++;
					if (TILE_ANIMATION_FRAME > 3){
						TILE_ANIMATION_FRAME = 0;
					}
					// Flag to redraw, to make sure animation animates
					REDRAW_WINDOW = 1;
				}
			}
		} else{
			// Clock not active, decrement keytimer 
			if (KEYTIMER != 0){
				KEYTIMER--;
			}
			if (JOYTIMER != 0){
				JOYTIMER--;
			}
		}
		// Check if any events are currently enabled
		if (EVENT_ENABLED){
			// Set EVENT_ENABLED to 0 and turn it back on if any of the events are still running
			EVENT_ENABLED = 0;
			// Timer for searching for a hidden item, video fade in/out, etc
			if (EVENT_TIMER){
				EVENT_TIMER--;
				EVENT_ENABLED = 1;
			}
			// Flash background for player damage
			if (DAMAGE_FLASH_STEPS_REMAINING){
				videoDamageFlashStep();
				EVENT_ENABLED = 1;
			}
			// Flash background for EMP flash
			if (EMP_FLASH_STEPS_REMAINING){
				videoEMPFlashStep();
				EVENT_ENABLED = 1;
			}
			// Shake screen for explosions
			if (SCREEN_SHAKE){
				videoScreenShakeStep();
				EVENT_ENABLED = 1;
			}
			// Animate cursor color if it is enabled or the minimap is open
			if (CURSOR_ENABLED){
				CURSOR.color_index++;
				if (CURSOR.color_index > 31){
					CURSOR.color_index = 0;
				}
				CURSOR.last_color = CURSOR.color;
				CURSOR.color = CURSOR_SELECTED_COLOR[CURSOR.color_index];
				if (VIDEO_USE_PALETTE_ANIMATION){
					VGA_videoAnimateCursorAndRobotColors();
				}
				EVENT_ENABLED = 1;
			}
		}
	}
	// Increment a tick count for the old interrupt to fire once it overflows at 65536
	oldIntTick += tickdiv;
	if (oldIntTick < tickdiv){
		// Call original interrupt when oldIntTick overflows 65536 (18.2Hz)
		oldIntVector();
	} else{
		// Send end of interrupt (note: may not be supported on Microchannel machines?)
		outp(0x20, 0x20);
	}
    _asm {
        sti // enable interrupts
    };
}

int interruptDisable(){
    if (oldIntVector != 0L){
		// Disable interrupts
		_disable();
		// Restore original interrupt vector
		_dos_setvect(TIMER_INTERRUPT_VECTOR, oldIntVector);
		// Program the PIT timer to trigger at the default 18.2Hz
		outp(0x43, 0x36);
		outp(0x40, 0);
		outp(0x40, 0);
		// Disable PC Speaker
		outp(0x61, inp(0x61) & 0xFC);
		// Enable interrupts
		_enable();
		// Reset oldIntVector pointer back to NULL, in case we want to call to enable interrupts again
		oldIntVector = 0L;
		return 1;
    }
	return 0;
}

void SignalHandler(int sig){
	// This is called when the user tries to break out of the program, so we can disable the interrupt first
	signal(sig, SIG_IGN);	// Ignore signal (for if we want to continue later)
	interruptDisable();
	keyboardUninstallInterrupt();
	ssy_music_stop();
	exit(0);
	// signal(SIGINT, INThandler); // To listen for the interrupt again if we continue
}

void interruptEnable(){
	// Enable interrupt if it isn't already enabled
	if (oldIntVector == 0L){
		// Initialize a tick count that calls the original interrupt when it overflows 65536 back to 0
		oldIntTick = 0;
		// Save old interrupt vector to restore upon exit
		oldIntVector = _dos_getvect(TIMER_INTERRUPT_VECTOR);
		// Disable interrupts
		_disable();
		// Set our interrupt routine as the new interrupt vector
		_dos_setvect(TIMER_INTERRUPT_VECTOR, interruptRoutine);
		// Program the PIT timer to trigger at a higher frequency
		outp(0x43, 0x36);
		outp(0x40, tickdiv & 0xff);
		outp(0x40, tickdiv >> 8);
		// Enable interrupts
		_enable();
		// Setup signal handler to uninstall interrupt when user breaks out of program
		signal(SIGINT, SignalHandler);
	}
}


