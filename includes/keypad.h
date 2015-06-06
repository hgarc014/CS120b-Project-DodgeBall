#ifndef KEYPAD_H
#define KEYPAD_H
#include <avr/io.h>
#include "C:\Users\student\Desktop\includes\bit.h"
// Returns '\0' if no key pressed, 
// Else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement:
 
   PC4   PC5 PC6 PC7
  col 1   2   3  4
row
PC0 1 1 | 2 | 3 | A
PC1 2 4 | 5 | 6 | B
PC2 3 7 | 8 | 9 | C
PC3 4 * | 0 | # | D
*/


enum KEY_STATES{keyRow1,keyRow2,keyRow3,keyRow4,key_default}KEY_STATE;
unsigned char GetKeypadKey() {
// Check keys in col 1
// Enable col 4 with 0, disable others with 1’s
// The delay allows PORTA to stabilize before checking


switch(KEY_STATE)
{
	case keyRow1:
	PORTA = 0xEF;
	asm("nop");
	if (GetBit(PINA,0)==0) { return('D'); }
	else if (GetBit(PINA,1)==0) { return('C'); }
	else if (GetBit(PINA,2)==0) { return('B'); }
	else if (GetBit(PINA,3)==0) { return('A'); }
	else KEY_STATE = keyRow2;
	break;
	
	case keyRow2:
	PORTA = 0xDF;
	asm("nop");
	if (GetBit(PINA,0)==0) { return('#'); }
	else if (GetBit(PINA,1)==0) { return('9'); }
	else if (GetBit(PINA,2)==0) { return('6'); }
	else if (GetBit(PINA,3)==0) { return('3'); }
	else KEY_STATE = keyRow3;
	break;
	
	case keyRow3:
	PORTA = 0xBF;
	asm("nop");
	if (GetBit(PINA,0)==0) { return('0'); }
	else if (GetBit(PINA,1)==0) { return('8'); }
	else if (GetBit(PINA,2)==0) { return('5'); }
	else if (GetBit(PINA,3)==0) { return('2'); }
	else KEY_STATE = keyRow4;
	break;
	
	case keyRow4:
	PORTA = 0x0F;
	asm("nop");
	if (GetBit(PINA,0)==0) { return('*'); }
	else if (GetBit(PINA,1)==0) { return('7'); }
	else if (GetBit(PINA,2)==0) { return('4'); }
	else if (GetBit(PINA,3)==0) { return('1'); }
	else KEY_STATE = key_default;
	break;
	
	case key_default:
	KEY_STATE = keyRow1;
	return('\0'); // default value
	break;
	
	default:
	KEY_STATE = keyRow1;
	
	break;
	
	
}
/*
PORTA = 0xEF;
asm("nop");
if (GetBit(PINA,0)==0) { return('D'); }
if (GetBit(PINA,1)==0) { return('C'); }
if (GetBit(PINA,2)==0) { return('B'); }
if (GetBit(PINA,3)==0) { return('A'); }
//if (GetBit(PINA,5)==0 && GetBit(PINA,0) == 0) { return('3'); }

// Check keys in col 2
// Enable col 5 with 0, disable others with 1’s
// The delay allows PORTA to stabilize before checking
PORTA = 0xDF;
asm("nop");
if (GetBit(PINA,0)==0) { return('#'); }
if (GetBit(PINA,1)==0) { return('9'); }
if (GetBit(PINA,2)==0) { return('6'); }
if (GetBit(PINA,3)==0) { return('3'); }
// ... *****FINISH*****
// Check keys in col 3
// Enable col 6 with 0, disable others with 1’s
// The delay allows PORTA to stabilize before checking
PORTA = 0xBF;
asm("nop");
if (GetBit(PINA,0)==0) { return('0'); }
if (GetBit(PINA,1)==0) { return('8'); }
if (GetBit(PINA,2)==0) { return('5'); }
if (GetBit(PINA,3)==0) { return('2'); }
// ... *****FINISH*****
// Check keys in col 4
PORTA = 0x0F;
asm("nop");
if (GetBit(PINA,0)==0) { return('*'); }
if (GetBit(PINA,1)==0) { return('7'); }
if (GetBit(PINA,2)==0) { return('4'); }
if (GetBit(PINA,3)==0) { return('1'); }
// ... *****FINISH*****
return('\0'); // default value*/
}

/*
int main(void)
{
	unsigned char x;
	// PORTB set to output, outputs init 0s
	DDRB = 0xFF; PORTB = 0x00;
	// PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRC = 0xF0; PORTA = 0x0F;
	while(1) {
		x = GetKeypadKey();
		switch (x) {
			// All 5 LEDs on
			case '\0': PORTB = 0x1F; break;
			// hex equivalent
			case '1': PORTB = 0x01; break;
			case '2': PORTB = 0x02; break;
			// . . . ***** FINISH *****
			case 'D': PORTB = 0x0D; break;
			case '*': PORTB = 0x0E; break;
			case '0': PORTB = 0x00; break;
			case '#': PORTB = 0x0F; break;
			// Should never occur. Middle LED off.
			default: PORTB = 0x1B; break;
		}
	}
}*/
#endif