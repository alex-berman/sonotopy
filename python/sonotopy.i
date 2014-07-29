%module sonotopy

%include "carrays.i"
%array_class(float, floatArray);

%{
    #define SWIG_FILE_WITH_INIT

    #include "../include/sonotopy/AudioParameters.hpp"
    #include "../include/sonotopy/SpectrumAnalyzerParameters.hpp"
    #include "../include/sonotopy/SpectrumMapParameters.hpp"
    #include "../include/sonotopy/GridMapParameters.hpp"
    #include "../include/sonotopy/CircleMapParameters.hpp"

    #include "../include/sonotopy/Topology.hpp"
    #include "../include/sonotopy/SpectrumAnalyzer.hpp"
    #include "../include/sonotopy/SOM.hpp"

    #include "../include/sonotopy/SpectrumBinDivider.hpp"
    #include "../include/sonotopy/BeatTracker.hpp"
    #include "../include/sonotopy/SpectrumMap.hpp"
    #include "../include/sonotopy/GridMap.hpp"
    #include "../include/sonotopy/CircleMap.hpp"
%}

%include "std_vector.i"
%template(BinDefinitionVector) std::vector<sonotopy::BinDefinition>;

// Parse the original header file
%include "../include/sonotopy/AudioParameters.hpp"
%include "../include/sonotopy/SpectrumAnalyzerParameters.hpp"
%include "../include/sonotopy/SpectrumMapParameters.hpp"
%include "../include/sonotopy/GridMapParameters.hpp"
%include "../include/sonotopy/CircleMapParameters.hpp"

%include "../include/sonotopy/Topology.hpp"
%include "../include/sonotopy/SpectrumAnalyzer.hpp"
%include "../include/sonotopy/SOM.hpp"

%include "../include/sonotopy/SpectrumBinDivider.hpp"
%include "../include/sonotopy/BeatTracker.hpp"
%include "../include/sonotopy/SpectrumMap.hpp"
%include "../include/sonotopy/GridMap.hpp"
%include "../include/sonotopy/CircleMap.hpp"
