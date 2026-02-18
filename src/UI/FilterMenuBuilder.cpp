#include "FilterMenuBuilder.h"
#include <QAction>
// #include <QFont>

QMenu* FilterMenuBuilder::createFilterMenu(QWidget* parent) {
    QMenu* menu = new QMenu(parent);
    
    // Override global Fira Code font for better menu usability
    // QFont menuFont;
    // menuFont.setFamily("system-ui, -apple-system");
    // menuFont.setPointSize(13);
    // menu->setFont(menuFont);
    
    // Get filters organized by category
    auto filtersByCategory = getFiltersByCategory();
    
    // Define category order for the menu
    QStringList categoryOrder = {
        "Routing && I/O", 
        "Multi-Input", 
        "Dynamics", 
        "EQ && Filters", 
        "Harmonics", 
        "Modulation", 
        "Time && Pitch", 
        "Stereo && Spatial", 
        "Restoration", 
        "Utility", 
        "Analysis", 
        "FFmpeg Filters (A-Z)", 
        "Custom Recipes"
    };
    
    // Add each category as a submenu
    for (const QString& category : categoryOrder) {
        if (filtersByCategory.contains(category)) {
            addCategoryToMenu(menu, category, filtersByCategory[category]);
        }
    }
    
    // Add separator
    menu->addSeparator();
    
    // Add "Custom FFmpeg..." at the bottom
    QAction* customAction = menu->addAction("Custom FFmpeg...");
    customAction->setData("custom");
    customAction->setToolTip("Enter raw FFmpeg parameters manually");
    
    return menu;
}

QMap<QString, QList<FilterMenuBuilder::FilterInfo>> FilterMenuBuilder::getFiltersByCategory() {
    QMap<QString, QList<FilterInfo>> categorized;
    
    // Get all filters
    QList<FilterInfo> allFilters = buildFilterRegistry();
    
    // Organize by category
    for (const FilterInfo& filter : allFilters) {
        categorized[filter.category].append(filter);
    }
    
    return categorized;
}

