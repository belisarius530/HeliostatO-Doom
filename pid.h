//*************************************************************************************
/** \file pid.h
 *  \brief This file contains the #defines, and function declarations for the PID 
 *  functions.
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

#ifndef PID_H_
#define PID_H_
int16_t pid_1(int16_t feedback_signal,int16_t reference_input);
int16_t pid_2(int16_t feedback_signal,int16_t reference_input);

#define INT_CLAMP_1 100
#define OUT_CLAMP_1 150
#define K_INT_1 0.001F
#define K_PROP_1 1.0F
#define K_DER_1 0.0F

#define INT_CLAMP_2 75
#define OUT_CLAMP_2 350
#define K_INT_2 0.001F
#define K_PROP_2 1.0F
#define K_DER_2 0.0F


#endif /* PID_H_ */