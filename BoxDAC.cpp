#include "BoxDAC.h"
#include <Wire.h>

#include <driverlib/prcm.h>
#include <driverlib/i2s.h>
#include <driverlib/udma.h>
#include <driverlib/rom_map.h>
#include <udma_if.h>

#include "Hackiebox.h"

//#include <libopus.h>

void BoxDAC::logDmaIrqChanges() {
    if (lastDmaIRQcount != dmaIRQcount) {
        Log.info("*dmaIRQcount=%i", dmaIRQcount);
        lastDmaIRQcount = dmaIRQcount;
    }
    if (lastPriIndexRx != priIndexRx) {
        Log.info("*priIndexRx=%i", priIndexRx);
        lastPriIndexRx = priIndexRx;
    }
    if (lastAltIndexRx != altIndexRx) {
        Log.info("*altIndexRx=%i", altIndexRx);
        lastAltIndexRx = altIndexRx;
    }

    if (lastDmaBufferFilled != dmaBufferFilled) {
        Log.info("*dmaBufferFilled=%i", dmaBufferFilled);
        lastDmaBufferFilled = dmaBufferFilled;
    }
    if (lastDmaBufferEmpty != dmaBufferEmpty) {
        Log.info("*dmaBufferEmpty=%i", dmaBufferEmpty);
        lastDmaBufferEmpty = dmaBufferEmpty;
    }
}

void dma_irq() {
    Box.boxDAC.dmaPingPingComplete();
}

void BoxDAC::begin() {
    Log.info("Init DAC...");
    audioBuffer.init();
    audioBuffer.logState();

    MAP_PinTypeI2S(PIN_50, PIN_MODE_4); //I2S Data0 (DIN)
    MAP_PinTypeI2S(PIN_53, PIN_MODE_2); //I2S ClockO (BCLK)
    MAP_PinTypeI2S(PIN_63, PIN_MODE_7); //I2S Frame Sync (WCLK)

    Log.info("Clock");
    MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_I2S);

    Log.info("Output");
    audioOutputI2S = new AudioOutputCC3200I2S(&audioBuffer);
    //audioOutputResample = new AudioOutputResample(48000, audioOutputI2S);
    //audioOutputBuffer = new AudioOutputBuffer(4096, audioOutputResample);
    audioOutput = audioOutputI2S;

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
        BoxAudioBufferTriple::I2S_MAX_ELEMENTS,
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
        BoxAudioBufferTriple::I2S_MAX_ELEMENTS,
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
    MAP_I2SSerializerConfig(I2S_BASE, I2S_DATA_LINE_0, I2S_SER_MODE_TX, I2S_INACT_LOW_LEVEL);
    MAP_I2SIntEnable(I2S_BASE, I2S_INT_XDATA);
    MAP_I2SEnable(I2S_BASE, I2S_MODE_TX_ONLY);


    /*
    for (uint32_t i = 0; i<5; i++) {
        beep();
        delay(200);
        beep();
        delay(100);
    }*/

    //beepTest();

    setInterval(0);

    setVolume(current_volume);
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_VOL_CTRL, 0x00);

    Log.info("...done");

    //samSay("Hackiebox by Team Revvox!");
}
void BoxDAC::opusTest() {
    /*
    Log.info("Start mp3");

    AudioGeneratorMP3 *mp3;
    AudioFileSourceFatFs *file;

    file = new AudioFileSourceFatFs("/pno-cs.mp3");
    mp3 = new AudioGeneratorMP3();
    mp3->begin(file, audioOutput);

    while (mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            Log.error("MP3 Loop error, stopped!");
        }
    }
    Log.info("Mp3 finished");
    */
    //AudioGeneratorTonie *opus;
    AudioFileSourceFatFs *file;

    //file = new AudioFileSourceFatFs("/gs-16b-2c-44100hz.opus");
    /*file = new AudioFileSourceFatFs("/CONTENT/6977960C/500304E0");
    opus = new AudioGeneratorTonie();
    opus->begin(file, audioOutput);
    while (opus->isRunning()) {
        if (!opus->loop())
            opus->stop();
    }
    free(file);
    free(opus);
    Log.info("Opus done");
    */

    playFile("/piano2.wav");
    //file = new AudioFileSourceFatFs("/LRMonoPhase4.wav");
    //file = new AudioFileSourceFatFs("/organfinale.wav");
    //file = new AudioFileSourceFatFs("/piano2.wav");

    /*
    OpusDecoder* decoder;
    int error;
    int channels = 2;

    Box.boxPower.feedSleepTimer();
    Log.info("Needed heap %ib", opus_decoder_get_size(channels));
    decoder = opus_decoder_create(16000, channels, &error);
    if (error != OPUS_OK) {
        Log.error("Couldn't create OPUS Decoder error=%i", error);
        return;
    }
    Box.boxPower.feedSleepTimer();
    opus_decoder_destroy(decoder);

    //opus_decode(decoder, data, len, pcmout, frameSize, 0);*/
}

