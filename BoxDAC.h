#ifndef BoxDAC_h
#define BoxDAC_h

#include "BaseHeader.h"
<<<<<<< HEAD
#include <EnhancedThread.h>

class BoxDAC : public EnhancedThread  { 
=======

class BoxDAC { 
>>>>>>> c54e8152d4ae4286ec45087a1fdc1b9715faffa6
    public:
        void
            begin(),
            loop();
<<<<<<< HEAD
    
    private:
        enum class PAGE {
            SERIAL_IO = 0x00,
            DAC_OUT_VOL = 0x01,
            MCLK_DIVIDER = 0x03,
            DAC_FILTER_DRC_COE_1A = 0x08,
            DAC_FILTER_DRC_COE_2A = 0x09,
            DAC_FILTER_DRC_COE_1B = 0x0C,
            DAC_FILTER_DRC_COE_2B = 0x0D,
        };
        enum class ADDR {
            PAGE_CONTROL = 0x00,
        };
        enum class ADDR_P0_SERIAL {
            SOFTWARE_RESET = 0x01,
            CLOCKGEN_MUX = 0x04,
            PLL_P_R_VAL = 0x05,
            PLL_J_VAL = 0x06,
            PLL_D_VAL_MSB = 0x07,
            PLL_D_VAL_LSB = 0x08,
            DAC_NDAC_VAL = 0x0B,
            DAC_MDAC_VAL = 0x0C,
            DAC_DOSR_VAL_MSB = 0x0D,
            DAC_DOSR_VAL_LSB = 0x0E,
            CODEC_INT_CTRL1 = 0x1B,
            DAC_PROC_BLOCK_SEL = 0x3C,
            DAC_VOL_CTRL = 0x40,
            BEEP_L_GEN = 0x47,
            BEEP_R_GEN = 0x48,
        };
        enum class ADDR_P1_DAC_OUT {
            HP_OUT_POP_REM_SET = 0x21,
            OUT_PGA_RAMP_DOWN_PER_CTRL = 0x22,
            DAC_LR_OUT_MIX_ROUTING = 0x23,
            HP_CTRL = 0x2C,
            MICBIAS = 0x2E,
        };
        enum class ADDR_P3_MCLK {
            TIMER_CLK_MCLK_DIV = 0x10,
        };

        void send(uint8_t target_register, uint8_t data);
        void send(ADDR target_register, PAGE data);
        void send(ADDR_P0_SERIAL target_register, uint8_t data);
        void send(ADDR_P1_DAC_OUT target_register, uint8_t data);
        void send(ADDR_P3_MCLK target_register, uint8_t data);
};
=======
    //private:

}
>>>>>>> c54e8152d4ae4286ec45087a1fdc1b9715faffa6

#endif