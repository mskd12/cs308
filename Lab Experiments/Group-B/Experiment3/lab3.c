/*

* Author: Texas Instruments 

* Editted by: Krishna Deepak, Bharadwaj 
	      CSE Department, IIT Bombay

* Description: This code will familiarize you with using PWM

* Filename: lab-3.c 

* Functions: setup(), main()  

* Global Variables: 

*/
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Setup PWM

* Example Call: setup();

*/
void setup(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

/*

* Function Name: main()

* Input: none

* Output: none

* Description: Auto mode - In Auto mode color of the RGB LED follows a pattern in a cycle.
	       Manual mode - In Manual mode, user must be able to select any one of the color from the color circle

* Example Call: 

*/
int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	int n = 0; // used for moving between various modes

	volatile uint16_t angle = 0; // circular angle
	volatile uint8_t ui8AdjustMax = 244; // max intensity
	volatile uint8_t ui8AdjustMin = 10; // min intensity
	volatile uint8_t ui8AdjustR = ui8AdjustMax; // red intensity
	volatile uint8_t ui8AdjustB = 0; // blue intensity
	volatile uint8_t ui8AdjustG = 0; // green intensity
	int delay = 1000000;
	
	setup();
	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
	while(1)
	{
		if(n==0){ // Auto mode - adjust intensities gradually depending on 'angle'
			angle = angle + 1;
			if(angle>=360){
				angle = 0;
			}
			if(angle<120){
				ui8AdjustG = ui8AdjustMin + angle*ui8AdjustMax/120.0;
				ui8AdjustR = ui8AdjustMin + ui8AdjustMax - ui8AdjustG;
				ui8AdjustB = ui8AdjustMin;
			} else if (angle<240){
				ui8AdjustB = ui8AdjustMin+(angle-120)*ui8AdjustMax/120.0;
				ui8AdjustG = ui8AdjustMin+ui8AdjustMax - ui8AdjustB;
				ui8AdjustR = ui8AdjustMin;
			} else if(angle<360){
				ui8AdjustR = ui8AdjustMin+(angle-240)*ui8AdjustMax/120.0;
				ui8AdjustB = ui8AdjustMin+ui8AdjustMax - ui8AdjustR;
				ui8AdjustG = ui8AdjustMin;
			}
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)) delay = delay-1000;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);

			SysCtlDelay(delay*10);
		}
		while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){// SW2 long press
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) && n<4){ // SW1 press
				n++;
			}
			delay = delay+1000;
			SysCtlDelay(delay*10);
		}
		if(n==1){ // Manual mode; Red color
			n=0;
			ui8AdjustR = 127;
			ui8AdjustB = 10;
			ui8AdjustG = 10;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					ui8AdjustR--;
					if (ui8AdjustR < 10)
					{
						ui8AdjustR = 10;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){ // SW2 long press
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&n<4){ // SW1 press
						n++;
					}

					ui8AdjustR++;
					if (ui8AdjustR > 254)
					{
						ui8AdjustR = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
				if(n>1) // change color
					break;
				SysCtlDelay(delay);
			}
		}
		else if (n==2){ // Manual mode; Blue color
			n=0;
			ui8AdjustB = 127;
			ui8AdjustR = 10;
			ui8AdjustG = 10;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					ui8AdjustB--;
					if (ui8AdjustB < 10)
					{
						ui8AdjustB = 10;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&n<4){
						n++;
					}

					ui8AdjustB++;
					if (ui8AdjustB > 254)
					{
						ui8AdjustB = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
				if(n!=0) // change color
					break;
				SysCtlDelay(delay);
			}
		}
		else if(n>2){ // Manual mode; Green color
			n=0;
			ui8AdjustG = 127;
			ui8AdjustB = 10;
			ui8AdjustR = 10;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					ui8AdjustG--;
					if (ui8AdjustG < 10)
					{
						ui8AdjustG = 10;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&n<4){
						n++;
					}

					ui8AdjustG++;
					if (ui8AdjustG > 254)
					{
						ui8AdjustG = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
				if(n!=0) // change color
					break;
				SysCtlDelay(delay);
			}
		}
	}

 }

