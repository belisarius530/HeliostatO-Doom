//*************************************************************************************
/** \file task_orient.c
 *  \brief This file contains functions for calculating desired orientation.
 *  \details The functions within this file are associated with the orientation
 *  algorithm, the magnetometer drivers, and the RTC drivers. Functionality to 
 *  determine the correct angle based on the target position, and command the motors
 *  to get there are contained here.
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
#include "task_orient.h"
#include "twi.h"

/// These are the magnetometer position variables
uint8_t x_h_SHARED;
uint8_t	x_l_SHARED;
	
uint8_t	z_h_SHARED;
uint8_t	z_l_SHARED;
	
uint8_t	y_h_SHARED;
uint8_t	y_l_SHARED;

//-------------------------------------------------------------------------------------
/** \brief This function initializes the HMC5883 magnetometer.
 */
void HMC5883_init(void)
{
	twi_init();
	printf("HMC Initialization started");
    twi_start();
	twi_write(0x3C); //Write SLA + W 
	twi_write(0x00); // FIRST CONTROL REGISTER
	twi_write(0x00);
	twi_write(0x00);
	twi_stop();
}

//-------------------------------------------------------------------------------------
/** \brief This function reads HMC5883 magnetometer data into shared variables.
 *  \details This function uses shared variables, and should therefore only be called
 *  when the calling task's priority set very high, or from within a critical 
 *  section.
 */
void HMC5883_read(void) 
{

	twi_start();
	twi_write(0x3D); //SLA + R

	x_h_SHARED = twi_read(READ_MULTI); //SHARED VARIABLE ACCESS
	x_l_SHARED = twi_read(READ_MULTI); //SHARED VARIABLE ACCESS
	
	z_h_SHARED = twi_read(READ_MULTI); //SHARED VARIABLE ACCESS
	z_l_SHARED = twi_read(READ_MULTI); //SHARED VARIABLE ACCESS
	
	y_h_SHARED = twi_read(READ_MULTI); //SHARED VARIABLE ACCESS
	y_l_SHARED = twi_read(READ_SINGLE); //SHARED VARIABLE ACCESS

	twi_stop();

}

//-------------------------------------------------------------------------------------
/** \brief This task function reads sensor data, and calculates the desired mirror angle
 *  \details This function wakes up every four minutes, determines time based on RTC 
 *  sensor, determines heading with the magnetometer, calculates the angle of the sun,
 *  calculates the desired angle of the mirror array, commands that the array track 
 *  the calculated angle, and blocks itself for another four minutes.
 */ 
void task_orient(void* pvParameters){
	uint8_t default_orient_prio = uxTaskPriorityGet(NULL);
	portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    HMC5883_init();
    while(1)
    {
    	/// The priority is set to max, since HMC5883_read accesses a shared variable.
     	vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
     	    HMC5883_read();
    	vTaskPrioritySet(NULL, default_orient_prio);
		position_cmd_M1_SHARED = 0; // To be changed as soon as the orientation algorithm
	    position_cmd_M2_SHARED = 0; // is implemented.
    	vTaskDelayUntil(&xLastWakeTime, 600000/portTICK_RATE_MS);
    }
}