void BoxDAC::loop() {
    if (audioPlaying) {
        loop(audioTimeoutMs);
    } else {
        loop(10);
    }
}
void BoxDAC::loop(uint16_t timeoutMs) {
    if (audioPlaying) {
        if (!audioGenerator || !audioSource) {
            audioPlaying = false;
            return;
        }

        BoxTimer timeout;
        timeout.setTimer(timeoutMs);

        while (timeout.isRunning() && audioGenerator->isRunning()) {
            if (!audioGenerator->loop())
                audioGenerator->stop();
            timeout.tick();
        }
        if (!audioGenerator->isRunning())
            audioPlaying = false;
    } else {
        generateZeroAudio(timeoutMs);
    }
}

void BoxDAC::play() {
    audioPlaying = true;
}
void BoxDAC::pause() {
    audioPlaying = false;
}
void BoxDAC::stop() {
    audioPlaying = false;
    if (audioSource && audioSource->isOpen())
        audioSource->seek(0, SEEK_SET);
}

bool BoxDAC::hasStopped() {/*
    if (audioGenerator)
        Log.info("audioGenerator: %T", audioGenerator->isRunning());
    if (audioSource)
        Log.info("audioSource: %T", audioSource->isOpen());*/

    if (!audioGenerator)
        return true;
    //Log.info("A");
    if (!audioGenerator->isRunning())
        return true;
        /*
    Log.info("B");
    if (!audioSource); //Fails even when audioSource is not NULL above
        return true;
    Log.info("C");
    if (!audioSource->isOpen());
        return true;*/
    
    //Log.info("hasStopped: false!");

    return false;
}

bool BoxDAC::playFile(const char* path) {
    Log.info("Start playing file %s...", path);

    if (audioGenerator && audioGenerator->isRunning()) {
        audioGenerator->stop();
        free(audioGenerator);
    }
    if (audioSource && audioSource->isOpen()) {
        audioSource->close();
        free(audioSource);
    }

    audioPlaying = false;
    return _playWAV(path);
}
bool BoxDAC::_playWAV(const char* path) {
    AudioGeneratorWAV *ag = new AudioGeneratorWAV();
    ag->SetBufferSize(128);

    audioGenerator = ag;
    audioSource = new AudioFileSourceFatFs(path);
    
    if (!audioGenerator->begin(audioSource, audioOutput)) {
        Log.error("Couldn't play wav?!");
        return false;
    }
    Log.info("WAV file loaded...");
    audioPlaying = true;
    return true;
}

void BoxDAC::generateFrequency(uint32_t frequency, uint16_t timeoutMs) {
    BoxTimer timeout;
    uint32_t halfWavelength = (audioOutputI2S->GetRate() / frequency) / 2;
    timeout.setTimer(timeoutMs);

    while (timeout.isRunning()) {
        while (writeBuffer->position<writeBuffer->size && timeout.isRunning()) {
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
                continue;
            }
        }
        break;
    }
}

