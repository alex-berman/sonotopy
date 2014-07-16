import time

from PyQt5.QtMultimedia import *
from PyQt5 import QtCore, QtWidgets


class Form(QtWidgets.QWidget):

    def __init__(self, parent=None):
        super(Form, self).__init__(parent)

        format = QAudioFormat()
        format.setSampleRate(44100)
        format.setChannelCount(1)
        format.setSampleSize(32)
        format.setCodec("audio/pcm")
        format.setByteOrder(QAudioFormat.LittleEndian);
        format.setSampleType(QAudioFormat.Float);

        info = QAudioDeviceInfo.defaultInputDevice()
        print("Using audio device: {}".format(info.deviceName()))
        if not info.isFormatSupported(format):
            print("Changing to nearest format")
            format = info.nearestFormat(format)

        self.buffer = QtCore.QBuffer()
        self.buffer.open(QtCore.QIODevice.WriteOnly)
        self.buffer.bytesWritten.connect(self.callback)

        self.audio = QAudioInput(format)
        self.audio.start(self.buffer)

    def closeEvent(self, event):
        print("Closing")
        self.audio.stop()
        self.buffer.close()

    def callback(self, bytes):
        print(bytes)

if __name__ == '__main__':
    import sys
 
    app = QtWidgets.QApplication(sys.argv)
 
    screen = Form()
    screen.show()
 
    sys.exit(app.exec_())