//*************************************************************************************
/** \file task_motors.c
 *  \brief This file contains functions for motor drivers, encoders, and the motor
 *  tasks.
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
#include "pid.h"
#include "shares.h"
#include "task_motors.h"
#include "math.h"
#include "task_master.h"

// These are shared variables used by the motor tasks.
volatile uint8_t int_occurred;
int16_t motor1_power_SHARED; // Set by the Joystick
int16_t motor2_power_SHARED;

volatile int16_t position_M2_SHARED; // Set by encoder ISR which is defined in main.
volatile int16_t position_M1_SHARED;

volatile uint16_t error_M1_SHARED;  // Set by encoder ISR which is defined in main.
volatile uint16_t error_M2_SHARED;

int16_t position_cmd_M1_SHARED; // Set by orientation algorithm
int16_t position_cmd_M2_SHARED;

int16_t position_targ_init_SHARED; // This will get by user in task_motors

//-------------------------------------------------------------------------------------
/** \brief This function initializes both motor encoders.
 *  \details This function initializes all of the required pins, variables, and
 *  interrupts necessary to use the quadrature encoders.
 */
void encoders_init(void){	// Set encoder pins as inputs.
	DDRA &= ~((1<<ENC_A_M1)|(1<<ENC_B_M1)|(1<<ENC_A_M2)|(1<<ENC_B_M2));

    //Global variables, defined in task_motors.c. Only access in critical
    //sections, or in tasks with priority : configMAX_PRIORITIES.
    //Initialization is the only exception to this rule.
    position_M1_SHARED = 0;
    position_M2_SHARED = 0;

    //Global variables, defined in task_motors.c. Only access in critical
    //sections, or in tasks with priority : configMAX_PRIORITIES.
    //Initialization is the only exception to this rule.
    error_M1_SHARED = 0;
    error_M2_SHARED = 0;

	// Set encoder inputs as Pin Change interrupts.
	PCICR |= (1<<PCIE0);
    PCMSK0 |= (1<<PCINT0)|(1<<PCINT1)|(1<<PCINT2)|(1<<PCINT3);
}

//-------------------------------------------------------------------------------------
/** \brief This function initializes both motors.
 *  \details This function initializes all required pins and variables for the motors,
 *  and also initializes timer 1 to generate an output pwm signal for both motor 
 *  controllers.
 */
void motors_init(void){
    /* Set Timer one to 10-bit Fast PWM mode, and configure
	 the PWM pin to set at BOTTOM, and clear at a compare match.*/
	TCCR1A |= (1<<WGM11) | (1<<WGM10) | (1<<COM1A1) | (1<<COM1B1);
	TCCR1A &= ~( (1<<COM1A0) | (1<<COM1B0) ) ;
	TCCR1B = (1<<WGM12) | (1<<CS11);
	OCR1A = 0x00;
	OCR1B = 0x00;

	// SET IN A/B pins as output for both motors
	DDRB |= (1<<IN_A_M1) | (1<<IN_B_M1); 
	DDRB |= (1<<IN_A_M2) | (1<<IN_B_M2);

	// SET PWM signals for motors as outputs
	DDRD |= (1<<PWM_M1) | (1<<PWM_M2);
	
	// Configure Motor 1 Enable A/B as an input with a pullup resistor.
	DDRB &= ~(1<<EN_AB_M1);
	PORTB |= (1<<EN_AB_M1);

	// Configure Motor 2 Enable A/B as an input with a pullup resistor.
	DDRC &= ~(1<<EN_AB_M2);
	PORTC |= (1<<EN_AB_M2);
	
	//Global variables, defined in task_motors.c. Only access in critical
    //sections, or in tasks with priority : configMAX_PRIORITIES.
    //Initialization is the only exception to this rule.
    motor1_power_SHARED = 0;
    motor2_power_SHARED = 0;
    position_cmd_M1_SHARED = 0;
    position_cmd_M2_SHARED = 0;
	position_targ_init_SHARED = 0;
}

//-------------------------------------------------------------------------------------
/** \brief This function sets the power level for motor 1.
 *  \details This function converts a signed integer into a pwm signal and direction
 *  command for the vnh5019 motor driver chips.
 *  \param power The signed 16-bit integer corresponding to the magnitude and direction
 *  of power applied to the motors. 
 */
void motor1_power(int16_t power){
    if(power > 0)
    {
		// Impose a saturation limitation on the motor power value. 
        if(power>PWR_LIMIT_M1)
		{
    		power = PWR_LIMIT_M1;
		}
		// Set direction to forward, through motor control pins.
		PORTB |= (1<<IN_A_M1);
    	PORTB &= ~(1<<IN_B_M1);
    	OCR1A = power;
    }
    else
    {
		// Impose a saturation limitation on the motor power value. 
    	if(power<-PWR_LIMIT_M1)
		{
    		power = -PWR_LIMIT_M1;
		}
		// Set direction to reverse, through motor control pins.
		PORTB |= (1<<IN_B_M1);
    	PORTB &= ~(1<<IN_A_M1);
        OCR1A = abs(power);
    }
}

//-------------------------------------------------------------------------------------
/** \brief This function sets the power level for motor 2.
 *  \details This function converts a signed integer into a pwm signal and direction
 *  command for the vnh5019 motor driver chips.
 *  \param power The signed 16-bit integer corresponding to the magnitude and direction
 *  of power applied to the motors. 
 */
