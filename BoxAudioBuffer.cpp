#include "BoxAudioBuffer.h"

void BoxAudioBuffer::init(uint8_t* buffer, uint16_t size) {
    _bufferStart = buffer;
    _bufferEnd = buffer+size-1;
    _bufferSize = size;

    _readPointer = _bufferStart;
    _writePointer = _bufferStart;

    _isFull = false;
    _isEmpty = true;
}

void BoxAudioBuffer::logState() {
    //noIRQ = true;
    Log.info("BoxAudioBuffer bufferStart=%X, _bufferEnd=%X, _bufferSize=%X, _readPointer=%X, _writePointer=%X", _bufferStart, _bufferEnd, _bufferSize, _readPointer, _writePointer);
    Log.info(" getBytesReadable()=%X, getBytesReadableBlock()=%X, getBytesWritable()=%X, getBytesWritableBlock()=%X", getBytesReadable(), getBytesReadableBlock(), getBytesWritable(), getBytesWritableBlock());
    //noIRQ = false;
}

bool BoxAudioBuffer::readPointerEqualsWritePointer() {
    return _readPointer == _writePointer;
}

bool BoxAudioBuffer::isEmpty(uint16_t threshold) {
    if (getBytesReadable() <= threshold) {
        if (threshold == 0) {
            return _isEmpty;
        }
        return true;
    }
    return false;
}
bool BoxAudioBuffer::isFull(uint16_t threshold) {
    if (getBytesWritable() <= threshold) {
        if (threshold == 0) {
            return _isFull;
        }
        return true;
    }
    return false;
}

uint8_t* BoxAudioBuffer::getReadPointer() {
    return _readPointer;
}
void BoxAudioBuffer::updateReadPointer(uint16_t readLength) {
    uint16_t readableBlock = getBytesReadableBlock();
    if (readLength < readableBlock) {
        _readPointer += readLength;
    } else {
        _readPointer = _bufferStart + (readLength - readableBlock);
    }
    if (readPointerEqualsWritePointer()) {
        _isFull = false;
        _isEmpty = true;
    }
}

void BoxAudioBuffer::write(uint8_t* buffer, uint16_t size) {
    noIRQ = true;
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
    if (readPointerEqualsWritePointer()) {
        _isEmpty = false;
        _isFull = true;
    }
    noIRQ = false;
}


uint16_t BoxAudioBuffer::getBytesReadable() {
    if (readPointerEqualsWritePointer() && _isEmpty)
        return 0;
    return _measureDistance(_writePointer, _readPointer);
}
uint16_t BoxAudioBuffer::getBytesReadableBlock() {
    if (readPointerEqualsWritePointer() && _isEmpty)
        return 0;
    return _measureDistanceBlock(_writePointer, _readPointer);
}

uint16_t BoxAudioBuffer::getBytesWritable() {
    if (readPointerEqualsWritePointer() && _isFull)
        return 0;
    return _measureDistance(_readPointer, _writePointer);
}
uint16_t BoxAudioBuffer::getBytesWritableBlock() {
    if (readPointerEqualsWritePointer() && _isFull)
        return 0;
    return _measureDistanceBlock(_readPointer, _writePointer);
}

uint16_t BoxAudioBuffer::_measureDistance(uint8_t* p1, uint8_t* p2) {
    int32_t offset = p1 - p2;

    if (offset > 0)
        return offset;
    
    return (_bufferEnd - p2 + 1) + (p1 - _bufferStart);
}
uint16_t BoxAudioBuffer::_measureDistanceBlock(uint8_t* p1, uint8_t* p2) {
    int32_t offset = p1 - p2;

    if (offset > 0)
        return offset;
    
    return _bufferEnd - p2 + 1;
}