#include "ResizeBasicKernels.h"

#define Min(a, b)               (((a)<(b))?(a):(b))

static int CoreCountDynamic = 1;
static int ActiveCore = gap8_ncore();

static inline unsigned int __attribute__((always_inline)) ChunkSize(unsigned int X)

{
        unsigned int NCore;
        unsigned int Log2Core;
        unsigned int Chunk;

        if (CoreCountDynamic) NCore = ActiveCore; else NCore = gap8_ncore();
        Log2Core = gap8_fl1(NCore);
        Chunk = (X>>Log2Core) + ((X&(NCore-1))!=0);
        return Chunk;
}

void KerResizeBilinear(KerResizeBilinear_ArgT *Arg)

{
	unsigned char * __restrict__ In  = Arg->In;
	unsigned int Win		 = Arg->Win;
	unsigned char * __restrict__ Out = Arg->Out;
	unsigned int Wout		= Arg->Wout;
	unsigned int HTileOut	    = Arg->HTileOut;
	unsigned int HStdTileOut	 = Arg->HStdTileOut;
	unsigned int TileOutIndex	= Arg->TileOutIndex;
	unsigned int WStep	       = Arg->WStep;
	unsigned int HStep	       = Arg->HStep;
	unsigned int FirstLineIndex      = Arg->FirstLineIndex;

	unsigned int CoreId = gap8_coreid();
	unsigned int ChunkCell = ChunkSize(Wout);
	unsigned int First = CoreId*ChunkCell, Last  = Min(Wout, First+ChunkCell);

	unsigned int x, y;
	unsigned int hCoeff = HStdTileOut*TileOutIndex*HStep;

	for (y = 0 ; y < HTileOut ; y++) {
		unsigned int offsetY = (hCoeff >> 16) - FirstLineIndex;
		unsigned int hc2 = (hCoeff >> 9) & 127;
		unsigned int hc1 = 128 - hc2;
		// unsigned int wCoeff = 0;
		unsigned int wCoeff = First*WStep;

		// for (x = 0 ; x < Wout ; x++) {
		for (x = First ; x < Last ; x++) {
			unsigned int offsetX = (wCoeff >> 16);
			unsigned int wc2 = (wCoeff >> 9) & 127;
			unsigned int wc1 = 128 - wc2;
			unsigned int P1 = In[offsetY*Win       + offsetX    ];
			unsigned int P2 = In[(offsetY + 1)*Win + offsetX    ];
			unsigned int P3 = In[offsetY*Win       + offsetX + 1];
			unsigned int P4 = In[(offsetY + 1)*Win + offsetX + 1];

			Out[y*Wout + x] = ((P1*hc1 + P2*hc2)*wc1 + (P3*hc1 + P4*hc2)*wc2) >> 14;
			wCoeff += WStep;
		}
		hCoeff += HStep;
	}
	gap8_waitbarrier(0);
}

void KerResizeBilinearVect(KerResizeBilinear_ArgT *Arg)

{
	unsigned char * __restrict__ In  = Arg->In;
	unsigned int Win		 = Arg->Win;
	unsigned char * __restrict__ Out = Arg->Out;
	unsigned int Wout		= Arg->Wout;
	unsigned int HTileOut	    = Arg->HTileOut;
	unsigned int HStdTileOut	 = Arg->HStdTileOut;
	unsigned int TileOutIndex	= Arg->TileOutIndex;
	unsigned int WStep	       = Arg->WStep;
	unsigned int HStep	       = Arg->HStep;
	unsigned int FirstLineIndex      = Arg->FirstLineIndex;

	unsigned int CoreId = gap8_coreid();
	unsigned int ChunkCell = ChunkSize(Wout);
	unsigned int First = CoreId*ChunkCell, Last  = Min(Wout, First+ChunkCell);

	unsigned int x, y;
	unsigned int hCoeff = HStdTileOut*TileOutIndex*HStep;

	for (y = 0 ; y < HTileOut ; y++) {
		unsigned int offsetY = (hCoeff >> 16) - FirstLineIndex;
		unsigned int hc2 = (hCoeff >> 9) & 127;
		unsigned int hc1 = 128 - hc2;
		v4u Vhc = (v4u) gap8_pack4(hc1, hc2, 0, 0);
		// unsigned int wCoeff = 0;
		unsigned int wCoeff = First*WStep;

		// for (x = 0 ; x < Wout ; x++) {
		for (x = First ; x < Last ; x++) {
			unsigned int offsetX = (wCoeff >> 16);
			unsigned int wc2 = (wCoeff >> 9) & 127;
			unsigned int wc1 = 128 - wc2;

			// unsigned int P1 = In[offsetY*Win       + offsetX    ];
			// unsigned int P2 = In[(offsetY + 1)*Win + offsetX    ];
			// unsigned int P3 = In[offsetY*Win       + offsetX + 1];
			// unsigned int P4 = In[(offsetY + 1)*Win + offsetX + 1];

			v4u VP1 = *(v4u *)(& In[offsetY*Win       + offsetX]); 		// {P1, P3, x, x}
			v4u VP2 = *(v4u *)(& In[(offsetY + 1)*Win + offsetX]); 		// {P2, P4, x, x}
			v4u VP11 = __builtin_shuffle(VP1, VP2, (v4u) {0, 4, 0, 0});	// {P1, P2, x, x}
			v4u VP21 = __builtin_shuffle(VP1, VP2, (v4u) {1, 5, 0, 0});	// {P3, P4, x, x}

			Out[y*Wout + x] = (gap8_dotpu4(VP11, Vhc)*wc1 + gap8_dotpu4(VP21, Vhc)*wc2) >> 14;
			// Out[y*Wout + x] = ((P1*hc1 + P2*hc2)*wc1 + (P3*hc1 + P4*hc2)*wc2) >> 14;
			wCoeff += WStep;
		}
		hCoeff += HStep;
	}
	gap8_waitbarrier(0);
}
