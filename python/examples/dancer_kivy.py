import struct
import random
import math
import time

import sonotopy
from audiostream import get_input

from kivy.app import App
from kivy.uix.widget import Widget
from kivy.clock import Clock
from kivy.graphics import *

TRACE_LIFETIME = 0.4
SPEED_FACTOR_MAX = 0.6
SPEED_FACTOR_MIN = 0.3
SPEED_OFFSET_MIN = -0.2
SPEED_OFFSET_MAX = 0

last_time = time.clock()

def random(v1=None, v2=None):
    """Returns a random value.
    
    This function does a lot of things depending on the parameters:
    - If one or more floats is given, the random value will be a float.
    - If all values are ints, the random value will be an integer.
    
    - If one value is given, random returns a value from 0 to the given value.
      This value is not inclusive.
    - If two values are given, random returns a value between the two; if two
      integers are given, the two boundaries are inclusive.
    """
    import random
    if v1 != None and v2 == None: # One value means 0 -> v1
        if isinstance(v1, float):
            return random.random() * v1
        else:
            return int(random.random() * v1)
    elif v1 != None and v2 != None: # v1 -> v2
        if isinstance(v1, float) or isinstance(v2, float):
            start = min(v1, v2)
            end = max(v1, v2)
            return start + random.random() * (end-start)
        else:
            start = min(v1, v2)
            end = max(v1, v2) + 1
            return int(start + random.random() * (end-start))
    else: # No values means 0.0 -> 1.0
        return random.random()

def mic_callback(buf):
    data_len = len(buf)/2
    data_int = struct.unpack("%dh"%(data_len), buf)
    data = sonotopy.floatArray(data_len)
    for i in range(data_len):
        data[i] = float(data_int[i])/32768.0

    demo.grid_map.feedAudio(data, data_len)
    demo.circle_map.feedAudio(data, data_len)
    demo.beat_tracker.feedFeatureVector(demo.spectrum_bin_divider.getBinValues())

    global last_time
    this_time = time.clock()
    diff_time = this_time - last_time
    for d in demo.dancers:
            d.update(diff_time)
    last_time = this_time

class Dancer():

    def __init__(self, circle_map, beat_tracker, window):
        self.circle_map = circle_map
        self.beat_tracker = beat_tracker
        self.window = window
        self.reset()

    def reset(self):
        self.speed = 0
        self.speed_factor = random(SPEED_FACTOR_MIN, SPEED_FACTOR_MAX)
        self.angle = 0
        self.angle_offset = random(0, 2 * math.pi)
        self.speed_offset = random(SPEED_OFFSET_MIN, SPEED_OFFSET_MAX)
        self.trace = []
        self.current_pos = {
        'x': random(0., 1.),
        'y': random(0., 1.),
        'start_time': 0 }
        self.current_time = 0

    def update(self, time_increment):
        self.angle = self.circle_map.getAngle() + self.angle_offset
        self.speed = (self.beat_tracker.getIntensity() + self.speed_offset) * self.speed_factor;
        aspect_ratio = float(self.window.height) / self.window.width;
        distance = self.speed * time_increment
        self.current_pos['x'] += math.cos(self.angle) * distance
        self.current_pos['y'] += math.sin(self.angle) * distance / aspect_ratio
        self.current_time += time_increment

    def render(self):
        self.updateTrace()
        self.renderTrace()
        if self.traceOutOfBounds():
            self.reset()

    def updateTrace(self):
        self.addCurrentPositionToTrace()
        self.removeOldTailFromTrace()

    def addCurrentPositionToTrace(self):
        p = {
            'x': self.current_pos['x'] * self.window.width,
            'y': self.current_pos['y'] * self.window.height,
            'start_time' : self.current_time
            }
        self.trace.append(p)

    def removeOldTailFromTrace(self):
        for i, t in enumerate(reversed(self.trace)):
            if (self.current_time - t['start_time']) > TRACE_LIFETIME:
                self.trace = self.trace[len(self.trace)-(i+1):]
                break

    def renderTrace(self):
        with self.window.canvas:
            if len(self.trace) > 1:
                for i, t in enumerate(self.trace[1:]):
                    col = float(i+1)/len(self.trace)
                    Color(col, col, col)
                    Line(points=[self.trace[i-1]['x'], self.trace[i-1]['y'],
                        self.trace[i]['x'], self.trace[i]['y']], width=2)

    def traceOutOfBounds(self):
        if len(self.trace) > 1:
            return self.outOfBounds(self.trace[0]) and \
                self.outOfBounds(self.trace[-1])
        return False

    def outOfBounds(self, p):
        if p['x'] < 0:
            return True
        if p['y'] < 0:
            return True
        if p['x'] > self.window.width:
            return True
        if p['y'] > self.window.height:
            return True
        return False

class Demo(App):

    def build(self):
        self.dancers = []
        self.audio_parameters = sonotopy.AudioParameters()
        self.grid_map_parameters = sonotopy.GridMapParameters()
        self.spectrum_analyzer_parameters = sonotopy.SpectrumAnalyzerParameters()
        self.circle_map_parameters = sonotopy.CircleMapParameters()

        self.grid_map = sonotopy.GridMap(self.audio_parameters,
            self.spectrum_analyzer_parameters, self.grid_map_parameters);
        self.circle_map = sonotopy.CircleMap(self.audio_parameters,
            self.spectrum_analyzer_parameters, self.circle_map_parameters);
        self.spectrum_bin_divider = self.grid_map.getSpectrumBinDivider();
        self.beat_tracker = sonotopy.BeatTracker(self.spectrum_bin_divider.getNumBins(),
            self.audio_parameters.bufferSize, self.audio_parameters.sampleRate)

        self.stream = get_input(callback=mic_callback)
        self.stream.start()

        Clock.schedule_interval(self.update, 1.0/60.0)

        return Widget()

    def on_start(self):
        for _ in range(40):
            d = Dancer(self.circle_map, self.beat_tracker, self.root)
            self.dancers.append(d)

    def update(self, dt):
        self.root.canvas.clear()
        for d in self.dancers:
            d.render()

demo = Demo()
demo.run()
