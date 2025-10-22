// 
// The NV1 emulator (The real one!)
// This is a 2D, 3D, audio, and DRM chip released by Nvidia, in collaboration with STMicroelectronics (then SGS-Thomson) in 1995.
// It failed in the market due to its quadratic texturing method not being compatible with Direct3D, high price and lack of SB16 compatibility,
// but the I/O architecutre was reused almost completely for the NV3 (RIVA 128), which sold 6 million chips, and later chips.
//
// Some notes:
// DMA is not emulated except when necessary. This is actually because the interface to the NV1 does not use DMA. The DMA method was introduced in NV3,
// then redesigned in NV4 (Riva TNT) to use pushbuffers, which survived until GeForce 8/G80, aka the start of CUDA.
// The classes here are not identical to those in the drivers until NV4.
// The classes provided by the public interface to the GPU that NV-direct apps use is provided by a NVidia Resource Manager running under Windows 3.1 or 95.
// Everything is 32-bit. All I/O, except to VGA, is 32-bit.
//


#include <nv1sim.hpp>
#include "nv1_regs.hpp"

#define VRAM_AMOUNT_1MB         1048576
#define VRAM_AMOUNT_2MB         2097152
#define VRAM_AMOUNT_4MB         4194304

// The main class, where everything cool happens.
class NV1
{

private: 


    // The settings of the GPU
    struct GPUSettings
    {
        uint32_t vram_amount; 
        uint32_t straps;
    }; 

    // The state of the NV1
    struct GPUState
    {
        bool running;                   // Is our GPU running?
        bool in_reset;                  // in reset
        uint32_t* video_ram32;          // Video RAM (32-bit addressing)
        uint16_t* video_ram16;          // Video RAM (16-bit addressing)
        uint8_t* video_ram8;            // Video RAM (8-bit addressing)
    };

    // Master Control 
    struct PMC
    {
        uint32_t enable;                // Master GPU Control
        uint32_t intr;                  // Interrupt status
        uint32_t intr_en;               // Master Interrupt Enable
    };

    // Real-Mode Communication
    struct PRMC
    {

    };

    // I/O Architecture & Submission 
    struct PFIFO
    {

    }; 

    // Framebuffer interface & control
    struct PFB
    {

    };

    // Bus Interface
    struct PBUS
    {

    }; 

    // 2D & 3D Rendering Engine 
    struct PGRAPH
    {

    };

    // Audio engine
    struct PAUDIO
    {

    };

    // DRM & Authentication Engine
    struct PAUTH
    {

    };

    GPUSettings settings;
    GPUState state;


public: 
    // NV1 Constructor
    NV1(GPUSettings new_settings) 
    { 
        settings = new_settings; 

        //temp - tracked alloc?
        state.video_ram32 = (uint32_t*)calloc(1, settings.vram_amount);
        state.video_ram16 = (uint16_t*)state.video_ram32;
        state.video_ram8 = (uint8_t*)state.video_ram32;
        
        state.running = false;
    };

    PMC pmc;
    PRMC prmc; 
    PFIFO pfifo;
    PFB pfb;
    PBUS pbus;
    PGRAPH pgraph;
    PAUDIO paudio;
    PAUTH pauth;
}; 