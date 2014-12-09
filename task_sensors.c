//*************************************************************************************
/** \file task_sensors.c
 *  \brief This file contains functions for the ADCs, joystick,
 *  and buttons.
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

xSemaphoreHandle adc_mutex_semaphore;
uint8_t btn_SHARED;
//-------------------------------------------------------------------------------------
/** \brief This Function prompts the AVR I2C device to read one or more bytes.
 *  \details This Function prompts the I2C device to read data from the I2C
 *  data lines in master receiver mode. If the argument "lastbit" 
 *  is 0, then after receiving the last byte of information, the 
 *  acknowledge bit is not pulled low - this indicates the end of 
 *  the transmission. If "lastbit" is set to 1, the acknowledge bit
 *  will be sent, and the I2C device will continue to read data. 
 * 
 *  @param lastbit This tells the function whether or not pull the i2c/twi data lines
 *  low for an "ACK" condition.
 *  @return Returns the single byte of recieved data.
 */

//-------------------------------------------------------------------------------------
/** \brief This function initializes the ADC on the ATMEGA 1284p.
 */
void adc_init(void){
	// Set the "ADC Enable" bit within the ADC Status Register A,
	// and set the ADC clock prescaler to divide by 32. Enable
	// interrupt on conversion complete. 
	ADCSRA = (1<<ADIE) | (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);
	ADMUX |= 1<<REFS0;// Set the analog reference voltage to AVCC.

}

//-------------------------------------------------------------------------------------
/** \brief This Function reads a byte from the selected ADC channel.
 *  @param adc_channel Takes one of the #define macros from task_sensors.h as a
 *  parameter, for the purpose of selecting one of the ADC channels.
 *  @return Returns and unsigned 16bit data value.
 */
uint16_t adc_read(uint8_t adc_channel){
	/// Use a mutex to control access to the ADC.
    xSemaphoreTake(adc_mutex_semaphore, portMAX_DELAY);
        
	    // Clear the ADC Multiplexor Selection bits, and the set them to
        // the appropriate ADC channel.
        ADMUX &= ~( (1<<MUX2)|(1<<MUX1)|(1<<MUX0) ); 
	    ADMUX |= adc_channel; 
		
		// Set the "ADC start conversion" bit, initiating a conversion.
		ADCSRA |= 1<<ADSC;
	    const uint16_t maxWaitCycles = 60000;
	    uint16_t counter = 0;
	    while(ADCSRA & (1<<ADSC) && maxWaitCycles > counter++);
        uint16_t temp = ADC;

	xSemaphoreGive(adc_mutex_semaphore);
	return temp;

}

//-------------------------------------------------------------------------------------
/** \brief This Function detects whether or not a button has been pressed.
 *  \details This function performs a software debounce, and updates a shared 
 *  variable(btn_SHARED) when a level change is detected, indicating that 
 *  the button has been pressed.
 *  @return Returns the status of the button level.
 */
uint8_t button_pressed(void){
	static uint16_t count = 0;
	static uint8_t btn_on = 0;
	
	if( PINC & (1<<PC3) ) // This indicates that the button is NOT pressed, since
	{                     // a pullup resistor, and a switch to ground are used
	  
		if (count >= BTN_OFF_THRESHHOLD) //If button has been released for a  
		{                               // certain number of cycles, consider
		                                // it to be "off".
		  
			
			
			if(btn_on)        // If the button was previously ON, update the
			{                 // shared button variable.
			  taskENTER_CRITICAL();
			      btn_SHARED = 0;
			  taskEXIT_CRITICAL();
			}
			btn_on = 0; // update the local button variable.
			count = count;    //clamp the count variable to the threshhold.
		}
		else // Button pin reading is low, but the button can't be certainly
		{    // said to be released, yet. Increment "count" variable until the 
		     // "Button OFF threshhold" has been met.
			count++;
		}
	}

	else // This indicates that  PINC & (1<<PC3) evaluates false, so the button pin is reading
	     // a signal indicating that the button is pressed. 
	{
		if (count <= 0) // IF button has been pressed for long enough for the
		                // count variable to decrement all the way to zero, 
		{               // The button is definitely being pressed.
			count = 0; // clamp the count to zero.
			
			if(!btn_on) // IF the button was previously OFF, update the shared
			{           // button variable.
			  taskENTER_CRITICAL();
			      btn_SHARED = 1;
			  taskEXIT_CRITICAL();
			}
			btn_on = 1; // update the local button variable.
		}
		
		
		else // The button pin is reading that the button is being pressed, but the 
		     // button can't be certainly said to be pressed yet. Continue decrementing
		     // count, until it hits zero, indicating a definite press of the button.
	        {    
	         	count--;
	        }
	}
	return btn_on;
}

//-------------------------------------------------------------------------------------
/** \brief This function initializes the "Target Set" button.
 */
void button_init(void){
    btn_SHARED = 0;
    DDRC &= ~(1<<PC3);
}

//-------------------------------------------------------------------------------------
/** \brief This is the task function for monitoring the button and joysticks.
 *  \details This function monitors the "Target Set" button, and when it is pressed,
 *  updates shared variables "motor1_power_SHARED", and "motor2_power_SHARED" with adc_channel
 *  readings. If the button is NOT pressed, the motor power shareds are set to the effective
 *  "zero power" value of 512.(1024 adc values corresponding to full forward(1023), and
 *  full reverse(0);
 */
void task_sensors(void* pvParameters){
	uint8_t default_sensor_prio = uxTaskPriorityGet(NULL);
	portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    adc_init();
    uint16_t joystick_y;
    uint16_t joystick_x;
    button_init();
       
    while(1)
    {
        if(button_pressed()){
            joystick_y = adc_read(ADC_JOYSTICK_Y);
            joystick_x = adc_read(ADC_JOYSTICK_X);
            vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
    	        motor1_power_SHARED = joystick_y;
    	        motor2_power_SHARED = joystick_x;
    	    vTaskPrioritySet(NULL, default_sensor_prio);
    	}
    	else{
	    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
    	        motor1_power_SHARED = 512;
    	        motor2_power_SHARED = 512;
    	    vTaskPrioritySet(NULL, default_sensor_prio);
	    }
    	vTaskDelayUntil(&xLastWakeTime, 100/portTICK_RATE_MS);
    }

	
}

