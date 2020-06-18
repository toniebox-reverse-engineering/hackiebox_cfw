#include "BoxDAC.h"
#include <Wire.h>

#include "driverlib/prcm.h"
#include "driverlib/i2s.h"

void BoxDAC::begin() { 
    Log.info("Initialize DAC...");

    MAP_PinTypeI2S(PIN_50, PIN_MODE_4); //I2S Data0 (DIN)
    MAP_PinTypeI2S(PIN_53, PIN_MODE_2); //I2S ClockO (BCLK)
    MAP_PinTypeI2S(PIN_63, PIN_MODE_7); //I2S Frame Sync (WCLK)

    MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_I2S);
    MAP_PRCMI2SClockFreqSet(512000); //512000 = 16*2*16000Khz(Num of bytes * STEREO * 16000 sampling)
    MAP_I2SConfigSetExpClk(I2S_BASE,512000,512000,I2S_SLOT_SIZE_16|I2S_PORT_DMA);
    //MAP_I2SConfigSetExpClk(I2S_BASE,512000,512000,I2S_SLOT_SIZE_16|I2S_PORT_CPU);
    MAP_I2SSerializerConfig(I2S_BASE,I2S_DATA_LINE_1,I2S_SER_MODE_RX, I2S_INACT_LOW_LEVEL);
    MAP_I2SSerializerConfig(I2S_BASE,I2S_DATA_LINE_0,I2S_SER_MODE_TX, I2S_INACT_LOW_LEVEL);
    MAP_I2SEnable(I2S_BASE, I2S_MODE_TX_ONLY);

    //RESET
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    delay(10);
    digitalWrite(62, LOW);
    delay(10);
    digitalWrite(62, HIGH);
    delay(10);

    Wire.begin();


    //Extracted from logic analyzer capture of box
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::SOFTWARE_RESET, 0x01); //Self-clearing software reset for control register
    delay(20);
    send(ADDR_P0_SERIAL::CLOCKGEN_MUX, 0x07);
    send(ADDR_P0_SERIAL::PLL_J_VAL, 0x20);
    send(ADDR_P0_SERIAL::PLL_D_VAL_MSB, 0x00);
    send(ADDR_P0_SERIAL::PLL_D_VAL_LSB, 0x00);
    send(ADDR_P0_SERIAL::PLL_P_R_VAL, 0x96);
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);
    send(ADDR_P0_SERIAL::DAC_MDAC_VAL, 0x86);
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_MSB, 0x01);
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_LSB, 0x00);
    send(ADDR_P0_SERIAL::CODEC_INT_CTRL1, 0x00);
    send(ADDR_P0_SERIAL::DAC_PROC_BLOCK_SEL, 0x19);

    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::HP_OUT_POP_REM_SET, 0x4E);
    send(ADDR_P1_DAC_OUT::OUT_PGA_RAMP_DOWN_PER_CTRL, 0x70);
    send(ADDR_P1_DAC_OUT::DAC_LR_OUT_MIX_ROUTING, 0x44);
    send(ADDR_P1_DAC_OUT::MICBIAS, 0x0B);
    send(ADDR_P1_DAC_OUT::HP_DRIVER_CTRL, 0xE0);

    send(ADDR::PAGE_CONTROL, PAGE::MCLK_DIVIDER);
    send(ADDR_P3_MCLK::TIMER_CLK_MCLK_DIV, 0x01);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::HEADSET_DETECT, 0x8C);
    send(ADDR_P0_SERIAL::INT1_CTRL_REG, 0x80);
    send(ADDR_P0_SERIAL::GPIO1_INOUT_CTRL, 0x14);
    //send(0x2E); Excel 161
    //read 0x18 addr

    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x7F);
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x7F);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 0x7F);
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x00);
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x00);
    send(ADDR_P1_DAC_OUT::SPK_DRIVER, 0x00);
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0x00);
    send(ADDR_P1_DAC_OUT::SPK_AMP, 0x00);
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0xC2);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x92);
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x92);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_DATA_PATH_SETUP, 0xD5);
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, 0xDC);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, 0xDC);
    //Excel 219
    // Extract END


    //Testing custom code, recommened Initialization code by chip doc
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::SOFTWARE_RESET, 0x01); //Self-clearing software reset for control register
    send(ADDR_P0_SERIAL::CLOCKGEN_MUX, 0x03);
    send(ADDR_P0_SERIAL::PLL_J_VAL, 0x08);
    send(ADDR_P0_SERIAL::PLL_D_VAL_MSB, 0x00); //30 07 00 00
    send(ADDR_P0_SERIAL::PLL_D_VAL_LSB, 0x00);
    send(ADDR_P0_SERIAL::PLL_P_R_VAL, 0x91);
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x88);
    send(ADDR_P0_SERIAL::DAC_MDAC_VAL, 0x82);
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_MSB, 0x00); //30 0D 00 80
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_LSB, 0x80); 

    send(ADDR_P0_SERIAL::CODEC_INT_CTRL1, 0x00);
    send(ADDR_P0_SERIAL::DAC_PROC_BLOCK_SEL, 0x0B);
    send(0x00, 0x08);//TODO!
    send(0x01, 0x04);//TODO!

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::VOL_MICDET_SAR_ADC, 0x00);

    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0x04);
    send(ADDR_P1_DAC_OUT::HP_OUT_POP_REM_SET, 0x4E);
    send(ADDR_P1_DAC_OUT::DAC_LR_OUT_MIX_ROUTING, 0x44);
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::SPK_DRIVER, 0x1C);
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0xC2);
    send(ADDR_P1_DAC_OUT::SPK_AMP, 0x86);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x92);
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x92);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 0x92);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_DATA_PATH_SETUP, 0xD4);
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, 0xD4);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, 0xD4);
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00);


    
    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x06);
    send(ADDR_P1_DAC_OUT::SPK_DRIVER, 0x1C);
    send(ADDR_P1_DAC_OUT::SPK_AMP, 0x86);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x92);
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x92);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 0x92);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_DATA_PATH_SETUP, 0xD4);
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, 0xD4);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, 0xD4);
    

    for (uint32_t i = 0; i<5; i++) {
        beep();
        delay(200);
        beep();
        delay(100);
    }

    Log.info("...initialized");
}

