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

#include "BaseHeader.h"
#include <Arduino.h>
#include "AudioOutputResample.h"

AudioOutputResample::AudioOutputResample(uint32_t maxRate, AudioOutputCC3200I2S *dest)
{
  this->sink = dest;
  this->leftSample = 0;
  this->rightSample = 0;

  this->resampleFactor = 1;
  this->resampleCount = 0;

  originalSampleRate = this->sink->GetRate();

  SetMaxRate(maxRate);
}

AudioOutputResample::~AudioOutputResample() { }

bool AudioOutputResample::SetRate(int hz) {
  this->originalSampleRate = hz;
  if (this->maxSampleRate >= hz) {
    this->resampleFactor = 1;
    Log.info("AudioOutputResample SetRate=%i", hz);
    return this->sink->SetRate(hz);
  }

  for (this->resampleFactor = 2; this->resampleFactor < 7; this->resampleFactor++) { // 48000/8000
    if (this->maxSampleRate >= (hz / this->resampleFactor))
      break;
  }
    Log.info("AudioOutputResample limited SetRate=%i, hz=%i, resampleFactor=%i", hz / this->resampleFactor, hz, this->resampleFactor);
  return sink->SetRate(hz / this->resampleFactor);
}
void AudioOutputResample::SetMaxRate(uint32_t hz) {
  switch (hz)
  {
  case 48000:
  case 44100:
  case 32000:
  case 24000:
  case 22050:
  case 16000:
  case 11025:
  case 8000:
    this->maxSampleRate = hz;
    break;
  default:
    this->maxSampleRate = 48000;
  }
  SetRate(this->originalSampleRate);
}
uint32_t AudioOutputResample::GetMaxRate() {
  return this->maxSampleRate;
}
int AudioOutputResample::GetRate() {
  return this->sink->GetRate();
}

bool AudioOutputResample::SetBitsPerSample(int bits)
{
  return this->sink->SetBitsPerSample(bits);
}

bool AudioOutputResample::SetChannels(int channels)
{
  return this->sink->SetChannels(channels);
}

bool AudioOutputResample::begin()
{
  return this->sink->begin();
}

bool AudioOutputResample::ConsumeSample(int16_t sample[2])
{
  if (1==0) { //Slow
    this->leftSample += sample[0];
    this->rightSample += sample[1];
    this->resampleCount++;
    if (this->resampleCount >= this->resampleFactor) {
      int16_t s[2] = {(int16_t)(leftSample/this->resampleFactor), (int16_t)(rightSample/this->resampleFactor)};
      if (!sink->ConsumeSample(s)) {
        return false;
      } else {
        this->leftSample = 0;
        this->rightSample = 0;
        this->resampleCount = 0;
      }
    }
  } else { //Fast
    this->resampleCount++;
    if (this->resampleCount >= this->resampleFactor) {
      if (!sink->ConsumeSample(sample)) {
        return false;
      } else {
        this->resampleCount = 0;
      }
    }
  }
  return true;
}

bool AudioOutputResample::stop()
{
  return this->sink->stop();
}


