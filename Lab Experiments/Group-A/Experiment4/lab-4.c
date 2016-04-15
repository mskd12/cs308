/*

Group A

Sashank Gondala 120050050
P Bharath 120050058

Filename : lab3.c
Theme : interfacing using UART
Global variables : mode, delay, ui8R, ui8G, ui8B, ui8Min, ui8Max, color

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


uint32_t ui32ADC0Value[4];   // sampling array 
volatile uint32_t ui32TempAvg;
int32_t setTemp = 0;
volatile uint32_t ui32TempValueC;  // temp in centigrade
volatile uint32_t ui32TempValueF;  // temp in farenheit
int32_t mode = 2;


void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while chars are available
	{
		char c = UARTCharGetNonBlocking(UART0_BASE);   // get the character
		if(mode == 3){
			UARTCharPutNonBlocking(UART0_BASE, c);  // echo 'c'
			setTemp = (c-'0')*10;
			mode = 4;
		}
		else if(c == 's' && mode == 1){
			mode = 2;
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
			mode = 1;
		}
	}
}


int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    // enabling UART peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // enabling GPIO port A
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);   // set Rx and Tx as UART pins
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // enable GPIO port F
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // enable output on pins 1,2 and 3
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
	(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));  // configuring baud rate
	IntMasterEnable(); // master interrupt enable
	IntEnable(INT_UART0); // enable the UART interrupt
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); 

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);  // enabling ADC peripheral
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);  // taking the first sample
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);  // taking the second sample
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);  // taking the third sample
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);  // taking the fourth sample and ending the sampling of ADC
	ADCSequenceEnable(ADC0_BASE, 1);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 0);
	while (1) //let interrupt handler do the UART echo function
	{
		if(mode == 1){
			ADCIntClear(ADC0_BASE, 1);   // clearing ADC interrupts
			ADCProcessorTrigger(ADC0_BASE, 1);   // trigger ADC conversion
			while(!ADCIntStatus(ADC0_BASE, 1, false))
			{
				ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);  // get the value 
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
			if(setTemp > ui32TempValueC){
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 8); // blink the appropriate LED
			}
			else{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2); // blink the appropriate LED
			}
		}
		else if(mode == 2){
			UARTCharPut(UART0_BASE, 'E');
			UARTCharPut(UART0_BASE, 'n');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'h');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, 'a');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'a');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'u');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, ':');
			UARTCharPut(UART0_BASE, ' ');
			mode = 3;
		}
		SysCtlDelay(SysCtlClockGet()/3);
	}
}
