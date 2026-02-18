======================================================================
FFMPEG AUDIO FILTER UNIT ANALYSIS
FFmpeg Version: 8.0-tessus
Total Filters: 165
======================================================================

## FFmpeg Parameter Types

Type                 Count    Examples
----------------------------------------------------------------------
int                  439      aap.order, aap.projection, aap.out_mode (+436 more)
double               362      acompressor.level_in, acompressor.threshold, acompressor.ratio (+359 more)
float                158      aap.mu, aap.delta, acontrast.contrast (+155 more)
string               136      acrossover.split, acrossover.gain, adelay.delays (+133 more)
boolean              108      acrossfade.overlap, acrossfade.o, acrusher.lfo (+105 more)
duration             41       acue.preroll, acue.buffer, acrossfade.duration (+38 more)
image_size           36       afir.size, aiir.size, anequalizer.size (+33 more)
video_rate           29       afir.rate, aiir.rate, a3dscope.rate (+26 more)
int64                19       acue.cue, acrossfade.nb_samples, acrossfade.ns (+16 more)
channel_layout       9        channelmap.channel_layout, channelsplit.channel_layout, join.channel_layout (+6 more)
flags                8        aspectralstats.measure, astats.measure_perchannel, astats.measure_overall (+5 more)
color                5        adrawgraph.bg, showvolume.dmc, avsynctest.fg (+2 more)
rational             2        showwaves.n, abuffer.time_base
binary               2        abuffersink.sample_fmts, abuffersink.sample_rates
sample_fmt           1        abuffer.sample_fmt


## Detected Unit Types (from descriptions)

Unit                 Count    Example Parameters
----------------------------------------------------------------------
channels             126      adelay.delays, adelay.all, adrc.channels (+123 more)
size                 81       adeclick.window, adeclick.w, adeclip.window (+78 more)
gain                 72       acompressor.level_in, acompressor.makeup, acompressor.level_sc (+69 more)
Hz (implied)         69       adynamicequalizer.dfrequency, adynamicequalizer.tfrequency, adynamicsmooth.basefreq (+66 more)
samples              66       acrossfade.nb_samples, acrossfade.ns, acrusher.samples (+63 more)
rate                 56       acrusher.lforate, afir.rate, aiir.rate (+53 more)
level                53       acrusher.level_in, acrusher.level_out, adenorm.level (+50 more)
width                48       allpass.width_type, allpass.t, allpass.width (+45 more)
time (duration)      40       acue.preroll, acue.buffer, acrossfade.nb_samples (+37 more)
mix/blend            30       acompressor.mix, acrusher.mix, aiir.mix (+27 more)
phase                24       aphaseshift.shift, firequalizer.zero_phase, firequalizer.min_phase (+21 more)
time (delay)         21       adelay.all, alimiter.latency, aphaser.delay (+18 more)
threshold            20       acompressor.threshold, adeclick.threshold, adeclick.t (+17 more)
order                19       aap.order, acrossover.order, adeclick.arorder (+16 more)
volume               12       amix.dropout_transition, compand.attacks, compand.decays (+9 more)
seconds              9        acue.preroll, acue.buffer, amix.dropout_transition (+6 more)
wet/dry              9        afir.dry, afir.wet, aiir.dry (+6 more)
time (attack)        8        acompressor.attack, adrc.attack, adynamicequalizer.attack (+5 more)
ratio                7        acompressor.ratio, adynamicequalizer.ratio, agate.ratio (+4 more)
time (release)       7        acompressor.release, adrc.release, adynamicequalizer.release (+4 more)
length               7        afir.length, afir.maxir, astats.length (+4 more)
Q factor             4        adynamicequalizer.dqfactor, adynamicequalizer.tqfactor, asuperpass.qfactor (+1 more)
dB                   4        headphone.gain, headphone.lfe, replaygain.track_gain (+1 more)
angle/degrees        4        sofalizer.anglestep, surround.angle, aphasemeter.angle (+1 more)
depth                3        acrusher.lforange, tremolo.d, vibrato.d
ms                   3        aphaser.delay, flanger.delay, flanger.depth
slope                3        asubboost.slope, atilt.slope, crossfeed.slope
bits                 2        acrusher.bits, hdcd.bits_per_sample
speed/rate           2        aphaser.speed, arealtime.speed
count                2        showcqt.count, showcqt.fcount
percent              1        afwtdn.percent
Hz                   1        flanger.speed


