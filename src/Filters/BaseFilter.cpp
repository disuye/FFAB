#include "BaseFilter.h"

QString BaseFilter::getDefaultCustomCommandTemplate() const {
    switch (position) {
        case Position::INPUT:
            return "-i input.wav";
        case Position::MIDDLE:
            return "..."; // this string is sent to -filter_complex "[0a]..."
        case Position::OUTPUT:
            return "-c:a pcm_s24le -ar 48000";
    }
    return "";
}
