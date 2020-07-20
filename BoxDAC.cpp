#include "BoxDAC.h"
#include <Wire.h>

#include <driverlib/prcm.h>
#include <driverlib/i2s.h>
#include <driverlib/udma.h>
#include <driverlib/rom_map.h>
#include <udma_if.h>

#include "AudioOutputCC3200I2S.h"
#include <ESP8266SAM.h>

#include "Hackiebox.h"


void BoxDAC::logDmaIrqChanges() {
    if (lastDmaIRQcount != dmaIRQcount) {
        Log.info("***dmaIRQcount=%i", dmaIRQcount);
        lastDmaIRQcount = dmaIRQcount;
    }
    if (lastPriIndexRx != priIndexRx) {
        Log.info("***priIndexRx=%i", priIndexRx);
        lastPriIndexRx = priIndexRx;
    }
    if (lastAltIndexRx != altIndexRx) {
        Log.info("***altIndexRx=%i", altIndexRx);
        lastAltIndexRx = altIndexRx;
    }

    if (lastDmaBufferFilled != dmaBufferFilled) {
        Log.info("***dmaBufferFilled=%i", dmaBufferFilled);
        lastDmaBufferFilled = dmaBufferFilled;
    }
    if (lastDmaBufferEmpty != dmaBufferEmpty) {
        Log.info("***dmaBufferEmpty=%i", dmaBufferEmpty);
        lastDmaBufferEmpty = dmaBufferEmpty;
    }
}

void dma_irq() {
    Box.boxDAC.dmaPingPingComplete();
}

void BoxDAC::begin() {
    Log.info("Initialize DAC...");
    audioBuffer.init();/*
    audioBuffer.logState();
    fillBuffer(25);
    fillBuffer(25);
    fillBuffer(25);*/
    audioBuffer.logState();

    uint32_t clock; //(Num of bits * STEREO * sampling)
    clock = 16*2*16000;

    MAP_PinTypeI2S(PIN_50, PIN_MODE_4); //I2S Data0 (DIN)
    MAP_PinTypeI2S(PIN_53, PIN_MODE_2); //I2S ClockO (BCLK)
    MAP_PinTypeI2S(PIN_63, PIN_MODE_7); //I2S Frame Sync (WCLK)

    Log.info("Clock");
    MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_I2S);

    initDACI2C();

    Log.info("DMA");

    UDMAInit();
    UDMAChannelSelect(UDMA_CH5_I2S_TX, NULL);
    
    writeBuffer = audioBuffer.getBuffer(BoxAudioBufferTriple::BufferType::WRITE);
    writeBuffer->state = BoxAudioBufferTriple::BufferState::WRITING;
    audioBuffer.logState(writeBuffer);

    BoxAudioBufferTriple::BufferStruct* buffer = audioBuffer.getBuffer(BoxAudioBufferTriple::BufferType::READ);
    buffer->state = BoxAudioBufferTriple::BufferState::READING;
    audioBuffer.logState(buffer);

    SetupTransfer(
        UDMA_CH5_I2S_TX,
        UDMA_MODE_PINGPONG,
        I2S_MAX_ELEMENTS,
        UDMA_SIZE_16,
        UDMA_ARB_8,
        (void *)buffer->buffer,
        UDMA_CHCTL_SRCINC_16,
        (void *)I2S_TX_DMA_PORT,
        UDMA_DST_INC_NONE
    );
    SetupTransfer(
        UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
        UDMA_MODE_PINGPONG,
        I2S_MAX_ELEMENTS,
        UDMA_SIZE_16,
        UDMA_ARB_8,
        (void *)buffer->buffer,
        UDMA_CHCTL_SRCINC_16,
        (void *)I2S_TX_DMA_PORT,
        UDMA_DST_INC_NONE
    );
    

    Log.info("I2S");
    MAP_I2SIntEnable(I2S_BASE, I2S_INT_XDATA);
    MAP_I2SIntRegister(I2S_BASE, dma_irq);
    MAP_I2STxFIFOEnable(I2S_BASE, 8, 1);
    MAP_PRCMI2SClockFreqSet(clock);

    MAP_I2SConfigSetExpClk(I2S_BASE, clock, clock, I2S_SLOT_SIZE_16|I2S_PORT_DMA);
    MAP_I2SSerializerConfig(I2S_BASE, I2S_DATA_LINE_0, I2S_SER_MODE_TX, I2S_INACT_LOW_LEVEL);

    MAP_I2SEnable(I2S_BASE, I2S_MODE_TX_ONLY);
    

    /*
    for (uint32_t i = 0; i<5; i++) {
        beep();
        delay(200);
        beep();
        delay(100);
    }*/

    //beepTest();

    /*
    AudioOutputCC3200I2S *out = NULL;
    out = new AudioOutputCC3200I2S();
    out->begin();
    ESP8266SAM *sam = new ESP8266SAM;
    sam->Say(out, "Can you hear me now?");
    delay(500);
    sam->Say(out, "I can't hear you!");
    delete sam;*/

    setInterval(0);

    setVolume(current_volume);
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00);

    Log.info("...initialized");
}

