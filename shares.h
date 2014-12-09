//*************************************************************************************
/** \file shares.h
 *  \brief This file contains the declarations for all shared variables.
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

#ifndef _SHARES_H_
#define _SHARES_H_

extern xQueueHandle comms_queue; //Defined in task_comms.c
extern xSemaphoreHandle adc_binary_semaphore;
extern xSemaphoreHandle adc_mutex_semaphore;

extern int16_t motor1_power_SHARED; //Defined, used in task_motors.c
extern int16_t motor2_power_SHARED; //Defined, used in task_motors.c

extern int16_t position_cmd_M1_SHARED; //Defined, used in task_motors.c
extern int16_t position_cmd_M2_SHARED; //Defined, used in task_motors.c

extern volatile int16_t position_M1_SHARED; //Defined, used in task_motors.c
extern volatile int16_t position_M2_SHARED; //Defined, used in task_motors.c

extern volatile uint16_t error_M1_SHARED; //Defined, used in task_motors.c
extern volatile uint16_t error_M2_SHARED; //Defined, used in task_motors.c

extern uint8_t  x_h_SHARED; // Defined in task_sensors.c,
extern uint8_t	x_l_SHARED; // protect by maxing priority before
	                 // accessing these variables in any given
extern uint8_t	z_h_SHARED; // task.
extern uint8_t	z_l_SHARED;
	
extern uint8_t	y_h_SHARED;
extern uint8_t	y_l_SHARED;

extern uint8_t	btn_SHARED; // Defined in task_sensors.c

extern uint8_t state_SHARED; // Defined in task_master.c

extern int16_t position_targ_init_SHARED; // Defined in task_motors.c

extern uint8_t safety_error_SHARED; // Defined in task_safety

#endif 