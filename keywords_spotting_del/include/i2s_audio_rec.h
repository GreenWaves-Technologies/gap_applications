/*
 * Copyright (c) 2017 GreenWaves Technologies SAS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of GreenWaves Technologies SAS nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __I2S_AUDIO_REC__
#define __I2S_AUDIO_REC_

#define WAV_HEADER_SIZE 44//in bytes
// 65408 16b samples @ 22.05kHz eq. to 2.99/2=1.495 sec of audio record in stereo
#define BUFFER_SIZE  ((16384*2)+WAV_HEADER_SIZE) //in bytes

typedef struct {
	unsigned int	l2Addr;
	unsigned int 	size;
	unsigned int	recReady;
	unsigned int	micReady;
  unsigned int  cStarted;
} I2sDescriptor;

extern RT_L2_DATA I2sDescriptor I2sOutHeader;
extern RT_L2_DATA unsigned char i2s_buffer_ch0[(BUFFER_SIZE * sizeof(short)) + WAV_HEADER_SIZE];

short int * sampling();
void printCfg(unsigned int i2s);
void CreateWAVHeader(unsigned char *I2sOut, I2sDescriptor *I2sOutHeader);

#endif