void BoxDAC::loop() { 
    fillBuffer(25);
}

void BoxDAC::fillBuffer(uint16_t timeoutMs) {
    BoxTimer timeout;
    uint32_t halfWavelength = (sampleRate / frequency) / 2;
    timeout.setTimer(timeoutMs);

    while (timeout.isRunning()) {
        while(writeBuffer->position<writeBuffer->size && timeout.isRunning()) {
            if (count % halfWavelength == 0)
                sample = -1 * sample; // invert the sample every half wavelength count multiple to generate square wave
            if (count % (2*halfWavelength) == 0) 
                count = 0;
            
            writeBuffer->buffer[writeBuffer->position++] = sample;
            writeBuffer->buffer[writeBuffer->position++] = sample;
            
            count++;
            i2sElmCount++;
            timeout.tick();
        }
        if (writeBuffer->position >= writeBuffer->size) {
            if (audioBuffer.flip(BoxAudioBufferTriple::BufferType::WRITE)) {
                writeBuffer = audioBuffer.getBuffer(BoxAudioBufferTriple::BufferType::WRITE);
                writeBuffer->state = BoxAudioBufferTriple::BufferState::WRITING;
                writeBuffer->position = 0;
                continue;
            }
            /*
            Log.info("##reset writePosition");
            audioBuffer.logState(writeBuffer);
            audioBuffer.logState();*/
        }
        break;
    }
}

void BoxDAC::dmaPingPingComplete() {
    MAP_I2SIntClear(I2S_BASE, I2S_INT_XDMA);

    unsigned long intStatus = MAP_uDMAIntStatus();
    
    dmaIRQcount++;
    if (intStatus & 0x20) { //TX IRQ I2S_INT_XDMA?
        unsigned long channelModePri = MAP_uDMAChannelModeGet(UDMA_CH5_I2S_TX | UDMA_PRI_SELECT);
        unsigned long channelModeAlt = MAP_uDMAChannelModeGet(UDMA_CH5_I2S_TX | UDMA_ALT_SELECT);

        unsigned long channel = 0x00;
        if (channelModePri == UDMA_MODE_STOP) {
            channel = UDMA_CH5_I2S_TX;
            priIndexRx++;
        } else if (channelModeAlt == UDMA_MODE_STOP) {
            channel = UDMA_CH5_I2S_TX|UDMA_ALT_SELECT;
            altIndexRx++;
        }
        if (channel) {
            if (audioBuffer.flip(BoxAudioBufferTriple::BufferType::READ)) {
                dmaBufferFilled++;
            } else {
                dmaBufferEmpty++;
            }
            BoxAudioBufferTriple::BufferStruct* readBuffer = audioBuffer.getBuffer(BoxAudioBufferTriple::BufferType::READ);
            readBuffer->state = BoxAudioBufferTriple::BufferState::READING;

            MAP_uDMAChannelTransferSet(channel,
                UDMA_MODE_PINGPONG,
                (void *)readBuffer->buffer,
                (void *)I2S_TX_DMA_PORT,
                I2S_MAX_ELEMENTS
            );
            MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
        }
    }
}

