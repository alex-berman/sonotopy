import time
import struct
import math
import array

from PyQt5.QtMultimedia import *
from PyQt5 import QtCore, QtGui, QtWidgets

import sonotopy

TRACE_LIFETIME = 0.4
SPEED_FACTOR_MAX = 1.2
SPEED_FACTOR_MIN = 0.6
SPEED_OFFSET_MIN = -0.4
SPEED_OFFSET_MAX = 0

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

def gradient(colors, steps):
    colors_per_step = float(steps) / len(colors) + 1
    #num_colors = int(colors_per_step) * len(colors)
    gradient = []
    for i, color in enumerate(colors):
        # start color...
        r1 = color[0]
        g1 = color[1]
        b1 = color[2]
 
        # end color...
        color2 = colors[(i + 1) % len(colors)]
        r2 = color2[0]
        g2 = color2[1]
        b2 = color2[2]
 
        # generate a gradient of one step from color to color:
        delta = 1.0 / colors_per_step
        for j in range(int(colors_per_step)):
            t = j * delta
            a = 1.0
            r = (1.0 - t) * r1 + t * r2
            g = (1.0 - t) * g1 + t * g2
            b = (1.0 - t) * b1 + t * b2
            gradient.append(r)
            gradient.append(g)
            gradient.append(b)
 
    return gradient


class Dancer():

    def __init__(self, circle_map, beat_tracker, widget):
        self.circle_map = circle_map
        self.beat_tracker = beat_tracker
        self.widget = widget
        self.reset()

    def reset(self):
        self.speed = 0
        self.speed_factor = random(SPEED_FACTOR_MIN, SPEED_FACTOR_MAX)
        self.angle = 0
        self.angle_offset = random(0, 2 * math.pi)
        self.speed_offset = random(SPEED_OFFSET_MIN, SPEED_OFFSET_MAX)
        self.trace = []
        self.current_pos = {
        'x': random(-1., 1.),
        'y': random(-1., 1.),
        'start_time': 0 }
        self.current_time = 0

    def update(self, time_increment):
        self.angle = self.circle_map.getAngle() + self.angle_offset
        self.speed = (self.beat_tracker.getIntensity() + self.speed_offset) * self.speed_factor;
        aspect_ratio = float(self.widget.height()) / self.widget.width();
        distance = self.speed * time_increment
        self.current_pos['x'] += math.cos(self.angle) * distance
        self.current_pos['y'] += math.sin(self.angle) * distance / aspect_ratio
        self.current_time += time_increment

    def prepareRender(self):
        self.updateTrace()
        #self.renderTrace(gl)
        if self.traceOutOfBounds():
            self.reset()

    def updateTrace(self):
        self.addCurrentPositionToTrace()
        self.removeOldTailFromTrace()

    def addCurrentPositionToTrace(self):
        p = {
            'x': self.current_pos['x'],
            'y': self.current_pos['y'],
            'start_time' : self.current_time
            }
        self.trace.append(p)

    def vertices(self):
        verts = []
        for t in self.trace:
            verts.append(t['x'])
            verts.append(t['y'])
        return verts

    def removeOldTailFromTrace(self):
        for i, t in enumerate(reversed(self.trace)):
            if (self.current_time - t['start_time']) > TRACE_LIFETIME:
                self.trace = self.trace[len(self.trace)-(i+1):]
                break

    def renderTrace(self, gl):
        if len(self.trace) == 0:
            return

        gl.glShadeModel(gl.GL_SMOOTH)
        gl.glLineWidth(2.0)

        gl.glBegin(gl.GL_LINE_STRIP)
        gl.glColor3f(0, 0, 0)
        gl.glVertex2f(self.trace[0]['x'], self.trace[0]['y'])

        if len(self.trace) > 1:
            for i, t in enumerate(self.trace[1:]):
                col = float(i+1)/len(self.trace)
                gl.glColor3f(col, col, col)
                gl.glVertex2f(t['x'], t['y'])

        gl.glEnd()

    def traceOutOfBounds(self):
        if len(self.trace) > 1:
            return self.outOfBounds(self.trace[0]) and \
                self.outOfBounds(self.trace[-1])
        return False

    def outOfBounds(self, p):
        if p['x'] < -1:
            return True
        if p['y'] < -1:
            return True
        if p['x'] > 1:
            return True
        if p['y'] > 1:
            return True
        return False


class OpenGLWindow(QtGui.QWindow):
    def __init__(self, parent=None):
        super(OpenGLWindow, self).__init__(parent)

        self.m_update_pending = False
        self.m_animating = False
        self.m_context = None
        self.m_gl = None

        self.setSurfaceType(QtGui.QWindow.OpenGLSurface)

    def initialize(self):
        pass

    def setAnimating(self, animating):
        self.m_animating = animating

        if animating:
            self.renderLater()

    def renderLater(self):
        if not self.m_update_pending:
            self.m_update_pending = True
            QtGui.QGuiApplication.postEvent(self, QtCore.QEvent(QtCore.QEvent.UpdateRequest))

    def renderNow(self):
        if not self.isExposed():
            return

        self.m_update_pending = False

        needsInitialize = False

        if self.m_context is None:
            self.m_context = QtGui.QOpenGLContext(self)
            self.m_context.setFormat(self.requestedFormat())
            self.m_context.create()

            needsInitialize = True

        self.m_context.makeCurrent(self)

        if needsInitialize:
            version = QtGui.QOpenGLVersionProfile()
            version.setVersion(2, 0)
            self.m_gl = self.m_context.versionFunctions(version)
            self.m_gl.initializeOpenGLFunctions()

            self.initialize()

        self.render(self.m_gl)

        self.m_context.swapBuffers(self)

        if self.m_animating:
            self.renderLater()

    def event(self, event):
        if event.type() == QtCore.QEvent.UpdateRequest:
            self.renderNow()
            return True

        return super(OpenGLWindow, self).event(event)

    def exposeEvent(self, event):
        self.renderNow()

    def resizeEvent(self, event):
        self.renderNow()


