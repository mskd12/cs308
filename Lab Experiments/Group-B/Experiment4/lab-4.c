/*

* Author: Texas Instruments 

* Editted by: Krishna Deepak, Bharadwaj 
	      CSE Department, IIT Bombay

* Description: This code will familiarize you with using ADC and UART

* Filename: lab-4.c 

* Functions: UARTIntHandler(), main()  

* Global Variables: setTemp, mode

*/

#include<stdint.h>
#include<stdbool.h>
#include"inc/hw_ints.h"
#include"inc/hw_memmap.h"
#include"inc/hw_types.h"
#include"driverlib/gpio.h"
#include"driverlib/interrupt.h"
#include"driverlib/pin_map.h"
#include"driverlib/sysctl.h"
#include"driverlib/uart.h"
#include"driverlib/adc.h"
#include"driverlib/uart.h"
#include"utils/uartstdio.h"
#include"utils/uartstdio.c"

int32_t setTemp = 0; // temperature to set
int32_t mode = 2; // like a state in state diagram

/*

* Function Name: UARTIntHandler()

* Input: none

* Output: none

* Description: Interrupt handler for UART. States changed acc to state diagram 
	       We read in characters and states are chaned accordingly 

* Example Call: 

*/
void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the interrupts

	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		char c = UARTCharGetNonBlocking(UART0_BASE);
		if(c == 's' && mode == 1){ // go to set temp
			mode = 2;
		}
		else if(mode == 3){ // take in the entered temperature
			setTemp = (c-'0')*10;
			UARTCharPutNonBlocking(UART0_BASE, c);
			mode = 4;
		}
		else if(mode == 4){
			UARTCharPutNonBlocking(UART0_BASE, c);
			setTemp = setTemp + (c-'0');
			UARTCharPutNonBlocking(UART0_BASE, '\n');
			UARTCharPut(UART0_BASE, 'S');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'u');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, 'd');
			UARTCharPut(UART0_BASE, 'a');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'd');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'o');
			UARTCharPut(UART0_BASE, ' ');
			UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
				(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
			UARTCharPut(UART0_BASE, setTemp/10 + '0');
			UARTCharPut(UART0_BASE, setTemp%10 + '0');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, '\n');
			mode = 1;
		}
	}
}

/*

* Function Name: main()

* Input: none

* Output: none

* Description: Setting up ADC and UART. Reads current temp and interacts the above interrupt handler.
	       LED color changes b/w green and red depending on current temp is greater than set temp.

* Example Call: 

*/
int main(void) {
	volatile uint32_t ui32TempValueC; // temperature in C
	volatile uint32_t ui32TempValueF; // temperature in F
	volatile uint32_t ui32TempAvg;
	uint32_t ui32ADC0Value[4]; // size of the ADC sequencer 

	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //enable pin for LED PF2
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
	(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	IntMasterEnable(); //enable processor interrupts
	IntEnable(INT_UART0); //enable the UART interrupt
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 0);
	while (1) 
	{
		if(mode == 2){ // print "Enter SetTemp:" using UART
			UARTCharPut(UART0_BASE, 'E');
			UARTCharPut(UART0_BASE, 'n');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'S');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, ':');
			UARTCharPut(UART0_BASE, ' ');
			mode = 3;
		}
		else if(mode == 1){ // current temp reading through ADC
			ADCIntClear(ADC0_BASE, 1);
			ADCProcessorTrigger(ADC0_BASE, 1);
			while(!ADCIntStatus(ADC0_BASE, 1, false))
			{ // read in average of temp
				ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
				ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
				ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
				ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
			}
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, 'u');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'n');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, '=');
			UARTCharPut(UART0_BASE, ' ');
			UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
				(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
			UARTCharPut(UART0_BASE, ui32TempValueC/10 + '0');
			UARTCharPut(UART0_BASE, ui32TempValueC%10 + '0');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, ',');
			UARTCharPut(UART0_BASE, ' ');

			UARTCharPut(UART0_BASE, 'S');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, '=');
			UARTCharPut(UART0_BASE, ' ');
			UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
				(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
			UARTCharPut(UART0_BASE, setTemp/10 + '0');
			UARTCharPut(UART0_BASE, setTemp%10 + '0');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, '\n');
			// LED color changes depending on the condition
			if(setTemp > ui32TempValueC)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 8); //green
			else
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2); //red
		}
		SysCtlDelay(SysCtlClockGet()/3);
	}
}

