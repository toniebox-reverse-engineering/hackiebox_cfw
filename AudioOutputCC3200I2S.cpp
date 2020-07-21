/*
  AudioOutputCC3200I2S
  Base class for I2S interface port
  
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


#include "AudioOutputCC3200I2S.h"

AudioOutputCC3200I2S::AudioOutputCC3200I2S(BoxAudioBufferTriple* audioBuffer)
{
  this->audioBuffer = audioBuffer;
  this->i2sOn = false;
  i2sOn = true;
  mono = false;
  bps = 16;
  channels = 2;
  SetGain(1.0);
  SetRate(16000); // Default
}

AudioOutputCC3200I2S::~AudioOutputCC3200I2S()
{
  i2sOn = false;
}

bool AudioOutputCC3200I2S::SetRate(int hz)
{
  this->hertz = hz;
  //TODO
  return true;
}

bool AudioOutputCC3200I2S::SetBitsPerSample(int bits)
{
  if ( (bits != 16) && (bits != 8) ) return false;
  this->bps = bits;
  return true;
}

bool AudioOutputCC3200I2S::SetChannels(int channels)
{
  if ( (channels < 1) || (channels > 2) ) return false;
  this->channels = channels;
  return true;
}

bool AudioOutputCC3200I2S::SetOutputModeMono(bool mono)
{
  this->mono = mono;
  return true;
}

bool AudioOutputCC3200I2S::ConsumeSample(int16_t sample[2])
{
  BoxAudioBufferTriple::BufferStruct* writeBuffer = audioBuffer->getBuffer(BoxAudioBufferTriple::BufferType::WRITE);
  if (writeBuffer->position >= writeBuffer->size) {
    if (audioBuffer->flip(BoxAudioBufferTriple::BufferType::WRITE)) {
      writeBuffer = audioBuffer->getBuffer(BoxAudioBufferTriple::BufferType::WRITE);
      writeBuffer->state = BoxAudioBufferTriple::BufferState::WRITING;
      writeBuffer->position = 0;
    } else {
      return false;
    }
  }

  int16_t ms[2];

  ms[0] = sample[0];
  ms[1] = sample[1];
  MakeSampleStereo16( ms );

  if (this->mono) {
    // Average the two samples and overwrite
    int32_t ttl = ms[LEFTCHANNEL] + ms[RIGHTCHANNEL];
    ms[LEFTCHANNEL] = ms[RIGHTCHANNEL] = (ttl>>1) & 0xffff;
  }

  writeBuffer->buffer[writeBuffer->position++] = ms[LEFTCHANNEL];
  writeBuffer->buffer[writeBuffer->position++] = ms[RIGHTCHANNEL];
}

void AudioOutputCC3200I2S::flush() {
  //Currently crashes?!
  if (!writeEmptyBuffer()) {
    writeEmptyBuffer();
  }
}

bool AudioOutputCC3200I2S::stop() {
  flush();
}

bool AudioOutputCC3200I2S::writeEmptyBuffer() {
  BoxAudioBufferTriple::BufferStruct* buffer = audioBuffer->getBuffer(BoxAudioBufferTriple::BufferType::WRITE);

  bool bufferEmpty = (buffer->position == 0);

  while (buffer->size < buffer->position) {
    buffer->buffer[buffer->position++] = 0x00;
  }
  while (!audioBuffer->flip(BoxAudioBufferTriple::BufferType::WRITE)) {}
  return bufferEmpty;
}