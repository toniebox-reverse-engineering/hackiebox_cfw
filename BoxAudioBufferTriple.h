#ifndef BoxAudioBufferTriple_h
#define BoxAudioBufferTriple_h

#include "BaseHeader.h"

class BoxAudioBufferTriple {
    public:
        enum class BufferState {
            UNKNOWN = 0x00,
            READY_FOR_WRITE = 0x01,
            READY_FOR_READ = 0x02,
            READING = 0x03,
            WRITING = 0x04
        };
        enum class BufferType {
            READ = 0x01,
            WRITE = 0x02,
            WAIT = 0x03
        };
        struct BufferStruct {
            uint8_t index;
            uint16_t* buffer;
            uint16_t size;
            uint16_t position;
            BufferState state;
            BufferStruct* next;
        };

        const static uint16_t I2S_MAX_ELEMENTS = 1024;
        const static uint16_t I2S_MAX_BYTES = 2*I2S_MAX_ELEMENTS;

        void init();
        void logState();
        void logState(BoxAudioBufferTriple::BufferStruct* buffer);

        bool isFull();
        bool isEmpty();

        uint16_t getBufferSize();

        BufferStruct* getBuffer(BoxAudioBufferTriple::BufferType type);

        bool flip(BoxAudioBufferTriple::BufferType type);

    private:
        
        BufferStruct* _bufferRead;
        BufferStruct* _bufferWrite;

        //static uint16_t _dataBufferSize = _eringbuffer - _ringbuffer; //TODO
        //uint8_t* _dataBuffer = (uint8_t*)_ringbuffer;
        const static uint16_t _dataBufferSize = 0x4000;
        uint8_t* _dataBuffer = (uint8_t*)0x20000000; //lower memory up to 0x4000 length;
        //uint8_t __attribute__((section(".blsec"))) _dataBuffer[_dataBufferSize];

        const static uint16_t _bufferSize = I2S_MAX_ELEMENTS;
        const static uint16_t _bufferCount = _dataBufferSize / I2S_MAX_BYTES;
        
        BufferStruct _bufferStruct[_bufferCount];
};

#endif