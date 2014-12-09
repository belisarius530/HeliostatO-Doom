//*************************************************************************************
/** \file main.c
 *  \brief This file contains function the main function, which creates tasks, and
 *  runs the scheduler.
 *
 *  Revisions:
 *    \li 11-27-2014 JF, ML, JR created original file
 *
 *  License:
 *		This file is copyright 2012 by JF, ML, JR and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <stdio.h>
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header
#include <string.h>   
#include <avr/interrupt.h>                  // Functions for C string handling

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h"                       // Header for co-routines and such
#include "semphr.h"

#include "priorities.h"                     // Header containing priority DEFINES
#include "shares.h"                         // Header containing global definitions

#include "task_orient.h"
#include "task_comms.h"                     // Communications task header
#include "task_sensors.h"
#include "task_motors.h"
#include "task_safety.h"
#include "task_master.h"
#include "uart.h"


//-------------------------------------------------------------------------------------
/** \brief This function converts the printf to use uart to print to the serial port.
 */
static int usart_putchar(char c, FILE *stream); 
static int usart_putchar(char c, FILE *stream)
{
      usart_send(c);
      return 0;
}
static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

//-------------------------------------------------------------------------------------
/** \brief The heartbeat task simply blinks an LED to show that the system is running.
 */
void task_heartbeat(void *pvParameters)
{
    DDRD |= (1<<PD7);
    char *heartbeat = "Heartbeat task is running...\n\r";
    while(1){
        PORTD |= (1<<PD7);
        vTaskDelay(configMS_TO_TICKS (1000));
	    PORTD &= ~(1<<PD7);
        vTaskDelay(configMS_TO_TICKS (1000));
      	xQueueSend(comms_queue,&hearbeat,0);
    }
}

//-------------------------------------------------------------------------------------
/** The main function sets up the RTOS.  Some test tasks are created. Then the 
 *  scheduler is started up; the scheduler runs until power is turned off or there's a 
 *  reset.
 *  @return This is a real-time microcontroller program which doesn't return. Ever.
 */
int main (void)
{
	// Disable the watchdog timer unless it's needed later. This is important because
	// sometimes the watchdog timer may have been left on...and it tends to stay on
    wdt_disable();
    stdout = &mystdout; // required to let printf work with the uart
    char* char_pointer;
	
	// Create a queue of 30 character pointers for the communications task.
    comms_queue = xQueueCreate(SIZE_COMMS_QUEUE, sizeof(char_pointer));
    adc_mutex_semaphore = xSemaphoreCreateMutex();

    xTaskCreate(task_sensors,"Sensors", STACK_SIZE_SENSORS, NULL, PRIORITY_SENSORS, NULL);
    xTaskCreate(task_comms,"Comms", STACK_SIZE_COMMS, NULL, PRIORITY_COMMS, NULL);  
    xTaskCreate(task_heartbeat,"Heartbeat", 280, NULL, PRIORITY_HEARTBEAT, NULL); 
    xTaskCreate(task_motor1, "Motor1", STACK_SIZE_MOTORS, NULL, PRIORITY_MOTORS, NULL);
    xTaskCreate(task_motor2, "Motor2", STACK_SIZE_MOTORS, NULL, PRIORITY_MOTORS, NULL);
    xTaskCreate(task_master, "Master", STACK_SIZE_MASTER, NULL, PRIORITY_MASTER, NULL);
    xTaskCreate(task_orient,"Orient", STACK_SIZE_ORIENT, NULL, PRIORITY_ORIENT, NULL);
    xTaskCreate(task_safety, "Safety", STACK_SIZE_SAFETY, NULL, PRIORITY_SAFETY, NULL);
    
	encoders_init();
	sei();
	
	vTaskStartScheduler ();
	while(1);
}

//-------------------------------------------------------------------------------------
/** \brief This ISR updates the motors position when a pin change interrupt has occured
 *  as a result of a change in the encoder output waveforms.
 */
ISR(PCINT0_vect){
	//previous state of Motor 1 is saved via static var
	static uint8_t previous_state_M1;

	//previous state of Motor 2 is saved via static var
	static uint8_t previous_state_M2;
	
	//get new encoder state
    uint8_t state_M1 = ( (1<<ENC_A_M1)|(1<<ENC_B_M1) ) & PINA;
    uint8_t state_M2 = ( (1<<ENC_A_M2)|(1<<ENC_B_M2) ) & PINA;

    state_M2 = (state_M2>>2); // Motor 2's encoders are on PORT A, bits 2 and 3. This
                              // line of code maps their possible states to unsigned 
                              // integer values 0 through 3.
	
	//check encoder state sequence
	const uint8_t sequence[6]={3,2,0,1,3,2};
	
	for (uint8_t i=1; i<5;i++)
	{
		if (state_M1==sequence[i])
		{
			//check for ccw, write to dir
			if (previous_state_M1==sequence[i-1])
			{
				position_M1_SHARED = position_M1_SHARED - 1;
				break;
			}
			//check for cw, write to dir
			else if(previous_state_M1==sequence[i+1])
			{
				position_M1_SHARED = position_M1_SHARED + 1;
				break;
			}
			//if same, pin change likely occured on M2
			else if (previous_state_M1==sequence[i])
			{
			        position_M1_SHARED = position_M1_SHARED;
				break;
			}
			//check for error, write to error
			else
			{
				error_M1_SHARED++;
				break;
			}
		}
	}
	for (uint8_t i=1; i<5;i++)
	{
		if (state_M2==sequence[i])
		{
			//check for ccw, write to dir
			if (previous_state_M2==sequence[i-1])
			{
				position_M2_SHARED = position_M2_SHARED - 1;
				break;
			}
			//check for cw, write to dir
			else if(previous_state_M2==sequence[i+1])
			{
				position_M2_SHARED = position_M2_SHARED + 1;
				break;
			}
			//if same, pin change likely occured on M2
			else if (previous_state_M2==sequence[i])
			{
			        position_M2_SHARED = position_M2_SHARED;
				break;
			}
			//check for error, write to error
			else
			{
				error_M2_SHARED++;
				break;
			}
		}
	}
	//save previous state
	previous_state_M1 = state_M1;
	previous_state_M2 = state_M2;
}