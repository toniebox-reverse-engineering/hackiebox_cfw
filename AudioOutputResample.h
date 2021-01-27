/*
  AudioOutputResample
  Adds additional bufferspace to the output chain
  
  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _AUDIOOUTPUTRESAMPLE_H
#define _AUDIOOUTPUTRESAMPLE_H

#include <AudioOutput.h>
#include "AudioOutputCC3200I2S.h"

class AudioOutputResample : public AudioOutput
{
  public:
    AudioOutputResample(uint32_t maxSampleRate, AudioOutputCC3200I2S *dest);
    virtual ~AudioOutputResample() override;
    virtual bool SetRate(int hz) override;
    virtual bool SetBitsPerSample(int bits) override;
    virtual bool SetChannels(int channels) override;
    virtual bool begin() override;
    virtual bool ConsumeSample(int16_t sample[2]) override;
    virtual bool stop() override;

    int GetRate();
    uint32_t GetMaxRate();
    void SetMaxRate(uint32_t hz);
    
  protected:
    AudioOutputCC3200I2S *sink;
    int32_t leftSample;
    int32_t rightSample;
    uint32_t maxSampleRate;
    uint8_t resampleFactor;
    uint8_t resampleCount;

    uint32_t originalSampleRate;
};

#endif

