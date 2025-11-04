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

#pragma once
#include <cassert>
#include <functional>
#include <nv1sim.hpp>
#include "nv1_regs.hpp"
#include <util/util.hpp>

namespace NV1Sim
{

    #define VRAM_AMOUNT_1MB         1048576
    #define VRAM_AMOUNT_2MB         2097152
    #define VRAM_AMOUNT_4MB         4194304

    // The settings of the GPU
    struct GPUSettings
    {
        uint32_t vram_amount; 
        uint32_t straps;
    }; 

    // The main class, where everything cool happens.
    class NV1
    {

    private: 
        typedef uint32_t (NV1::*GpuReadRegFn)(); 
        typedef void (NV1::*GpuWriteRegFn)(uint32_t value);

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

        // Real-Mode I/O
        struct PRM
        {
            uint32_t intr;                  // Interrupt status
            uint32_t intr_en;               // Master Interrupt Enable
            PRMC rmc;
        };

        // Cache for object storage
        // TODO: Consider separating pull0/1
        struct PFIFOCacheBase
        {
            uint32_t push_access_enable;
            uint32_t push_channel_id;
            uint32_t pull0;                         // bit8 - hw/sw device; bit4 - ramht hash generation success indicator; bit0-access enabled
            uint32_t pull1;                         // bit8 - object changed; bit4 - context clean/dity; bit2:0 - subchannel [0-7]
            uint32_t status;
            uint32_t get_address;
            uint32_t put_address;
            uint32_t context[NV_PFIFO_CACHE1_CTX__SIZE_1];


        };

        struct PFIFOCache0
        {
            PFIFOCacheBase cache_data;
        };

        struct PFIFOCache1
        {
            PFIFOCacheBase cache_data;

            // Get free spaces (cache1 only)
            uint32_t GetFreeSpaces()
            {
                // convert to slot number
                uint8_t binary_get_address = Util_Gray2Binary(cache_data.get_address) << 2;
                uint8_t binary_put_address = Util_Gray2Binary(cache_data.put_address) << 2;

                return (binary_get_address - binary_put_address - 4) & 0x7C; //GUARANTEED fifo depth
            }
        };

        // PFIFO cache entry
        struct PFIFOCacheEntry
        {
            uint32_t method;                        // Method
            uint32_t param;                         // Parameter for the object method
        };

        // I/O Architecture & Submission 
        struct PFIFO
        {
            uint32_t intr;                          // Interrupt status
            uint32_t intr_en;                       // Master Interrupt Enable
            uint32_t config;
            uint32_t cache_reassignment;            // Allow context switching?
            PFIFOCache0 cache0;
            PFIFOCacheEntry cache0_data;            // PFIFO (CACHE0 - software method injection) context
            PFIFOCache1 cache1;
            PFIFOCacheEntry cache1_data[NV_PFIFO_CACHE1_METHOD__SIZE_1];            // PFIFO (CACHE1 - general submission) Gray code context
        
            uint32_t runout_status;
            uint32_t runout_get_address;
            uint32_t runout_put_address;
        }; 

        // Framebuffer interface & control
        struct PFB
        {
            uint32_t boot;                  // Boot register
            uint32_t intr;                  // Interrupt status
            uint32_t intr_en;               // Master Interrupt Enable
            uint32_t config;                // Configuration 
        };

        // Bus Interface
        struct PBUS
        {

        }; 

        // 2D & 3D Rendering Engine ("BPORT" probably not needed)
        struct PGRAPH
        {
            uint32_t debug_0;
            uint32_t debug_1;
            uint32_t debug_2;
            uint32_t debug_3; 
            uint32_t intr_0;                // Interrupt status
            uint32_t intr_1;                // Interrupt status
            uint32_t intr_en_0;             // Master Interrupt Enable
            uint32_t intr_en_1;             // Master Interrupt Enable
            uint32_t ctx_switch;            // this is format of grobj pointed to in ramht
            uint32_t ctx_control;           // misc
            uint32_t misc;
            uint32_t status; 
            uint32_t trapped_addr;
            uint32_t trapped_data;
            uint32_t canvas_misc;
            uint32_t clip0_min;             // 31:16 - y, 15:0 - x
            uint32_t clip0_max;             // 31:16 - y, 15:0 - x
            uint32_t clip1_min;             // 31:16 - y, 15:0 - x
            uint32_t clip1_max;             // 31:16 - y, 15:0 - x
            uint32_t clip_misc;
            uint32_t notify;
            // do we need DMA register? in effect all dma registers are contiguous!

