#include "BoxAudioBufferTriple.h"

void BoxAudioBufferTriple::init() {
    _bufferSize = ((_dataBufferSize / 3) / 8) * 4; // -> 0xAAA crashes, 0xAA8 not?!

    for (uint8_t i = 0; i < 3; i++) {
        _bufferStruct[i].index = i;
        _bufferStruct[i].buffer = (uint16_t*)(_dataBuffer + 2*i*_bufferSize); //bytes
        _bufferStruct[i].size = _bufferSize; //words
        _bufferStruct[i].state = BufferState::READY_FOR_WRITE;
    }

    _emptyStruct.index = 3;
    _emptyStruct.buffer = _emptyBuffer;
    _emptyStruct.size = _emptyBufferSize;
    _emptyStruct.state = BufferState::UNKNOWN;
    _bufferStruct[3] = _emptyStruct;


    for (uint16_t i = 0; i < _emptyStruct.size; i++)
        _emptyStruct.buffer[i] = random(INT8_MIN*4, INT8_MAX*4);
    
}
void BoxAudioBufferTriple::logState() {
    Log.info("BoxAudioBufferTriple state");
    for (uint8_t i = 0; i < 4; i++) 
        logState(_bufferStruct[i]);
}

void BoxAudioBufferTriple::logState(BoxAudioBufferTriple::BufferStruct buffer) {
    Log.info(" [%i] buffer=%X, size=%X, state=%X", buffer.index, buffer.buffer, buffer.size, buffer.state);
}

BoxAudioBufferTriple::BufferStruct BoxAudioBufferTriple::getBuffer(BoxAudioBufferTriple::BufferState state) {
    for (uint8_t i = 0; i < 3; i++) {
        if (_bufferStruct[i].state == state)
            return _bufferStruct[i];
    }
    return _emptyStruct;
}
void BoxAudioBufferTriple::setBufferState(BoxAudioBufferTriple::BufferState state, uint8_t index) {
    if (index < 3)
        _bufferStruct[index].state = state;
}