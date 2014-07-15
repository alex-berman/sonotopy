swig -python -c++ sonotopy.i
c++ -c sonotopy_wrap.cxx -I /usr/local//Cellar/python/2.7.5/Frameworks/Python.framework/Versions/2.7/include/python2.7
c++ -shared sonotopy_wrap.o -lpython2.7 -lsonotopy -lfftw3 -L/usr/local//Cellar/python/2.7.5/Frameworks/Python.framework/Versions/2.7/lib -o _sonotopy.so