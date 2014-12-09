//*************************************************************************************
/** \file task_safety.c
 *  \brief This file contains functions which monitor the motors and turns them off
 *  if they fault or over current.
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h" 
#include "semphr.h"

#include "shares.h"
#include "task_sensors.h"
#include "twi.h"
#include "task_motors.h"
#include "task_safety.h"

uint8_t safety_error_SHARED;

//-------------------------------------------------------------------------------------
/** \brief This task function for the safety task.
 *  \details monitors the the motor diag_enable A/B pins, and the
 *  current, and shuts the motors off if any problems are detected.
 */
void task_safety(void* pvParameters){
	uint8_t default_safety_prio = uxTaskPriorityGet(NULL);
	portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
	uint8_t safety_error_SHARED = 0;
    uint16_t current_val_M1 = 0;
    uint16_t current_val_M2 = 0;
    const char *over_current_m1 = "Error: Excess current in Motor 1!";
    const char *over_current_m2 = "Error: Excess current in Motor 2!";
    const char *fault_m1 = "Error: Fault on Motor 1!";
    const char *fault_m2 = "Effor: Fault on Motor 2!";

    while(1)
    {   
		// Check for over current in motor 1.
    	current_val_M1 = adc_read(ADC_CURRENT_M1); 
    	if(current_val_M1>MAX_CURRENT_M1)
    	{
			// Critical section unnecessary for accessing the shared var,
			// since the safety task has the top priority :D
			// (also no ISR touches this var)
			safety_error_SHARED = 1; //Flag an error!
            xQueueSend(comms_queue,&over_current_m1,0);
    	}          
    	
    	// Check for over current in motor 2.
    	current_val_M2 = adc_read(ADC_CURRENT_M2);
    	if(current_val_M2>MAX_CURRENT_M2)
    	{
			safety_error_SHARED = 1;//Flag an error!
            xQueueSend(comms_queue,&over_current_m2,0);
    	}
        
        // Check for fault on motor 1.
    	if( !(PINC & (1<<EN_AB_M1)) )
    	{
			safety_error_SHARED = 1;//Flag an error!
    		xQueueSend(comms_queue,&fault_m1,0);
    	}

    	// Check for fault on motor 2.
    	if( !(PINC & (1<<EN_AB_M2)) )
    	{
			safety_error_SHARED = 1;//Flag an error!
    		xQueueSend(comms_queue,&fault_m2,0);
    	}
    	
    	vTaskDelayUntil(&xLastWakeTime, 100/portTICK_RATE_MS);
    }

	
}
