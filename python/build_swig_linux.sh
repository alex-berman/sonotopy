swig -python -c++ sonotopy.i
g++ -c sonotopy_wrap.cxx -I /usr/include/python2.7 -fPIC
g++ -shared sonotopy_wrap.o -lpython2.7 -lsonotopy -lfftw3 -L /usr/local/lib -o _sonotopy.so -fPIC