void BoxDAC::beepTest() {
    uint16_t pauseLen = 50;
    uint8_t baseNote = 48 + 1*12;
    beepMidi(baseNote, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+2, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+4, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+5, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, 2*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, 2*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, 4*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+9, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, 4*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+5, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+5, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+5, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+5, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+4, 2*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+4, 2*pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote+7, pauseLen);
    Box.delayTask(pauseLen);
    beepMidi(baseNote, 4*pauseLen);
}

void BoxDAC::beepRaw(uint16_t sin, uint16_t cos, uint32_t length) {
    beepRaw(sin, cos, length, convertDacVol2BeepVol(current_volume));
}
void BoxDAC::beepRaw(uint16_t sin, uint16_t cos, uint32_t length, uint8_t volume) {
    Log.info("beep sin=%i, cos=%i, len=%l, vol=%X", sin, cos, length, volume);
    logBeepVolume(volume);

    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);

    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x0C); //mute DACs //optional
    //f 30 26 xxx1xxx1 # wait for DAC gain flag to be set
    while ((readByte(ADDR_P0_SERIAL::DAC_FLAG_REG) & 0b00010001) != 0b00010001) { }
    //send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x02); //power down NDAC divider - Page 41 (but makes glitches?!)

    send(ADDR_P0_SERIAL::BEEP_LEN_MSB, (length>>16)&0xFF);
    send(ADDR_P0_SERIAL::BEEP_LEN_MID, (length>>8)&0xFF);
    send(ADDR_P0_SERIAL::BEEP_LEN_LSB, length);

    send(ADDR_P0_SERIAL::BEEP_SIN_MSB, (sin>>8)&0xFF);
    send(ADDR_P0_SERIAL::BEEP_SIN_LSB, sin);

    send(ADDR_P0_SERIAL::BEEP_COS_MSB, (cos>>8)&0xFF);
    send(ADDR_P0_SERIAL::BEEP_COS_LSB, cos);

    send(ADDR_P0_SERIAL::BEEP_R_GEN, 0x80);
    send(ADDR_P0_SERIAL::BEEP_L_GEN, 0x80|(volume&0x3F)); //enable beep generator with right channel volume,
    
    //send(ADDR_P0_SERIAL::DAC_NDAC_VAL, 0x84);  //power up NDAC divider - Page 41 (but makes glitches?!)

    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00); //unmute DACs optional
    
}
void BoxDAC::beepMidi(uint8_t midiId, uint16_t lengthMs, bool async) {
    //TODO Check boundaries!
    uint16_t samplerate = 16000;
    int32_t freq = frequencyTable[midiId]; //fixed point /100
    int16_t sin = beepTable16000[midiId][0];
    int16_t cos = beepTable16000[midiId][1];


    int32_t cycles = 2*freq*lengthMs/1000/100;
    int32_t samples_opt = samplerate*(cycles)*100/freq/2;

    //int32_t samples = lengthMs * samplerate / 1000; //check length
    //Log.info("samplerate=%i, lengthMs=%i, freq=%i, sin=%i, cos=%i", samplerate, lengthMs, freq, sin, cos);
    //Log.info("samples=%i, cycles=%i, samples_opt=%i", samples, cycles, samples_opt);

    beepRaw(sin, cos, samples_opt);
    if (!async) {
        while ((readByte(ADDR_P0_SERIAL::BEEP_L_GEN) & 0b10000000) == 0b10000000) {
            Box.watchdog_feed();
        }
    }
}
void BoxDAC::beep() {
    //beepRaw(0x30FC, 0x7642, 0x640);
    beepMidi(84, 1000, false);
}

