/*

Group A

Sashank Gondala 120050050
P Bharath 120050058

Filename : lab3.c
Theme : Controlling the intensity of LEDs using SW1 and SW2
Global variables : mode, delay, ui8R, ui8G, ui8B, ui8Min, ui8Max, color

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


#define PWM_FREQUENCY 55    // defining the base freq of the servo

volatile uint8_t ui8R;   // brightness of red LED between ui8Min and ui8Max
volatile uint8_t ui8B;   // brightness of blue LED between ui8Min and ui8Max
volatile uint8_t ui8G;    // brightness of green LED between ui8Min and ui8Max
volatile uint8_t ui8Max;  // max brightness of any LED
volatile uint8_t ui8Min;  // min brightness of any LED
volatile uint16_t color;   
int mode;     // mode is either 1,2 or 3
int delay;    // delay between two readings
int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;   // used in programming PWM
	color = 0;
	ui8Max = 254;
	ui8Min = 5;
	ui8R = ui8Max;
	ui8B = 0;
	ui8G = 0;


	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);   // setting the clock and PWM module

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  // enable PWM1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // enable GPIOF

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);  // pin 1 configured as a PWM output
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);  // pin 2 configured as a PWM output
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);  // pin 3 configured as a PWM output
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;  //  ulocking the 
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;            //     GPIO 
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;              //      commit control register
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);   // configure PF0 and PF4 as inputs
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // configuring pull-up registers 

	ui32PWMClock = SysCtlClockGet() / 64;                
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;         // loading value into ui32Load
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);   // configuring generator 2 as a down-counter
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);        // setting the time period of the generator
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);   // configuring generator 3 as a down-counter
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);        //

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8R * ui32Load / 1000);  // setting pulse widths of red LED
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8B * ui32Load / 1000);  // setting pulse widths of blue LED
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8G * ui32Load / 1000);  // setting pulse widths of green LED
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);  // enabling the PWM output
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);   // enabling generators 2 & 3
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
	delay = 100000;
	mode=0;
	while(1)
	{
		if(mode==0){
			color = color + 1;
			if(color>=360){
				color = 0;
			}
			if(color<120){
				ui8G = ui8Min + color*ui8Max/120.0;
				ui8R = ui8Min + ui8Max - ui8G;
				ui8B = ui8Min;
			} else if (color<240){
				ui8B = ui8Min+(color-120)*ui8Max/120.0;
				ui8G = ui8Min+ui8Max - ui8B;
				ui8R = ui8Min;
			} else if(color<360){
				ui8R = ui8Min+(color-240)*ui8Max/120.0;
				ui8B = ui8Min+ui8Max - ui8R;
				ui8G = ui8Min;
			}
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)) delay = delay-1000;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8R * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8B * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8G * ui32Load / 1000);

			SysCtlDelay(delay*10);
		}
		//n=0;
		while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&mode<4){
				mode++;

			}
			delay = delay+1000;
			SysCtlDelay(delay*10);
		}
		if(mode==1){    // red LED control
			mode=0;
			ui8R = 127;
			ui8B = 5;
			ui8G = 5;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					ui8R--;      // decreasing the brightness of red LED
					if (ui8R < 5)
					{
						ui8R = 5;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&mode<4){
						mode++;
					}

					ui8R++;    // increasing the brightness of red LED
					if (ui8R > 254)
					{
						ui8R = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8R * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8B * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8G * ui32Load / 1000);   // setting pulse widths of LEDs
				if(mode>1)
					break;
				SysCtlDelay(delay);
			}
		}
		else if (mode==2){    // blue LED control
			mode=0;
			ui8B = 127;
			ui8R = 5;
			ui8G = 5;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					ui8B--;         // decreasing the brightness of blue LED
					if (ui8B < 5)
					{
						ui8B = 5;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&mode<4){
						mode++;
					}

					ui8B++;        // increasing the brightness of blue LED
					if (ui8B > 254)
					{
						ui8B = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8R * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8B * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8G * ui32Load / 1000);   // setting pulse widths of LEDs
				if(mode!=0)
					break;
				SysCtlDelay(delay);
			}
		}
		else if(mode>2){     // green LED control
			mode=0;
			ui8G = 127;
			ui8B = 5;
			ui8R = 5;
			while(1){
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)   // decreasing the brightness of green LED
				{
					ui8G--;
					if (ui8G < 5)
					{
						ui8G = 5;
					}
				}
				while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
					if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)&&mode<4){
						mode++;
					}

					ui8G++;     // increasing the brightness of green LED
					if (ui8G > 254)
					{
						ui8G = 254;
					}
					SysCtlDelay(delay*10);
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8R * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8B * ui32Load / 1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8G * ui32Load / 1000);   // setting pulse widths of LEDs
				if(mode!=0)
					break;
				SysCtlDelay(delay);
			}
		}
	}

 }
