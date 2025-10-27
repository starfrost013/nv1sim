//
// NV1Sim - The Nvidia NV1 Multimedia Accelerator Simulator
// Copyright © 2025 starfrost
//
// nv1_class.hpp: Base for NV1 CLASS METHODS implementation
//

#pragma once
#include <nv/nv1.hpp>

namespace NV1Sim
{
    class NV1UBase
    {
    public: 
        struct NV1MethodMapping
        {
            // Methods are write-only
            void (NV1UBase::*function)();
            const char* name;
            uint32_t start;
            uint32_t end;
        };

        NV1UBase(NV1* gpuref)
        {
            gpu = gpuref; 
        }  
    
        virtual NV1MethodMapping GetMapping(uint32_t addr)
        {
            return baseMappings[addr]; 
        };
    private: 
        NV1* gpu;


        // Methods below 0x0100 are here
        std::unordered_map<uint32_t, NV1MethodMapping> baseMappings;


    };
}