## Range Patterns

Pattern                        Count   
--------------------------------------------------
integer range                  547     
0-1 (normalized)               188     
float range                    103     
-1 to 1 (bipolar)              15      
0-100 (percentage)             9       
audio frequency range          3       


## Automation Mapping Summary


Based on analysis, these mapping strategies are needed:

| Input Type          | FFmpeg Accepts        | Mapping Required           |
|---------------------|----------------------|----------------------------|
| 0-1 normalized      | 0-1                  | None (direct)              |
| 0-1 normalized      | 0-100                | multiply by 100            |
| 0-1 normalized      | min-max float        | linear interpolation       |
| 0-1 normalized      | frequency (Hz)       | logarithmic interpolation  |
| 0-1 normalized      | dB                   | linear (dB is perceptual)  |
| 0-1 normalized      | time (ms/s)          | linear or log (context)    |
| -1 to 1 (bipolar)   | centered range       | scale and offset           |
| boolean             | 0/1                  | threshold at 0.5           |
| enum/string         | specific values      | quantize to options        |


## Suggested Curve Types for Automation


| Parameter Context     | Curve Type   | Reason                              |
|-----------------------|--------------|-------------------------------------|
| Frequency, Hz, cutoff | Logarithmic  | Perceptually linear (octaves)       |
| dB, gain, level       | Linear       | Already perceptually scaled         |
| Time (attack/release) | Logarithmic  | Small values more critical          |
| Time (delay/duration) | Linear       | Usually linear perception           |
| Mix, wet/dry, blend   | Linear       | Direct percentage                   |
| Q factor              | Logarithmic  | Wide range, low values critical     |
| Ratio (compressor)    | Logarithmic  | 1:1 to 20:1 spans wide range        |
| Depth, width          | Linear       | Usually 0-1 or percentage           |
| Percentage (0-100)    | Linear       | Direct mapping                      |


======================================================================
SIMPLE UNIT TYPE LIST
======================================================================

FFmpeg types: binary, boolean, channel_layout, color, double, duration, flags, float, image_size, int, int64, rational, sample_fmt, string, video_rate

Detected units: Hz, Hz (implied), Q factor, angle/degrees, bits, channels, count, dB, depth, gain, length, level, mix/blend, ms, order, percent, phase, rate, ratio, samples, seconds, size, slope, speed/rate, threshold, time (attack), time (delay), time (duration), time (release), volume, wet/dry, width


