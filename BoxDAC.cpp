#include "BoxDAC.h"
#include <Wire.h>

#include "driverlib/prcm.h"
#include "driverlib/i2s.h"

#include "AudioOutputCC3200I2S.h"
#include <ESP8266SAM.h>

void BoxDAC::begin() {
    Log.info("Initialize DAC...");
    
    uint32_t clock; //(Num of bytes * STEREO * sampling)
    clock = 16*2*16000;

    MAP_PinTypeI2S(PIN_50, PIN_MODE_4); //I2S Data0 (DIN)
    MAP_PinTypeI2S(PIN_53, PIN_MODE_2); //I2S ClockO (BCLK)
    MAP_PinTypeI2S(PIN_63, PIN_MODE_7); //I2S Frame Sync (WCLK)

    MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_I2S);
    MAP_PRCMI2SClockFreqSet(clock);
    MAP_I2SConfigSetExpClk(I2S_BASE,clock,clock,I2S_SLOT_SIZE_16|I2S_PORT_DMA);
    //MAP_I2SConfigSetExpClk(I2S_BASE,clock,clock,I2S_SLOT_SIZE_16|I2S_PORT_CPU);
    MAP_I2SSerializerConfig(I2S_BASE,I2S_DATA_LINE_1,I2S_SER_MODE_RX, I2S_INACT_LOW_LEVEL);
    MAP_I2SSerializerConfig(I2S_BASE,I2S_DATA_LINE_0,I2S_SER_MODE_TX, I2S_INACT_LOW_LEVEL);
    MAP_I2SEnable(I2S_BASE, I2S_MODE_TX_ONLY);

    //Extracted from logic analyzer capture of box
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::SOFTWARE_RESET, 0x01);     //Self-clearing software reset for control register
        
    send(ADDR_P0_SERIAL::CLOCKGEN_MUX, 0x07);       //0000:reserved, 01:PLL_CLKIN=BCLK, 11:CODEC_CLKIN=PLL_CLK 
    send(ADDR_P0_SERIAL::PLL_J_VAL, 0x20);          //00:reserved, 100000:PLL multiplier J=32 (0x20)
    send(ADDR_P0_SERIAL::PLL_D_VAL_MSB, 0x00);      //00:reserved, 000000:fraktional multiplier D-value = 0
    send(ADDR_P0_SERIAL::PLL_D_VAL_LSB, 0x00);      //00:reserved, 000000:fraktional multiplier D-value = 0
    send(ADDR_P0_SERIAL::PLL_P_R_VAL, 0x96);        //1:PLL is power up, 001:PLL divider P=1, 110:PLL multiplier R=6
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);       //1:NDAC divider powered up, 0000100:DAC NDAC divider=4
    send(ADDR_P0_SERIAL::DAC_MDAC_VAL, 0x86);       //1:MDAC divider powered up, 0000100:DAC MDAC divider=6
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_MSB, 0x01);   //000000:reserved, 01:DAC OSR MSB =256
    send(ADDR_P0_SERIAL::DAC_DOSR_VAL_LSB, 0x00);   //00000000:DAC OSR LSB

    delay(10);            //w PLL Start-Up
    
    send(ADDR_P0_SERIAL::CODEC_IF_CTRL1, 0x00);     //00:Codec IF=I2S, 00: Codec IF WL=16 bits, 0:BCLK=Input, 0:WCKL=Output, 0:reserved        // w IF statt INT
    send(ADDR_P0_SERIAL::DAC_PROC_BLOCK_SEL, 0x19); //000:reserved, 11001:DAC signal-processing block PRB_P25

    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::HP_OUT_POP_REM_SET, 0x4E);          //0:simultan.DAC/HP/SP, 1001:power-on-time=1.22s*,11:drv.ramp-up=3.9ms,0:CM voltage 
    send(ADDR_P1_DAC_OUT::OUT_PGA_RAMP_DOWN_PER_CTRL, 0x70);  //0:reserved, 111=30.5ms*, 0000:reserved *8.2MHz
    send(ADDR_P1_DAC_OUT::DAC_LR_OUT_MIX_ROUTING, 0x44);      //01:DAC_L to MixAmp_L,00:AIN1/2 not routed, 01:DAC_R to MIxAmp_R, 00:AIN1/2 not routed
    send(ADDR_P1_DAC_OUT::MICBIAS, 0x0B);                     //0:SwPowDwn not enabled, 000:reserved, 1:MICBIAS powered up, 0:reserved, 11:MICBIAS=AVDD
    send(ADDR_P1_DAC_OUT::HP_DRIVER_CTRL, 0xE0);              //000:Debounce Time=0us, 01:DAC perform.increased, 1:HPL output=lineout, 1:HPR output=lineout, 0:reserved ??? LINE

    send(ADDR::PAGE_CONTROL, PAGE::MCLK_DIVIDER);
    send(ADDR_P3_MCLK::TIMER_CLK_MCLK_DIV, 0x01);             //0:Internal oscillator for delay timer, 0000001: MCLK divider=1 

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::HEADSET_DETECT, 0x8C);               //1:Headset detection enabled, RR, 011:Debounce Prog.Glitch=128ms, 00:Debounce Prog.Glitch=0ms
    send(ADDR_P0_SERIAL::INT1_CTRL_REG, 0x80);                //1:Headset-insertion detect IRQ INT1, 0:Button-press detect, ...., 0=INT1 is only one pulse 2ms
    send(ADDR_P0_SERIAL::GPIO1_INOUT_CTRL, 0x14);             //XX:reserved, 0101:GPIO1=INT1 output, X=GPIO1 input buffer value, GPIO1 Output=X

    
    
    //send(0x2E); Excel 161

    // PAUSE 0,2s
    
    //read 0x18 addr



    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);  // MUTE ALL
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x7F);    // HPL Vol -oo
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x7F);    // HPL Vol -oo
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 0x7F);    // SPK Vol -oo


    // PAUSE 50ms
    delay(50);            //w Ramp

                                                // MUTE HP Driver AND SPK Driver
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x02);    // HPL driver is muted ??? must 1
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x02);    // HPR driver is muted ??? must 1
    send(ADDR_P1_DAC_OUT::SPK_DRIVER, 0x00);    // SPK driver is muted

    send(ADDR_P1_DAC_OUT::SPK_DRIVER, 0x04);    // an TEST   gehört hier nicht hin
    

    //PAUSE 50ms
    delay(50);            //w Ramp

                                                // AMPS Power Down
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0x00);    // HPL HPR Driver Power Down  ??? must 1
    //send(ADDR_P1_DAC_OUT::SPK_AMP, 0x00);       // falscher Wert SPK Amp Power Down ??? must 11
    send(ADDR_P1_DAC_OUT::SPK_AMP, 0x06);       // SPK Amp Power Down ??? must 000011


    //PAUSE 50ms
    delay(50);            //w Ramp

    
    send(ADDR_P1_DAC_OUT::HPL_DRIVER, 0x06);  // HPL driver 0dB, not muted 
    send(ADDR_P1_DAC_OUT::HPR_DRIVER, 0x06); // HPR drvier 0dB, not muted
    //send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0xC2); // Falscher Wert must 1
    send(ADDR_P1_DAC_OUT::HP_DRIVERS, 0xC4); // HPL HPR is power up, 1,35V, Shortcut=Error ??? must 1
    send(ADDR_P1_DAC_OUT::L_VOL_TO_HPL, 0x92);  // ??? Aux to HP ???
    send(ADDR_P1_DAC_OUT::R_VOL_TO_HPR, 0x92);  // ??? Aux to HP ???

    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 0x00);  // !!! FEHLTE !!!
    send(ADDR_P1_DAC_OUT::SPK_AMP, 0x86);       // !!! FEHLTE !!! SPK Amp Power Up



    // PAUSE 50ms
    delay(50);            //w Ramp


    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    //send(ADDR_P0_SERIAL::DAC_DATA_PATH_SETUP, 0xD5);  // DAC power on, Left=left, Right=Right, DAC Softstep HP STEREO
    send(ADDR_P0_SERIAL::DAC_DATA_PATH_SETUP, 0xF1);  // DAC power on, Left=left, Right=Right, DAC Softstep SPEAKER MONO
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, 0xDC);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, 0xDC);
    
    //Excel 219
    // Extract END
    send(ADDR::PAGE_CONTROL, PAGE::DAC_OUT_VOL);
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 128+64+32+16);
    for (uint32_t i = 0; i<5; i++) {
        beep();
        delay(200);
        beep();
        delay(100);
    }

    AudioOutputCC3200I2S *out = NULL;
    out = new AudioOutputCC3200I2S();
    out->begin();
    ESP8266SAM *sam = new ESP8266SAM;
    sam->Say(out, "Can you hear me now?");
    delay(500);
    sam->Say(out, "I can't hear you!");
    delete sam;

    Log.info("...initialized");
}

void BoxDAC::beep() {
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);

    //send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x0C); //mute DACs //optinal
    //f 30 26 xxx1xxx1 # wait for DAC gain flag to be set

    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x02); //power down NDAC divider

    send(ADDR_P0_SERIAL::BEEP_LEN_MID, 0x06);
    send(ADDR_P0_SERIAL::BEEP_LEN_LSB, 0x40);
    send(ADDR_P0_SERIAL::BEEP_SIN_MSB, 0x30);
    send(ADDR_P0_SERIAL::BEEP_SIN_LSB, 0xFC);
    send(ADDR_P0_SERIAL::BEEP_COS_MSB, 0x76);
    send(ADDR_P0_SERIAL::BEEP_COS_LSB, 0x42);

    send(ADDR_P0_SERIAL::BEEP_L_GEN, 0x80); //enable beep generator with left channel volume = 0dB,
    send(ADDR_P0_SERIAL::BEEP_R_GEN, 0x00);
    
    send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);  //power up NDAC divider

    //send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00); //unmute DACs optinal
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