#ifndef BoxAudioBuffer_h
#define BoxAudioBuffer_h

#include "BaseHeader.h"

class BoxAudioBuffer  { 
    public:
        bool noIRQ = false;

        void init(uint8_t* buffer, uint16_t size);
        void logState();

        bool readPointerEqualsWritePointer();

        bool
            isEmpty(uint16_t threshold = 0),
            isFull(uint16_t threshold = 0);

        uint8_t* getReadPointer();
        void updateReadPointer(uint16_t readLength);

        void write(uint8_t* buffer, uint16_t size);

        uint16_t
            getBytesReadable(),
            getBytesReadableBlock(),
            getBytesWritable(),
            getBytesWritableBlock();

    private:
        bool _ready = false;

        bool
            _isFull,
            _isEmpty;
        

        uint8_t* _bufferStart;
        uint8_t* _bufferEnd;
        uint16_t _bufferSize;

        uint8_t* _readPointer;
        uint8_t* _writePointer;

        uint16_t
            _measureDistance(uint8_t* p1, uint8_t* p2),
            _measureDistanceBlock(uint8_t* p1, uint8_t* p2);
};
#endif