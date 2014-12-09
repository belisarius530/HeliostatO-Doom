//*************************************************************************************
/** \file pid.c
 *  \brief This file contains function definitions for both PID functions.
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
#include "pid.h"

int16_t pid_1(int16_t feedback_signal, int16_t reference_input)
{    
	static float int_out_prev = 0; 
	static float error_prev = 0;
	
	float error;
	float der_out;
	float prop_out;
	float int_out;
		
	
	error = reference_input - feedback_signal;
	///////////////////////////////////////
	// Proportional Term Calculation
	prop_out = K_PROP_1*error;
	
	
	///////////////////////////////////////
	// Integral Term Calculation
	
	int_out = K_INT_1*(float)error + int_out_prev; 
	
	if( int_out > INT_CLAMP_1 ) // This integrator saturation limit should be something that is specified as a parameter of the constructor
	{
		int_out = INT_CLAMP_1;
	}
	if( int_out < -INT_CLAMP_1 )
	{
		int_out = -INT_CLAMP_1;
	}
	
	int_out_prev = int_out;
	//////////////////////////////////////
	// Derivative Term Calculation
	der_out = K_DER_1*(error - error_prev );     //derivative of measurement--constant ref drops out.
	
	error_prev = error;

	int16_t out = prop_out + int_out + der_out;
	
	if( out  > OUT_CLAMP_1 ) out = OUT_CLAMP_1;
	if( out < -OUT_CLAMP_1 ) out = -OUT_CLAMP_1;
	return out;
}

int16_t pid_2(int16_t feedback_signal, int16_t reference_input)
{    
	static float int_out_prev = 0; 
	static float error_prev = 0;
	
	float error;
	float der_out;
	float prop_out;
	float int_out;
		
	
	error = reference_input - feedback_signal;
	///////////////////////////////////////
	// Proportional Term Calculation
	prop_out = K_PROP_2*error;
	
	
	///////////////////////////////////////
	// Integral Term Calculation
	
	int_out = K_INT_2*(float)error + int_out_prev; 
	
	if( int_out > INT_CLAMP_2 ) // This integrator saturation limit should be something that is specified as a parameter of the constructor
	{
		int_out = INT_CLAMP_2;
	}
	if( int_out < -INT_CLAMP_2 )
	{
		int_out = -INT_CLAMP_2;
	}

	int_out_prev = int_out;
	//////////////////////////////////////
	// Derivative Term Calculation
	der_out = K_DER_2*(error - error_prev );     //derivative of measurement--constant ref drops out.
	
	error_prev = error;

	int16_t out = prop_out + int_out + der_out;
	
	if( out  > OUT_CLAMP_2 ) out = OUT_CLAMP_2;
	if( out < -OUT_CLAMP_2 ) out = -OUT_CLAMP_2;
	return out;
}