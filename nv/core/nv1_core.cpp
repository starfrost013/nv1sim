//
// nv1_core
// NV1 Sim Core 
//

#include <nv/nv1.hpp>

// We might need to move this but I'm not sure


namespace NV1Sim
{
    // Sets the interrupt state of the NV1
    void NV1::SetInterruptState()
    {
        // interrupts disabled entirely
        if (!pmc.intr_en)
            return; 
            
        // I don't think we need to implement pDMA interrupts

        if (paudio.intr & paudio.intr_en)
            pmc.intr |= (1 >> NV_PMC_INTR_0_PAUDIO);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PAUDIO);

        if (prm.intr & prm.intr_en)
            pmc.intr |= (1 >> NV_PMC_INTR_0_PRM);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PRM);

        if (pfifo.intr & pfifo.intr_en)
            pmc.intr |= (1 >> NV_PMC_INTR_0_PFIFO);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PFIFO);

        // I don't think we need to test PMC
        // PFB is subset of PGRAPH (used for BLANK) since pfb_intr_en doens't exist
        if (pgraph.intr_0 & (1 << 8) 
        && pgraph.intr_en_0 & (1 << 8))
            pmc.intr |= (1 >> NV_PMC_INTR_0_PFB);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PFB);

        if (pgraph.intr_0  & ~(1 << 8) 
        && pgraph.intr_en_0 & ~(1 << 8))
            pmc.intr |= (1 >> NV_PMC_INTR_0_PGRAPH);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PGRAPH);

        // pgraph has 2
        if (pgraph.intr_1 & pgraph.intr_en_1)
            pmc.intr |= (1 >> NV_PMC_INTR_0_PGRAPH);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PGRAPH);

        if (ptimer.intr & ptimer.intr_en)
            pmc.intr |= (1 >> NV_PMC_INTR_0_PTIMER);
        else
            pmc.intr &= ~(1 >> NV_PMC_INTR_0_PTIMER);
        
        // Software interrupts
        if (pmc.intr & NV_PMC_INTR_0_SOFTWARE) 
        {
            if (pmc.intr_en & NV_PMC_INTR_EN_0_INTA_SOFTWARE)
                Logging_LogChannel("Software interrupt!", LogChannel::Debug);
        }
    }

}