class DancerWindow(OpenGLWindow):

    colors = [ [0.0, 0.0, 0.0, 1.0], [0.4, 0.4, 0.4, 1.0], [1.0, 0.0, 0.0, 1.0] ]
    
    vertexShaderSource = '''
attribute highp vec2 posAttr;
attribute lowp vec4 colAttr;
varying lowp vec4 col;

void main() {
    col = colAttr;
    gl_Position = vec4(posAttr, 0.0, 1.0);
}
'''

    fragmentShaderSource = '''
varying lowp vec4 col;
void main() {
    gl_FragColor = col;
}
'''

    def __init__(self, parent=None):
        super(DancerWindow, self).__init__(parent)

        self.last_time = time.clock()
        self.dancers = []

        self.init_sonotopy()
        self.init_audio()
        self.init_dancers()

    def initialize(self):
        self.program = QtGui.QOpenGLShaderProgram(self)

        self.program.addShaderFromSourceCode(QtGui.QOpenGLShader.Vertex,
                self.vertexShaderSource)
        self.program.addShaderFromSourceCode(QtGui.QOpenGLShader.Fragment,
                self.fragmentShaderSource)

        self.program.link()

        self.posAttr = self.program.attributeLocation('posAttr')
        self.colAttr = self.program.attributeLocation('colAttr')

    def init_sonotopy(self):
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

    def init_audio(self):
        format = QAudioFormat()
        format.setSampleRate(self.audio_parameters.sampleRate)
        format.setChannelCount(1)
        format.setSampleSize(16)
        format.setCodec("audio/pcm")
        format.setByteOrder(QAudioFormat.LittleEndian);
        format.setSampleType(QAudioFormat.SignedInt);

        info = QAudioDeviceInfo.defaultInputDevice()
        print("Using audio device: {}".format(info.deviceName()))
        if not info.isFormatSupported(format):
            print("Changing to nearest format")
            format = info.nearestFormat(format)

        self.buffer = QtCore.QBuffer()
        self.buffer.open(QtCore.QIODevice.ReadWrite)
        self.buffer.bytesWritten.connect(self.callback)

        self.audio = QAudioInput(format)
        self.audio.setBufferSize(2048)
        self.audio.start(self.buffer)

    def init_dancers(self):
        for _ in range(40):
            d = Dancer(self.circle_map, self.beat_tracker, self)
            self.dancers.append(d)

    def event(self, event):
        if event.type() == QtCore.QEvent.Close:
            self.audio.stop()
            self.buffer.close()
            return True

        return super(DancerWindow, self).event(event)

    def callback(self, bytes):
        data_len = int(self.buffer.size()/2)
        data_bytes = self.buffer.data().data()
        self.buffer.reset()

        data_int = struct.unpack("%dh"%(data_len), data_bytes)
        data = sonotopy.floatArray(data_len)
        for i in range(data_len):
            data[i] = float(data_int[i])/32768.0

        self.grid_map.feedAudio(data, data_len)
        self.circle_map.feedAudio(data, data_len)
        self.beat_tracker.feedFeatureVector(self.spectrum_bin_divider.getBinValues())

        this_time = time.clock()
        diff_time = this_time - self.last_time
        for d in self.dancers:
            d.update(diff_time)
        self.last_time = this_time

    def render(self, gl):
        gl.glViewport(0, 0, self.width(), self.height())
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        gl.glLoadIdentity()
        gl.glShadeModel(gl.GL_SMOOTH)
        gl.glLineWidth(2.0)

        self.program.bind()

        for d in self.dancers:
            d.prepareRender()
            verts = d.vertices()
            if len(verts) > 0:
                vertices = array.array('f', verts)

                gl.glVertexAttribPointer(self.posAttr, 2, gl.GL_FLOAT, False, 0,
                    vertices)
                gl.glEnableVertexAttribArray(self.posAttr)

                cols = gradient(self.colors, len(d.trace))
                colors = array.array('f', cols)

                gl.glVertexAttribPointer(self.colAttr, 3, gl.GL_FLOAT, False, 0,
                    colors)
                gl.glEnableVertexAttribArray(self.colAttr)

                gl.glDrawArrays(gl.GL_LINE_STRIP, 0, len(d.trace))

        self.program.release()


if __name__ == '__main__':
    import sys
 
    app = QtWidgets.QApplication(sys.argv)
    
    format = QtGui.QSurfaceFormat()
    format.setSamples(4)

    window = DancerWindow()
    window.setFormat(format)
    window.resize(640, 480)
    window.show()

    window.setAnimating(True)

    sys.exit(app.exec_())