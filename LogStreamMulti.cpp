#include "LogStreamMulti.h"

//LogStreamMulti::LogStreamMulti(): Stream() {}
void LogStreamMulti::setSlot(Stream* stream, uint8_t id) {
    if (id < sizeof(_streams))
        _streams[id] = stream;
}

size_t LogStreamMulti::write(uint8_t character) {
    write(&character, 1);
}
size_t LogStreamMulti::write(const uint8_t *buffer, size_t size) {
    size_t result = 0; //TODO
    for (uint8_t i = 0; i < sizeof(_streams); i++) {
        result = _streams[i]->write(buffer, size);
    }

    return result;
}
size_t LogStreamMulti::println() {
    size_t result = 0; //TODO
    for (uint8_t i = 0; i < sizeof(_streams); i++) {
        result = _streams[i]->print("\r\n");
    }
    return result;
}