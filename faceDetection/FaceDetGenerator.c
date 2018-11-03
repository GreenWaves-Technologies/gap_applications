/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "AutoTilerLib.h"

void LoadFaceDetectionLibrary()
{
	LibKernel("KerResizeBilinear", CALL_PARALLEL,
		CArgs(8,
			TCArg("unsigned char * __restrict__", "In"),
			TCArg("unsigned int", "Win"),
			TCArg("unsigned int", "Hin"),
			TCArg("unsigned char * __restrict__", "Out"),
			TCArg("unsigned int", "Wout"),
			TCArg("unsigned int", "Hout"),
			TCArg("unsigned int", "HTileOut"),
			TCArg("unsigned int", "FirstLineIndex")),
		"KerResizeBilinear_ArgT"
	);

	LibKernel("KerIntegralImagePrime", CALL_PARALLEL,
            CArgs(2,
            TCArg("unsigned int * __restrict__", "KerBuffer"),
			TCArg("unsigned int", "W")
		),
		"KerPrimeImage_ArgT"
	);
	LibKernel("KerIntegralImageProcess", CALL_PARALLEL,
               CArgs(5,
			TCArg("unsigned char * __restrict__", "In"),
			TCArg("unsigned int", "W"),
			TCArg("unsigned int", "H"),
			TCArg("unsigned int * __restrict__", "IntegralImage"),
			TCArg("unsigned int * __restrict__", "KerBuffer")

		),
		"KerProcessImage_ArgT"
	);


	LibKernel("KerSquaredIntegralImageProcess", CALL_PARALLEL,
               CArgs(5,
			TCArg("unsigned char * __restrict__", "In"),
			TCArg("unsigned int", "W"),
			TCArg("unsigned int", "H"),
			TCArg("unsigned int * __restrict__", "IntegralImage"),
			TCArg("unsigned int * __restrict__", "KerBuffer")

		),
		"KerProcessImage_ArgT"
	);

	LibKernel("KerEvaluateCascade", CALL_SEQUENTIAL,
            CArgs(8,
			TCArg("unsigned int * __restrict__", "IntegralImage"),
			TCArg("unsigned int * __restrict__", "SquaredIntegralImage"),
			TCArg("unsigned int", "W"),
			TCArg("unsigned int", "H"),
			TCArg("void *","cascade_model"),
			TCArg("unsigned char","WinW"),
			TCArg("unsigned char","WinH"),
			TCArg("int * __restrict__", "CascadeReponse")
		),
		"KerEvaluateCascade_ArgT"
	);
}


void GenerateResize(char *Name, int Wi, int Hi, int Wo, int Ho)
{
	UserKernel(Name,
		0,
		KernelIterationOrder(1, KER_TILE),
		TILE_HOR,
		CArgs(2, TCArg("unsigned char *", "In"), TCArg("unsigned char *", "Out")),
		Calls(1, Call("KerResizeBilinear", LOC_INNER_LOOP,
			Bindings(8, K_Arg("In", KER_ARG_TILE),
				        K_Arg("In", KER_ARG_W),
				        K_Arg("In", KER_ARG_H),
				        K_Arg("Out", KER_ARG_TILE),
				        K_Arg("Out", KER_ARG_W),
				        K_Arg("Out", KER_ARG_H),
				        K_Arg("Out", KER_ARG_TILE_H),
				        K_Arg("In", KER_ARG_TILE_BASE)))),
		KerArgs(2,
			KerArg("In",  OBJ_IN_DB,  Wi, Hi, sizeof(char), 1, OBJ_CONSTRAINTS_DYNAMIC, 0, "In", 0),
			KerArg("Out", OBJ_OUT_DB, Wo, Ho, sizeof(char), 0, OBJ_CONSTRAINTS_DYNAMIC, 0, "Out", 0)
		)
	);

}

void GenerateIntegralImage(char *Name,
		unsigned int W,			/* Image width */
		unsigned int H 		/* Image Height */
	)
{

	UserKernel(AppendNames("Process", Name),
		KernelDimensions(0, W, H, 0),
		KernelIterationOrder(1, KER_TILE),
		TILE_HOR,
		CArgs(2,
			TCArg("unsigned char *  __restrict__",  "ImageIn"),
			TCArg("unsigned int *  __restrict__", "IntegralImage")
		),
		Calls(2,
			Call("KerIntegralImagePrime", LOC_INNER_LOOP_PROLOG,
				Bindings(2,
					K_Arg("KerBuffer",KER_ARG),
					K_Arg("KerIn", KER_ARG_TILE_W)
				)
			),
			Call("KerIntegralImageProcess", LOC_INNER_LOOP,
				Bindings(5,
					K_Arg("KerIn", KER_ARG_TILE),
					K_Arg("KerIn", KER_ARG_TILE_W),
					K_Arg("KerIn", KER_ARG_TILE_H),
					K_Arg("KerOut",KER_ARG_TILE),
					K_Arg("KerBuffer",KER_ARG)
				)
			)
		),
			KerArgs(3,
                KerArg("KerIn",     OBJ_IN_DB,           W,  H, sizeof(char), 0, 0, 0, "ImageIn", 0),
                KerArg("KerBuffer", O_BUFF | O_NDB | O_NOUT | O_NIN | O_NTILED ,   W,  1, sizeof(int),  0, 0, 0, 0, 0),
                KerArg("KerOut",    OBJ_OUT_DB,          W,  H, sizeof(int),  0, 0, 0, "IntegralImage", 0)
            )
	);
}


