#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"

bool item_RollCall[3] = {false,false,false};
char Pc2uart_BAR_TEMP[15];
char item0_Info [15];
char item1_Info [15];
char item2_Info [15];
char msg2Printer [100];

void PortFunctionInit(void){
		//Enable GPIO System Clock
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		
		
    // Enable pin PF4 for GPIOInput
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	
		//Unlock GPIO_INPUT ports
		HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTE_BASE + GPIO_O_CR) = 0x1;

	  HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x1;
	
		// Enable pin PF0 for GPIOInput
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	
    // Enable pin PE1, PE2 & PE3 for GPIOInput
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); 
	
		// Enable pin Port F LED pins for GPIOOutput
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
		
		//Enable pull-up on PE1, PE2, PE3
		GPIO_PORTE_PUR_R |= 0x0E;
		
		//Enable pull-up resistor on PF4 & PF0
		GPIO_PORTF_PUR_R |= 0x11;

		// Enable both PF1 and PF0 for digital function.
    GPIO_PORTF_DEN_R |= 0x0E;
		
    // Set the direction of LEDS as output
    GPIO_PORTF_DIR_R |= 0x0E;
}


void UART1Int(void){
		SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
		// Enable UART1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		
		// Configure UART 1 Pins
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
		// Configure UART 1 Baudrate, parity, etc...
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 19200,
    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void Timer0A_Init(unsigned long period){  	
  // Enable Peripheral Clocks 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); 		// configure for 32-bit timer mode
  TimerLoadSet(TIMER0_BASE, TIMER_A, period -1);      //reload value
	IntPrioritySet(INT_TIMER0A, 0x00);  	 // configure Timer0A interrupt priority as 0
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);      // arm timeout interrupt
  TimerEnable(TIMER0_BASE, TIMER_A);      // enable timer0A
}
void Timer1A_Init(unsigned long period_debounce){  	
  // Enable Peripheral Clocks 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
  TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT); 		// configure for 32-bit timer mode
  TimerLoadSet(TIMER1_BASE, TIMER_A, period_debounce -1);      //reload value
	IntPrioritySet(INT_TIMER1A, 0x03);  	 // configure Timer0A interrupt priority as 0
  IntEnable(INT_TIMER1A);    				// enable interrupt 19 in NVIC (Timer0A)
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);      // arm timeout interrupt
  TimerEnable(TIMER1_BASE, TIMER_A);      // enable timer0A
}
void Interrupt_Init(void){
  IntEnable(INT_GPIOE);  							// enable interrupt NVIC (GPIOE)
	IntPrioritySet(INT_GPIOE, 0x01); 		// configure GPIOE interrupt priority as 2
	GPIO_PORTE_IM_R |= 0x0E;   		// arm interrupt 
	GPIO_PORTE_IS_R &= ~0x0E;     // are edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x0E;   	// both edges trigger 
  //GPIO_PORTE_IEV_R &= ~0x0E;  	// falling edge event
	
	IntEnable(INT_GPIOF);  							// enable interrupt 30 in NVIC (GPIOF)
	IntPrioritySet(INT_GPIOF, 0x02); 		// configure GPIOF interrupt priority as 1
	GPIO_PORTF_IM_R |= 0x11;   		// arm interrupt on PF0 and PF4
	GPIO_PORTF_IS_R &= ~0x11;     // PF0 and PF4 are edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;   	// PF0 and PF4 not both edges trigger 
  GPIO_PORTF_IEV_R &= ~0x11;  	// PF0 and PF4 falling edge event
	
	IntMasterEnable();       		// globally enable interrupt
}
void Timer0A_Handler(void){
		// acknowledge flag for Timer0A timeout
		TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	
		GPIO_PORTF_DATA_R ^= GPIO_PIN_2;
}
void Timer1A_Handler(void){
		// acknowledge flag for Timer0A timeout
		TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}
