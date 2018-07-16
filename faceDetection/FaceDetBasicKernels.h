/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#ifndef __IIBASICKERNELS_H__
#define __IIBASICKERNELS_H__

#include "Gap8.h"
#include "StdTypes.h"
#include "cascade.h"

#define Max(a, b)               (((a)>(b))?(a):(b))
#define Min(a, b)               (((a)<(b))?(a):(b))

typedef struct {
	unsigned char * __restrict__ In;
	unsigned int Win;
	unsigned int Hin;
	unsigned char * __restrict__ Out;
	unsigned int Wout;
	unsigned int Hout;
	unsigned int HTileOut;
	unsigned int FirstLineIndex;
} KerResizeBilinear_ArgT;

void KerResizeBilinear(KerResizeBilinear_ArgT *KerArg);

typedef struct {
	Wordu32 * __restrict__ KerBuffer;
	Wordu32 W;
} KerPrimeImage_ArgT;

void KerIntegralImagePrime(KerPrimeImage_ArgT *KerArg);

typedef struct {
	Wordu8 * __restrict__ In;
	Wordu32 W;
	Wordu32 H;
	Wordu32 * __restrict__ IntegralImage;
	Wordu32 * __restrict__ KerBuffer;
} KerProcessImage_ArgT;

void KerIntegralImageProcess(KerProcessImage_ArgT *KerArg);


void KerSquaredIntegralImageProcess(KerProcessImage_ArgT *KerArg);

typedef struct {
	Wordu32 * __restrict__ IntegralImage;
	Wordu32 * __restrict__ SquaredIntegralImage;
	Wordu32 W;
	Wordu32 H;
	void * cascade_model;
	Wordu8 WinW;
	Wordu8 WinH;
	Wordu8 * __restrict__ CascadeReponse;
} KerEvaluateCascade_ArgT;



typedef struct{
    int stage_index;
    int* stage_sum;
    unsigned int *integralImage;
    int img_w;
    single_cascade_t *cascade_stage;
    int std;
    int off_x;
    int off_y;

} eval_weak_classifier_Arg_T;

void KerEvaluateCascade(
	Wordu32 * __restrict__ IntegralImage,
	Wordu32 * __restrict__ SquaredIntegralImage,
	Wordu32 W,
	Wordu32 H,
	void * cascade_model,
	Wordu8 WinW,
	Wordu8 WinH,
	Word32 * __restrict__ CascadeReponse);

#endif //__IIBASICKERNELS_H__