QList<FilterMenuBuilder::FilterInfo> FilterMenuBuilder::buildFilterRegistry() {
    QList<FilterInfo> registry;
    
    // ========== ROUTING && I/O ==========
    
    registry.append({"audio-input", "SideChain / Audio Input", "Routing && I/O", "Inject additional audio input for sidechain compression, IR convolution, or channel merging"});
    registry.append({"aux-output", "Aux Output", "Routing && I/O", "Write audio stream to a separate output file - automatically branches on main chain, terminates on sub-chain"});
    registry.append({"asplit", "Audio Split", "Routing && I/O", "Split audio into multiple parallel processing streams (up to 8 streams total)"});
    registry.append({"SmartAuxReturn", "Smart Aux Return", "Routing && I/O", "Automatically mix processed insert chains back into main chain - based on physical position"});
    registry.append({"ff-acrossover", "Crossover Multi-output", "Routing && I/O", "Split audio into frequency bands"});

    // ========== MULTI-INPUT ==========
    
    registry.append({"ff-afir", "Convolution IR (afir)", "Multi-Input", "Apply impulse response convolution - reverb, cabinet simulation, room acoustics"});
    registry.append({"ff-amerge", "Channel Merge (amerge)", "Multi-Input", "Merge multiple mono inputs into multichannel output (stereo, 5.1, 7.1, etc.)"});
    registry.append({"ff-amix", "Audio Mix (amix)", "Multi-Input", "Mix multiple audio streams together with individual volume weights"});
    registry.append({"ff-sidechaincompress", "Sidechain Compressor", "Multi-Input", "Sidechain compressor for ducking, pumping effects - triggered by external audio"});
    registry.append({"ff-sidechaingate", "Sidechain Gate", "Multi-Input", "Sidechain gate - open/close audio based on external trigger signal"});
    registry.append({"ff-acrossfade", "Crossfade (acrossfade)", "Multi-Input", "Cross-fade between two audio streams with customizable curves"});
    registry.append({"ff-join", "Join Channels", "Multi-Input", "Join multiple audio streams into multi-channel output"});
    
    // ==========  DYNAMICS ==========
    
    registry.append({"ff-agate", "Gate", "Dynamics", "Audio gate with threshold, ratio, range, and envelope controls"});
    registry.append({"ff-alimiter", "Limiter", "Dynamics", "Lookahead limiter with auto sidechain compression"});
    registry.append({"ff-acompressor", "Compressor", "Dynamics", "Audio compressor with threshold, ratio, attack, release, and knee controls"});
    registry.append({"ff-compand", "Compander", "Dynamics", "Compress or expand dynamic range with attack/decay curves"});
    registry.append({"ff-mcompand", "Multiband Compander", "Dynamics", "Multiband dynamic range compression/expansion"});
    registry.append({"ff-dynaudnorm", "Dynamic Normalizer", "Dynamics", "Dynamic audio normalizer with RMS-based loudness control"});
    registry.append({"ff-adynamicequalizer", "Dynamic EQ", "Dynamics", "Frequency-dependent compression"});
    registry.append({"ff-adynamicsmooth", "Dynamic Smooth", "Dynamics", "Adaptive low-pass filter that preserves transients"});
    registry.append({"ff-aexciter", "Exciter", "Dynamics", "Enhance high frequency harmonics"});
    registry.append({"ff-deesser", "De-esser", "Dynamics", "Reduce sibilance"});
    registry.append({"ff-adrc", "DRC (Spectral)", "Dynamics", "Spectral dynamic range control"});
    registry.append({"ff-loudnorm", "Loudness Norm", "Dynamics", "EBU R128 loudness normalization"});
    registry.append({"ff-speechnorm", "Speech Norm", "Dynamics", "Speech-specific normalization for podcasts"});
    registry.append({"ff-dialoguenhance", "Dialogue Enhance", "Dynamics", "Enhance dialogue/voice clarity in stereo"});
    registry.append({"ff-acontrast", "Contrast", "Dynamics", "Simple dynamic range expansion/compression"});

    // ========== RESTORATION ==========

    registry.append({"ff-afftdn", "FFT Denoise", "Restoration", "FFT-based noise reduction"});
    registry.append({"ff-adeclick", "Declick", "Restoration", "Remove clicks and pops"});
    registry.append({"ff-adeclip", "Declip", "Restoration", "Remove clipping distortion"});

    // ========== HARMONICS ==========

    registry.append({"ff-asoftclip", "Soft Clipper", "Harmonics", "Soft clipping with multiple curve types"});
    registry.append({"ff-acrusher", "Bit Crusher", "Harmonics", "Reduce bit resolution with LFO"});
    registry.append({"ff-apsyclip", "Psychoacoustic Clipper", "Harmonics", "Adaptive clipper using psychoacoustic principles"});
    registry.append({"ff-crystalizer", "Crystalizer", "Harmonics", "Audio noise sharpening filter"});

    // ========== MODULATION ==========
 
    registry.append({"ff-chorus", "Chorus", "Modulation", "Multi-voice chorus effect with configurable delays"});
    registry.append({"ff-flanger", "Flanger", "Modulation", "Classic flanger effect with regeneration and phase control"});
    registry.append({"ff-aphaser", "Phaser", "Modulation", "Phase shifter with variable stages and LFO speed"});
    registry.append({"ff-aphaseshift", "Phase Shifter", "Modulation", "Hilbert transform-based phase shifter"});
    registry.append({"ff-tremolo", "Tremolo", "Modulation", "Amplitude modulation effect"});
    registry.append({"ff-vibrato", "Vibrato", "Modulation", "Pitch modulation effect"});
    registry.append({"ff-afreqshift", "Frequency Shifter", "Modulation", "Shift all frequencies by constant amount"});
    registry.append({"ff-apulsator", "Pulsator", "Modulation", "Rhythmic amplitude modulation"});

    // ========== EQ && Filters ==========
   
    registry.append({"channel-eq", "Channel EQ", "EQ && Filters", "GUI for ffmpeg's 'anequalizer' filter"});
    registry.append({"ff-allpass", "All Pass", "EQ && Filters", "Phase shift without amplitude change - useful for creative effects"});
    registry.append({"ff-highpass", "High Pass", "EQ && Filters", "High-pass filter with adjustable frequency, width, and slope"});
    registry.append({"ff-lowpass", "Low Pass", "EQ && Filters", "Low-pass filter with adjustable frequency, width, and slope"});
    registry.append({"ff-bandpass", "Band Pass", "EQ && Filters", "Two-pole Butterworth band-pass filter"});
    registry.append({"ff-bandreject", "Band Reject (Notch)", "EQ && Filters", "Two-pole Butterworth band-reject/notch filter"});
    registry.append({"ff-equalizer", "Parametric EQ", "EQ && Filters", "Two-pole peaking equalizer with frequency, gain, and Q control"});
    registry.append({"ff-anequalizer", "Parametric Multi EQ", "EQ && Filters", "Multi-band parametric equalizer"});
    registry.append({"ff-highshelf", "High Shelf", "EQ && Filters", "High shelf filter for boosting or cutting highs"});
    registry.append({"ff-lowshelf", "Low Shelf", "EQ && Filters", "Low shelf filter for boosting or cutting lows"});
    registry.append({"ff-tiltshelf", "Tilt Shelf", "EQ && Filters", "Tilt EQ - boost highs while cutting lows, or vice versa"});
    registry.append({"ff-atilt", "Tilt EQ", "EQ && Filters", "Spectral tilt (bass/treble balance)"});
    registry.append({"ff-treble", "Treble", "EQ && Filters", "Boost or cut upper frequencies with shelving filter"});
    registry.append({"ff-bass", "Bass", "EQ && Filters", "Boost or cut lower frequencies with shelving filter"});
    registry.append({"ff-asubboost", "Sub Boost", "EQ && Filters", "Boost subwoofer frequencies"});
    registry.append({"ff-virtualbass", "Virtual Bass", "EQ && Filters", "Psychoacoustic bass enhancement"});
    registry.append({"ff-asubcut", "Sub Cut", "EQ && Filters", "Butterworth highpass for subsonic removal"});
    registry.append({"ff-asupercut", "Super Cut", "EQ && Filters", "Butterworth lowpass for ultrasonic removal"});
    registry.append({"ff-asuperpass", "Super Bandpass", "EQ && Filters", "High-order Butterworth bandpass"});
    registry.append({"ff-asuperstop", "Super Notch", "EQ && Filters", "High-order Butterworth band-stop"});
    registry.append({"ff-biquad", "Biquad", "EQ && Filters", "Generic biquad with direct coefficients"});
    registry.append({"ff-aemphasis", "Emphasis", "EQ && Filters", "Audio pre-emphasis and de-emphasis for vinyl, FM, CD, etc."});
    registry.append({"ff-superequalizer", "18-Band EQ", "EQ && Filters", "18-band graphic equalizer"});
    registry.append({"ff-firequalizer", "FIR Equalizer", "EQ && Filters", "Finite Impulse Response parametric equalizer"});

    // ========== STEREO & SPATIAL ==========

    registry.append({"ff-stereotools", "Stereo Tools", "Stereo && Spatial", "Comprehensive M/S processing, balance, phase, and stereo manipulation"});
    registry.append({"ff-stereowiden", "Stereo Widen", "Stereo && Spatial", "Delay-based stereo widening effect"});
    registry.append({"ff-extrastereo", "Extra Stereo", "Stereo && Spatial", "Increase or decrease stereo separation"});
    registry.append({"ff-adecorrelate", "Decorrelate", "Stereo && Spatial", "Stereo decorrelation for enhanced width"});
    registry.append({"ff-crossfeed", "Crossfeed", "Stereo && Spatial", "Headphone crossfeed for more natural stereo imaging"});
    registry.append({"ff-bs2b", "BS2B Crossfeed", "Stereo && Spatial", "Bauer stereo-to-binaural headphone DSP"});
    registry.append({"ff-pan", "Pan / Remix", "Stereo && Spatial", "Remix channels with coefficients (panning)"});
    registry.append({"ff-haas", "Haas Effect", "Stereo && Spatial", "Stereo enhancer using Haas precedence effect"});
    registry.append({"ff-sofalizer", "SOFA Spatializer", "Stereo && Spatial", "HRTF binaural spatialization using SOFA files"});
    registry.append({"ff-earwax", "Earwax", "Stereo && Spatial", "Fixed HRTF-like effect for headphone spatialization"});
    registry.append({"ff-surround", "Surround Upmix", "Stereo && Spatial", "Upmix stereo to surround sound formats"});
    registry.append({"ff-channelmap", "Channel Map", "Stereo && Spatial", "Remap, reorder, or duplicate channels"});
    registry.append({"ff-channelsplit", "Channel Split", "Stereo && Spatial", "Split audio into per-channel streams"});

    // ========== TIME & PITCH ==========

    registry.append({"ff-aloop", "Loop", "Time && Pitch", "Loop audio samples"});
    registry.append({"ff-atrim", "Trim", "Time && Pitch", "Pick one continuous section from the input, drop the rest"});
    registry.append({"ff-areverse", "Reverse", "Time && Pitch", "Reverse an audio clip"});
    registry.append({"ff-apad", "Pad", "Time && Pitch", "Pad audio with silence"});
    registry.append({"ff-afade", "Fade", "Time && Pitch", "Fade in/out input audio with multiple curve options"});
    registry.append({"ff-compensationdelay", "Compensation Delay", "Time && Pitch", "Distance-based delay for time alignment"});
    registry.append({"ff-asetpts", "Set PTS", "Time && Pitch", "Set PTS for the output audio frame"});
    registry.append({"ff-rubberband", "Rubberband", "Time && Pitch", "Time-stretching and pitch-shifting"});
    registry.append({"ff-adelay", "Delay", "Time && Pitch", "Delay one or more audio channels"});
    registry.append({"ff-aecho", "Echo", "Time && Pitch", "Add echoing to the audio"});
    registry.append({"ff-atempo", "Tempo", "Time && Pitch", "Adjust audio tempo without changing pitch"});
        
    // ========== ANALYSIS ==========
    
    registry.append({"ff-showwavespic", "Waveform Image", "Analysis", "Generate static waveform to image file"});
    registry.append({"ff-silencedetect", "Silence Detect", "Analysis", "Detects silent sections in audio and reports timestamps"});
    registry.append({"ff-volumedetect", "Volume Detect", "Analysis", "Measures and reports volume statistics (mean, max, histogram)"});
    registry.append({"ff-astats", "Audio Stats", "Analysis", "Measure audio statistics (peak, RMS, etc.)"});
    registry.append({"ff-ashowinfo", "Show Info", "Analysis", "Display detailed frame information for debugging"});
    registry.append({"ff-drmeter", "DR Meter", "Analysis", "Measures audio dynamic range (DR value)"});
    registry.append({"ff-replaygain", "ReplayGain", "Analysis", "Calculates ReplayGain values for loudness normalization"});
    registry.append({"ff-apsnr", "PSNR Measure", "Analysis", "Measure Peak Signal-to-Noise Ratio (2 inputs)"});
    registry.append({"ff-asdr", "SDR Measure", "Analysis", "Measure Signal-to-Distortion Ratio (2 inputs)"});
    registry.append({"ff-asisdr", "SI-SDR Measure", "Analysis", "Measure Scale-Invariant SDR (2 inputs)"});
    registry.append({"ff-axcorrelate", "Cross-Correlate", "Analysis", "Cross-correlate two audio streams"});
    
    // ========== UTILITY ==========

    registry.append({"ff-volume", "Volume", "Utility", "Adjust audio volume in decibels (-20dB to +20dB)"});
    registry.append({"ff-aformat", "Format", "Utility", "Convert the input audio to one of the specified formats"});
    registry.append({"ff-aresample", "Resample", "Utility", "Resample audio data"});
    registry.append({"ff-asetrate", "Set Rate", "Utility", "Change the sample rate without altering the data"});
    registry.append({"ff-dcshift", "DC Shift", "Utility", "Apply a DC shift to the audio"});
    registry.append({"ff-silenceremove", "Silence Remove", "Utility", "Remove silence from audio"});
    registry.append({"ff-adenorm", "Anti-Denorm", "Utility", "Adds extremely low-level noise to prevent CPU-intensive denormal numbers"});
    registry.append({"ff-hdcd", "HDCD Decoder", "Utility", "Decodes High Definition Compatible Digital encoded audio"});
    registry.append({"ff-aderivative", "Derivative", "Utility", "Compute derivative of audio - emphasizes transients, high-pass effect"});
    registry.append({"ff-aintegral", "Integral", "Utility", "Compute integral of audio - smooths signal, low-pass effect"});
    registry.append({"ff-acue", "Audio Cue", "Utility", "Delay filtering to match a cue timestamp for synchronized playback"});
    registry.append({"ff-acopy", "Audio Copy", "Utility", "Pass-through filter that copies audio unchanged"});
    registry.append({"ff-anull", "Null", "Utility", "Pass the source unchanged to the output"});
    registry.append({"ff-anullsink", "Null Sink", "Utility", "Destroy the source, send to nowhere..."});

    // ========== FFMPEG FILTERS (A-Z) ==========
    
    registry.append({"ff-aap", "aap", "FFmpeg Filters (A-Z)", "Apply Affine Projection algorithm"});
    registry.append({"ff-acompressor", "acompressor", "FFmpeg Filters (A-Z)", "Audio compressor"});
    registry.append({"ff-acontrast", "acontrast", "FFmpeg Filters (A-Z)", "Simple audio dynamic range compression/expansion"});
    registry.append({"ff-acopy", "acopy", "FFmpeg Filters (A-Z)", "Copy the input audio unchanged"});
    registry.append({"ff-acrossfade", "acrossfade", "FFmpeg Filters (A-Z)", "Cross fade two audio streams"});
    registry.append({"ff-acrossover", "acrossover", "FFmpeg Filters (A-Z)", "Split audio into frequency bands"});
    registry.append({"ff-acrusher", "acrusher", "FFmpeg Filters (A-Z)", "Reduce audio bit resolution"});
    registry.append({"ff-acue", "acue", "FFmpeg Filters (A-Z)", "Delay audio filtering until a cue"});
    registry.append({"ff-adeclick", "adeclick", "FFmpeg Filters (A-Z)", "Remove impulsive noise from input audio"});
    registry.append({"ff-adeclip", "adeclip", "FFmpeg Filters (A-Z)", "Remove clipping from input audio"});
    registry.append({"ff-adecorrelate", "adecorrelate", "FFmpeg Filters (A-Z)", "Apply decorrelation to input audio"});
    registry.append({"ff-adelay", "adelay", "FFmpeg Filters (A-Z)", "Delay one or more audio channels"});
    registry.append({"ff-adenorm", "adenorm", "FFmpeg Filters (A-Z)", "Remedy denormal numbers"});
    registry.append({"ff-aderivative", "aderivative", "FFmpeg Filters (A-Z)", "Compute derivative of input audio"});
    registry.append({"ff-aintegral", "aintegral", "FFmpeg Filters (A-Z)", "Compute integral of input audio"});
    registry.append({"ff-adrc", "adrc", "FFmpeg Filters (A-Z)", "Audio Spectral Dynamic Range Controller"});
    registry.append({"ff-adynamicequalizer", "adynamicequalizer", "FFmpeg Filters (A-Z)", "Apply Dynamic Equalization of input audio"});
    registry.append({"ff-adynamicsmooth", "adynamicsmooth", "FFmpeg Filters (A-Z)", "Apply Dynamic Smoothing of input audio"});
    registry.append({"ff-aecho", "aecho", "FFmpeg Filters (A-Z)", "Add echoing to the audio"});
    registry.append({"ff-aemphasis", "aemphasis", "FFmpeg Filters (A-Z)", "Audio emphasis"});
    registry.append({"ff-aeval", "aeval", "FFmpeg Filters (A-Z)", "Filter audio signal according to a specified expression"});
    registry.append({"ff-aexciter", "aexciter", "FFmpeg Filters (A-Z)", "Enhance high frequency part of audio"});
    registry.append({"ff-afade", "afade", "FFmpeg Filters (A-Z)", "Fade in/out input audio"});
    registry.append({"ff-afftdn", "afftdn", "FFmpeg Filters (A-Z)", "Denoise audio samples using FFT"});
    registry.append({"ff-afftfilt", "afftfilt", "FFmpeg Filters (A-Z)", "Apply arbitrary expressions to samples in frequency domain"});
    registry.append({"ff-afir", "afir", "FFmpeg Filters (A-Z)", "Apply Finite Impulse Response filter with supplied coefficients"});
    registry.append({"ff-aformat", "aformat", "FFmpeg Filters (A-Z)", "Convert the input audio to one of the specified formats"});
    registry.append({"ff-afreqshift", "afreqshift", "FFmpeg Filters (A-Z)", "Apply frequency shifting to input audio"});
    registry.append({"ff-afwtdn", "afwtdn", "FFmpeg Filters (A-Z)", "Denoise audio stream using Wavelets"});
    registry.append({"ff-agate", "agate", "FFmpeg Filters (A-Z)", "Audio gate"});
    registry.append({"ff-aiir", "aiir", "FFmpeg Filters (A-Z)", "Apply Infinite Impulse Response filter with supplied coefficients"});
    registry.append({"ff-alimiter", "alimiter", "FFmpeg Filters (A-Z)", "Audio limiter"});
    registry.append({"ff-allpass", "allpass", "FFmpeg Filters (A-Z)", "Apply a two-pole all-pass filter"});
    registry.append({"ff-aloop", "aloop", "FFmpeg Filters (A-Z)", "Loop audio samples"});
    registry.append({"ff-amerge", "amerge", "FFmpeg Filters (A-Z)", "Merge two or more audio streams into a single multi-channel stream"});
    registry.append({"ff-amix", "amix", "FFmpeg Filters (A-Z)", "Audio mixing"});
    registry.append({"ff-amultiply", "amultiply", "FFmpeg Filters (A-Z)", "Multiply two audio streams"});
    registry.append({"ff-anequalizer", "anequalizer", "FFmpeg Filters (A-Z)", "Apply high-order audio parametric multi band equalizer"});
    registry.append({"ff-anlmdn", "anlmdn", "FFmpeg Filters (A-Z)", "Reduce broadband noise from stream using Non-Local Means"});
    registry.append({"ff-anlmf", "anlmf", "FFmpeg Filters (A-Z)", "Apply Normalized Least-Mean-Fourth algorithm to input audio"});
    registry.append({"ff-anlms", "anlms", "FFmpeg Filters (A-Z)", "Apply Normalized Least-Mean-Squares algorithm to input audio"});
    registry.append({"ff-anull", "anull", "FFmpeg Filters (A-Z)", "Pass the source unchanged to the output"});
    registry.append({"ff-apad", "apad", "FFmpeg Filters (A-Z)", "Pad audio with silence"});
    registry.append({"ff-aphaser", "aphaser", "FFmpeg Filters (A-Z)", "Add a phasing effect to the audio"});
    registry.append({"ff-aphaseshift", "aphaseshift", "FFmpeg Filters (A-Z)", "Apply phase shifting to input audio"});
    registry.append({"ff-apsnr", "apsnr", "FFmpeg Filters (A-Z)", "Measure Audio Peak Signal-to-Noise Ratio"});
    registry.append({"ff-apsyclip", "apsyclip", "FFmpeg Filters (A-Z)", "Audio Psychoacoustic Clipper"});
    registry.append({"ff-apulsator", "apulsator", "FFmpeg Filters (A-Z)", "Audio pulsator"});
    registry.append({"ff-aresample", "aresample", "FFmpeg Filters (A-Z)", "Resample audio data"});
    registry.append({"ff-areverse", "areverse", "FFmpeg Filters (A-Z)", "Reverse an audio clip"});
    registry.append({"ff-arls", "arls", "FFmpeg Filters (A-Z)", "Apply Recursive Least Squares algorithm to input audio"});
    registry.append({"ff-arnndn", "arnndn", "FFmpeg Filters (A-Z)", "Reduce noise from speech using Recurrent Neural Networks"});
    registry.append({"ff-asdr", "asdr", "FFmpeg Filters (A-Z)", "Measure Audio Signal-to-Distortion Ratio"});
    registry.append({"ff-asetnsamples", "asetnsamples", "FFmpeg Filters (A-Z)", "Set the number of samples for each output audio frames"});
    registry.append({"ff-asetrate", "asetrate", "FFmpeg Filters (A-Z)", "Change the sample rate without altering the data"});
    registry.append({"ff-ashowinfo", "ashowinfo", "FFmpeg Filters (A-Z)", "Show textual information for each audio frame"});
    registry.append({"ff-asisdr", "asisdr", "FFmpeg Filters (A-Z)", "Measure Audio Scale-Invariant Signal-to-Distortion Ratio"});
    registry.append({"ff-asoftclip", "asoftclip", "FFmpeg Filters (A-Z)", "Audio Soft Clipper"});
    registry.append({"ff-aspectralstats", "aspectralstats", "FFmpeg Filters (A-Z)", "Show frequency domain statistics about audio frames"});
    registry.append({"ff-asr", "asr", "FFmpeg Filters (A-Z)", "Automatic Speech Recognition"});
    registry.append({"ff-astats", "astats", "FFmpeg Filters (A-Z)", "Show time domain statistics about audio frames"});
    registry.append({"ff-asubboost", "asubboost", "FFmpeg Filters (A-Z)", "Boost subwoofer frequencies"});
    registry.append({"ff-asubcut", "asubcut", "FFmpeg Filters (A-Z)", "Cut subwoofer frequencies"});
    registry.append({"ff-asupercut", "asupercut", "FFmpeg Filters (A-Z)", "Cut super frequencies"});
    registry.append({"ff-asuperpass", "asuperpass", "FFmpeg Filters (A-Z)", "Apply high order Butterworth band-pass filter"});
    registry.append({"ff-asuperstop", "asuperstop", "FFmpeg Filters (A-Z)", "Apply high order Butterworth band-stop filter"});
    registry.append({"ff-atempo", "atempo", "FFmpeg Filters (A-Z)", "Adjust audio tempo"});
    registry.append({"ff-atilt", "atilt", "FFmpeg Filters (A-Z)", "Apply spectral tilt to audio"});
    registry.append({"ff-atrim", "atrim", "FFmpeg Filters (A-Z)", "Pick one continuous section from the input, drop the rest"});
    registry.append({"ff-axcorrelate", "axcorrelate", "FFmpeg Filters (A-Z)", "Cross-correlate two audio streams"});
    registry.append({"ff-bandpass", "bandpass", "FFmpeg Filters (A-Z)", "Apply a two-pole Butterworth band-pass filter"});
    registry.append({"ff-bandreject", "bandreject", "FFmpeg Filters (A-Z)", "Apply a two-pole Butterworth band-reject filter"});
    registry.append({"ff-bass", "bass / lowshelf", "FFmpeg Filters (A-Z)", "Boost or cut lower frequencies"});
    registry.append({"ff-biquad", "biquad", "FFmpeg Filters (A-Z)", "Apply a biquad IIR filter with the given coefficients"});
    registry.append({"ff-bs2b", "bs2b", "FFmpeg Filters (A-Z)", "Bauer stereo-to-binaural transformation"});
    registry.append({"ff-channelmap", "channelmap", "FFmpeg Filters (A-Z)", "Remap audio channels"});
    registry.append({"ff-channelsplit", "channelsplit", "FFmpeg Filters (A-Z)", "Split audio into per-channel streams"});
    registry.append({"ff-chorus", "chorus", "FFmpeg Filters (A-Z)", "Add a chorus effect to the audio"});
    registry.append({"ff-compand", "compand", "FFmpeg Filters (A-Z)", "Compress or expand audio dynamic range"});
    registry.append({"ff-compensationdelay", "compensationdelay", "FFmpeg Filters (A-Z)", "Audio Compensation Delay Line"});
    registry.append({"ff-crossfeed", "crossfeed", "FFmpeg Filters (A-Z)", "Apply headphone crossfeed filter"});
    registry.append({"ff-crystalizer", "crystalizer", "FFmpeg Filters (A-Z)", "Simple audio noise sharpening filter"});
    registry.append({"ff-dcshift", "dcshift", "FFmpeg Filters (A-Z)", "Apply a DC shift to the audio"});
    registry.append({"ff-deesser", "deesser", "FFmpeg Filters (A-Z)", "Apply de-essing to the audio"});
    registry.append({"ff-dialoguenhance", "dialoguenhance", "FFmpeg Filters (A-Z)", "Audio Dialogue Enhancement"});
    registry.append({"ff-drmeter", "drmeter", "FFmpeg Filters (A-Z)", "Measure audio dynamic range"});
    registry.append({"ff-dynaudnorm", "dynaudnorm", "FFmpeg Filters (A-Z)", "Dynamic Audio Normalizer"});
    registry.append({"ff-earwax", "earwax", "FFmpeg Filters (A-Z)", "Widen the stereo image"});
    registry.append({"ff-equalizer", "equalizer", "FFmpeg Filters (A-Z)", "Apply two-pole peaking equalization (EQ) filter"});
    registry.append({"ff-extrastereo", "extrastereo", "FFmpeg Filters (A-Z)", "Increase difference between stereo audio channels"});
    registry.append({"ff-firequalizer", "firequalizer", "FFmpeg Filters (A-Z)", "Finite Impulse Response Equalizer"});
    registry.append({"ff-flanger", "flanger", "FFmpeg Filters (A-Z)", "Apply a flanging effect to the audio"});
    registry.append({"ff-haas", "haas", "FFmpeg Filters (A-Z)", "Apply Haas Stereo Enhancer"});
    registry.append({"ff-hdcd", "hdcd", "FFmpeg Filters (A-Z)", "Apply HDCD decoding"});
    registry.append({"ff-headphone", "headphone", "FFmpeg Filters (A-Z)", "Apply headphone binaural spatialization with HRTFs"});
    registry.append({"ff-highpass", "highpass", "FFmpeg Filters (A-Z)", "Apply a high-pass filter with 3dB point frequency"});
    registry.append({"ff-join", "join", "FFmpeg Filters (A-Z)", "Join multiple audio streams into multi-channel output"});
    registry.append({"ff-ladspa", "ladspa", "FFmpeg Filters (A-Z)", "Apply LADSPA effect"});
    registry.append({"ff-loudnorm", "loudnorm", "FFmpeg Filters (A-Z)", "EBU R128 loudness normalization"});
    registry.append({"ff-lowpass", "lowpass", "FFmpeg Filters (A-Z)", "Apply a low-pass filter with 3dB point frequency"});
    registry.append({"ff-lv2", "lv2", "FFmpeg Filters (A-Z)", "Apply LV2 effect"});
    registry.append({"ff-mcompand", "mcompand", "FFmpeg Filters (A-Z)", "Multiband Compress or expand audio dynamic range"});
    registry.append({"ff-pan", "pan", "FFmpeg Filters (A-Z)", "Remix channels with coefficients"});
    registry.append({"ff-replaygain", "replaygain", "FFmpeg Filters (A-Z)", "ReplayGain scanner"});
    registry.append({"ff-resample", "resample", "FFmpeg Filters (A-Z)", "Audio resampling and conversion"});
    registry.append({"ff-rubberband", "rubberband", "FFmpeg Filters (A-Z)", "Apply time-stretching and pitch-shifting"});
    registry.append({"ff-sidechaincompress", "sidechaincompress", "FFmpeg Filters (A-Z)", "Sidechain compressor"});
    registry.append({"ff-sidechaingate", "sidechaingate", "FFmpeg Filters (A-Z)", "Audio sidechain gate"});
    registry.append({"ff-silencedetect", "silencedetect", "FFmpeg Filters (A-Z)", "Detect silence in audio"});
    registry.append({"ff-silenceremove", "silenceremove", "FFmpeg Filters (A-Z)", "Remove silence from the audio"});
    registry.append({"ff-sofalizer", "sofalizer", "FFmpeg Filters (A-Z)", "SOFAlizer (Spatially Oriented Format for Acoustics)"});
    registry.append({"ff-speechnorm", "speechnorm", "FFmpeg Filters (A-Z)", "Speech Normalizer"});
    registry.append({"ff-stereotools", "stereotools", "FFmpeg Filters (A-Z)", "Apply various stereo tools"});
    registry.append({"ff-stereowiden", "stereowiden", "FFmpeg Filters (A-Z)", "Apply stereo widening effect"});
    registry.append({"ff-superequalizer", "superequalizer", "FFmpeg Filters (A-Z)", "Apply 18 band equalization filter"});
    registry.append({"ff-surround", "surround", "FFmpeg Filters (A-Z)", "Apply audio surround upmix filter"});
    registry.append({"ff-tiltshelf", "tiltshelf", "FFmpeg Filters (A-Z)", "Apply a tilt shelf filter"});
    registry.append({"ff-treble", "treble / highshelf", "FFmpeg Filters (A-Z)", "Boost or cut upper frequencies"});
    registry.append({"ff-tremolo", "tremolo", "FFmpeg Filters (A-Z)", "Apply tremolo effect"});
    registry.append({"ff-vibrato", "vibrato", "FFmpeg Filters (A-Z)", "Apply vibrato effect"});
    registry.append({"ff-virtualbass", "virtualbass", "FFmpeg Filters (A-Z)", "Audio Virtual Bass"});
    registry.append({"ff-volume", "volume", "FFmpeg Filters (A-Z)", "Change input volume"});
    registry.append({"ff-volumedetect", "volumedetect", "FFmpeg Filters (A-Z)", "Detect audio volume"});
    registry.append({"ff-whisper", "whisper", "FFmpeg Filters (A-Z)", "Whisper effect"});
    
    return registry;
}

void FilterMenuBuilder::addCategoryToMenu(QMenu* parentMenu, const QString& category, 
                                         const QList<FilterInfo>& filters) {
    // Create submenu for this category
    QMenu* categoryMenu = parentMenu->addMenu(category);
    
    // For large menus, lazy-load the contents
    if (filters.size() > 16) {
        // Connect to aboutToShow to populate menu only when needed
        QObject::connect(categoryMenu, &QMenu::aboutToShow, [categoryMenu, filters]() {
            // Only populate if empty (first time showing)
            if (categoryMenu->isEmpty()) {
                for (const FilterInfo& filter : filters) {
                    QAction* action = categoryMenu->addAction(filter.displayName);
                    action->setData(filter.type);
                    //action->setToolTip(filter.description);
                }
            }
        });
    } else {
        // Small menus can be populated immediately
        for (const FilterInfo& filter : filters) {
            QAction* action = categoryMenu->addAction(filter.displayName);
            action->setData(filter.type);
            //action->setToolTip(filter.description);
        }
    }
}
