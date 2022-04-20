#ifndef BoxPlayer_h
#define BoxPlayer_h

#include "BaseHeader.h"

class BoxPlayer {
    public:
        enum class PLAYER_EVENT {
        };
        enum class PLAYER_MODE {
            NONE = 0x0,
            FILE = 0x1,
            DIR = 0x2
        };

        enum class PLAYER_FLAGS {
            NONE = 0x0,
            LOOP = 0x1,
            RANDOM = 0x2
        };
        void
            begin(),
            loop();
        
        void
            play(),
            pause(),
            stop(),
            next(),
            previous(),
            rewind(),
            songEnded();

        bool
            playDir(const char* path, PLAYER_FLAGS flags),
            playFile(const char* file, PLAYER_FLAGS flags);

        bool
            isPlaying(),
            isStopped();

    private:
        PLAYER_MODE _mode;
        PLAYER_FLAGS _flags;
        char _dirPath[256];
        uint8_t _currentSongId;
        uint8_t _dirSongCount;

        bool _play(const char* path);

};

#endif