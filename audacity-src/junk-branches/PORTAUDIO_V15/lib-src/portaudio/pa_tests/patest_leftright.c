/*
 * patest_leftright.c
 * Play different tone sine waves that alternate between left and right channel.
 *
 * Authors:
 *    Ross Bencina <rossb@audiomulch.com>
 *    Phil Burk <philburk@softsynth.com>
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.audiomulch.com/portaudio/
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#define NUM_SECONDS   (8)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (512)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif
#define TABLE_SIZE   (200)
typedef struct {
	float sine[TABLE_SIZE];
	int left_phase;
	int right_phase;
	int toggle;
	int countDown;
}paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(   void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             PaTimestamp outTime, void *userData )
{
	paTestData *data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	int i;
	int framesToCalc;
	int finished = 0;
	(void) outTime; /* Prevent unused variable warnings. */
	(void) inputBuffer;
	for( i=0; i<framesPerBuffer; i++ )
	{
		if( data->toggle )
		{
			*out++ = data->sine[data->left_phase];		/* left */
			*out++ = 0;		/* right */
		}
		else
		{
			*out++ = 0;		/* left */
			*out++ = data->sine[data->right_phase];		/* right */
		}
		
		data->left_phase += 1;
		if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
		data->right_phase += 3; /* higher pitch so we can distinguish left and right. */
		if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
	}
	
	if( data->countDown < 0 )
	{
		data->countDown = SAMPLE_RATE;
		data->toggle = !data->toggle;
	}
	data->countDown -= framesPerBuffer;
	
	return finished;
}
/*******************************************************************/
int main(void);
int main(void)
{
	PortAudioStream *stream;
	PaError err;
	paTestData data;
	int i;
	int timeout;
	printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);
	/* initialise sinusoidal wavetable */
	for( i=0; i<TABLE_SIZE; i++ )
	{
		data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
	}
	data.left_phase = data.right_phase = 0;
	data.countDown = SAMPLE_RATE;
	
	err = Pa_Initialize();
	if( err != paNoError ) goto error;
	err = Pa_OpenStream(
				&stream,
				paNoDevice,/* default input device */
				0,              /* no input */
				paFloat32,		/* 32 bit floating point input */
				NULL,
				Pa_GetDefaultOutputDeviceID(), /* default output device */
				2,          /* stereo output */
				paFloat32,      /* 32 bit floating point output */
				NULL,
				SAMPLE_RATE,
				FRAMES_PER_BUFFER,            /* frames per buffer */
				0,              /* number of buffers, if zero then use default minimum */
				paClipOff,      /* we won't output out of range samples so don't bother clipping them */
				patestCallback,
				&data );
	if( err != paNoError ) goto error;
	err = Pa_StartStream( stream );
	if( err != paNoError ) goto error;
	printf("Play for several seconds.\n");
	timeout = NUM_SECONDS * 4;
	while( timeout > 0 )
	{
		printf("Countdown = %d, Toggle = %d\n", data.countDown, data.toggle );
		fflush( stdout );
		Pa_Sleep( 250 );
		timeout -= 1;
	}
	
	err = Pa_StopStream( stream );
	if( err != paNoError ) goto error;
	err = Pa_CloseStream( stream );
	if( err != paNoError ) goto error;
	Pa_Terminate();
	printf("Test finished.\n");
	return err;
error:
	Pa_Terminate();
	fprintf( stderr, "An error occured while using the portaudio stream\n" ); 
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return err;
}
