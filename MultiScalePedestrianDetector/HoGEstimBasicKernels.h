#ifndef __HOGESTIMBASICKERNELS_H__
#define __HOGESTIMBASICKERNELS_H__
#include "Gap8.h"
#include "StdTypes.h"
#include "HoGEstimParameters.h"

typedef struct {
	Wordu16 * __restrict__ In;
	Wordu32 H;
	Wordu32 FeatureSize;
	Wordu32 EstimWidth;
	Wordu16 ** __restrict__ HoGFeatCols;
	Wordu32 HoGFeatColIndex;
} KerReadHoGFeatCol_ArgT;
void KerReadHoGFeatCol(KerReadHoGFeatCol_ArgT *Arg);

typedef struct {
	Wordu16 ** __restrict__ HoGFeatCols;
	Wordu32 FeatureSize;
	Wordu32 WEstimator;
	Wordu32 HEstimator;
	Wordu32 HFeatCols;
	Wordu32 * __restrict__ Out;
} KerEstimate_ArgT;
void KerEstimate(KerEstimate_ArgT *Arg);

void KerEstimateWin(
        Wordu16 ** __restrict__ HoGFeatCols,
        Wordu32 FeatureSize,
        Wordu32 WEstimator,
        Wordu32 HEstimator,
        Wordu32 HFeatCols,
        Wordu32 * __restrict__ Out,
        Wordu16 * __restrict__ Buffer);

void KerWeakEstimateWin(
        Wordu16 ** __restrict__ HoGFeatCols,
        Wordu32 FeatureSize,
        Wordu32 WEstimator,
        Wordu32 HEstimator,
        Wordu32 HFeatCols,
        Wordu8 * __restrict__ Out,
        Wordu16 * __restrict__ Buffer,
        HoGWeakPredictor_T * __restrict__ Model,
        Wordu32 ModelSize);

typedef struct {
        Wordu16 ** __restrict__ HoGFeatCols;
	Wordu32 ColumnIndexM1;
        Wordu32 HEstimator;
        Wordu32 HFeatCols;
        Wordu32 FeatureSize;
        HoGWeakPredictorBis_T * __restrict__ Model;
        Wordu32 ModelSize;
} KerWeakEstimate_ArgT;
void KerWeakEstimateAllWindows(KerWeakEstimate_ArgT *Arg);

void InstallModel(
	HoGWeakPredictor_T *From,
	HoGWeakPredictor_T *To,
	unsigned int N);

void InstallModelBis(
	HoGWeakPredictorBis_T *From,
	HoGWeakPredictorBis_T *To,
	unsigned int N);
#endif
