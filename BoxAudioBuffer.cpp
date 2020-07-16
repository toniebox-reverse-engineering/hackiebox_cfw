#include "BoxAudioBuffer.h"

void BoxAudioBuffer::init(uint8_t* buffer, uint16_t size) {
    _bufferStart = buffer;
    _bufferEnd = buffer+size;
    _bufferSize = size;

    _readPointer = _bufferStart;
    _writePointer = _bufferStart;

    Log.info("BoxAudioBuffer init bufferStart=%X, _bufferEnd=%X, _bufferSize=%X, _readPointer=%X, _writePointer=%X", _bufferStart, _bufferEnd, _bufferSize, _readPointer, _writePointer);
    Log.info(" getBytesReadable()=%X, getBytesReadableBlock()=%X, getBytesWritable()=%X, getBytesWritableBlock()=%X", getBytesReadable(), getBytesReadableBlock(), getBytesWritable(), getBytesWritableBlock());
}

bool BoxAudioBuffer::readPointerEqualsWritePointer() {
    return _readPointer == _writePointer;
}

bool BoxAudioBuffer::isEmpty(uint16_t threshold) {
    return (getBytesReadable() <= threshold);
}
bool BoxAudioBuffer::isFull(uint16_t threshold) {
    return (getBytesWritable() <= threshold);
}

uint8_t* BoxAudioBuffer::getReadPointer() {
    return _readPointer;
}
void BoxAudioBuffer::updateReadPointer(uint16_t readLength) {
    uint16_t readableBlock = getBytesReadableBlock();
    if (readLength < readableBlock) {
        _readPointer += readLength;
    } else {
        _readPointer = _bufferStart + (readLength - readableBlock - 1);
    }
}

void BoxAudioBuffer::write(uint8_t* buffer, uint16_t size) {
    isWriting = true;
    uint16_t writableBlock = getBytesWritableBlock();
    //Log.info("write *buffer=%X, size=%i, writableBlock=%i", buffer, size, writableBlock);

    if (size <= writableBlock) {
        //Log.info(" _memcpy(%X, %X, %i);",  _writePointer, buffer, size);
        memcpy(_writePointer, buffer, size);
        _writePointer += size;
        if (_writePointer > _bufferEnd)
            _writePointer = _bufferStart;
    } else {
        //Log.info(" _memcpy(%X, %X, %i);",  _writePointer, buffer, writableBlock);
        memcpy(_writePointer, buffer, writableBlock);
        //Log.info(" _memcpy(%X, %X, %i);",  _bufferStart, buffer + writableBlock, size - writableBlock);
        memcpy(_bufferStart, buffer + writableBlock, size - writableBlock);
        _writePointer = _bufferStart + (size - writableBlock);
    }
    isWriting = false;
}


uint16_t BoxAudioBuffer::getBytesReadable() {
    return _measureDistance(_writePointer, _readPointer);
}
uint16_t BoxAudioBuffer::getBytesReadableBlock() {
    return _measureDistanceBlock(_writePointer, _readPointer);
}

uint16_t BoxAudioBuffer::getBytesWritable() {
    return _measureDistance(_readPointer, _writePointer);
}
uint16_t BoxAudioBuffer::getBytesWritableBlock() {
    return _measureDistanceBlock(_readPointer, _writePointer);
}

uint16_t BoxAudioBuffer::_measureDistance(uint8_t* p1, uint8_t* p2) {
    int32_t offset = p1 - p2;

    if (offset >= 0)
        return offset;
    
    return (_bufferEnd - p2 + 1) + (p1 - _bufferStart);
}
uint16_t BoxAudioBuffer::_measureDistanceBlock(uint8_t* p1, uint8_t* p2) {
    int32_t offset = p1 - p2;

    if (offset >= 0)
        return offset;
    
    return _bufferEnd - p2 + 1;
}