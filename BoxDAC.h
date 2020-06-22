#ifndef BoxDAC_h
#define BoxDAC_h

#include "BaseHeader.h"
#include <EnhancedThread.h>

class BoxDAC : public EnhancedThread  { 
    public:
        void
            begin(),
            loop();

        void beep();
    
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
            CODEC_IF_CTRL1 = 0x1B,
            INT1_CTRL_REG = 0x30,
            GPIO1_INOUT_CTRL = 0x33,
            DAC_PROC_BLOCK_SEL = 0x3C,
            DAC_DATA_PATH_SETUP = 0x3F,
            DAC_VOL_CTRL = 0x40,
            DAC_VOL_L_CTRL = 0x41,
            DAC_VOL_R_CTRL = 0x42,
            HEADSET_DETECT = 0x43,
            BEEP_L_GEN = 0x47,
            BEEP_R_GEN = 0x48,
            BEEP_LEN_MSB = 0x49,
            BEEP_LEN_MID = 0x4A,
            BEEP_LEN_LSB = 0x4B,
            BEEP_SIN_MSB = 0x4C,
            BEEP_SIN_LSB = 0x4D,
            BEEP_COS_MSB = 0x4E,
            BEEP_COS_LSB = 0x4F,
            VOL_MICDET_SAR_ADC = 0x74,
        };
        enum class ADDR_P1_DAC_OUT {
            HP_DRIVERS = 0x1F,
            SPK_AMP = 0x20,
            HP_OUT_POP_REM_SET = 0x21,
            OUT_PGA_RAMP_DOWN_PER_CTRL = 0x22,
            DAC_LR_OUT_MIX_ROUTING = 0x23,
            L_VOL_TO_HPL = 0x24,
            R_VOL_TO_HPR = 0x25,
            L_VOL_TO_SPK = 0x26,
            HPL_DRIVER = 0x28,
            HPR_DRIVER = 0x29,
            SPK_DRIVER = 0x2A,
            HP_DRIVER_CTRL = 0x2C,
            MICBIAS = 0x2E,
        };
        enum class ADDR_P3_MCLK {
            TIMER_CLK_MCLK_DIV = 0x10,
        };

        bool
            send_raw(uint8_t data),
            send(uint8_t target_register, uint8_t data),
            send(ADDR target_register, PAGE data),
            send(ADDR_P0_SERIAL target_register, uint8_t data),
            send(ADDR_P1_DAC_OUT target_register, uint8_t data),
            send(ADDR_P3_MCLK target_register, uint8_t data);

};

#endif