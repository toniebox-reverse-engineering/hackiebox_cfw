#include "LogStreamMulti.h"

//LogStreamMulti::LogStreamMulti(): Stream() {}
void LogStreamMulti::setSlot(Stream* stream, uint8_t id) {
    if (id < LOG_STREAM_MULTI_MAX_SIZE) {
        flush();
        _streams[id] = stream;
    }
}

size_t LogStreamMulti::write(uint8_t character) {
    return write(&character, 1);
}
size_t LogStreamMulti::write(const uint8_t *buffer, size_t size) {
    size_t position = 0;

    if (_getBufferPosition() + size > LOG_STREAM_MULTI_BUFFER_SIZE - 1) {
        flush();
    
        size_t part_size;
        while (size - position > LOG_STREAM_MULTI_BUFFER_SIZE - 1) {
            part_size = LOG_STREAM_MULTI_BUFFER_SIZE - 1;
            memcpy(&_buffer[0], buffer+position, part_size);
            position += part_size;
            flush();
        }
    }
    memcpy(&_buffer[_getBufferPosition()], buffer+position, size-position);

    for (uint8_t i=position; i<size-position; i++) {
        if (_buffer[i] == '\n') {
            flush();
            break;
        }
    }

    size_t result = 0; //TODO
    return size;
}
size_t LogStreamMulti::println() {
    size_t result = 0; //TODO
    for (uint8_t i = 0; i < LOG_STREAM_MULTI_MAX_SIZE; i++) {
        if (_streams[i] == 0)
            continue;
        result = _streams[i]->println();//("\r\n");
    }
    flush();
    return 2;
}

void LogStreamMulti::flush() {
    for (uint8_t i = 0; i < LOG_STREAM_MULTI_MAX_SIZE; i++) {
        if (_streams[i] == 0)
            continue;
        _streams[i]->write((const uint8_t*)&_buffer, _getBufferPosition());
        _streams[i]->flush();
    }
    memset(_buffer, '\0', LOG_STREAM_MULTI_BUFFER_SIZE);
}

size_t LogStreamMulti::_getBufferPosition() {
    return strlen((const char*)_buffer);
}