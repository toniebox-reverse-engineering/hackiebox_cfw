#include "BoxPlayer.h"
#include "Hackiebox.h"

void BoxPlayer::begin() {  
    Log.info("Init Player...");
    _mode = PLAYER_MODE::NONE;
    _flags = PLAYER_FLAGS::NONE;
    _dirPath[0] = 0x00;
    Log.info("...done");
}

void BoxPlayer::loop() {  
   
}

void BoxPlayer::play() {
    Log.info("Player play");
    Box.boxDAC.play();
    Box.boxPower.feedSleepTimer();
}
void BoxPlayer::pause() {
    Log.info("Player pause");
    Box.boxDAC.pause();
    Box.boxPower.feedSleepTimer();
}
void BoxPlayer::stop() {
    Log.info("Player stop");
    Box.boxDAC.stop();
    Box.boxPower.feedSleepTimer();
}
void BoxPlayer::rewind() {
    //TODO
}

void BoxPlayer::songEnded() {
    Log.info("Song ended with mode=%X and flags=%X...", _mode, _flags);
    if (_mode == PLAYER_MODE::FILE) {
        if (_flags == PLAYER_FLAGS::LOOP) {
            rewind();
        } else {
            stop();
        }
    } else if (_mode == PLAYER_MODE::DIR) {
        _currentSongId++;
        if (_currentSongId >= _dirSongCount) {
            if (_flags == PLAYER_FLAGS::LOOP) {
                _currentSongId = 0;
            } else {
                Log.info("Played all %i songs from dir %s", _dirSongCount, _dirPath);
                stop();
                return;
            }
        }
        
        DirFs dir;
        if (dir.openDir(_dirPath)) {
            char songPath[256];
            uint8_t songIndex = 0;
            while (dir.nextFile()) {
                if (dir.isDir())
                    continue;
                if (_currentSongId == songIndex) {
                    Log.info("Playing next song in dir %s with id %i...", _dirPath, _currentSongId);
                    sprintf(songPath,"%s/%s", _dirPath, (const char*)dir.fileName());
                }
                songIndex++;
            }
            dir.closeDir();
            if (songIndex > 0)
                _play(songPath);
        } else {
            Log.error("Player could not open dir %s...", _dirPath);
        }
    }
}

bool BoxPlayer::_play(const char* path) {
    Box.boxPower.feedSleepTimer();
    return Box.boxDAC.playFile(path);
}

bool BoxPlayer::playDir(const char* path, PLAYER_FLAGS flags) {
    Log.info("Playing dir %s", _dirPath);

    char songPath[256];

    _mode = BoxPlayer::PLAYER_MODE::DIR;
    _flags = flags;
    _currentSongId = 0;
    _dirSongCount = 0;
    memcpy((char*)_dirPath, (const char*)path, 256);

    DirFs dir;
    if (dir.openDir(_dirPath)) {
        while (dir.nextFile()) {
            if (dir.isDir())
                continue;
            if (_dirSongCount == 0) {
                sprintf(songPath,"%s/%s",_dirPath, (const char*)dir.fileName());
            }
            _dirSongCount++;
        }
        dir.closeDir();
        if (_dirSongCount > 0) {
            return _play(songPath);
        } else {
            Log.error("Player could not find songs to play...");
        }
    } else {
        Log.error("Player could not open dir %s...", _dirPath);
    }
    return false;
}
bool BoxPlayer::playFile(const char* file, PLAYER_FLAGS flags) {
    _mode = PLAYER_MODE::FILE;
    _flags = flags;
    Log.info("Playing file %s", file);
    return _play(file);
}

bool BoxPlayer::isPlaying() {
    return (Box.boxDAC.audioPlaying);
}
bool BoxPlayer::isStopped() {
    return (Box.boxDAC.hasStopped());
}