            // pattern shit (slightly renamed frrom original NV registers)
            uint32_t patt_0_rgb;
            uint32_t patt_0_a;
            uint32_t patt_1_rgb;
            uint32_t patt_1_a;
            uint32_t pattern_bitmap_high;   // 63:32 
            uint32_t pattern_bitmap_low;    // 31:0
            uint32_t pattern_shape;         // 0 - 8x8; 1 - 64x1; 2 - 1x64
            uint32_t mono_color0;           // colour expanded bitblit
            uint32_t mono_color1;           // colour expanded bitblit
            uint32_t rop3;                  // GDI ROP3
            uint32_t chroma_key;            // Colour Key for Operations
            uint32_t beta;                  // Beta factor for blending

            // XY logic (do we need this? this is most likely how PGRAPH plots pixels to PFB)
            uint32_t abs_x_ram[NV_PGRAPH_XY_LOGIC_RAM_SIZE]; // absolute
            uint32_t rel_x_ram[NV_PGRAPH_XY_LOGIC_RAM_SIZE]; // relative
            uint32_t x_ram[NV_PGRAPH_XY_LOGIC_RAM_SIZE];
            uint32_t rel_y_ram[NV_PGRAPH_XY_LOGIC_RAM_SIZE];
            uint32_t abs_y_ram[NV_PGRAPH_XY_LOGIC_RAM_SIZE];

            uint32_t xy_logic_misc0;
            uint32_t xy_logic_misc1;
            uint32_t x_misc;
            uint32_t y_misc;

            uint32_t abs_uclip_xmin;
            uint32_t abs_uclip_xmax;
            uint32_t abs_uclip_ymin;
            uint32_t abs_uclip_ymax;
            uint32_t rel_uclip_xmin;
            uint32_t rel_uclip_xmax;
            uint32_t rel_uclip_ymin;
            uint32_t rel_uclip_ymax;

            uint32_t abs_iclip_xmax;
            uint32_t abs_iclip_ymax;
            uint32_t rel_iclip_xmax;
            uint32_t rel_iclip_ymax;

            // Subdivision for quad patching
            uint32_t source_color;
            uint32_t subdivide; 
            uint32_t exceptions;
            uint32_t edgefill;

            uint32_t beta_factor_ram[NV_PGRAPH_BETA_RAM__SIZE_1];
            uint32_t bit33;         // overflow

        };

        // Audio engine
        struct PAUDIO
        {
            uint32_t intr;                  // Interrupt status
            uint32_t intr_en;               // Master Interrupt Enable
        };

        // DRM & Authentication Engine
        struct PAUTH
        {

        };

        // Programmable Interval Timer on-die
        struct PTIMER
        {
            uint32_t intr;                  // Interrupt status
            uint32_t intr_en;               // Master Interrupt Enable
            uint32_t numerator;
            uint32_t denominator;
        };

        // RAMIN config
        struct PRAM
        {
            uint32_t config;

            // locations of structures within ramin
            uint32_t ramht_start;
            uint32_t ramht_size;
            uint32_t ramro_start;
            uint32_t ramro_size;
            uint32_t ramfc_start;
            uint32_t ramfc_size;
            uint32_t ramau_start;
            uint32_t ramau_size;
            uint32_t rampw_start;
            uint32_t rampw_size;    
        };

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

        void StaticInit();
        // Core Private Methods
        void FirePendingInterrupts();

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

            StaticInit();

            Logging_LogChannel("NV1 init completed. Video RAM = %d MB", LogChannel::Message, settings.vram_amount >> 20);
        };

