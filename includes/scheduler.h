#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include "C:\Users\student\Desktop\includes\bit.h"
#include "C:\Users\student\Desktop\includes\timer.h"
#include <stdio.h>
//--------Find GCD function -------------------------------
unsigned long int findGCD (unsigned long int a,
 unsigned long int b)
{
unsigned long int c;
while(1){
c = a%b;
if(c==0){return b;}
a = b;
b = c;
}
return 0;
}
//--------End find GCD function ---------------------------
//--------Task scheduler data structure--------------------
// Struct for Tasks represent a running process in our 
// simple real-time operating system.
/*Tasks should have members that include: state, period, a 
measurement of elapsed time, and a function pointer.*/
typedef struct _task {
//Task's current state, period, and the time elapsed
// since the last tick
signed char state;
unsigned long int period;
unsigned long int elapsedTime;
//Task tick function
int (*TickFct)(int);
} task;
//--------End Task scheduler data structure----------------
//--------Shared Variables---------------------------------
unsigned char SM2_output = 0x00;
unsigned char SM3_output = 0x00;
unsigned char pause = 0;
//--------End Shared Variables-----------------------------

/*
//--------User defined FSMs--------------------------------
enum SM1_States { SM1_wait, SM1_press, SM1_release };
// Monitors button connected to PA0. When the button is 
// pressed, shared variable "pause" is toggled.
int SMTick1(int state) {
// Local Variables
unsigned char press = ~PINA & 0x01;
//State machine transitions
switch (state) {
// Wait for button press
case SM1_wait:
if (press == 0x01) { state = SM1_press; }
break;
// Button remains pressed
case SM1_press:
state = SM1_release;
break;
// Wait for button release
case SM1_release:
if (press == 0x00) { state = SM1_wait; }
break;
// default: Initial state
default:
state = SM1_wait; 
break;
}
//State machine actions
switch(state) {
case SM1_wait: break;
case SM1_press: // toggle pause
pause = (pause == 0) ? 1 : 0; 
break;
case SM1_release: break;
default: break;
}
return state;
}

enum SM2_States { SM2_wait, SM2_blink };
// If paused: Do NOT toggle LED connected to PB0
// If unpaused: toggle LED connected to PB0
int SMTick2(int state) {
//State machine transitions
switch (state) {
case SM2_wait:
// If unpaused, go to blink state
if (pause == 0) { state = SM2_blink; }
break;
case SM2_blink:
// If paused, go to wait state
if (pause == 1) { state = SM2_wait; }
break;
default:
state = SM2_wait;
break;
}
//State machine actions
switch(state) {
case SM2_wait:
break;
//toggle LED
case SM2_blink:
SM2_output = (SM2_output == 0x00) ? 0x01 : 0x00; 
break;
default:
break;
}
return state;
}

enum SM3_States { SM3_wait, SM3_blink };
// If paused: Do NOT toggle LED connected to PB1
// If unpaused: toggle LED connected to PB1
int SMTick3(int state) {
//State machine transitions
switch (state) {
case SM3_wait:
// If unpaused, go to blink state
if (pause == 0) { state = SM3_blink; }
break;
case SM3_blink:
// If paused, go to wait state
if (pause == 1) { state = SM3_wait; }
break;
default:
state = SM3_wait;
break;
}
//State machine actions
switch(state) {
case SM3_wait:
break;
case SM3_blink:
//toggle LED
SM3_output = (SM3_output == 0x00) ? 0x02 : 0x00; 
break;
default:
break;
}
return state;
}

enum SM4_States { SM4_display };
// Combine blinking LED outputs from SM2 and SM3, and 
//output on PORTB
int SMTick4(int state) {
unsigned char output;
//State machine transitions
switch (state) {
case SM4_display:
break;
default:
state = SM4_display;
break;
}
//State machine actions
switch(state) {
case SM4_display:
// write shared outputs to local variables
output = SM2_output | SM3_output; 
break;
default:
break;
}
// Write combined, shared output variables to PORTB
PORTB = output;
return state;
}
*/
// --------END User defined FSMs---------------------------
/*
// Implement scheduler code from PES.
int main()
{
// Set Data Direction Registers
// Buttons PORTA[0-7], set AVR PORTA 
// to pull down logic
DDRA = 0x00; PORTA = 0xFF;
DDRB = 0xFF; PORTB = 0x00;
// . . . etc
// Period for the tasks
unsigned long int SMTick1_calc = 50;
unsigned long int SMTick2_calc = 500;
unsigned long int SMTick3_calc = 1000;
unsigned long int SMTick4_calc = 10;
//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);
//Greatest common divisor for all tasks 
// or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;
//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;
//Declare an array of tasks 
static task task1, task2, task3, task4;
task *tasks[] = { &task1, &task2, &task3, &task4 };
const unsigned short numTasks =
sizeof(tasks)/sizeof(task*);
// Task 1
task1.state = -1;
task1.period = SMTick1_period;
task1.elapsedTime = SMTick1_period;
task1.TickFct = &SMTick1// Task 2
task2.state = -1;
task2.period = SMTick2_period;
task2.elapsedTime = SMTick2_period;
task2.TickFct = &SMTick2;
// Task 3
task3.state = -1;
task3.period = SMTick3_period;
task3.elapsedTime = SMTick3_period; 
task3.TickFct = &SMTick3; 
// Task 4
task4.state = -1;
task4.period = SMTick4_period;
task4.elapsedTime = SMTick4_period;
task4.TickFct = &SMTick4;
// Set the timer and turn it on
TimerSet(GCD);
TimerOn();
// Scheduler for-loop iterator
unsigned short i;
while(1) {
// Scheduler code
for ( i = 0; i < numTasks; i++ ) {
// Task is ready to tick
if ( tasks[i]->elapsedTime ==
tasks[i]->period ) {
// Setting next state for task
tasks[i]->state =
 tasks[i]->TickFct(tasks[i]->state);
// Reset elapsed time for next tick.
tasks[i]->elapsedTime = 0;
}
tasks[i]->elapsedTime += 1;
}
while(!TimerFlag);
TimerFlag = 0;
}
// Error: Program should not exit!
return 0;
}*/
#endif