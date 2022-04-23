#ifndef LogStreamMulti_h
#define LogStreamMulti_h

#include <Stream.h>

#define LOG_STREAM_MULTI_MAX_SIZE 3
#define LOG_STREAM_MULTI_BUFFER_SIZE 256

class LogStreamMulti : public Stream {
    public:
        //LogStreamMulti();
        void setSlot(Stream* stream, uint8_t id);

        size_t println();

        size_t write(uint8_t character);
        size_t write(const uint8_t *buffer, size_t size);
        int available() { return 0; };
        int read() { return 0; };
        int peek() { return 0; };
        void flush();

    private:
        Stream* _streams[LOG_STREAM_MULTI_MAX_SIZE] = { };
        uint8_t _buffer[LOG_STREAM_MULTI_BUFFER_SIZE] = { };

        size_t _getBufferPosition();

};
#endif