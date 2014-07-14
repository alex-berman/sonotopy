import struct

import sonotopy
from audiostream import get_input
import numpy

from kivy.app import App
from kivy.uix.widget import Widget
from kivy.clock import Clock

def mic_callback(buf):
    #a = numpy.fromstring(buf, dtype="<h")
    #a = a/32768.0
    #a.dtype = numpy.float32
    #data = a.astype(numpy.float32).tostring()
    
    data_len = len(buf)/2
    data_int = struct.unpack("%dh"%(data_len), buf)
    data = sonotopy.floatArray(data_len)
    for i in range(data_len):
        data[i] = float(data_int[i])/32768.0

    demo.grid_map.feedAudio(data, data_len)
    demo.beat_tracker.feedFeatureVector(demo.spectrum_bin_divider.getBinValues())

class Demo(App):
    stream = None
    audiobuffer = None
    audio_parameters = None
    grid_map_parameters = None
    spectrum_analyzer_parameters = None
    beat_tracker = None
    grid_map = None
    spectrum_bin_divider = None

    def build(self):
        self.audio_parameters = sonotopy.AudioParameters()
        #self.audio_parameters.bufferSize = 4096
        self.grid_map_parameters = sonotopy.GridMapParameters()
        self.spectrum_analyzer_parameters = sonotopy.SpectrumAnalyzerParameters()

        self.grid_map = sonotopy.GridMap(self.audio_parameters,
            self.spectrum_analyzer_parameters, self.grid_map_parameters);
        self.spectrum_bin_divider = self.grid_map.getSpectrumBinDivider();
        self.beat_tracker = sonotopy.BeatTracker(self.spectrum_bin_divider.getNumBins(),
            self.audio_parameters.bufferSize, self.audio_parameters.sampleRate)

        self.stream = get_input(callback=mic_callback)
        self.stream.start()

        #self.update(None)
        Clock.schedule_interval(self.update, 1.0/60.0)

        return Widget()

    def update(self, dt):
        print(self.beat_tracker.getIntensity())

demo = Demo()
demo.run()
