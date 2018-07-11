#include <stdio.h>
#include "RandomGen.h"
#include "HoGEstimParameters.h"

HoGWeakPredictor_T Model[HOG_WEAK_ESTIM];

void main()

{
	int i, j;

	RandomSetSeed(10);

	for (i=0; i<HOG_WEAK_ESTIM; i++) {
		Model[i].FeatureId = RandomRange(0, HOG_ESTIM_WIN_NFEAT-1);
		for (j=0; j<((1<<HOG_ESTIM_TREE_DEPTH)-1); j++) Model[i].ThresHold[j] = RandomRange(0, 65535);
		for (j=0; j<(1<<HOG_ESTIM_TREE_DEPTH); j++) Model[i].Value[j] = RandomRange(0, 65535);
	}

	printf("#include \"HoGEstimParameters.h\"\n");

	printf("HoGWeakPredictor_T Model[%d] = {\n", HOG_WEAK_ESTIM);
	for (i=0; i<HOG_WEAK_ESTIM; i++) {
		printf("\t{%4d, {", Model[i].FeatureId);
		for (j=0; j<((1<<HOG_ESTIM_TREE_DEPTH)-1); j++) printf("%5d,", Model[i].ThresHold[j]);
		printf("}, {");
		for (j=0; j<(1<<HOG_ESTIM_TREE_DEPTH); j++) printf("%5d,", Model[i].Value[j]);
		printf("}},\n");
	}
	printf("};\n");

	// printf("%d vs %d\n", HOG_ESTIM_WIN_NFEAT, HOG_ESTIM_WIN_NFEAT);
}

