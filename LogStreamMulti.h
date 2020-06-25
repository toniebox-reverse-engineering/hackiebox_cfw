#ifndef LogStreamMulti_h
#define LogStreamMulti_h

#include <Stream.h>

class LogStreamMulti : public Stream {
    public:
        //LogStreamMulti();
        void setSlot(Stream* stream, uint8_t id);

        size_t println();

        size_t write(uint8_t character);
        size_t write(const uint8_t *buffer, size_t size);
        int available() {};
        int read(){};
        void flush(){};
        int peek(){};

    private:
        Stream* _streams[2];
};
#endif