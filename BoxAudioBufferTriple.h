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
        struct BufferStruct {
            uint8_t index;
            uint16_t* buffer;
            uint16_t size;
            BufferState state;
        };

        void init();
        void logState();

        uint16_t getBufferSize();

        BufferStruct getBuffer(BoxAudioBufferTriple::BufferState state);
        void setBufferState(BoxAudioBufferTriple::BufferState state, uint8_t index);

    private:
        
        BufferStruct _bufferStruct[4];
        BufferStruct _emptyStruct;

        uint8_t
            _indexReadBuffer,
            _indexWriteBuffer,
            _indexWaitBuffer;

        uint16_t _bufferSize;

        uint8_t* _dataBuffer = (uint8_t*)0x20000000; //lower memory up to 0x4000 length;
        uint16_t _dataBufferSize = 0x4000;

        const static uint16_t _emptyBufferSize = 0x200;
        uint16_t _emptyBuffer[_emptyBufferSize];
};

#endif