void BoxDAC::beep() {
    //recommened cdoe for beep by chip doc
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_PROC_BLOCK_SEL, 0x19);
    //send(ADDR_P0_SERIAL::BEEP_LEN_MSB, 0x04);
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x0C); //mute DACs //optinal
    //f 30 26 xxx1xxx1 # wait for DAC gain flag to be set
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x02); //power down NDAC divider
    send(ADDR_P0_SERIAL::BEEP_L_GEN, 0x80); //enable beep generator with left channel volume = 0dB,
    send(ADDR_P0_SERIAL::BEEP_R_GEN, 0x00);
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);  //power up NDAC divider
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00); //unmute DACs optinal
}

void BoxDAC::loop() { 
}

bool BoxDAC::send_raw(uint8_t data) {
    if (!Wire.write(data)) {
        Log.error("Could not write into I2C Buffer");
        return false;
    }
    return true;
}
bool BoxDAC::send(uint8_t target_register, uint8_t data) {
    //0x30 - 8bit / 0x18 - 7bit
    Wire.beginTransmission(0x18);
    if (!send_raw(target_register)) return false;
    if (!send_raw(data)) return false;
    
    uint8_t result = Wire.endTransmission(false);
    if (!result) return true;
    Log.error("Could not send I2C buffer, error=%i", result);
}
bool BoxDAC::send(ADDR target_register, PAGE data) {
    return send((uint8_t)target_register, (uint8_t)data);
}
bool BoxDAC::send(ADDR_P0_SERIAL target_register, uint8_t data) {
    return send((uint8_t)target_register, data);
}
bool BoxDAC::send(ADDR_P1_DAC_OUT target_register, uint8_t data) {
    return send((uint8_t)target_register, data);
}
bool BoxDAC::send(ADDR_P3_MCLK target_register, uint8_t data) {
    return send((uint8_t)target_register, data);
}