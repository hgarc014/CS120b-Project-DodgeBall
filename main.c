#include <avr/io.h>

#include "C:\Users\student\Desktop\includes\keypad.h"
#include "C:\Users\student\Desktop\includes\scheduler.h"
#include "C:\Users\student\Desktop\includes\io.c"
#include "C:\Users\student\Desktop\includes\io.h"


#define ICON_BALL 0
#define ICON_PLAY1 1
#define ICON_PLAY2 2
#define ICON_PLAY3 3
#define ICON_PLAY4 4
#define ICON_PLAY5 5
#define ICON_PLAY6 6
#define ICON_PLAY7 7


unsigned volatile char start = 0;
unsigned volatile char gameOver = 0;
unsigned volatile char charSelect = 0;
unsigned volatile char scoreDisplay = 0;

unsigned volatile char moveLeft = 0;
unsigned volatile char moveRight = 0;
unsigned volatile char select = 0;

unsigned volatile char character = 0;
unsigned volatile char characterPosition = -1;

unsigned short score = 0;
/////////////////////////////////////////////////////////////////


void set_PWM(double frequency) {


    // Keeps track of the currently set frequency
    // Will only update the registers when the frequency
    // changes, plays music uninterrupted.
    static double current_frequency;
    if (frequency != current_frequency) {

        if (!frequency) TCCR3B &= 0x08; //stops timer/counter
        else TCCR3B |= 0x03; // resumes/continues timer/counter

        // prevents OCR3A from overflowing, using prescaler 64
        // 0.954 is smallest frequency that will not result in overflow
        if (frequency < 0.954) OCR3A = 0xFFFF;

        // prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
        else if (frequency > 31250) OCR3A = 0x0000;

        // set OCR3A based on desired frequency
        else OCR3A = (short)(8000000 / (1280 * frequency)) - 1;

        TCNT3 = 0; // resets counter
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    // COM3A0: Toggle PB6 on compare match between counter and OCR3A
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    // WGM32: When counter (TCNT3) matches OCR3A, reset counter
    // CS31 & CS30: Set a prescaler of 64
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

void saveCustom(unsigned char loc, unsigned char *custom)
{
    LCD_WriteCommand(loc);//locations 0x40,0x48,0x50,0x58,0x60,68,70,78
    for(unsigned char i=0; i<8; i++){
        LCD_WriteData(custom[i]);
    }
    LCD_WriteCommand(0x80);
}

unsigned char checkCollision(unsigned char first, unsigned char second)
{
    if(first == second)
    {
        start = 0;
        gameOver = 1;
        return 1;
    }
    return 0;
}

void displayScore(unsigned char line, unsigned short score)
{
    int i = 0;
    int num;
    int divide = 10000;
    int notFirst = 0;

    int clr = 0;
    for(;clr < 5; ++clr){
        LCD_Cursor(line + clr);
        LCD_WriteData(0x20);//clears the space
    }	
    while( score > 0 || divide > 0)
    {
        num = score / divide;
        score = score % divide;
        divide = divide / 10;
        if(num != 0 || notFirst)
        {
            notFirst = 1;
            LCD_Cursor(i + line);
            LCD_WriteData( num + '0');
            ++i;
        }
    }
}

unsigned char displayMovement(unsigned char pos, unsigned char newpos,unsigned char data)
{
    LCD_Cursor(pos);
    LCD_WriteData(0x20);//clears the space
    LCD_Cursor(newpos);
    LCD_WriteData(data);
    LCD_Cursor(0);
    return newpos;
}

void selectCharacter(unsigned char pos)
{
    if(pos == 17)
        character = ICON_PLAY1;
    else if(pos == 19)
        character = ICON_PLAY2;
    else if(pos == 21)
        character = ICON_PLAY3;
    else if(pos == 23)
        character = ICON_PLAY4;
    else if(pos == 25)
        character = ICON_PLAY5;
    else if(pos == 27)
        character = ICON_PLAY6;
    else if(pos == 29)
        character = ICON_PLAY7;
}


enum SM1_states {SM1_init, SM1_start, SM1_up, SM1_down}; //ball movement states

int tick1(int state) // ball 1 function
{
    static unsigned pos;
    static unsigned startpos = 32;
    if(!start)
        state= SM1_init;
    switch(state)
    {
        case SM1_init:
            if(start)
            {
                pos = startpos;
                state = SM1_start;
            }
            else
                state = SM1_init;
            break;

        case SM1_start:
            if(pos > 16)
                state = SM1_up;
            else
                state = SM1_down;
            state = SM1_up;
            break;

        case SM1_up:
            if(pos > 16){
                state = SM1_up;
            }				
            else
                state = SM1_down;
            break;

        case SM1_down:
            if(pos < 17)
                state = SM1_down;
            else
                state = SM1_up;
            break;

        default:
            state = SM1_init;
    }
    switch(state)
    {
        case SM1_init:
            break;

        case SM1_start:
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            displayMovement(pos,pos,ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            break;

        case SM1_up:
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            pos = displayMovement(pos,pos - 17,ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            break;

        case SM1_down:
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            pos = displayMovement(pos, pos + 15, ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM1_init;
            break;

        default:
            state = SM1_init;
    }
    return state;
}



enum SM2_states {SM2_init,SM2_start, SM2_wait, SM2_left, SM2_right}; // player movements

int tick2(int state) //player movement function
{
    switch(state)
    {
        case SM2_init:
            if(start)
            {
                state = SM2_start;
                characterPosition = 17;
            }
            else
                state = SM2_init;
            break;

        case SM2_start:
            if(moveLeft)
            {
                moveLeft = 0;
                state = SM2_left;
            }					
            if(moveRight)
            {
                moveRight= 0;
                state = SM2_right;
            }			
            else
                state = SM2_wait;
            break;

        case SM2_wait:
            if(!start)
                state = SM2_init;
            else if(moveLeft)
            {
                moveLeft = 0;
                state = SM2_left;
            }				
            else if (moveRight)
            {
                moveRight = 0;
                state = SM2_right;
            }				
            break;

        case SM2_left:
            if(!start)
                state = SM2_init;
            else if(moveLeft)
            {
                moveLeft = 0;
                state = SM2_left;
            }				
            else
                state = SM2_wait;
            break;



        case SM2_right:
            if(!start)
                state = SM2_init;
            else if(moveRight)
            {
                moveRight = 0;
                state = SM2_right;
            }				
            else
                state = SM2_wait;
            break;

        default:
            state = SM2_init;
            break;
    }
    switch(state)
    {
        case SM2_init:
            break;

        case SM2_start:
            characterPosition = displayMovement(characterPosition,characterPosition,character);
            break;

        case SM2_wait:
            break;


        case SM2_left:
            if(characterPosition > 17)
            {
                characterPosition = displayMovement(characterPosition,characterPosition - 1,character);
            }			
            break;

        case SM2_right:
            if(characterPosition < 32)
            {
                characterPosition = displayMovement(characterPosition,characterPosition + 1,character);	
            }
            break;

        default:
            state = SM2_init;
            break;
    }
    return state;
}

enum SM3_states {SM3_init,SM3_display,SM3_left,SM3_right,SM3_wait,SM3_leftwait,SM3_rightwait,SM3_select}; //character selection states

int tick3(int state) //character selection
{
    static unsigned pos;
    switch(state)
    {
        case SM3_init:
            if(charSelect)
            {
                state = SM3_display;
                select = 0;
            }				
            else
                state = SM3_init;
            break;

        case SM3_display:
            if(select)
            {
                select = 0;
                state = SM3_select;
            }		   
            else if(moveLeft)
            {
                moveLeft = 0;
                state = SM3_left;
            }				
            else if(moveRight)
            {
                moveRight = 0;
                state = SM3_right;
            }				
            else
                state = SM3_wait;
            break;

        case SM3_left:
            if(select)
            {
                select=0;
                state = SM3_select;
            }
            else if(moveLeft)
            {
                state = SM3_leftwait;
                moveLeft = 0;
            }				
            else
                state = SM3_wait;
            break;

        case SM3_leftwait:
            if(select)
            {
                select = 0;
                state = SM3_select;
            }

            else if(moveLeft)
            {
                moveLeft = 0;
                state = SM3_leftwait;
            }				
            else
                state = SM3_wait;
            break;

        case SM3_right:
            if(select)
            {
                select = 0;
                state = SM3_select;
            }
            else if(moveRight)
            {
                moveRight = 0;
                state = SM3_rightwait;			
            }				
            else
                state = SM3_wait;
            break;

        case SM3_rightwait:
            if(select)
            {
                select = 0;
                state = SM3_select;
            }				
            else if(moveRight)
            {
                moveRight = 0;
                state = SM3_rightwait;
            }				
            else
                state = SM3_wait;
            break;

        case SM3_wait:
            if(select)
            {
                select = 0;
                state = SM3_select;
            }

            else if(moveRight)
            {
                moveRight = 0;
                state = SM3_right;
            }
            else if(moveLeft)
            {
                moveLeft =0;
                state = SM3_left;	
            }
            else
                state = SM3_wait;
            break;

        case SM3_select:
            state = SM3_init;
            break;

        default:
            state = SM3_init;
            break;
    }

    switch(state)
    {
        case SM3_init:
            break;

        case SM3_display:
            pos = 17;
            LCD_DisplayString(1,"Character Select");

            LCD_Cursor(17);
            LCD_WriteData(ICON_PLAY1);
            LCD_Cursor(19);
            LCD_WriteData(ICON_PLAY2);
            LCD_Cursor(21);
            LCD_WriteData(ICON_PLAY3);
            LCD_Cursor(23);
            LCD_WriteData(ICON_PLAY4);
            LCD_Cursor(25);
            LCD_WriteData(ICON_PLAY5);
            LCD_Cursor(27);
            LCD_WriteData(ICON_PLAY6);
            LCD_Cursor(29);
            LCD_WriteData(ICON_PLAY7);
            LCD_Cursor(pos);
            break;

        case SM3_left:
            if(pos > 17)
                pos -= 2;
            LCD_Cursor(pos);
            break;

        case SM3_leftwait:
            break;

        case SM3_right:
            if(pos < 29)
                pos += 2;
            LCD_Cursor(pos);
            break;

        case SM3_rightwait:
            break;

        case SM3_wait:
            break;

        case SM3_select:
            LCD_ClearScreen();
            selectCharacter(pos);
            charSelect = 0;
            gameOver = 0;
            start = 1;
            state = SM3_init;
            break;

        default:
            state = SM3_init;
    }
    return state;
}


enum SM4_states {SM4_init, SM4_gameOver, SM4_wait}; // game over states

int tick4(int state) //game over function
{
    switch(state)
    {
        case SM4_init:
            if(gameOver)
            {
                select = 0;
                LCD_ClearScreen();
                state = SM4_gameOver;
            }				
            else
                state = SM4_init;
            break;

        case SM4_gameOver:
            state = SM4_wait;
            break;

        case SM4_wait:
            if(select)
            {
                state = SM4_init;
                LCD_ClearScreen();
                score = 0;
                select = 0;
                gameOver= 0;
                charSelect = 1;
            }
            else
                state = SM4_wait;
            break;

        default:
            state = SM4_init;
            break;
    }
    switch(state)
    {
        case SM4_init:
            break;

        case SM4_gameOver:
            LCD_DisplayString(1,"select to play!");
            LCD_DisplayString(17,"score:");
            displayScore(24,score);
            break;

        case SM4_wait:
            break;

        default:
            state = SM4_init;
            break;
    }
    return state;
}

enum SM5_states {SM5_play,SM5_wait}; // sound states

#define C4 126.63
#define D4 293.66
#define E4 329.63
#define F4 349.23
#define G4 392.23
#define A4 440.00
#define B4 493.88
#define C5 523.25


/*
//first song
#define x 20
int song[x] = {D4,D4, D4, D4, G4, E4, D4, E4, E4, D4, F4, G4, F4, F4, A4, A4, G4, G4, E4, E4};
int length[x]={3, 1,  1,  1,  1 , 1,  2,  1,  1,  1,  1,  1,  1,  2,  2,  2,  1,  1 , 1,  2};
*/

/*
//second song
#define x 18
int song[x] = {D4,  D4,  F4,  G4,  G4,  G4,  F4,  E4,  D4,  E4,  A4,  A4,  A4,  B4,  B4,  G4,  F4,  E4};
int length[x]={900, 300, 300, 300, 600, 600, 300, 600, 300, 300, 300, 600, 600, 300, 600, 300, 600, 300};    
*/


/*
//third song
#define x 26
int song[x] = {D4,   D4,  F4,  G4,  G4, G4, F4,  E4,  D4,  E4,  A4,  A4,  A4,  B4,  B4,  G4,  F4,  E4, D4,  F4, D4, F4, D4, E4,  D4,  F4};
int length[x]={900, 300, 300, 100, 100, 600, 200, 600, 300, 200, 300, 600, 600, 300, 200, 300, 600, 300, 100,  100, 100, 100, 50, 100, 200, 900};  
*/


#define x 26
int song[x] = {E4 ,  F4,    A4,  A4,  F4,  G4,  F4,  F4,  G4,  F4,  A4,  B4,  G4,  E4,  D4,  F4,  G4,  E4,  F4, G4, D4,   E4,   F4,  G4,  F4,  A4};
int length[x] = {300, 300, 200, 200, 200, 200, 200, 300, 300, 200, 300, 300, 300, 200, 200, 200, 200, 300, 300, 400, 200, 200, 300, 300, 200, 600};

#define waittime 20

int currNote = 0;
int currPeriod = 0;

int tick5(int state) //sound function
{
    switch(state)
    {
        case SM5_play:
            state = SM5_wait;
            break;

        case SM5_wait:
            state = SM5_play;
            break;

        default:
            state = SM5_play;
    }

    switch(state)
    {
        case SM5_play:
            if(currNote >= x)
                currNote = 0;
            set_PWM(song[currNote]);
            currPeriod =  length[currNote];
            ++currNote;
            break;

        case SM5_wait:
            set_PWM(0);
            currPeriod = waittime;
            break;

        default:
            state = SM5_play;
    }

    return state;
}

enum SM6_states {SM6_init, SM6_start, SM6_up, SM6_down}; //ball 2 movement states

int tick6(int state) //ball 2 function
{
    static unsigned pos;
    static unsigned startpos = 33;

    if(!start)
        state= SM6_init;
    switch(state)
    {
        case SM6_init:
            if(start)
            {
                pos = startpos;
                state = SM6_start;
            }
            else
                state = SM6_init;
            break;

        case SM6_start:

            if(pos > 16){
                state = SM6_up;
            }
            else
                state = SM6_down;
            break;

        case SM6_up:
            if(pos > 16){
                state = SM6_up;
            }				
            else
                state = SM6_down;
            break;

        case SM6_down:
            if(pos < 17)
                state = SM6_down;
            else
                state = SM6_up;
            break;

        default:
            state = SM6_init;
    }
    switch(state)
    {
        case SM6_init:
            break;

        case SM6_start:
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            displayMovement(pos,pos,ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            break;

        case SM6_up:
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            pos = displayMovement(pos,pos - 17,ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            break;

        case SM6_down:
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            pos = displayMovement(pos, pos + 15, ICON_BALL);
            if(checkCollision(characterPosition,pos))
                state = SM6_init;
            break;

        default:
            state = SM6_init;
    }
    return state;
}



///////////////////////////////////////////////////////////
int main()
{
    //set portb as outputs for speaker
    DDRB = 0xFF; PORTB = 0x00;
    //set portc to outputs for lcd
    DDRC = 0xFF; PORTC = 0xFF;
    // set port a inputs for buttons
    DDRA = 0x00; PORTA = 0xFF;
    //set port d for outputs to lcd
    DDRD = 0xFF; PORTD = 0xFF;

    unsigned char ball[] = { 0x0,0x0,0x0,0xe,0x15,0x1f,0x15,0xe};
    unsigned char play1[] = {0xf,0xe,0xc,0x4,0x1f,0x4,0xa,0x11};
    unsigned char play2[] = {0x1f,0xe,0x4,0x1f,0x4,0x1f,0x15,0x15};
    unsigned char play3[] = {0x1b,0xe,0x4,0x4,0xe,0xe,0xa,0x11};
    unsigned char play4[] = {0xe,0x15,0x11,0x1f,0x4,0x4,0xe,0x1b};
    unsigned char play5[] = {0x7,0x3,0x2,0x2,0xe,0x1e,0xc,0x12};
    unsigned char play6[] = {0xe,0xa,0xa,0x1f,0x1f,0xe,0xe,0x1b};
    unsigned char play7[] = {0x0,0xe,0x1f,0x1f,0x1f,0xe,0xa,0x1b};

    charSelect = 1;

    saveCustom(0x40,ball);
    saveCustom(0x48,play1);
    saveCustom(0x50,play2);
    saveCustom(0x58,play3);
    saveCustom(0x60,play4);
    saveCustom(0x68,play5);
    saveCustom(0x70,play6);
    saveCustom(0x78,play7);

    LCD_init();

    unsigned long int SM1_period = 500;
    unsigned long int SM2_period = 300;
    unsigned long int SM3_period = 50;
    unsigned long int SM4_period = 50;
    unsigned long int SM5_period = waittime;
    unsigned long int SM6_period = 600;

    unsigned long int gcd = 1;
    gcd = findGCD(SM1_period, SM2_period);
    gcd = findGCD(gcd, SM3_period);
    gcd = findGCD(gcd, SM4_period);
    gcd = findGCD(gcd, SM5_period);
    gcd = findGCD(gcd, SM6_period);

    SM1_period = SM1_period / gcd;
    SM2_period = SM2_period / gcd;
    SM3_period = SM3_period / gcd;
    SM4_period = SM4_period / gcd;
    SM5_period = SM5_period / gcd;
    SM6_period = SM6_period / gcd;

    PWM_on();
    TimerSet(gcd);
    TimerOn();

    static task task1, task2, task3, task4, task5, task6;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};

    const unsigned short taskSize = sizeof(tasks)/sizeof(task*);

    task1.state = -1;
    task1.period = SM1_period;
    task1.elapsedTime = SM1_period;
    task1.TickFct = &tick1;

    task2.state = -1;
    task2.period = SM2_period;
    task2.elapsedTime = SM2_period;
    task2.TickFct = &tick2;

    task3.state = -1;
    task3.period = SM3_period;
    task3.elapsedTime = SM3_period;
    task3.TickFct = &tick3;

    task4.state = -1;
    task4.period = SM4_period;
    task4.elapsedTime = SM4_period;
    task4.TickFct = &tick4;

    task5.state = -1;
    task5.period = SM5_period;
    task5.elapsedTime = SM5_period;
    task5.TickFct = &tick5;
    currPeriod = SM5_period;

    task6.state = -1;
    task6.period = SM6_period;
    task6.elapsedTime = SM6_period;
    task6.TickFct = &tick6;

    unsigned short i;
    int j;

    while(1)
    {
        if(!GetBit(PINA,0))
        {
            select = 1;
        }

        if(!GetBit(PINA,1))
        {
            moveRight = 1;
        }		
        if(!GetBit(PINA,2))
        {
            moveLeft = 1;
        }		


        for(i = 0; i < taskSize; ++i)
        {
            if(tasks[i]->elapsedTime >= tasks[i]->period)
            {

                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
                if(i == 4)
                    tasks[i]->period = currPeriod / gcd;
                else if(i == 0 && start && tasks[0]->period > gcd)
                    tasks[i]->period = tasks[i]->period - 1;
                else if (i == 0)
                    tasks[i]->period = 500 / gcd;
                else if(i == 6 && start && tasks[6]->period > gcd)
                    tasks[i]->period = tasks[i]->period - 1;
                else if (i == 6)
                    tasks[i]->period = 600 / gcd;

            }
            tasks[i]->elapsedTime += 1;
        }

        while(!TimerFlag);
        TimerFlag = 0;
        if(start)
            ++score;
    }
}
