/* GNOME sound-Monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 * fft code written by Mahmood Shakir Qasmi <qasmims@mcmaster.ca>
 *
 * NOTE: this code is slow, and mostly broken, if anyone wants to change this to
 * an accurate, speedy fft please do so, as this was never finished.
 *
 */


#include "fft.h"

/******************************************************************\
*       Super_Class::fft::Public
*       >>>>>>>>>>>>>>>>>>>>>>>>
*
*       Uses: Audio Stream Array in the form of "double" Data-Type
*
*       Output: Array for fft Display in N-Channels
*
*
\******************************************************************/

#include <stdio.h>
#include <math.h>

/*Public FFT Physical Parameters*/
#define pie                     3.141592653589793238462643
#define BAND                    32       /*      Maximum Array Size    */
#define MIN_FREQUENCY           200      /*      SI Units: Hz          */
#define FREQUENCY_INCREMENT     2000     /*      SI Units: Hz          */
#define MAX_FREQUENCY           12000    /*      SI Units: Hz          */
#define START                   0        /*      SI Units: Sec         */

/*Public FFT Macros*/
#define SwAp(a,b)               TEMP=a;a=b;b=TMP
#define dB(a)                   20*log(a)
#define w(x)                    2*pie*x
#define INCREMENT(x)            x=x+1

/*Public FFT Enumeration Structure*/
typedef enum {
        In_Phase= 1,
        Out_of_Phase= -1
} quadrature;


/******************************************************************\
*       Super_Class::fft::Private
*       >>>>>>>>>>>>>>>>>>>>>>>>>
*
*       Uses: Audio Stream Array in the form of "double" Data-Type
*
*       Output: Array for fft Display in N-Channels
*
*
\******************************************************************/

/*Private FFT Definitions*/
#define ACTIVATED 0

/*Private FFT Data Structure*/
typedef struct complex {
        float real;
        float imag;
}_CompleX_;

/*Private FFT Data Structure Manipulation Routine*/
static float AbS(_CompleX_ Input){
        return(sqrt((Input.real*Input.real)+(Input.imag*Input.imag)));
}

/* Private FFT Function Procedure
 *
 * <JOHN> (June 13th 2000) um, yes this is currently only doing one channel (the left, or first)
 * <PASHA> (June 14th 2000) um, not any more ;)
 *
 */

float *fft(short *dataT, long array_length, float *response, int bands)
{

	long		Frequency;

	long		T_index=ACTIVATED;
	long		F_index=ACTIVATED;

	_CompleX_	First_complex_response;
	_CompleX_	Second_complex_response;

	long		increment;

	increment = (MAX_FREQUENCY - MIN_FREQUENCY) / bands;

	for ((Frequency=MIN_FREQUENCY);(Frequency<MAX_FREQUENCY);(Frequency+=increment)){
		for(T_index=START;T_index<array_length;){

			/*Left Channel Calculations*/
			First_complex_response.real += dataT[T_index]*(cos(w(Frequency)*T_index*Frequency/array_length));
			First_complex_response.imag += dataT[T_index]*(sin(w(Frequency)*T_index*Frequency/array_length));
			INCREMENT(T_index);

			/*Right Channel Calculations*/
			Second_complex_response.real += dataT[T_index]*(cos(w(Frequency)*T_index*Frequency/array_length));
			Second_complex_response.imag += dataT[T_index]*(sin(w(Frequency)*T_index*Frequency/array_length));
			INCREMENT(T_index);

			/*  Make sure that the length of the
			 *  time sample array is an even number!
			 */

		}

		/*  The Frequency Response will
		 *  Alternate between First and Second Channel...
		 *  Again, make sure the FFT array is an even
		 *  number and that its length is TWICE that
		 *  of each channel!
		 */

		/*Left Channel Output*/
		response[F_index]=log(AbS(First_complex_response));
		INCREMENT(F_index);

		/*Right Channel Output*/
		response[F_index]=log(AbS(Second_complex_response));
		INCREMENT(F_index);

		/*  I think at some point we will consider having
		 *  16 sub-bands per Band and take their average
		 *  to get the response at that Band! But the more
		 *  sub-bands we have the longer would be the
		 *  Iteration!
		 */
	}
	return (response);
}

