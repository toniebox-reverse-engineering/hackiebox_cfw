#ifndef LogStream_h
#define LogStream_h

#include <Stream.h>

class LogStream : public Stream {
    public:
        size_t println();

        size_t write(uint8_t);
        int available() {};
        int read(){};
        void flush(){};
        int peek(){};

        bool isLineFinished();
        void setSsePaused(bool paused);

    private:
        bool _lineFinished = true;
        bool _ssePaused = true;
};

#endif