======================================================================
FFMPEG AUDIO FILTERS: ffmpeg -filters 2>&1 | grep "A->"
GET PARAMS: ffmpeg -h filter=volume
======================================================================
 
 TS aap               AA->A      Apply Affine Projection algorithm to first audio stream.
 .. abench            A->A       Benchmark part of a filtergraph.
 .. acompressor       A->A       Audio compressor.
 .. acontrast         A->A       Simple audio dynamic range compression/expansion filter.
 .. acopy             A->A       Copy the input audio unchanged to the output.
 .. acue              A->A       Delay filtering to match a cue.
 .. acrossfade        AA->A      Cross fade two input audio streams.
 .S acrossover        A->N       Split audio into per-bands streams.
 T. acrusher          A->A       Reduce audio bit resolution.
 TS adeclick          A->A       Remove impulsive noise from input audio.
 TS adeclip           A->A       Remove clipping from input audio.
 TS adecorrelate      A->A       Apply decorrelation to input audio.
 T. adelay            A->A       Delay one or more audio channels.
 TS adenorm           A->A       Remedy denormals by adding extremely low-level noise.
 T. aderivative       A->A       Compute derivative of input audio.
 TS adrc              A->A       Audio Spectral Dynamic Range Controller.
 TS adynamicequalizer A->A       Apply Dynamic Equalization of input audio.
 T. adynamicsmooth    A->A       Apply Dynamic Smoothing of input audio.
 .. aecho             A->A       Add echoing to the audio.
 TS aemphasis         A->A       Audio emphasis.
 T. aeval             A->A       Filter audio signal according to a specified expression.
 T. aexciter          A->A       Enhance high frequency part of audio.
 T. afade             A->A       Fade in/out input audio.
 TS afftdn            A->A       Denoise audio samples using FFT.
 TS afftfilt          A->A       Apply arbitrary expressions to samples in frequency domain.
 .. aformat           A->A       Convert the input audio to one of the specified formats.
 TS afreqshift        A->A       Apply frequency shifting to input audio.
 TS afwtdn            A->A       Denoise audio stream using Wavelets.
 T. agate             A->A       Audio gate.
 .S aiir              A->N       Apply Infinite Impulse Response filter with supplied coefficients.
 T. aintegral         A->A       Compute integral of input audio.
 T. alatency          A->A       Report audio filtering latency.
 T. alimiter          A->A       Audio lookahead limiter.
 TS allpass           A->A       Apply a two-pole all-pass filter.
 .. aloop             A->A       Loop audio samples.
 T. ametadata         A->A       Manipulate audio frame metadata.
 .. amultiply         AA->A      Multiply two audio streams.
 TS anequalizer       A->N       Apply high-order audio parametric multi band equalizer.
 TS anlmdn            A->A       Reduce broadband noise from stream using Non-Local Means.
 TS anlmf             AA->A      Apply Normalized Least-Mean-Fourth algorithm to first audio stream.
 TS anlms             AA->A      Apply Normalized Least-Mean-Squares algorithm to first audio stream.
 .. anull             A->A       Pass the source unchanged to the output.
 T. apad              A->A       Pad audio with silence.
 T. aperms            A->A       Set permissions for the output audio frame.
 .. aphaser           A->A       Add a phasing effect to the audio.
 TS aphaseshift       A->A       Apply phase shifting to input audio.
 TS apsnr             AA->A      Measure Audio Peak Signal-to-Noise Ratio.
 TS apsyclip          A->A       Audio Psychoacoustic Clipper.
 .. apulsator         A->A       Audio pulsator.
 .. arealtime         A->A       Slow down filtering to match realtime.
 .. aresample         A->A       Resample audio data.
 .. areverse          A->A       Reverse an audio clip.
 TS arls              AA->A      Apply Recursive Least Squares algorithm to first audio stream.
 TS arnndn            A->A       Reduce noise from speech using Recurrent Neural Networks.
 TS asdr              AA->A      Measure Audio Signal-to-Distortion Ratio.
 .. asegment          A->N       Segment audio stream.
 .. aselect           A->N       Select audio frames to pass in output.
 .. asendcmd          A->A       Send commands to filters.
 T. asetnsamples      A->A       Set the number of samples for each output audio frames.
 .. asetpts           A->A       Set PTS for the output audio frame.
 .. asetrate          A->A       Change the sample rate without altering the data.
 .. asettb            A->A       Set timebase for the audio output link.
 .. ashowinfo         A->A       Show textual information for each audio frame.
 T. asidedata         A->A       Manipulate audio frame side data.
 TS asisdr            AA->A      Measure Audio Scale-Invariant Signal-to-Distortion Ratio.
 TS asoftclip         A->A       Audio Soft Clipper.
 .S aspectralstats    A->A       Show frequency domain statistics about audio frames.
 .. asplit            A->N       Pass on the audio input to N audio outputs.
 .S astats            A->A       Show time domain statistics about audio frames.
 TS asubboost         A->A       Boost subwoofer frequencies.
 TS asubcut           A->A       Cut subwoofer frequencies.
 TS asupercut         A->A       Cut super frequencies.
 TS asuperpass        A->A       Apply high order Butterworth band-pass filter.
 TS asuperstop        A->A       Apply high order Butterworth band-stop filter.
 .. atempo            A->A       Adjust audio tempo.
 TS atilt             A->A       Apply spectral tilt to audio.
 .. atrim             A->A       Pick one continuous section from the input, drop the rest.
 .. axcorrelate       AA->A      Cross-correlate two audio streams.
 .. azmq              A->A       Receive commands through ZMQ and broker them to filters.
 TS bandpass          A->A       Apply a two-pole Butterworth band-pass filter.
 TS bandreject        A->A       Apply a two-pole Butterworth band-reject filter.
 TS bass              A->A       Boost or cut lower frequencies.
 TS biquad            A->A       Apply a biquad IIR filter with the given coefficients.
 .. channelmap        A->A       Remap audio channels.
 .. channelsplit      A->N       Split audio into per-channel streams.
 .. chorus            A->A       Add a chorus effect to the audio.
 .. compand           A->A       Compress or expand audio dynamic range.
 T. compensationdelay A->A       Audio Compensation Delay Line.
 T. crossfeed         A->A       Apply headphone crossfeed filter.
 TS crystalizer       A->A       Simple audio noise sharpening filter.
 T. dcshift           A->A       Apply a DC shift to the audio.
 T. deesser           A->A       Apply de-essing to the audio.
 T. dialoguenhance    A->A       Audio Dialogue Enhancement.
 .. drmeter           A->A       Measure audio dynamic range.
 TS dynaudnorm        A->A       Dynamic Audio Normalizer.
 .. earwax            A->A       Widen the stereo image.
 .. ebur128           A->N       EBU R128 scanner.
 TS equalizer         A->A       Apply two-pole peaking equalization (EQ) filter.
 T. extrastereo       A->A       Increase difference between stereo audio channels.
 .. firequalizer      A->A       Finite Impulse Response Equalizer.
 .. flanger           A->A       Apply a flanging effect to the audio.
 .. haas              A->A       Apply Haas Stereo Enhancer.
 .. hdcd              A->A       Apply High Definition Compatible Digital (HDCD) decoding.
 TS highpass          A->A       Apply a high-pass filter with 3dB point frequency.
 TS highshelf         A->A       Apply a high shelf filter.
 .. loudnorm          A->A       EBU R128 loudness normalization
 TS lowpass           A->A       Apply a low-pass filter with 3dB point frequency.
 TS lowshelf          A->A       Apply a low shelf filter.
 .. mcompand          A->A       Multiband Compress or expand audio dynamic range.
 .. pan               A->A       Remix channels with coefficients (panning).
 .. replaygain        A->A       ReplayGain scanner.
 .. rubberband        A->A       Apply time-stretching and pitch-shifting.
 .. sidechaincompress AA->A      Sidechain compressor.
 T. sidechaingate     AA->A      Audio sidechain gate.
 .. silencedetect     A->A       Detect silence.
 T. silenceremove     A->A       Remove silence.
 .S sofalizer         A->A       SOFAlizer (Spatially Oriented Format for Acoustics).
 T. speechnorm        A->A       Speech Normalizer.
 T. stereotools       A->A       Apply various stereo tools.
 T. stereowiden       A->A       Apply stereo widening effect.
 .. superequalizer    A->A       Apply 18 band equalization filter.
 .S surround          A->A       Apply audio surround upmix filter.
 TS tiltshelf         A->A       Apply a tilt shelf filter.
 TS treble            A->A       Boost or cut upper frequencies.
 T. tremolo           A->A       Apply tremolo effect.
 T. vibrato           A->A       Apply vibrato effect.
 T. virtualbass       A->A       Audio Virtual Bass.
 T. volume            A->A       Change input volume.
 .. volumedetect      A->A       Detect audio volume.
 .. anullsink         A->|       Do absolutely nothing with the input audio.
 .. a3dscope          A->V       Convert input audio to 3d scope video output.
 .. abitscope         A->V       Convert input audio to audio bit scope video output.
 .. adrawgraph        A->V       Draw a graph using input audio metadata.
 .. agraphmonitor     A->V       Show various filtergraph stats.
 .. ahistogram        A->V       Convert input audio to histogram video output.
 .. aphasemeter       A->N       Convert input audio to phase meter video output.
 .S avectorscope      A->V       Convert input audio to vectorscope video output.
 .. showcqt           A->V       Convert input audio to a CQT (Constant/Clamped Q Transform) spectrum video output.
 .S showcwt           A->V       Convert input audio to a CWT (Continuous Wavelet Transform) spectrum video output.
 .. showfreqs         A->V       Convert input audio to a frequencies video output.
 .S showspatial       A->V       Convert input audio to a spatial video output.
 .S showspectrum      A->V       Convert input audio to a spectrum video output.
 .S showspectrumpic   A->V       Convert input audio to a spectrum video output single picture.
 .. showvolume        A->V       Convert input audio volume to video output.
 .. showwaves         A->V       Convert input audio to a video output.
 .. showwavespic      A->V       Convert input audio to a video output single picture.
 .. abuffersink       A->|       Buffer audio frames, and make them available to the end of the filter graph.