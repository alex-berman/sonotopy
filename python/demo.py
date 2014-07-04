import struct
from ctypes import *

import sonotopy
import pyaudio
import numpy

from kivy.app import App
from kivy.uix.widget import Widget
from kivy.clock import Clock

FORMAT = pyaudio.paFloat32
CHANNELS = 1

c_float_p = POINTER(c_float)

def array_ctypes(points):
    n = len(points)
    return (c_float_p*n)(*[p for p in points])

class Demo(App):
    stream = None
    audio_parameters = None
    grid_map_parameters = None
    spectrum_analyzer_parameters = None
    beat_tracker = None
    grid_map = None
    spectrum_bin_divider = None

    def build(self):
        self.audio_parameters = sonotopy.AudioParameters()
        self.grid_map_parameters = sonotopy.GridMapParameters()
        self.spectrum_analyzer_parameters = sonotopy.SpectrumAnalyzerParameters()

        self.grid_map = sonotopy.GridMap(self.audio_parameters,
            self.spectrum_analyzer_parameters, self.grid_map_parameters);
        self.spectrum_bin_divider = self.grid_map.getSpectrumBinDivider();
        self.beat_tracker = sonotopy.BeatTracker(self.spectrum_bin_divider.getNumBins(),
            self.audio_parameters.bufferSize, self.audio_parameters.sampleRate)

        if self.stream and self.stream.is_active():
            self.stream.close()
        self.stream = None

        p = pyaudio.PyAudio()
        self.stream = p.open(format = FORMAT,
            channels = CHANNELS,
            rate = self.audio_parameters.sampleRate,
            input = True,
            frames_per_buffer = int(self.audio_parameters.bufferSize))

        #self.update(None)
        Clock.schedule_interval(self.update, 1.0)

        return Widget()

    def update(self, dt):
        data = None
        try:
            data = self.stream.read(self.audio_parameters.bufferSize)
        except IOError as ex:
            if ex[1] != pyaudio.paInputOverflowed:
                raise
            print "error read step 1"
        if (data):
            self.grid_map.feedAudio(data, self.audio_parameters.bufferSize)
            self.beat_tracker.feedFeatureVector(self.spectrum_bin_divider.getBinValues())
            print(self.beat_tracker.getIntensity())

Demo().run()