        PMC pmc;                                // Master control
        PRM prm;                                // Real-mode I/O
        PFIFO pfifo;                            // FIFO for object submission
        PFB pfb;                                // Framebuffer Interface
        PBUS pbus;                              // Bus
        PGRAPH pgraph;                          // 2D/3D Graphics
        PAUDIO paudio;                          // Audio Engine
        PAUTH pauth;                            // DRM
        PTIMER ptimer;                          // Programmable interval timer
        PRAM pram;
        uint32_t straps;                        // OEM Configuration

        GPUSettings settings;
        GPUState state;

        #define NV1_SINGLE_REGISTER             0xFFFFFFFF // means that this is a single register

        struct NV1Mapping
        {
            uint32_t* reg;


            uint32_t (NV1::*read_func)();
            void (NV1::*write_func)(uint32_t value);

            const char* description;
            uint32_t end;           // optional for multigpu

            // THESE MUST BE THE LAST ELEMENT DUE TO PROGRAMMING TERRORISM THAT WE DID!
            uint32_t real_ptr = 0xFFFFFFFF;
            uint32_t index = 0; 
        };

        // nearly every register is 32bit so we can get away with this 
        // we don't bother emulating the DAC, because the "DAC" is basically SDL
        std::unordered_map<uint32_t, NV1Mapping> mappings32 =
        {
            // PMC
            { NV_PMC_BOOT_0, { &this->pmc.boot, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } }, 
            { NV_PMC_INTR_0, { &this->pmc.intr, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } },
            { NV_PMC_INTR_EN_0, { &this->pmc.intr_en, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } }, 
            { NV_PMC_INTR_READ_0, { &this->pmc.intr_read, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } },
            { NV_PMC_ENABLE, { &this->pmc.enable, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } },

            // PFB
            { NV_PFB_BOOT_0, { &this->pfb.boot, nullptr, nullptr, "Framebuffer Manufacture-Time Configuration", NV1_SINGLE_REGISTER } },
            { NV_PFB_CONFIG_0, { &this->pfb.config, nullptr, nullptr, nullptr, NV1_SINGLE_REGISTER } }, 
        
            // PFIFO
            { NV_PFIFO_INTR_0, { &this->pfifo.intr, nullptr, nullptr, "PFIFO Interrupt Status", NV1_SINGLE_REGISTER } } ,
            { NV_PFIFO_INTR_EN_0, { &this->pfifo.intr_en, nullptr, nullptr, "PFIFO Interrupt Enable", NV1_SINGLE_REGISTER } } ,
            { NV_PFIFO_CONFIG_0, { &this->pfifo.config, nullptr, nullptr, "PFIFO General Config", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHES, { &this->pfifo.cache_reassignment, nullptr, nullptr, "PFIFO Cache Reassignment (Context Switching) Enable", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE0_PUSH0, { &this->pfifo.cache0.cache_data.push_access_enable, nullptr, nullptr, "PFIFO CACHE0 Push0 (Push Access Enabled)", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE1_PUSH0, { &this->pfifo.cache1.cache_data.push_access_enable, nullptr, nullptr, "PFIFO CACHE1 Push0 (Push Access Enabled)", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE0_PUSH1, { &this->pfifo.cache0.cache_data.push_channel_id, nullptr, nullptr, "PFIFO CACHE0 Push1 (Channel ID)", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE1_PUSH1, { &this->pfifo.cache1.cache_data.push_channel_id, nullptr, nullptr, "PFIFO CACHE1 Push0 (Channel ID)", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE0_PULL0, { &this->pfifo.cache0.cache_data.pull0, nullptr, nullptr, "PFIFO CACHE0 Pull Settings 0 (bit8 - Hardware or Software (object?) - bit4 set if hash failed; bit 0 - access enabled)", NV1_SINGLE_REGISTER }} ,
            { NV_PFIFO_CACHE1_PULL0, { &this->pfifo.cache1.cache_data.pull0, nullptr, nullptr, "PFIFO CACHE1 Pull Settings 0 (bit8 - Hardware or Software (method?) - bit4 set if hash failed; bit 0 - access enabled)", NV1_SINGLE_REGISTER }} ,
            { NV_PFIFO_CACHE0_PULL1, { &this->pfifo.cache0.cache_data.pull1, nullptr, nullptr, "PFIFO CACHE0 Pull Settings 1 (bit8 - Object Changed?; bit4 - 1 if context is dirty; bits 2-0: subchannel", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE1_PULL1, { &this->pfifo.cache1.cache_data.pull1, nullptr, nullptr, "PFIFO CACHE1 Pull Settings 1 (bit8 - Object Changed?; bit4 - 1 if context is dirty; bits 2-0: subchannel", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE0_STATUS, { &this->pfifo.cache0.cache_data.status, nullptr, nullptr, "PFIFO CACHE0 Status", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE1_STATUS, { &this->pfifo.cache1.cache_data.status, nullptr, nullptr, "PFIFO CACHE1 Status", NV1_SINGLE_REGISTER } },
            { NV_PFIFO_CACHE0_CTX(0), { &this->pfifo.cache0.cache_data.context[0], nullptr, nullptr, "PFIFO Cache0 Subchannel Context Registers", NV_PFIFO_CACHE0_CTX(NV_PFIFO_CACHE0_CTX__SIZE_1) } },

            // PRAM
            { NV_PRAM_CONFIG_0, { &this->pram.config, nullptr, &NV1::SetRAMINConfig, nullptr } }, 
            
            // PEXTDEV/STRAPS
            { NV_PEXTDEV_BOOT_0, { &this->straps, nullptr, nullptr, "Straps (OEM Configuration)"}  }, 
        }; 

        void Start()
        {
            state.running = true;
        }

        uint32_t ReadRegister32(uint32_t addr) 
        { 
            if (addr <= NV_USER_START)
            {
                // check the address exists
                // by default, operator[] creates an element, so we create every index implicitly (once)
                // this is probably faster than a manual search
                
                NV1Mapping mapping = mappings32[addr];
                bool isRedirect = (mapping.reg == nullptr);
                uint32_t index = 0;

                if (mapping.reg == nullptr
                && isRedirect)
                {
                    uint32_t start_addr = addr;

                    // mapping doesn't exist, find one that does
                    while (mapping.reg == nullptr
                    && addr >= 0)
                    {
                        addr -= 4;
                        mapping = mappings32[addr];
                    }

                    // now we are pointing to a real mapping
                    uint32_t real_ptr_new = addr;
                    uint32_t cur_index = (start_addr - addr) >> 2;                 // index *our* addr reprensets

                    for (uint32_t temp_addr = start_addr + 4; temp_addr > addr; temp_addr -= 4)
                    {
                        // index 0 not needed
                        mappings32[temp_addr].real_ptr = addr; 
                        mappings32[temp_addr].index = cur_index;
                        cur_index--;
                    }

                    //use access function
                }

                // get the real pointer
                if (mapping.real_ptr != 0xFFFFFFFF)
                {
                    //store index
                    index = mapping.index;
                    mapping = mappings32[mapping.real_ptr];
                }

                if (mappings32[addr].read_func)
                    return (this->*this->mappings32[addr].read_func)();
                else
                    return *mappings32[addr].reg; 
            }
            else
            {
                
                uint32_t channel_number = (addr & 0x7FFFFF);
                uint32_t subchannel_number = (addr >> 13) % 8; // number of subchannels

                // channel offset
                switch (addr & 0x1FFC)
                {
                    case NV_CHANNEL_OFFSET_FREE_COUNT_START ... NV_CHANNEL_OFFSET_FREE_COUNT_END:
                        return pfifo.cache1.GetFreeSpaces();
                }
            }
        };

        void WriteRegister32(uint32_t addr, uint32_t value)
        { 
            if (addr <= NV_USER_START)
            {
                if (mappings32[addr].write_func)
                    (this->*this->mappings32[addr].write_func)(value);
                else 
                    *mappings32[addr].reg = value; 
            }
            else
            {
                uint32_t channel_number = (addr & 0x7FFFFF);
                uint32_t subchannel_number = (addr >> 13) % 8; // number of subchannels

                // channel offset
                switch (addr & 0x1FFC)
                {
                    
                }
            }
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
    
        // Register stuff
        void SetRAMINConfig(uint32_t value);

        void PFIFOCache0Push();
        void PFIFOCache0Pull();
        void PFIFOCache1Push();
        void PFIFOCache1Pull();
    }; 
}