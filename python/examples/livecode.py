import sys
import time
import struct
import math
import array

from PyQt5.QtMultimedia import *
from PyQt5 import QtCore, QtGui, QtWidgets

import sonotopy

# Window creation function.
def create_window(window_class):
    """Create a QT window in Python, or interactively in IPython with QT GUI
    event loop integration:
    # in ~/.ipython/ipython_config.py
    c.TerminalIPythonApp.gui = 'qt'
    c.TerminalIPythonApp.pylab = 'qt'
    See also:
    http://ipython.org/ipython-doc/dev/interactive/qtconsole.html#qt-and-the-qtconsole
    """
    app = QtCore.QCoreApplication.instance()
    
    if app is None:
        print("GUI loop not hooked.")
        return None

    app.references = set()

    window = window_class()
    #window.setAnimating(True)
    app.references.add(window)
    window.showMaximized()

    return window


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

VS = '''
void main()
{
    gl_Position = gl_Vertex;
}
'''

FS = '''
uniform vec2 resolution;
uniform vec2 spec;
uniform float time;
 
void main(void) {
    vec2 uv = 2.0 * (gl_FragCoord.xy / resolution) - 1.0;
    float col = 0.0;
    uv.x += sin(time*6.0 + uv.y*1.5) * spec.y;
    col += abs(0.066/uv.x) * spec.y;
    gl_FragColor = vec4(col, col, col, 1.0);
}
'''

class LiveCodeWindow(OpenGLWindow):

    def __init__(self, parent=None):
        super(LiveCodeWindow, self).__init__(parent)

        self.program = None
        self._vs = VS
        self._fs = FS
        self.start_time = time.time()

        self.init_sonotopy()
        self.init_audio()

    def fs():
        doc = "The fs property."
        def fget(self):
            return self._fs
        def fset(self, value):
            self._fs = value
            self.initialize()
        def fdel(self):
            del self._fs
        return locals()
    fs = property(**fs())

    def vs():
        doc = "The vs property."
        def fget(self):
            return self._vs
        def fset(self, value):
            self._vs = value
            self.initialize()
        def fdel(self):
            del self._vs
        return locals()
    vs = property(**vs())

    def initialize(self):
        self.program = QtGui.QOpenGLShaderProgram(self)

        self.program.addShaderFromSourceCode(
            QtGui.QOpenGLShader.Vertex,
            self.vs)
        self.program.addShaderFromSourceCode(
            QtGui.QOpenGLShader.Fragment,
            self.fs)

        self.program.link()

        self.program.bind()
        self.program.setUniformValue("resolution", self.width(), self.height())
        self.program.setUniformValue("spec", 0.2, 0.1)
        self.program.release()

    def init_sonotopy(self):
        self.audio_parameters = sonotopy.AudioParameters()
        self.spectrum_parameters = sonotopy.SpectrumAnalyzerParameters()

        bin1 = sonotopy.BinDefinition()
        bin1.centerFreqHz = 4000
        bin1.bandWidthHz = 3000
        bin2 = sonotopy.BinDefinition()
        bin2.centerFreqHz = 12000
        bin2.bandWidthHz = 5000

        self.spectrum_analyzer = sonotopy.SpectrumAnalyzer(self.spectrum_parameters)
        self.bin_divider = sonotopy.SpectrumBinDivider(
            self.audio_parameters.sampleRate,
            self.spectrum_analyzer.getSpectrumResolution(),
            [ bin1, bin2 ]
        )
        #print(self.bin_divider.getNumBins())
        self.bin_values = [ 0.0, 0.0 ]


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

    def event(self, event):
        if event.type() == QtCore.QEvent.Close:
            self.audio.stop()
            self.buffer.close()
            return True

        return super(LiveCodeWindow, self).event(event)

    def callback(self, bytes):
        data_len = int(self.buffer.size()/2)
        data_bytes = self.buffer.data().data()
        self.buffer.reset()

        data_int = struct.unpack("%dh"%(data_len), data_bytes)
        data = sonotopy.floatArray(data_len)
        for i in range(data_len):
            data[i] = float(data_int[i])/32768.0

        self.spectrum_analyzer.feedAudioFrames(data, data_len)
        spectrum = self.spectrum_analyzer.getSpectrum()
        self.bin_divider.feedSpectrum(spectrum, data_len)

        _bin_values = sonotopy.floatArray_frompointer(
            self.bin_divider.getBinValues())
        self.bin_values[0] = _bin_values[0]*1024*16
        self.bin_values[1] = _bin_values[1]*1024*16

        self.renderLater()       

    def render(self, gl):
        gl.glViewport(0, 0, self.width(), self.height())
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)
        gl.glLoadIdentity()

        self.program.bind()

        self.program.setUniformValue("time", self.start_time-time.time())
        self.program.setUniformValue("spec", self.bin_values[0], self.bin_values[1])

        gl.glRectf(1.0, 1.0, -1.0, -1.0)
        
        self.program.release()

    def resizeEvent(self, event):
        super(LiveCodeWindow, self).resizeEvent(event)
        if self.program:
            self.program.bind()
            self.program.setUniformValue("resolution", self.width(), self.height())
            self.program.release()

if __name__ == '__main__':
    import sys
 
    app = QtWidgets.QApplication(sys.argv)
    
    format = QtGui.QSurfaceFormat()
    format.setSamples(4)

    window = LiveCodeWindow()
    window.setFormat(format)
    window.resize(640, 480)
    window.show()

    window.setAnimating(True)

    sys.exit(app.exec_())