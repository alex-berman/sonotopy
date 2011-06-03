// Copyright (C) 2011 Alexander Berman
//
// Sonotopy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sonotopy/AudioParameters.hpp>
#include <portaudio.h>
#include <sndfile.h>

class AudioIO {
public:
  AudioIO();
  ~AudioIO();
  int audioCallback(float *inputBuffer, float *outputBuffer, unsigned long framesPerBuffer);

protected:
  void initializeAudio();
  void openAudioStream();
  void openAudioInputFile();
  void readAudioBufferFromFile();
  virtual void processAudio(float *) {}
  bool useAudioInputFile;
  char *audioInputFilename;
  PaStream *paStream;
  const char *audioDeviceName;
  float *spectrumMapInputBuffer;
  bool echoAudio;
  SNDFILE *audioInputFile;
  float *audioFileBuffer;
  sonotopy::AudioParameters audioParameters;
};
