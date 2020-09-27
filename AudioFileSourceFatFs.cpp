/*
  AudioFileSourceSPIFFS
  Input SD card "file" to be used by AudioGenerator
  
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

#include "AudioFileSourceFatFs.h"

AudioFileSourceFatFs::AudioFileSourceFatFs()
{
  _isOpen = false;
}

AudioFileSourceFatFs::AudioFileSourceFatFs(const char *filename)
{
  _isOpen = false;
  open(filename);
}

bool AudioFileSourceFatFs::open(const char *filename)
{
  if (_isOpen) file.close();
  _isOpen = file.open((char*)filename, FA_OPEN_EXISTING | FA_READ);
  return _isOpen;
}

AudioFileSourceFatFs::~AudioFileSourceFatFs()
{
  if (_isOpen) file.close();
}

uint32_t AudioFileSourceFatFs::read(void *data, uint32_t len)
{
  return file.read(data, len);
}

bool AudioFileSourceFatFs::seek(int32_t pos, int dir)
{
  if (!_isOpen) return false;
  if (dir==SEEK_SET) return file.seekSet(pos);
  else if (dir==SEEK_CUR) return file.seekSet(file.curPosition() + pos);
  else if (dir==SEEK_END) return file.seekSet(file.fileSize() + pos);
  return false;
}

bool AudioFileSourceFatFs::close()
{
  _isOpen = false;
  return file.close();
}

bool AudioFileSourceFatFs::isOpen()
{
  return _isOpen;
}

uint32_t AudioFileSourceFatFs::getSize()
{
  if (!_isOpen) return 0;
  return file.fileSize();
}

uint32_t AudioFileSourceFatFs::getPos()
{
  if (!_isOpen) return 0;
  return file.curPosition();
}
