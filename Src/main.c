/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/*-----------------------------------------------------------------------------*
 * INCLUDES
 *----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------------------------*
 * MACROS AND DEFINES
 *----------------------------------------------------------------------------*/
 
 /* TASK PERIODS */
#define PERIOD_BTN1         		50
#define PERIOD_BTN2         		50
#define PERIOD_TRANSMITTER     		100
#define PERIOD_UART            		20
#define PERIOD_LOAD1           		10
#define PERIOD_LOAD2           		100

/* STRINGS */
#define STR_POSITIVE_BTN1  		"\n\nButton 1 :: Positive Edge\n"
#define STR_NEGATIVE_BTN1  		"\n\nButton 1 :: Negative Edge\n"
#define STR_POSITIVE_BTN2  		"\n\nButton 2 :: Positive Edge\n"
#define STR_NEGATIVE_BTN2 		"\n\nButton 2 :: Negative Edge\n"
#define STR_TX           		"\nPeriodic Transmitter 100ms."

/* DELAYS */
#define DELAY_5ms			60000
#define DELAY_12ms			144000

/*-----------------------------------------------------------------------------*
 * TASKS HANDLERS
 *----------------------------------------------------------------------------*/

TaskHandle_t BTN1_Handle				= NULL;
TaskHandle_t BTN2_Handle				= NULL;
TaskHandle_t Periodic_Handle				= NULL;
TaskHandle_t UART_Handle				= NULL;
TaskHandle_t Load1_Handle				= NULL;
TaskHandle_t Load2_Handle				= NULL;
BaseType_t   xReturned;



/*-----------------------------------------------------------------------------*
 * QUEUE HANDLER
 *----------------------------------------------------------------------------*/

QueueHandle_t xQueue__Buffer = NULL;



/*-----------------------------------------------------------------------------*
 * TASKS
 *----------------------------------------------------------------------------*/

void vApplicationTickHook (void)					/* TICK Visualization */
{
	
	GPIO_write(PORT_1,PIN1,PIN_IS_HIGH);
	GPIO_write(PORT_1,PIN1,PIN_IS_LOW);

}

void Button_1_Monitor( void * pvParameters )				/* BTN 1: Monitor the if any change happens on Port 0, Pin 0 */
{

	uint8_t i = 0;
    pinState_t Button1_NewState;
	pinState_t  Button1_OldState = GPIO_read(PORT_0 , PIN0);
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		
		Button1_NewState = GPIO_read(PORT_0 , PIN0);
		
		if( Button1_NewState == PIN_IS_HIGH &&  Button1_OldState == PIN_IS_LOW)
		{
			
			/* +ve Edge*/
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueSend( xQueue__Buffer , STR_POSITIVE_BTN1 + i ,100);
			}
			
		}
		else if (Button1_NewState == PIN_IS_LOW &&  Button1_OldState == PIN_IS_HIGH)
		{
			
			/* -ve Edge*/
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueSend( xQueue__Buffer , STR_NEGATIVE_BTN1 + i ,100);
			}
			
		}

		Button1_OldState = Button1_NewState;
		vTaskDelayUntil( &xLastWakeTime , PERIOD_BTN1);
	}
}

void Button_2_Monitor( void * pvParameters )				/* BTN 2: Monitor if any change happens on Port 0, Pin 1 */
{

	uint8_t i = 0;
	pinState_t  Button2_OldState = GPIO_read(PORT_0 , PIN1);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	pinState_t Button2_NewState;

	for( ;; )
	{
		
		Button2_NewState = GPIO_read(PORT_0 , PIN1);

		if( Button2_NewState == PIN_IS_HIGH &&  Button2_OldState == PIN_IS_LOW)
		{
			/* +ve Edge*/
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueSend( xQueue__Buffer , STR_POSITIVE_BTN2+i ,100);
			}
			
		}
		else if (Button2_NewState == PIN_IS_LOW &&  Button2_OldState == PIN_IS_HIGH)
		{
			/* -ve Edge*/
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueSend( xQueue__Buffer , STR_NEGATIVE_BTN2 + i ,100);
			}
						
		}

		Button2_OldState = Button2_NewState;
		vTaskDelayUntil( &xLastWakeTime , PERIOD_BTN2);
	}
}


