#include "Cluster_FLL_optimal_settings.h"

void __attribute__ ((noinline)) SetFllConfig2(hal_fll_e WhichFll, unsigned int Assert, unsigned int Deassert, unsigned int Tolerance, unsigned int Gain, unsigned int Dithering, unsigned int OpenLoop)
{
        FllConfigT Config;

        Config.Raw = GetFllConfiguration(WhichFll, FLL_CONFIG2);
        Config.ConfigReg2.AssertCycles = Assert;
        Config.ConfigReg2.DeAssertCycles = Deassert;
        Config.ConfigReg2.LockTolerance = Tolerance;
        Config.ConfigReg2.LoopGain = Gain;
        Config.ConfigReg2.Dithering = Dithering;
        Config.ConfigReg2.OpenLoop = OpenLoop;
        SetFllConfiguration(WhichFll, FLL_CONFIG2, (unsigned int) Config.Raw);
}