bool BoxDAC::send(uint8_t target_register, uint8_t data) {
    //0x30 - 8bit / 0x18 - 7bit
    return Box.boxI2C.send(0x18, target_register, data);
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

uint8_t BoxDAC::readByte(uint8_t source_register) {
    //0x30 - 8bit / 0x18 - 7bit
    return Box.boxI2C.readByte(0x18, source_register);
}
uint8_t BoxDAC::readByte(ADDR source_register) {
    return readByte((uint8_t)source_register);
}
uint8_t BoxDAC::readByte(ADDR_P0_SERIAL source_register) {
    return readByte((uint8_t)source_register);
}
uint8_t BoxDAC::readByte(ADDR_P1_DAC_OUT source_register) {
    return readByte((uint8_t)source_register);
}
uint8_t BoxDAC::readByte(ADDR_P3_MCLK source_register) {
    return readByte((uint8_t)source_register);
}

bool BoxDAC::increaseVolume() {
    bool result = false;
    if (current_volume < VOL_MAX) {
        current_volume += VOL_STEP;
        setVolume(current_volume);
        beepRaw(0x278A, 0x79BD, 0x000140); //16kHz
        //beepMidi(78,50,true);
        result =  true;
    } else {
        beepRaw(0x30F9, 0x763F, 0x000140); //16kHz
        Box.delayTask(50);
        beepRaw(0x30F9, 0x763F, 0x000140); //16kHz
        //beepMidi(84,50,true);
        Log.info("Maximum volume reached.");
    }
    logVolume();
    return result;
}
bool BoxDAC::decreaseVolume() {
    bool result = false;
    if (current_volume > VOL_MIN) {
        current_volume -= VOL_STEP;
        setVolume(current_volume);
        beepRaw(0x18F5, 0x7D87, 0x000140); //16kHz
        //beepMidi(70, 50, true);
        result = true;
    } else {
        beepRaw(0x0F0A, 0x7F1A, 0x000140); //16kHz
        Box.delayTask(50);
        beepRaw(0x0F0A, 0x7F1A, 0x000140); //16kHz
        //beepMidi(62, 50, true);
        Log.info("Minimal volume reached.");
    }
    logVolume();
    return result;
}

void BoxDAC::setVolume(uint8_t volume) {
    int8_t volumeConv = (int8_t)(volume-0x7F);
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, volumeConv);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, volumeConv);
    while ((readByte(ADDR_P0_SERIAL::DAC_FLAG_REG) & 0b00010001) != 0b00010001) { }
}

void BoxDAC::logVolume() {
    uint8_t volume = current_volume;
    uint8_t volumeConv = (volume-0x7F)&0xFF;

    int8_t dbVal = (volume-64*2)/2;
    int8_t dbDotVal = 10*((volume-64*2)/2)-(10*dbVal);

    Log.info("Volume %X(%X) means %i.%idB", volumeConv, volume, dbVal, dbDotVal);
}
void BoxDAC::logBeepVolume(uint8_t volume) {
    int8_t dbVal = 2-volume;
    Log.info("Beep volume %X means %i.0dB", volume, dbVal);
}

uint8_t BoxDAC::convertDacVol2BeepVol(uint8_t dacVol) {
    //DAC  Range -63.5dB - +24.0dB
    //BEEP Range -61.0dB - +02.0dB
    int8_t dbVal = (dacVol-64*2)/2;

    if (dbVal>2) {
        dbVal = 2;
    } else if (dbVal<-61) {
        dbVal = -61;
    }

    int8_t beepVol = 2-dbVal;

    return beepVol & 0x3F;
}

void BoxDAC::initDACI2C() {
    Log.info("I2C");

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
    send(ADDR_P1_DAC_OUT::L_VOL_TO_SPK, 128);
}