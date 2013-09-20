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

#include "AudioIO.hpp"
#include <stdlib.h>
#include <string.h>

AudioIO::AudioIO() {
  monauralInputBuffer = NULL;
  audioFileBuffer = NULL;
  audioDeviceName = NULL;
  videoExportEnabled = false;
}

AudioIO::~AudioIO() {
  if(monauralInputBuffer) delete monauralInputBuffer;
  if(useAudioInputFile) sf_close(audioInputFile);
  if(audioFileBuffer) delete audioFileBuffer;
}

void AudioIO::initializeAudio() {
  PaError err;
  if(useAudioInputFile) openAudioInputFile();

  monauralInputBuffer = new float [audioParameters.bufferSize];
  bzero(monauralInputBuffer, sizeof(float) * audioParameters.bufferSize);

  if(!videoExportEnabled) {
    err = Pa_Initialize();
    if( err != paNoError ) {
      printf("failed to initialize portaudio\n");
      exit(0);
    }
    if(!useAudioInputFile) {
      printf("listening to audio device\n");
    }
  }
}

static int AudioIO_portaudioCallback(
  const void *inputBuffer,
  void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo *timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData)
{
  AudioIO *audioIO = (AudioIO *)userData;
  return audioIO->audioCallback((float *) inputBuffer, (float *) outputBuffer, framesPerBuffer);
}

void AudioIO::openAudioStream() {
  if(!videoExportEnabled)
    portaudioOpenAudioStream();
}

void AudioIO::portaudioOpenAudioStream() {
  int numInputChannels = 2;
  int numOutputChannels = echoAudio ? 2 : 0;
  int audioDeviceId = 0;
  PaStreamParameters outputParameters;
  PaStreamParameters inputParameters;
  PaError err;
  void *callbackUserData = (void *) this;

  const PaDeviceInfo *deviceInfo;
  int numDevices = Pa_GetDeviceCount();

  if(audioDeviceName != NULL) {
    for(int i=0; i<numDevices; i++) {
      deviceInfo = Pa_GetDeviceInfo(i);
      if(strncmp(audioDeviceName, deviceInfo->name, strlen(audioDeviceName)) == 0)
	audioDeviceId = i;
    }
  }
  for(int i=0; i<numDevices; i++) {
    deviceInfo = Pa_GetDeviceInfo(i);
    printf("device %d%s: '%s', %d in, %d out\n",
      i, (i == audioDeviceId) ? " SELECTED" : "",
      deviceInfo->name, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
  }

  bzero(&inputParameters, sizeof(inputParameters));
  inputParameters.channelCount = numInputChannels;
  inputParameters.device = audioDeviceId;
  inputParameters.sampleFormat = paFloat32;

  if(echoAudio) {
    bzero(&outputParameters, sizeof(outputParameters));
    outputParameters.channelCount = numOutputChannels;
    outputParameters.device = audioDeviceId;
    outputParameters.sampleFormat = paFloat32;
  }

  printf("opening audio stream: sample rate %d, buffer size %d\n",
	 audioParameters.sampleRate,
	 audioParameters.bufferSize);

  err = Pa_OpenStream(
    &paStream,
    &inputParameters,
    echoAudio ? &outputParameters : NULL,
    audioParameters.sampleRate,
    audioParameters.bufferSize,
    paNoFlag,
    AudioIO_portaudioCallback,
    callbackUserData
  );

  if( err != paNoError ) {
    printf("failed to open audio stream (%s)\n", Pa_GetErrorText(err));
    exit(0);
  }

  if(Pa_StartStream( paStream ) != paNoError ) {
    printf("failed to start stream\n");
    exit(0);
  }
}

int AudioIO::audioCallback(float *inputBuffer, float *outputBuffer, unsigned long framesPerBuffer) {
  if(framesPerBuffer != audioParameters.bufferSize) {
    printf("expected %ld frames in audio callback, got %ld\n", audioParameters.bufferSize, framesPerBuffer);
    exit(0);
  }

  static float *inputPtr;
  static float *monauralInputBufferPtr;
  if(useAudioInputFile) {
    readAudioBufferFromFile();
    inputPtr = audioFileBuffer;
  }
  else {
    monauralInputBufferPtr = monauralInputBuffer;
    inputPtr = inputBuffer;
    for(unsigned long i = 0; i < audioParameters.bufferSize; i++) {
      *monauralInputBufferPtr++ = *inputPtr;
      inputPtr += 2;
    }
    inputPtr = inputBuffer;
  }

  float *outputPtr = (float *) outputBuffer;
  for(unsigned long i = 0; i < audioParameters.bufferSize; i++) {
    if(echoAudio) {
      *outputPtr++ = *inputPtr++;
      *outputPtr++ = *inputPtr++;
    }
    else {
      inputPtr += 2;
    }
  }

  processAudio(monauralInputBuffer);

  return 0;
}

void AudioIO::openAudioInputFile() {
  SF_INFO sfinfo;
  if(!(audioInputFile = sf_open(audioInputFilename, SFM_READ, &sfinfo))) {
    printf("failed to open audio input file %s\n", audioInputFilename);
    exit(0);
  }
  if(sfinfo.samplerate != (int) audioParameters.sampleRate) {
    printf("expected sample rate %d\n", audioParameters.sampleRate);
    exit(0);
  }
  if(sfinfo.channels != 2) {
    printf("expected stereo audio\n");
    exit(0);
  }
  audioFileBuffer = new float [audioParameters.bufferSize * 2];
}

void AudioIO::readAudioBufferFromFile() {
  float *audioFileBufferPtr = audioFileBuffer;
  float *monauralInputBufferPtr = monauralInputBuffer;
  int framesLeft = audioParameters.bufferSize;
  while(framesLeft > 0) {
    int framesRead = sf_readf_float(audioInputFile, audioFileBufferPtr, framesLeft);
    if(framesRead < framesLeft) {
      if(videoExportEnabled) {
	printf("export finished\n");
	exit(0);
      }
      else
	rewindAudioInputFile();
    }
    framesLeft -= framesRead;
    for(int i = 0; i < framesRead; i++) {
      *monauralInputBufferPtr++ = *audioFileBufferPtr;
      audioFileBufferPtr += 2;
    }
  }
}

void AudioIO::rewindAudioInputFile() {
  sf_seek(audioInputFile, 0, SEEK_SET);
}

void AudioIO::audioEnableVideoExport() {
  videoExportEnabled = true;
}