void Periodic_Transmitter (void * pvParameters )			/* Transmitter: Periodiclly send data to the UART*/
{

	uint8_t i = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for( ; ; )
	{
		
		for( i = 0 ; i < 28 ; i++)
		{
			xQueueSend( xQueue__Buffer , STR_TX + i ,100);
		}
		
		vTaskDelayUntil( &xLastWakeTime , PERIOD_TRANSMITTER);
	}
	
}

void Uart_Receiver (void * pvParameters )				/* UART: Recieve the data sent to the UART */
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	char Rx_String[28];
	uint8_t i = 0;
	
	for( ; ; )
	{
		
		if( uxQueueMessagesWaiting(xQueue__Buffer) != 0)
		{
			
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueReceive( xQueue__Buffer, (Rx_String+i) , 0);
			}
			vSerialPutString( (signed char *) Rx_String, 28);
			xQueueReset(xQueue__Buffer);
			
		}
		
		vTaskDelayUntil( &xLastWakeTime , PERIOD_UART);
	}
}
	




void Load_1_Simulation ( void * pvParameters )				/* Load 1: Perform CPU Load for 5ms*/
{
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint32_t i = 0;
	uint32_t Period = DELAY_5ms;
	
	for( ; ; )
	{
		
		for( i = 0 ; i <= Period; i++)
		{
			/* Delay 5ms */
		}

		vTaskDelayUntil( &xLastWakeTime , PERIOD_LOAD1);

	}
}

void Load_2_Simulation ( void * pvParameters )				/* Load 2: Perform CPU Load for 12ms*/
{
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint32_t i = 0;
	uint32_t Period = DELAY_12ms; 
		
	for( ; ; )
	{		
		
		for( i = 0 ; i <= Period; i++)
		{
			/* Delay 12ms */
		}

		vTaskDelayUntil( &xLastWakeTime , PERIOD_LOAD2);
	
	}
}
 
 
/*-----------------------------------------------------------*/

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );


/*-----------------------------------------------------------*/

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */

int main( void )
{
	
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	
	xQueue__Buffer = xQueueCreate( 28, sizeof(char) );

    /* Tasks Creation */
	xTaskPeriodicCreate(
			Button_1_Monitor,						/* Task */
			"BTN 1",							/* Name */
			100,								/* Size */
			( void * ) 0,							/* Parameter in*/
			1,								/* Priority */
			&BTN1_Handle,							/* Handle*/
			PERIOD_BTN1);							/* Periodicity*/

	xTaskPeriodicCreate(
			Button_2_Monitor,						/* Task */
			"BTN 2",							/* Name */
			100, 								/* Size */
			( void * ) 0,							/* Parameter in*/
			1,								/* Priority */
			&BTN2_Handle,							/* Handle */
			PERIOD_BTN2);							/* Periodicity */

	xTaskPeriodicCreate(
			Periodic_Transmitter,						/* Task */
			"PERIODIC TRANSMITTER",						/* Name */
			100,								/* Size */
			( void * ) 0,							/* Parameter in*/
			1,								/* Priority */
			&Periodic_Handle,						/* Handle */
			PERIOD_TRANSMITTER);						/* Periodicity */

	xTaskPeriodicCreate(
			Uart_Receiver,							/* Task */
			"UART",								/* Name */
			100,								/* Size */
			( void * ) 0,							/* Parameter in */
			1,								/* Priority */
			&UART_Handle,							/* Handle */
			PERIOD_UART);							/* Periodicity */

	xTaskPeriodicCreate(
			Load_1_Simulation,						/* Task */
			"LOAD1",							/* Name */
			100,								/* Size */
			( void * ) 0,							/* Parameter in */
			1,								/* Priority */
			&Load1_Handle,							/* Handle */
			PERIOD_LOAD1);							/* Periodicity */

	xTaskPeriodicCreate(
			Load_2_Simulation,						/* Task */
			"LOAD2",							/* Name */
			100,								/* Size */
			( void * ) 0,							/* Parameter in */
			1,								/* Priority */
			&Load2_Handle,							/* Handle */
			PERIOD_LOAD2);							/* Periodicity */

	
		
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();
	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );

}

/*-----------------------------------------------------------*/


/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000; //20 kHhZ
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}

/*-----------------------------------------------------------------------------*
 * END FILE
 *----------------------------------------------------------------------------*/