void motor2_power(int16_t power){
    if(power > 0)
    {
		// Impose a saturation limitation on the motor power value. 
        if(power>PWR_LIMIT_M2)
		{
    		power = PWR_LIMIT_M2;
		}
		// Set direction to forward, through motor control pins.
		PORTC |= (1<<IN_A_M2);
    	PORTC &= ~(1<<IN_B_M2);
    	OCR1B = power;
    }
    else
    {
		// Impose a saturation limitation on the motor power value. 
    	if(power<-PWR_LIMIT_M2)
		{
    		power = -PWR_LIMIT_M2;
		}
		// Set direction to reverse, through motor control pins.
		PORTC |= (1<<IN_B_M2);
    	PORTC &= ~(1<<IN_A_M2);
        OCR1B  = abs( power);
    }
}

//-------------------------------------------------------------------------------------
/** \brief This is the task function for motor 1.
 *  \details This function decides, based on the system state, which control signal gets
 *  to set the power level of motor 1. System state is decided/updated in task_master.
 */
void task_motor1(void *pvParameters){
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    motors_init();
    int16_t motor1_power_cmd = 0;
	int16_t motor1_joystick_cmd = 0;
    int16_t motor1_position_cmd = 0;
    int16_t motor1_position = 0;
	int16_t state = 0;
    while(1){
    	taskENTER_CRITICAL();
		    // Motor1_power_shared is updated by the joystick adc reading
		    motor1_joystick_cmd = motor1_power_SHARED; 
			
			// Position_cmd_M1_SHARED is updated by task_orient
     	    motor1_position_cmd = position_cmd_M1_SHARED; 
			
			// Motor1_position is updated by the encoders.
	        motor1_position = position_M1_SHARED;
			
			// State_shared is updated by task_master
			state = state_SHARED;
        taskEXIT_CRITICAL();

	    switch(state){
			case  AWAITING_CAL : //Note: states are defined/described in task_master.h/.c
		        motor1_power(0);
		    	break; 
			
		    case CALIBRATION  :
				// map 0-1023 ADC reading to (-1024,+1023)
		    	motor1_power_cmd = motor1_joystick_cmd*2-1023;
		     	motor1_power(motor1_power_cmd);
		    	taskENTER_CRITICAL();
			        position_M1_SHARED = 0;
				taskEXIT_CRITICAL();
				break;
				
			case AWAITING_TARG  :
				motor1_power(0);
				break;
				
			case SELECT_TARG  :
				// map 0-1023 ADC reading to (-1024,+1023)
				motor1_power_cmd = motor1_joystick_cmd*2-1023;
				motor1_power(motor1_power_cmd);
				taskENTER_CRITICAL();
				    position_targ_init_SHARED = motor1_position;
				taskEXIT_CRITICAL();
				break;
				
			case TRACK_TARG  :// Position_cmd_M1_SHARED(aka motor1_position_cmd) is updated by task_orient
				motor1_power_cmd = -pid_1(motor1_position, motor1_position_cmd);
				motor1_power(motor1_power_cmd);
				break;
				
			case ERROR  :
				motor1_power(0);
				break;
				
			case IDLE  :
				motor1_power(0);
				break;
				
			default : 
				motor1_power(0);
		}
        vTaskDelayUntil(&xLastWakeTime, 50/portTICK_RATE_MS);
    }
}

//-------------------------------------------------------------------------------------
/** \brief This is the task function for motor 2.
 *  \details This function decides, based on the system state, which control signal gets
 *  to set the power level of motor 2. System state is decided in task_master.
 */
void task_motor2(void *pvParameters){
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    motors_init();
    int16_t motor2_power_cmd = 0;
	int16_t motor2_joystick_cmd = 0;
    int16_t motor2_position_cmd = 0;
    int16_t motor2_position = 0;
	int16_t state = 0;
    while(1){
    	taskENTER_CRITICAL();
		    // Motor2_power_shared is updated by the joystick adc reading.
		    motor2_joystick_cmd = motor2_power_SHARED;
			
			// Position_cmd_M2_SHARED is updated by task_orient.
     	    motor2_position_cmd = position_cmd_M2_SHARED;
			
			// Motor2_position is updated by the encoders.
	        motor2_position = position_M2_SHARED;
			
			// State_shared is updated by task_master.
			state = state_SHARED;
        taskEXIT_CRITICAL();

	switch(state){
		case  AWAITING_CAL : //Note: states are defined/described in task_master.h/.c
		    motor2_power(0);
			break; 
			
		case CALIBRATION  :
			// map 0-1023 ADC reading to (-1024,+1023)
			motor2_power_cmd = motor2_joystick_cmd*2-1023; 
			motor2_power(motor2_power_cmd);
			taskENTER_CRITICAL();
			    position_M2_SHARED = 0;
			taskEXIT_CRITICAL();
			break;
			
		case AWAITING_TARG  :
			motor2_power(0);
			break;
			
		case SELECT_TARG  :
			// map 0-1023 ADC reading to (-1024,+1023)
			motor2_power_cmd = motor2_joystick_cmd*2-1023;
			motor2_power(motor2_power_cmd);
			taskENTER_CRITICAL();
			    position_targ_init_SHARED = motor2_position;
			taskEXIT_CRITICAL();
			break;
			
		case TRACK_TARG  : // Position_cmd_M1_SHARED(aka motor1_position_cmd) is updated by task_orient
			motor2_power_cmd = pid_2(motor2_position, motor2_position_cmd);
            motor2_power(motor2_power_cmd);
			break;
			
		case ERROR  :
			motor2_power(0);
			break;
				
		case IDLE  :
			motor2_power(0);
			break;
				
		default : 
			motor2_power(0);
		
	}
        vTaskDelayUntil(&xLastWakeTime, 50/portTICK_RATE_MS);
    }
}
