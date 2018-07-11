#include <stdint.h>
#include <stdio.h>
#include "AutoTilerLib.h"
#include "StdTypes.h"

void LoadResizeLibrary()

{
        LibKernel("KerResizeBilinear", CALL_PARALLEL,
                  CArgs(10,
			TCArg("unsigned char * __restrict__", "In"),
			TCArg("unsigned int", "Win"),
			TCArg("unsigned char * __restrict__", "Out"),
			TCArg("unsigned int", "Wout"),
			TCArg("unsigned int", "HTileOut"),
			TCArg("unsigned int", "HStdTileOut"),
			TCArg("unsigned int", "TileOutIndex"),
			TCArg("unsigned int", "WStep"),
			TCArg("unsigned int", "HStep"),
			TCArg("unsigned int", "FirstLineIndex")
		),
		"KerResizeBilinear_ArgT"
        );
}

void GenerateResize(char *Name, unsigned int Wi, unsigned int Hi, unsigned int Wo, unsigned int Ho)

{
        UserKernel(Name,
                0,
                KernelIterationOrder(1, KER_TILE),
                TILE_HOR,
                CArgs(4,
			TCArg("unsigned char *", "In"),
			TCArg("unsigned char *", "Out"),
			TCArg("unsigned int", "WStep"),
			TCArg("unsigned int", "HStep")
		),
                Calls(1, Call("KerResizeBilinear", LOC_INNER_LOOP,
                        	Bindings(10,
					K_Arg("In", KER_ARG_TILE),
					K_Arg("In", KER_ARG_W),
					K_Arg("Out", KER_ARG_TILE),
					K_Arg("Out", KER_ARG_W),
					K_Arg("Out", KER_ARG_TILE_H),
					K_Arg("Out", KER_ARG_TILE_H0),
					K_Arg("Out", KER_ARG_TILEINDEX),
					C_Arg("WStep"),
					C_Arg("HStep"),
					K_Arg("In", KER_ARG_TILE_BASE)
				)
			)
		),
                KerArgs(2,
                        KerArg("In",  OBJ_IN_DB,  Wi, Hi, sizeof(char), 1, OBJ_CONSTRAINTS_DYNAMIC, 0, "In", 0),
                        KerArg("Out", OBJ_OUT_DB, Wo, Ho, sizeof(char), 0, OBJ_CONSTRAINTS_DYNAMIC, 0, "Out", 0)
                )
        );
}

void LoadResizeDependencies()

{
        SetUsedFilesNames("KernelLibStdTypes.h", 1, "ResizeBasicKernels.h");
}

void ResizeConfiguration(unsigned int L1Memory)

{
        // SetInlineMode(ALWAYS_INLINE);
	SetInlineMode(NEVER_INLINE);
	SetSymbolNames("Resize_L1_Memory", "Resize_L2_Memory", "Resize_KernelDescr", "Resize_KernelArgs");
        SetSymbolDynamics();
	LoadResizeDependencies();
        SetGeneratedFilesNames("ResizeKernelsInit.c", "ResizeKernelsInit.h", "ResizeKernels.c", "ResizeKernels.h");
        SetL1MemorySize(L1Memory);
}
