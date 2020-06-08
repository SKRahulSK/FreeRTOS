/*
 * MutexExample.c
 *
 *  Created on: 30-May-2020
 *      Author: Rahul
 */


#include "FreeRTOS.h"
#include "task.h"
#include "stm32wbxx.h"
#include "stm32wbxx_nucleo.h"
#include "stdio.h"
#include "string.h"
#include "semphr.h"
#include "stdlib.h"

//Task handles and functions
xTaskHandle xTask1Handle;
xTaskHandle xTask2Handle;
void PrintFunction(void *params);

//UART Handle and Init types
UART_HandleTypeDef Uart1;
UART_InitTypeDef Uart1Init;
GPIO_InitTypeDef GpioUARTpins;

//Mutex Handle
xSemaphoreHandle xMutex;

//Private helper functions and variables
static void prvSetupUART(void);
void printmsg(char *msg);
char UsrMsg[250];

int main()
{
	// Enable the DWT Cycle Count Register (SEGGER Settings)

	DWT->CTRL |= (1 << 0);

	// Private function called to setup the Hardware
	prvSetupUART();

	//Start Recording for SEGGER SystemView
	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	sprintf(UsrMsg,"Example of Mutual Exclusion for synchronization between 2 Tasks using Binary Semaphore \r\n");
	printmsg(UsrMsg);

	//Create the mutex
	xMutex = xSemaphoreCreateMutex();;

	if(xMutex != NULL)
	{
		//Create tasks
		xTaskCreate(PrintFunction, "Task1", configMINIMAL_STACK_SIZE, "*****Task1*****\r\n", 2, &xTask1Handle);
		xTaskCreate(PrintFunction, "Task2", configMINIMAL_STACK_SIZE, "-----Task2-----\r\n", 2, &xTask2Handle);

		//Give the Binary semaphore for the first time, so that it is available to the tasks.
		xSemaphoreGive(xMutex);

		//Initialize the srand function
		srand(0);

		//Schedule tasks
		vTaskStartScheduler();

	}

	else
	{
		sprintf(UsrMsg,"Binary Semaphore creation failed.. :( \r\n");
		printmsg(UsrMsg);
	}

	for(;;);

}


void PrintFunction(void *params)
{
	char *Taskdata = (char *) params;

	while(1)
	{
		xSemaphoreTake(xMutex, portMAX_DELAY);

		sprintf(UsrMsg, "%s", Taskdata);
		printmsg(UsrMsg);

		xSemaphoreGive(xMutex);

		vTaskDelay(pdMS_TO_TICKS(rand() & 0xF));
	}
}


static void prvSetupUART(void)
{
	//1. Enable the UART1 and GPIOB Peripheral Clocks
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//In UART connection with Virtual COM-port, PB6->TX and PB7->RX
	//2. Alternate Functionality Configuration to make Port B pins work as UART pins

	//Zeroing each and every member element of the structure.
	memset(&GpioUARTpins, 0, sizeof(GpioUARTpins));
	GpioUARTpins.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GpioUARTpins.Mode = GPIO_MODE_AF_PP;
	GpioUARTpins.Alternate = GPIO_AF7_USART1;
	GpioUARTpins.Pull = GPIO_PULLUP;

	HAL_GPIO_Init(GPIOB, &GpioUARTpins);

	//3. Configure and initialize UART parameters

	//Zeroing each and every member element of the structure.
	memset(&Uart1Init, 0, sizeof(Uart1Init));
	memset(&Uart1, 0, sizeof(Uart1));

	//UART Initialization
	Uart1Init.BaudRate = 115200;
	Uart1Init.WordLength = UART_WORDLENGTH_8B;
	Uart1Init.HwFlowCtl = UART_HWCONTROL_NONE;
	Uart1Init.Mode = UART_MODE_TX_RX;
	Uart1Init.Parity = UART_PARITY_NONE;
	Uart1Init.StopBits = UART_STOPBITS_1;

	Uart1.Init = Uart1Init;
	Uart1.Instance = USART1;

	//4. Initialize the UART peripheral
	uint16_t UARTSetUpResult = HAL_UART_Init(&Uart1);

	if(UARTSetUpResult == HAL_ERROR)
	{
		//printf("USART Initialization was not successful \n");
	}
}

void printmsg(char *msg)
{
	HAL_UART_Transmit(&Uart1, (uint8_t *)msg, strlen(msg), 1);
}

//Implement the Idle Hook function
void vApplicationIdleHook()
{
	//Send the CPU to normal sleep mode
	__WFI();
}

