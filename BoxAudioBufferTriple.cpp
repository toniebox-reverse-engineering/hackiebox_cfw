#include "BoxAudioBufferTriple.h"

void BoxAudioBufferTriple::init() {
    _bufferSize = ((_dataBufferSize / 3) / 8) * 4; // -> 0xAAA crashes, 0xAA8 not?!

    for (uint8_t i = 0; i < 3; i++) {
        _bufferStruct[i].index = i;
        _bufferStruct[i].buffer = (uint16_t*)(_dataBuffer + 2*i*_bufferSize); //bytes
        _bufferStruct[i].size = _bufferSize; //words
        for (uint16_t j = 0; j < _bufferStruct[i].size; j++)
            _bufferStruct[i].buffer[j] = 0x00;
    }

    _emptyStruct.index = 3;
    _emptyStruct.buffer = _emptyBuffer;
    _emptyStruct.size = _emptyBufferSize;
    _emptyStruct.state = BufferState::UNKNOWN;
    _bufferStruct[3] = _emptyStruct;


    for (uint16_t i = 0; i < _emptyStruct.size; i++)
        _emptyStruct.buffer[i] = random(INT8_MIN*4, INT8_MAX*4);

    _bufferStruct[0].state = BufferState::READY_FOR_READ;
    _bufferRead = _bufferStruct[0];
    _bufferStruct[1].state = BufferState::READY_FOR_WRITE;
    _bufferWrite = _bufferStruct[1];
    _bufferStruct[2].state = BufferState::READY_FOR_WRITE;
    _bufferWait = _bufferStruct[2];
    
}
void BoxAudioBufferTriple::logState() {
    Log.info("BoxAudioBufferTriple state");
    for (uint8_t i = 0; i < 4; i++) 
        logState(_bufferStruct[i]);
}

bool BoxAudioBufferTriple::flip(BoxAudioBufferTriple::BufferType type) {
    BufferStruct bufferTmp = _bufferWait;
    if (type == BufferType::READ && bufferTmp.state == BufferState::READY_FOR_READ) {
        _bufferRead.state = BufferState::READY_FOR_WRITE;
        _bufferWait = _bufferRead;
        _bufferRead = bufferTmp;
        return true;
    } else if (type == BufferType::WRITE && bufferTmp.state == BufferState::READY_FOR_WRITE) {
        _bufferWrite.state = BufferState::READY_FOR_READ;
        _bufferWait = _bufferWrite;
        _bufferWrite = bufferTmp;
        return true;
    }
    return false;
}

void BoxAudioBufferTriple::logState(BoxAudioBufferTriple::BufferStruct buffer) {
    Log.info(" [%i] buffer=%X, size=%X, state=%X", buffer.index, buffer.buffer, buffer.size, buffer.state);
}

BoxAudioBufferTriple::BufferStruct BoxAudioBufferTriple::getBuffer(BoxAudioBufferTriple::BufferType type) {
    switch (type) {
    case BufferType::READ:
        return _bufferRead;
    case BufferType::WRITE:
        return _bufferWrite;
    }
    return _emptyStruct;
}