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
        };

        void init();
        void logState();
        void logState(BoxAudioBufferTriple::BufferStruct* buffer);

        uint16_t getBufferSize();

        BufferStruct* getBuffer(BoxAudioBufferTriple::BufferType type);

        bool flip(BoxAudioBufferTriple::BufferType type);

    private:
        
        BufferStruct _bufferStruct[4];
        
        BufferStruct* _bufferRead;
        BufferStruct* _bufferWrite;
        BufferStruct* _bufferWait;
        BufferStruct* _emptyStruct;

        uint8_t
            _indexReadBuffer,
            _indexWriteBuffer,
            _indexWaitBuffer;

        uint16_t _bufferSize;

        const static uint16_t _dataBufferSize = 0x4000;
        uint8_t* _dataBuffer = (uint8_t*)0x20000000; //lower memory up to 0x4000 length;
        //uint8_t __attribute__((section(".blsec"))) _dataBuffer[_dataBufferSize];

        const static uint16_t _emptyBufferSize = 0x400;
        uint16_t _emptyBuffer[_emptyBufferSize];
};

#endif