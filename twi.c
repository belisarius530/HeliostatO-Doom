//*************************************************************************************
/** \file twi.c
 *  \brief This file contains functions used to interface with AVR twi/i2c hardware.
 *
 *  Revisions:
 *    \li 04-01-2014 JF created original file
 *
 *  License:
 *		This file is copyright 2012 by Jonathan Fish and released under the Lesser GNU 
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
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "twi.h"

//-------------------------------------------------------------------------------------
/** \brief This Function prompts the AVR I2C device to perform the start condition.
 */
void twi_start(void) // 
{
	TWCR = ( (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) );

	while( (TWCR & (1<<TWINT)) == 0 )
	{
		__asm__("nop");
    }
}

//-------------------------------------------------------------------------------------
/** \brief This Function prompts the AVR I2C device to perform the stop condition.
 */
void twi_stop(void)
{
	TWCR = ( (1<<TWINT) | (1<<TWSTO) | (1<<TWEN) );
}

//-------------------------------------------------------------------------------------
/** \brief This Function prompts the AVR I2C device to write one byte of data to the I2C lines.
 */
void twi_write(uint8_t data)
{
	TWDR = data;
	TWCR = ( (1<<TWINT) | (1<<TWEN) );

    while( (TWCR & (1<<TWINT)) == 0 )
	{
		__asm__("nop");
    }
}

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
uint8_t twi_read(uint8_t lastbit)
{
	if(lastbit == 0)
	{
		TWCR = ( (1<<TWINT) | (1<<TWEN) );
	}
	else
	{
		TWCR = ( (1<<TWINT) | (1<<TWEA) | (1<<TWEN) );
	}

	while( ((1<<TWINT) & TWCR) == 0 )
	{
		__asm__("nop");
    }
	
    return TWDR;
}

//-------------------------------------------------------------------------------------
/** \brief This Function enable and initializes the I2C device.
 */
void twi_init(void)
{
	//PRR &= ~(1<<7); // ensure that twi is enabled
	TWBR = 0x0C; // Set transmission rate to 400kHz
	TWSR &= ~( (1<<TWPS1) | (1<<TWPS0) );
	TWCR = (1<<TWEN);
}

