/*
Group A

Sashank Gondala : 120050050
P Bharath : 120050058

* Author: Texas Instruments

* Editted by: Saurav Shandilya, Vishwanathan Iyer
	      ERTS Lab, CSE Department, IIT Bombay

* Theme : using interru[pts to detect long key presses

* Filename: lab-2.c

* Functions: setup(), ledPinConfig(), switchPinConfig(), main()

* Global Variables: sw2Status, currentColor, state1, state2
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency and enable GPIO Peripherals  

* Example Call: setup();

*/

void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

/*

* Function Name: switchPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 0 and Pin 4 as input. Note that Pin 0 is locked.

* Example Call: switchPinConfig();

*/
void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

//	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-0 of PORT F as Input. Modifiy this to use another switch
//	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
//	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

/*

* Function Name: ledPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 1, Pin 2, Pin 3 as output.

* Example Call: ledPinConfig();

*/

void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.
}

int sw2Status = 1;     // count of no of times SW2 is pressed
int currentColor = 2;    // used to determine which LED should be lit

int main(void){

	setup();
	ledPinConfig();
	switchPinConfig();

	uint32_t timePeriod;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);  // configuring the system clock

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);    // enabling GPIO and configuring the LEDs

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);    // enabling the clock
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);  // configuring timer-0 to be periodic

	timePeriod = (SysCtlClockGet() / 10) ;    // calculating the required time period
	TimerLoadSet(TIMER0_BASE, TIMER_A, timePeriod -1);   // loading the timePeriod into timer

	IntEnable(INT_TIMER0A);   // enalble timer0A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);   // enables an event in the timer0A to generate interrupt
	IntMasterEnable();   // master interrupt enable

	TimerEnable(TIMER0_BASE, TIMER_A);   // enabling the timer

	while(1){
	}
}

// giving the idle, press, release, pressed states a number to help in using them

#define idle 1
#define press 2
#define release 3
#define pressed 0

int state1 = idle;    // state1 to keep track of press of SW1
int state2 = idle;    // state2 to keep track of press of SW2

/*

* Function Name: myIntHadler()

* Input: none

* Output: none

* Description: on interrupt generation, detects if a key is pressed and does the required

*/


void detectKeyPress(void){

	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == pressed) {      // check if SW1 is pressed
		if (state1  == idle) state1 = press;
		else if (state1 == press) {state1 = release;
		currentColor = currentColor / 2;           // changing value of currentColor which according changes the LED color when SW1 is pressed again
			if(currentColor == 1){
				currentColor = 8;
			}

			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, currentColor);    // lighting up the required LED
		}
	}
	else {
		if (state1 == press) state1 = idle;
		else if (state1 == release) state1 = idle;   // changing state1 according to previous it's state
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);   // turning off all LEDs when SW1 is not pressed
	}

	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == pressed) {       // check if SW2 is pressed
		if (state2  == idle) state2 = press;
		else if (state2 == press) {state2 = release;
			sw2Status++;  // SW2 is pressed again
		}
	}

	else {
		if (state2 == press) state2 = idle;
		else if (state2 == release) state2 = idle;    // changing state2 according to previous it's state
	}
}
