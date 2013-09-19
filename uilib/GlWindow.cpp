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

#include "GlWindow.hpp"
#include <fstream>

GlWindow *_glWindow;

void GlWindow_display() {
  _glWindow->glDisplay();
}

void GlWindow_reshape(int width, int height) {
  _glWindow->glReshape(width, height);
}

void GlWindow_keyboard(unsigned char key, int x, int y) {
  _glWindow->glKeyboard(key, x, y);
}

void GlWindow_special(int key, int x, int y) {
  _glWindow->glSpecial(key, x, y);
}

GlWindow::GlWindow(int _argc, char **_argv) {
  _glWindow = this;
  exportEnabled = false;
  argc = _argc;
  argv = _argv;
}

void GlWindow::setWindowSize(int _width, int _height) {
  windowWidth = _width;
  windowHeight = _height;
}

void GlWindow::initializeGraphics() {
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize (windowWidth, windowHeight);
  glutCreateWindow (argv[0]);
  glutDisplayFunc(GlWindow_display);
  glutIdleFunc(GlWindow_display);
  glutReshapeFunc(GlWindow_reshape);
  glutKeyboardFunc(GlWindow_keyboard);
  glutSpecialFunc(GlWindow_special);
  windowWidth = windowHeight = 0;
}

void GlWindow::glDisplay() {
  if(windowWidth != 0 and windowHeight != 0) {
    display();
    if(exportEnabled)
      exportFrame();
  }
}

void GlWindow::glReshape(int _newWidth, int _newHeight) {
  windowWidth = _newWidth;
  windowHeight = _newHeight;
  glViewport (0, 0, (GLint) windowWidth - 1, (GLint) windowHeight - 1);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0.0, (GLdouble) windowWidth, (GLdouble) windowHeight, 0.0, -1.0, 1.0);
  resizedWindow();
}

void GlWindow::glText(int x, int y, const char *text) {
  glPushMatrix();
  glLoadIdentity();
  glOrtho (0.0, (GLdouble) windowWidth, 0.0, (GLdouble) windowHeight, -1.0, 1.0);
  glTranslatef(x, windowHeight - y, 0);
  glLineWidth(1.0f);
  glScalef(0.08f, 0.08f, 1.0f);

  const char *i = text;
  while (*i) glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *i++);
  glPopMatrix();
}

void GlWindow::windowEnableVideoExport() {
  exportEnabled = true;
  exportFrameNum = 0;
  exportFrameData = NULL;
}

void GlWindow::exportFrame() {
  if(windowWidth > 0 && windowHeight > 0) {
    char filename[1024];
    sprintf(filename, "export/frame%05d.ppm", exportFrameNum);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if(exportFrameData == NULL)
      exportFrameData = (GLubyte *) malloc(3 * windowWidth * windowHeight);
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, exportFrameData);

    std::ofstream out(filename, std::ios_base::binary);
    out << "P6\n" << windowWidth << " " << windowHeight << "\n" << "255\n";
    GLubyte *rgbPointer = exportFrameData;
    unsigned char r, g, b;
    for (int y = 0; y < windowHeight; y++) {
      for (int x = 0; x < windowWidth; x++) {
	r = *rgbPointer++;
	g = *rgbPointer++;
	b = *rgbPointer++;
	out << r << g << b;
      }
    }

    exportFrameNum++;
  }
}
