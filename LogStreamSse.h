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

        bool isLineFinished();
        void setSsePaused(bool paused);

    private:
        bool _lineFinished = true;
        bool _ssePaused = true;
};

#endif