void GPIOPortE_Handler(void){
	//Can be removed if these do not work
	uint32_t intStatus;
	intStatus = GPIOIntStatus(GPIO_PORTE_BASE, 1u);
	
	IntDisable(INT_GPIOE);
	IntEnable(INT_TIMER1A);
	IntEnable(INT_GPIOE);
		
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PORTF_DIR_R, 0x00);
	
	//SWITCH 1
	if(intStatus == GPIO_PIN_1){			
			IntDisable(INT_TIMER0A);		// Disables blinking LED
			//The item is present
			if((GPIO_PORTE_DATA_R & GPIO_PIN_1) == 0x00){
				//Is there a present UART msg?
				if(UARTCharsAvail(UART1_BASE)){
						for(int i = 0; i < (sizeof(item0_Info)/sizeof(item0_Info[0])); i++){
								item0_Info[i] = 0;
								item0_Info[i] = UARTCharGetNonBlocking(UART1_BASE);
						}
						IntDisable(INT_UART1);
				}
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PORTF_DIR_R, 0x00);	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
				
				item_RollCall[0] = true;
				IntEnable(INT_UART1);
			} 
  }
	//SWITCH 2
	if(intStatus == GPIO_PIN_2){
			IntDisable(INT_TIMER0A);		// Disables blinking LED
			if((GPIO_PORTE_DATA_R&GPIO_PIN_2)==0x00){
				
				if(UARTCharsAvail(UART1_BASE)){
						for(int i = 0; i < (sizeof(item0_Info)/sizeof(item0_Info[0])); i++){
								item1_Info[i] = 0;
								item1_Info[i] = UARTCharGetNonBlocking(UART1_BASE);
						}
				IntDisable(INT_UART1);
					}
				
				//LED -- Triggered Switch
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PORTF_DIR_R, 0x00);	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
				
				item_RollCall[1] = true;
				IntEnable(INT_UART1);
			}
  }
	//SWITCH 3
	if(intStatus == GPIO_PIN_3){
			if((GPIO_PORTE_DATA_R&GPIO_PIN_3)==0x00){
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PORTF_DIR_R, 0x00);	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
				
				item_RollCall[2] = true; 
			}
  }
	GPIOIntClear(GPIO_PORTE_BASE, intStatus);
}
void GPIOPortF_Handler(void){			//Grocery List Handling; Prints out grocery list upon request
	IntDisable(INT_GPIOF);
	IntEnable(INT_TIMER1A);
	IntEnable(INT_GPIOF);
	
	//SW1 is pressed
	if(GPIO_PORTF_RIS_R&0x10)
	{
		// acknowledge flag for PF4
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); 
		if((GPIO_PORTF_DATA_R&0x10)==0x00)
		{
				for(int i = 0; i < (sizeof(item_RollCall)/sizeof(item_RollCall[0])); i++){
						if(item_RollCall[i] == false){
							char itemName[7] = {'\n','I','t','e','m',':',' '};
							
							for(int i = 0; i < sizeof(itemName); i++){
								UARTCharPut(UART1_BASE, itemName[i]);
							}
								UARTCharPut(UART1_BASE, (char)(i+48)); //+48 is the ASCII offest
								UARTCharPut(UART1_BASE, '\n');
							/*
							//Print out item_Information
							for(int i = 0; i < 32; i++){
							UARTCharPut(UART1_BASE,item0_Info[i]);
							}
							*/
				}
			}
		}
	}
	
	//SW2 is pressed
  if(GPIO_PORTF_RIS_R&0x01)
	{
		// acknowledge flag for PF0
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
		//counter imcremented by 1
		if((GPIO_PORTF_DATA_R&0x01)==0x00)
		{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PORTF_DIR_R, 0x00);	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
		}
	}
}
void UART1IntHandler(void){				//Handles UART communication between TI
    uint32_t ui32Status;
    
    ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status

    UARTIntClear(UART1_BASE, ui32Status); //clear the asserted interrupts
				/*
        //Clear the TEMP character array
        for(int i = 0; i < (sizeof(Pc2uart_BAR_TEMP)/sizeof(Pc2uart_BAR_TEMP[0])); i++){
                Pc2uart_BAR_TEMP[i] = 0;        
        }    
        
        //loop while there are chars
				for(int i = 0; i < UART1_BASE; i++){			 
						Pc2uart_BAR_TEMP[i] = UARTCharGet(UART1_BASE);	
        }
				*/
        IntEnable(INT_TIMER0A);
}
		

int main(void) {
	unsigned long period = 16000000; //reload value to Timer0A to generate one second delay
	unsigned long period_debounce = 26000; //reload value to Timer0A to generate one second delay
		PortFunctionInit();
		Timer0A_Init(period);
		Timer1A_Init(period_debounce);
		Interrupt_Init();
		UART1Int();
    IntMasterEnable(); //enable processor interrupts
    
		IntEnable(INT_UART1); //enable the UART interrupt
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts
    while (1) //let interrupt handler do the UART echo function
    {
			//The item on switch 1 is absent
			if((GPIO_PORTE_DATA_R & GPIO_PIN_1) == 0x02)
					item_RollCall[0] = false;
			if((GPIO_PORTE_DATA_R & GPIO_PIN_2) == 0x04)
					item_RollCall[1] = false;
			if((GPIO_PORTE_DATA_R & GPIO_PIN_3) == 0x08)
					item_RollCall[2] = false;
    }
}
