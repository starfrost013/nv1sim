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

namespace NV1Sim
{

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
            uint32_t boot;                  // Boot configuration register
            uint32_t intr;                  // Interrupt status
            uint32_t intr_en;               // Master Interrupt Enable
            uint32_t intr_read;             // Interrupt Read
            uint32_t enable;                // Master GPU Control

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
            uint32_t config; 
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

        // Move to private cpp file?
        inline __attribute__((always_inline)) uint32_t GetRAMINAddress(uint32_t addr)
        {
            addr ^= 4; 

            // This one is straight from envytools: https://envytools.readthedocs.io/en/latest/hw/memory/nv1-vram.html?highlight=ramin
            // Note: NV3 has 4 buffers, NV4 has 6!
            if ((pfb.config >> NV_PFB_CONFIG_0_SECOND_BUFFER))
            {
                addr = (addr & 0xFF) | ((addr >> 1) & ~0xFF);
                addr %= (settings.vram_amount >> 1);
                addr += (settings.vram_amount >> 1) * (addr >> 8) & 1;
            }
            else
                addr %= settings.vram_amount;

            return addr; 
        }

    public: 
        // NV1 Constructor
        NV1(GPUSettings new_settings) 
        { 
            settings = new_settings; 

            //temp - tracked alloc?
            state.video_ram32 = (uint32_t*)calloc(1, settings.vram_amount); // POD so its ok
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
    
        struct NV1Mapping
        {
            uint32_t* reg;

            uint32_t (NV1::*read_func)(uint32_t addr);
            void (NV1::*write_func)(uint32_t addr, uint32_t value);
        };

        // nearly every register is 32bit so we can get away with this 
        // we don't bother emulating the DAC, because the "DAC" is basically SDL
        std::unordered_map<uint32_t, NV1Mapping> mappings32 =
        {
            // PMC
            { NV_PMC_BOOT_0, { &this->pmc.boot, nullptr, nullptr } }, 
            { NV_PMC_INTR_0, { &this->pmc.intr, nullptr, nullptr } },
            { NV_PMC_INTR_EN_0, { &this->pmc.intr_en, nullptr, nullptr } }, 
            { NV_PMC_INTR_READ_0, { &this->pmc.intr_read, nullptr, nullptr } },
            { NV_PMC_ENABLE, { &this->pmc.enable, nullptr, nullptr } },

            // PFB
            { NV_PFB_CONFIG_0, { &this->pfb.config, nullptr, nullptr } }, 
        }; 

        uint32_t ReadRegister32(uint32_t addr) 
        { 
            if (mappings32[addr].read_func)
                return (this->*this->mappings32[addr].read_func)(addr);
            else
                return *mappings32[addr].reg; 
        };

        void WriteRegister32(uint32_t addr, uint32_t value)
        { 
            if (mappings32[addr].write_func)
                (this->*this->mappings32[addr].write_func)(addr, value);
            else 
                *mappings32[addr].reg = value; 
        };

        uint8_t ReadVRAM8(uint32_t addr) { return state.video_ram8[addr]; }; 
        uint16_t ReadVRAM16(uint32_t addr) { return state.video_ram16[addr >> 1]; }; 
        uint32_t ReadVRAM32(uint32_t addr) { return state.video_ram32[addr >> 2]; }; 
        void WriteVRAM8(uint32_t addr, uint32_t value) { state.video_ram8[addr] = value; }; 
        void WriteVRAM16(uint32_t addr, uint32_t value) { state.video_ram16[addr >> 1] = value; }; 
        void WriteVRAM32(uint32_t addr, uint32_t value) { state.video_ram32[addr >> 2] = value; }; 
        
        // RAMIN
        uint32_t ReadRAMIN32(uint32_t addr) { return state.video_ram32[GetRAMINAddress(addr) >> 2]; };
        void WriteRAMIN32(uint32_t addr, uint32_t value) { state.video_ram32[GetRAMINAddress(addr) >> 2] = value; };
    
    }; 
}