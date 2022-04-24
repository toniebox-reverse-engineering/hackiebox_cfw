#include "BoxAudioBufferTriple.h"

void BoxAudioBufferTriple::init() {
    for (uint16_t i = 0; i < _dataBufferSize; i++)
        _dataBuffer[i] = 0x00;

    for (uint8_t i = 0; i < _bufferCount; i++) {
        _bufferStruct[i].index = i;
        _bufferStruct[i].buffer = (uint16_t*)(_dataBuffer + 2*i*_bufferSize); //bytes
        _bufferStruct[i].size = _bufferSize; //words
        _bufferStruct[i].position = 0;
        _bufferStruct[i].state = BufferState::READY_FOR_WRITE;
        _bufferStruct[i].next = &_bufferStruct[i+1];
    }
    _bufferStruct[0].state = BufferState::READY_FOR_READ;
    _bufferStruct[_bufferCount-1].next = &_bufferStruct[0];

    _bufferRead = &_bufferStruct[0];
    _bufferWrite = &_bufferStruct[1];
    
}
void BoxAudioBufferTriple::logState() {
    Log.info("BoxAudioBufferTriple state");
    for (uint8_t i = 0; i < _bufferCount; i++) 
        logState(&_bufferStruct[i]);
}

bool BoxAudioBufferTriple::flip(BoxAudioBufferTriple::BufferType type) {
    if (type == BufferType::READ) {
        if (_bufferRead->next->state == BufferState::READY_FOR_READ) {
            _bufferRead->state = BufferState::READY_FOR_WRITE;
            _bufferRead = _bufferRead->next;
            _bufferRead->position = 0;
            return true;
        }
    } else if (type == BufferType::WRITE) {
        if (_bufferWrite->next->state == BufferState::READY_FOR_WRITE) {
            _bufferWrite->state = BufferState::READY_FOR_READ;
            _bufferWrite = _bufferWrite->next;
            _bufferWrite->position = 0;
            return true;
        }
    }
    return false;
}

void BoxAudioBufferTriple::logState(BoxAudioBufferTriple::BufferStruct* buffer) {
    Log.info(" [%i] buffer=%X, size=%X, state=%X, nextBuffer=%X", buffer->index, buffer->buffer, buffer->size, buffer->state, buffer->next->buffer);
}

BoxAudioBufferTriple::BufferStruct* BoxAudioBufferTriple::getBuffer(BoxAudioBufferTriple::BufferType type) {
    switch (type) {
    case BufferType::READ:
        return _bufferRead;
    case BufferType::WRITE:
        return _bufferWrite;
    case BufferType::WAIT:
        return NULL;
    }
    return NULL;
}


bool BoxAudioBufferTriple::isFull() {
    if (_bufferWrite->next->state != BufferState::READY_FOR_WRITE) {
        return true;
    }
    return false;
}
bool BoxAudioBufferTriple::isEmpty() {
    if (_bufferRead->next->state != BufferState::READY_FOR_READ) {
        return true;
    }
    return false;
}