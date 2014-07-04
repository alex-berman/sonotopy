%module sonotopy

%typemap(in) const float *audio {
    if (!PyString_Check($input)) {
        PyErr_SetString(PyExc_ValueError,"Expected a string");
        return NULL;
    }

    $1 = (float *)$input;
} 

%{
    #define SWIG_FILE_WITH_INIT

    #include "../include/sonotopy/AudioParameters.hpp"
    #include "../include/sonotopy/SpectrumAnalyzerParameters.hpp"
    #include "../include/sonotopy/SpectrumMapParameters.hpp"
    #include "../include/sonotopy/GridMapParameters.hpp"

    #include "../include/sonotopy/Topology.hpp"
    #include "../include/sonotopy/SpectrumAnalyzer.hpp"
    #include "../include/sonotopy/SOM.hpp"

    #include "../include/sonotopy/SpectrumBinDivider.hpp"
    #include "../include/sonotopy/BeatTracker.hpp"
    #include "../include/sonotopy/SpectrumMap.hpp"
    #include "../include/sonotopy/GridMap.hpp"
%}

// Parse the original header file
%include "../include/sonotopy/AudioParameters.hpp"
%include "../include/sonotopy/SpectrumAnalyzerParameters.hpp"
%include "../include/sonotopy/SpectrumMapParameters.hpp"
%include "../include/sonotopy/GridMapParameters.hpp"

%include "../include/sonotopy/Topology.hpp"
%include "../include/sonotopy/SpectrumAnalyzer.hpp"
%include "../include/sonotopy/SOM.hpp"

%include "../include/sonotopy/SpectrumBinDivider.hpp"
%include "../include/sonotopy/BeatTracker.hpp"
%include "../include/sonotopy/SpectrumMap.hpp"
%include "../include/sonotopy/GridMap.hpp"
