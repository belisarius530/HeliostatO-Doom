//*************************************************************************************
/** \file task_master.c
 *  \brief This file contains function declarations for the master task. This task
 *  is an implementation of a state machine, which governs the state of the system
 *  based on the values of certain shared variables.
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
#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h" 
#include "semphr.h"

#include "uart.h"
#include "shares.h"
#include "task_master.h"

uint8_t state_SHARED;

 //-------------------------------------------------------------------------------------
/** \brief This is the task function for the master task.
 *  \details This function decides which state the system is in, based on the previous
 *  state, and the current values of certain shared variables. The seven possible
 *  system states are as follows:
 * 
 *      AWAITING_CAL - Awaiting Calibration - The system is waiting for the user to 
 *      press the target set button, to initialize the zero reference position.
 *      Motor power: 0
 * 
 *      CALIBRATION  - Calibration State - The user is holding the target set button,
 *      and controlling the mirror array with the joystick. Once the mirror array is
 *      in the zero reference position, the user can release the button.
 *      Motor power: Joystick ADC Reading
 * 
 *      AWAITING_TARG - Awaiting Target - The system is waiting for the user to 
 *      press the target set button, to initialize the first target.
 *      Motor power: 0
 * 
 *      SELECT_TARG  - Target Selection - The user is holding the target set button,
 *      and controlling the mirror array with the joystick. Once the sun is correctly
 *      being redirected to the target, the user can release the button.
 *      Motor power: Joystick ADC Reading
 * 
 *      TRACK_TARG - The mirror array is actively tracking the sun, and redirecting
 *      the beam of light to the target.
 *      Motor power: PI controller
 * 
 *      ERROR - The safety task has decided that an error has occurred, and the motors
 *      are powered down in response.
 *      Motor power: 0
 * 
 *      IDLE - state which allows the user to reset the system from the ERROR state to
 *      the SELECT_TARG state.
 *      Motor power: 0
 */
void task_master(void* pvParameters){
    state_SHARED = AWAITING_CAL;
    uint8_t btn = 0;
	uint8_t error = 0;
    while(!btn)
    {
        vTaskDelay(configMS_TO_TICKS (200));
        taskENTER_CRITICAL();
            state_SHARED = AWAITING_CAL;
            btn = btn_SHARED;
        taskEXIT_CRITICAL();
    }
    while(btn)
    {
        vTaskDelay(configMS_TO_TICKS (200));
        taskENTER_CRITICAL();
	        state_SHARED = CALIBRATION;
	        btn = btn_SHARED;
        taskEXIT_CRITICAL();
    }
    while(!btn)
    {
       vTaskDelay(configMS_TO_TICKS (200));
       taskENTER_CRITICAL();
          state_SHARED = AWAITING_TARG;
          btn = btn_SHARED;
       taskEXIT_CRITICAL();
    }
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        taskENTER_CRITICAL();
            btn = btn_SHARED;
		    error = safety_error_SHARED;
        taskEXIT_CRITICAL(); 
	    if(!error){
		   
			if(btn)
			{
			taskENTER_CRITICAL();
					state_SHARED = SELECT_TARG;
			taskEXIT_CRITICAL();
			}
			else
			{
			taskENTER_CRITICAL();
					state_SHARED = TRACK_TARG;
				taskEXIT_CRITICAL();
			}
		}
		else
		{
			state_SHARED = ERROR;
		}	
       vTaskDelayUntil(&xLastWakeTime, 200/portTICK_RATE_MS);
    }
}
