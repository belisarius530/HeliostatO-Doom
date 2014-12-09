//*************************************************************************************
/** \file task_motors.h
 *  \brief This file contains #defines and function declarations for motor tasks.
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

#ifndef _TASK_MOTORS_H_
#define _TASK_MOTORS_H_

#define STACK_SIZE_MOTORS 280

// MOTOR 1 CONTROL SIGNALS
#define IN_A_M1 PB0
#define IN_B_M1 PB1
#define EN_AB_M1 PB2
#define PWM_M1 PD5

// MOTOR 2 CONTROL SIGNALS
#define IN_A_M2 PC4
#define IN_B_M2 PC5
#define EN_AB_M2 PC6
#define PWM_M2 PD4

// ENCODER CONTROL SIGNALS
#define ENC_A_M1 PA0
#define ENC_B_M1 PA1
#define ENC_A_M2 PA2
#define ENC_B_M2 PA3

// MOTOR POWER SATURATION LIMIT
#define PWR_LIMIT_M1 150
#define PWR_LIMIT_M2 350


void encoders_init(void);

void motors_init(void);

void motor1_power(int16_t power);
void motor2_power(int16_t power);

void task_motor1(void *pvParameters);
void task_motor2(void *pvParameters);

#endif