#ifndef LogStreamSse_h
#define LogStreamSse_h

#include <Stream.h>

class LogStreamSse : public Stream {
    public:
        size_t println();

        size_t write(uint8_t);
        int available() { return 0; };
        int read() { return 0; };
        int peek() { return 0; };
        void flush() { };

        void setSsePaused(bool paused);

    private:
        bool _ssePaused = true;
        bool _tagIsOpen = false;
};

#endif