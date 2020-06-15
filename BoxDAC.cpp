#include "BoxDAC.h"
#include <Wire.h>

void BoxDAC::begin() { 
    Log.info("Initialize DAC...");
    //RESET
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    delayMicroseconds(1); //Be sure 

    Wire.begin();

    //Extracted from logic analyzer capture of box
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::SOFTWARE_RESET, 0x01); //Self-clearing software reset for control register

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
    send(ADDR_P1_DAC_OUT::HP_CTRL, 0xE0);

    send(ADDR::PAGE_CONTROL, PAGE::MCLK_DIVIDER);
    send(ADDR_P3_MCLK::TIMER_CLK_MCLK_DIV, 0x01);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(0x43, 0x8C);
    send(0x30, 0x80);
    send(0x33, 0x14);

    //send(0x2E);
    //read 0x31 addr

    //Testing custom code
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x0C); //mute DACs
    //f 30 26 xxx1xxx1 # wait for DAC gain flag to be set
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x02); //power down NDAC divider
    for (uint32_t i = 0; i<10; i++) {
        send(ADDR_P0_SERIAL::BEEP_L_GEN, 0x80); //enable beep generator with left channel volume = 0dB,
        delay(50);
        send(ADDR_P0_SERIAL::BEEP_L_GEN, 0x80); //enable beep generator with left channel volume = 0dB,
        delay(100);
    }
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);  //power up NDAC divider
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00); //unmute DACs



    Log.info("...initialized");
}

void BoxDAC::loop() { 
}

bool BoxDAC::send_raw(uint8_t data) {
    delay(1);
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