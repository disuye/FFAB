#pragma once

#include <cmath>

// Logarithmic frequency slider helpers
// Maps Hz values to slider position (0-1000) using log scale
inline int freqToSlider(double freq, double minFreq, double maxFreq) {
    double logMin = std::log10(minFreq);
    double logMax = std::log10(maxFreq);
    double logFreq = std::log10(std::clamp(freq, minFreq, maxFreq));
    return static_cast<int>((logFreq - logMin) / (logMax - logMin) * 1000.0);
}

// Maps slider position (0-1000) to Hz value using log scale
inline double sliderToFreq(int slider, double minFreq, double maxFreq) {
    double logMin = std::log10(minFreq);
    double logMax = std::log10(maxFreq);
    double logFreq = logMin + (slider / 1000.0) * (logMax - logMin);
    return std::pow(10.0, logFreq);
}