void GenerateSquaredIntegralImage(char *Name,
		unsigned int W,			/* Image width */
		unsigned int H 		/* Image Height */
	)
{

	UserKernel(AppendNames("Process", Name),
		KernelDimensions(0, W, H, 0),
		KernelIterationOrder(1, KER_TILE),
		TILE_HOR,
		CArgs(2,
			TCArg("unsigned char *  __restrict__",  "ImageIn"),
			TCArg("unsigned int *  __restrict__", "IntegralImage")
		),
		Calls(2,
			Call("KerIntegralImagePrime", LOC_INNER_LOOP_PROLOG,
				Bindings(2,
					K_Arg("KerBuffer",KER_ARG),
					K_Arg("KerIn", KER_ARG_TILE_W)
				)
			),
			Call("KerSquaredIntegralImageProcess", LOC_INNER_LOOP,
				Bindings(5,
					K_Arg("KerIn", KER_ARG_TILE),
					K_Arg("KerIn", KER_ARG_TILE_W),
					K_Arg("KerIn", KER_ARG_TILE_H),
					K_Arg("KerOut",KER_ARG_TILE),
					K_Arg("KerBuffer",KER_ARG)
				)
			)
		),
			KerArgs(3,
                KerArg("KerIn",     OBJ_IN_DB,           W,  H, sizeof(char), 0, 0, 0, "ImageIn", 0),
                KerArg("KerBuffer", O_BUFF | O_NDB | O_NOUT | O_NIN | O_NTILED ,   W,  1, sizeof(int),  0, 0, 0, 0, 0),
                KerArg("KerOut",    OBJ_OUT_DB,          W,  H, sizeof(int),  0, 0, 0, "IntegralImage", 0)
            )
	);
}

//result = windows_cascade_classifier(ArgC->ImageIntegral,ArgC->SquaredImageIntegral,face_model,24,24,Wout,j,i);

void GenerateCascadeClassifier(char *Name,
		unsigned int W,     /* Image width */
		unsigned int H,     /* Image Height */
		unsigned int WinW,  /* Detection window width */
		unsigned int WinH   /* Detection window Height */
	)
{


	UserKernel(AppendNames("Process", Name),
		KernelDimensions(0, W, H, 0),
		KernelIterationOrder(1, KER_TILE),
		TILE_HOR,
		CArgs(4,
			TCArg("unsigned int *  __restrict__",  "IntegralImage"),
			TCArg("unsigned int *  __restrict__", "SquaredIntegralImage"),
			TCArg("void * ", "cascade_model"),
			TCArg("int  *  __restrict__", "CascadeReponse")
		),
		Calls(1,

			Call("KerEvaluateCascade", LOC_INNER_LOOP,
				Bindings(8,
					K_Arg("KerII", KER_ARG_TILE),
					K_Arg("KerIISQ", KER_ARG_TILE),
					K_Arg("KerII", KER_ARG_TILE_W),
					K_Arg("KerII", KER_ARG_TILE_H),
					C_Arg("cascade_model"),
					Imm(WinW),
					Imm(WinH),
					K_Arg("KerOut",KER_ARG_TILE)
				)
			)
		),
			KerArgs(3,
				KerArg("KerII",      OBJ_IN_DB,   W,  H, sizeof(unsigned int), WinH-1, 0, 0, "IntegralImage", 0),
				KerArg("KerIISQ",    OBJ_IN_DB,   W,  H, sizeof(unsigned int), WinH-1, 0, 0, "SquaredIntegralImage", 0),
				KerArg("KerOut",    OBJ_OUT_DB,   W-WinW+1,  H-WinH+1, sizeof(int),  0, 0, 0, "CascadeReponse", 0)
            )
	);
}


void FaceDetectionConfiguration(unsigned int L1Memory)

{
    SetInlineMode(ALWAYS_INLINE);
    //SetInlineMode(NEVER_INLINE);
	SetSymbolNames("FaceDet_L1_Memory", "FaceDet_L2_Memory", "FaceDet_KernelDescr", "FaceDet_KernelArgs");
    SetSymbolDynamics();
	SetKernelOpts(KER_OPT_NONE, KER_OPT_BUFFER_PROMOTE);

    SetUsedFilesNames(0, 1, "FaceDetBasicKernels.h");
    SetGeneratedFilesNames("FaceDetKernelsInit.c", "FaceDetKernelsInit.h", "FaceDetKernels.c", "FaceDetKernels.h");

    SetL1MemorySize(L1Memory);
}

