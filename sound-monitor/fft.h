/* GNOME sound-Monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 * fft code written by Mahmood Shakir Qasmi <qasmims@mcmaster.ca>
 *
 */


#ifndef FFT_H
#define FFT_H

float *fft(short *dataT, long array_length, float *response, int bands);

/*
        Inputs:
        ^^^^^^^
        dataT           Array Containing Samples of Sound Output
        sampling_time   Time Elapsed between Two Samples
        array_length    Total Number of Samples
	response	pointer to an allocated array floats to hold the data
	bands		number of bands to compute (response must be of adequate size)
*/


#endif
