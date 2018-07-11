#include "StdTypes.h"
#ifndef __RESIZE_GENERATOR_H__
#define __RESIZE_GENERATOR_H__

/** @brief Loads basic kernels used by resize generator */
void LoadResizeLibrary();

/** @brief Generates a user kernel named Name performing resize from input [Wi x Hi] into output [Wo x Ho], uses dynamically computed memory transfers */
void GenerateResize(
	char *Name,		/**< Name of the resize function to be generated */
	unsigned int Wi,	/**< Input width */
	unsigned int Hi,	/**< Input height */
	unsigned int Wo,	/**< Output width */
	unsigned int Ho		/**< Output height */
	);

/** @brief Add basic kernels dependencies for resize generator, should be used when the generator is called from another generator */
void LoadResizeDependencies();

/** @brief Setup configuration for resize generator, should be used when the generator is used alone */
void ResizeConfiguration(
	unsigned int L1Memory	/**< Usable memory in shared L1 */
	);

#endif
