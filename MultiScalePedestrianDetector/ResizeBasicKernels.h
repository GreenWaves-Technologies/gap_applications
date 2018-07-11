#ifndef __RESIZEBASICKERNELS_H__
#define __RESIZEBASICKERNELS_H__
#include "Gap8.h"
#include "StdTypes.h"

typedef struct {
        unsigned char * __restrict__ In;	/**< Pointer to input tile, horizontal */
        unsigned int Win;			/**< Input's tile width */
        unsigned char * __restrict__ Out;	/**< Pointer to output tile */
        unsigned int Wout;			/**< Width of output tile */
        unsigned int HTileOut;			/**< Height of output tile */
        unsigned int HStdTileOut;		/**< Standard Height of output tile, not the last one */
        unsigned int TileOutIndex;		/**< Output tile number, from 0 */
        unsigned int WStep;			/**< Fraction of output width consummed for 1 output column, Q16.16 format */
        unsigned int HStep;			/**< Fraction of output height consummed for 1 output line, Q16.16 format */
        unsigned int FirstLineIndex;		/**< Where is the base of the input tile in the associated input plane, expressed as a line index starting at 0 */
} KerResizeBilinear_ArgT;

void KerResizeBilinear(KerResizeBilinear_ArgT *Arg);

#endif
