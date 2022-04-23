#ifndef LogStreamSd_h
#define LogStreamSd_h

#include <Stream.h>
#include <FatFs.h>

class LogStreamSd : public Stream {
    public:
        size_t println();

        size_t write(uint8_t character);
        size_t write(const uint8_t *buffer, size_t size);
        int available() { return 0; };
        int read() { return 0; };
        int peek() { return 0; };
        void flush() { };

    private:
        FileFs _file;
        bool _isOpen = false;
};

#endif