void BoxDAC::generateZeroAudio(uint16_t timeoutMs) {
    BoxTimer timeout;
    timeout.setTimer(timeoutMs);

    while (timeout.isRunning()) {
        while (writeBuffer->position<writeBuffer->size) {
            writeBuffer->buffer[writeBuffer->position++] = 0;
            writeBuffer->buffer[writeBuffer->position++] = 0;
        }
        timeout.tick();
        if (writeBuffer->position >= writeBuffer->size) {
            if (audioBuffer.flip(BoxAudioBufferTriple::BufferType::WRITE)) {
                writeBuffer = audioBuffer.getBuffer(BoxAudioBufferTriple::BufferType::WRITE);
                writeBuffer->state = BoxAudioBufferTriple::BufferState::WRITING;
                continue;
            }
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
            channel = UDMA_CH5_I2S_TX | UDMA_PRI_SELECT;
            priIndexRx++;
        } else if (channelModeAlt == UDMA_MODE_STOP) {
            channel = UDMA_CH5_I2S_TX | UDMA_ALT_SELECT;
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

            MAP_uDMAChannelTransferSet(
                channel,
                UDMA_MODE_PINGPONG,
                (void *)readBuffer->buffer,
                (void *)I2S_TX_DMA_PORT,
                readBuffer->size
            );
            MAP_uDMAChannelEnable(channel);
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
    while ((readByte(ADDR_P0_SERIAL::DAC_FLAG_REG) & 0b00010001) != 0b00010001) { Box.delayTask(1); }
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
    uint16_t samplerate = audioOutputI2S->GetRate();
    int32_t freq = frequencyTable[midiId]; //fixed point /100
    int16_t sin = beepTable16000[midiId][0];
    int16_t cos = beepTable16000[midiId][1];

    switch (samplerate) {
        case 22050:
            sin = beepTable22050[midiId][0];
            cos = beepTable22050[midiId][1];
            break;
        case 32000:
            sin = beepTable32000[midiId][0];
            cos = beepTable32000[midiId][1];
            break;
        case 44100:
            sin = beepTable44100[midiId][0];
            cos = beepTable44100[midiId][1];
            break;
        case 48000:
            sin = beepTable48000[midiId][0];
            cos = beepTable48000[midiId][1];
            break;
    }

    int32_t cycles = 2*freq*lengthMs/1000/100;
    int32_t samples_opt = samplerate*(cycles)*100/freq/2;

    //int32_t samples = lengthMs * samplerate / 1000; //check length
    Log.info("samplerate=%i, lengthMs=%i, freq=%i, sin=%i, cos=%i", samplerate, lengthMs, freq, sin, cos);
    //Log.info("samples=%i, cycles=%i, samples_opt=%i", samples, cycles, samples_opt);

    beepRaw(sin, cos, samples_opt);
    if (!async) {
        while ((readByte(ADDR_P0_SERIAL::BEEP_L_GEN) & 0b10000000) == 0b10000000) {
            Box.watchdog_feed();
            Box.delayTask(1);
        }
    }
}
void BoxDAC::beep() {
    //beepRaw(0x30FC, 0x7642, 0x640);
    beepMidi(84, 1000, false);
}

void BoxDAC::samSay(const char *text, enum ESP8266SAM::SAMVoice voice, uint8_t speed, uint8_t pitch, uint8_t throat, uint8_t mouth, bool sing, bool phoentic) {
    #ifdef FEATURE_FLAG_TEXT2SPEECH
        int samplerate = audioOutputI2S->GetRate();
        audioOutput->flush();
        ESP8266SAM* sam = new ESP8266SAM();

        sam->SetVoice(voice);
        if (speed > 0)
            sam->SetSpeed(speed);
        if (pitch > 0)
            sam->SetSpeed(pitch);
        if (throat > 0)
            sam->SetSpeed(throat);
        if (mouth > 0)
            sam->SetSpeed(mouth);
        sam->SetSingMode(sing);
        sam->SetPhonetic(phoentic);

        sam->Say(audioOutput, text);
        audioOutput->flush();
        delete sam;

        audioOutput->SetRate(samplerate);
    #else
        Log.error("Text-To-Speech off (FEATURE_FLAG_TEXT2SPEECH not set)");
    #endif
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
        Log.info("Max volume reached");
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
        Log.info("Min volume reached");
    }
    logVolume();
    return result;
}

void BoxDAC::setVolume(uint8_t volume) {
    int8_t volumeConv = (int8_t)(volume-0x7F);
    send(ADDR::PAGE_CONTROL, PAGE::SERIAL_IO);
    send(ADDR_P0_SERIAL::DAC_VOL_L_CTRL, volumeConv);
    send(ADDR_P0_SERIAL::DAC_VOL_R_CTRL, volumeConv);
    while ((readByte(ADDR_P0_SERIAL::DAC_FLAG_REG) & 0b00010001) != 0b00010001) { Box.delayTask(1); }
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