# FFmpeg Audio Filters Reference

> **FFmpeg Version:** 8.0-tessus  
> **Generated:** 2026-01-30T20:13:13Z  
> **Total Filters:** 165

---

## Table of Contents

- [a3dscope](#a3dscope)
- [aap](#aap)
- [abench](#abench)
- [abitscope](#abitscope)
- [abuffer](#abuffer)
- [abuffersink](#abuffersink)
- [acompressor](#acompressor)
- [acontrast](#acontrast)
- [acopy](#acopy)
- [acrossfade](#acrossfade)
- [acrossover](#acrossover)
- [acrusher](#acrusher)
- [acue](#acue)
- [adeclick](#adeclick)
- [adeclip](#adeclip)
- [adecorrelate](#adecorrelate)
- [adelay](#adelay)
- [adenorm](#adenorm)
- [aderivative](#aderivative)
- [adrawgraph](#adrawgraph)
- [adrc](#adrc)
- [adynamicequalizer](#adynamicequalizer)
- [adynamicsmooth](#adynamicsmooth)
- [aecho](#aecho)
- [aemphasis](#aemphasis)
- [aeval](#aeval)
- [aevalsrc](#aevalsrc)
- [aexciter](#aexciter)
- [afade](#afade)
- [afdelaysrc](#afdelaysrc)
- [afftdn](#afftdn)
- [afftfilt](#afftfilt)
- [afir](#afir)
- [afireqsrc](#afireqsrc)
- [afirsrc](#afirsrc)
- [aformat](#aformat)
- [afreqshift](#afreqshift)
- [afwtdn](#afwtdn)
- [agate](#agate)
- [agraphmonitor](#agraphmonitor)
- [ahistogram](#ahistogram)
- [aiir](#aiir)
- [aintegral](#aintegral)
- [ainterleave](#ainterleave)
- [alatency](#alatency)
- [alimiter](#alimiter)
- [allpass](#allpass)
- [aloop](#aloop)
- [amerge](#amerge)
- [ametadata](#ametadata)
- [amix](#amix)
- [amultiply](#amultiply)
- [anequalizer](#anequalizer)
- [anlmdn](#anlmdn)
- [anlmf](#anlmf)
- [anlms](#anlms)
- [anoisesrc](#anoisesrc)
- [anull](#anull)
- [anullsink](#anullsink)
- [anullsrc](#anullsrc)
- [apad](#apad)
- [aperms](#aperms)
- [aphasemeter](#aphasemeter)
- [aphaser](#aphaser)
- [aphaseshift](#aphaseshift)
- [apsnr](#apsnr)
- [apsyclip](#apsyclip)
- [apulsator](#apulsator)
- [arealtime](#arealtime)
- [aresample](#aresample)
- [areverse](#areverse)
- [arls](#arls)
- [arnndn](#arnndn)
- [asdr](#asdr)
- [asegment](#asegment)
- [aselect](#aselect)
- [asendcmd](#asendcmd)
- [asetnsamples](#asetnsamples)
- [asetpts](#asetpts)
- [asetrate](#asetrate)
- [asettb](#asettb)
- [ashowinfo](#ashowinfo)
- [asidedata](#asidedata)
- [asisdr](#asisdr)
- [asoftclip](#asoftclip)
- [aspectralstats](#aspectralstats)
- [asplit](#asplit)
- [astats](#astats)
- [asubboost](#asubboost)
- [asubcut](#asubcut)
- [asupercut](#asupercut)
- [asuperpass](#asuperpass)
- [asuperstop](#asuperstop)
- [atempo](#atempo)
- [atilt](#atilt)
- [atrim](#atrim)
- [avectorscope](#avectorscope)
- [avsynctest](#avsynctest)
- [axcorrelate](#axcorrelate)
- [azmq](#azmq)
- [bandpass](#bandpass)
- [bandreject](#bandreject)
- [bass](#bass)
- [biquad](#biquad)
- [channelmap](#channelmap)
- [channelsplit](#channelsplit)
- [chorus](#chorus)
- [compand](#compand)
- [compensationdelay](#compensationdelay)
- [crossfeed](#crossfeed)
- [crystalizer](#crystalizer)
- [dcshift](#dcshift)
- [deesser](#deesser)
- [dialoguenhance](#dialoguenhance)
- [drmeter](#drmeter)
- [dynaudnorm](#dynaudnorm)
- [earwax](#earwax)
- [ebur128](#ebur128)
- [equalizer](#equalizer)
- [extrastereo](#extrastereo)
- [firequalizer](#firequalizer)
- [flanger](#flanger)
- [haas](#haas)
- [hdcd](#hdcd)
- [headphone](#headphone)
- [highpass](#highpass)
- [highshelf](#highshelf)
- [hilbert](#hilbert)
- [join](#join)
- [loudnorm](#loudnorm)
- [lowpass](#lowpass)
- [lowshelf](#lowshelf)
- [mcompand](#mcompand)
- [pan](#pan)
- [replaygain](#replaygain)
- [rubberband](#rubberband)
- [showcqt](#showcqt)
- [showcwt](#showcwt)
- [showfreqs](#showfreqs)
- [showspatial](#showspatial)
- [showspectrum](#showspectrum)
- [showspectrumpic](#showspectrumpic)
- [showvolume](#showvolume)
- [showwaves](#showwaves)
- [showwavespic](#showwavespic)
- [sidechaincompress](#sidechaincompress)
- [sidechaingate](#sidechaingate)
- [silencedetect](#silencedetect)
- [silenceremove](#silenceremove)
- [sinc](#sinc)
- [sine](#sine)
- [sofalizer](#sofalizer)
- [spectrumsynth](#spectrumsynth)
- [speechnorm](#speechnorm)
- [stereotools](#stereotools)
- [stereowiden](#stereowiden)
- [superequalizer](#superequalizer)
- [surround](#surround)
- [tiltshelf](#tiltshelf)
- [treble](#treble)
- [tremolo](#tremolo)
- [vibrato](#vibrato)
- [virtualbass](#virtualbass)
- [volume](#volume)
- [volumedetect](#volumedetect)

---

## Legend

| Symbol | Meaning |
|--------|---------|
| ✓ | Feature supported |
| ✗ | Feature not supported |
| 🎚️ | Parameter is automatable (can be changed at runtime) |

### Parameter Types

| Type | Description | Units/Notes |
|------|-------------|-------------|
| integer | Whole number | count, samples, bits |
| float | Decimal number | linear gain, ratio |
| boolean | true/false | 0/1 |
| string | Text value | expressions, filenames |
| duration | Time value | seconds, or HH:MM:SS.ms |
| flags | Bitfield | combined options |
| channel_layout | Channel config | stereo, 5.1, etc. |
| sample_format | Sample format | s16, s32, flt, dbl |

### Common Unit Conventions

| Suffix/Context | Unit | Example |
|----------------|------|---------|
| dB | Decibels | `volume=6dB` |
| Hz | Hertz (frequency) | `lowpass=f=1000` |
| ms | Milliseconds | `adelay=500` |
| % | Percentage | (rarely used directly) |

---

## Filters


---

### a3dscope

Convert input audio to 3d scope video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `size` | image_size | — | `"hd720"` | — | set video size |
| `s` | image_size | — | `"hd720"` | — | set video size |
| `fov` | float | 40 → 150 | `90` | 🎚️ | set camera FoV |
| `roll` | float | -180 → 180 | `0` | 🎚️ | set camera roll |
| `pitch` | float | -180 → 180 | `0` | 🎚️ | set camera pitch |
| `yaw` | float | -180 → 180 | `0` | 🎚️ | set camera yaw |
| `xzoom` | float | 0.01 → 10 | `1` | 🎚️ | set camera zoom |
| `yzoom` | float | 0.01 → 10 | `1` | 🎚️ | set camera zoom |
| `zzoom` | float | 0.01 → 10 | `1` | 🎚️ | set camera zoom |
| `xpos` | float | -60 → 60 | `0` | 🎚️ | set camera position |
| `ypos` | float | -60 → 60 | `0` | 🎚️ | set camera position |
| `zpos` | float | -60 → 60 | `0` | 🎚️ | set camera position |
| `length` | integer | 1 → 60 | `15` | — | set length |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "a3dscope" output.wav
```


---

### aap

Apply Affine Projection algorithm to first audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `order` | integer | 1 → 32767 | `16` | — | set the filter order |
| `projection` | integer | 1 → 256 | `2` | — | set the filter projection |
| `mu` | float | 0 → 1 | `0.0001` | 🎚️ | set the filter mu |
| `delta` | float | 0 → 1 | `0.001` | 🎚️ | set the filter delta |
| `out_mode` | integer | 0 → 4 | `o` | 🎚️ | set output mode |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aap" output.wav
```


---

### abench

Benchmark part of a filtergraph.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `action` | integer | 0 → 1 | `start` | — | set action |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "abench" output.wav
```


---

### abitscope

Convert input audio to audio bit scope video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `size` | image_size | — | `"1024x256"` | — | set video size |
| `s` | image_size | — | `"1024x256"` | — | set video size |
| `colors` | string | — | `"red|green|blue|yellow|orange|lime|pink|magenta|brown"` | — | set channels colors |
| `mode` | integer | 0 → 1 | `bars` | — | set output mode |
| `m` | integer | 0 → 1 | `bars` | — | set output mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "abitscope" output.wav
```


---

### abuffer

Buffer audio frames, and make them accessible to the filterchain.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `time_base` | rational | ≥ 0 | `0/1` | — |  |
| `sample_rate` | integer | ≥ 0 | `0` | — |  |
| `sample_fmt` | sample_format | — | `none` | — |  |
| `channels` | integer | ≥ 0 | `0` | — |  |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "abuffer" output.wav
```


---

### abuffersink

Buffer audio frames, and make them available to the end of the filter graph.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_fmts` | binary | — | — | — | set the supported sample formats |
| `sample_rates` | binary | — | — | — | set the supported sample rates |
| `ch_layouts` | string | — | — | — | set a '\|'-separated list of supported channel layouts |
| `all_channel_counts` | boolean | — | `false` | — | accept all channel counts |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "abuffersink" output.wav
```


---

### acompressor

Audio compressor.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input gain |
| `mode` | integer | 0 → 1 | `downward` | 🎚️ | set mode |
| `threshold` | float | 0.000976563 → 1 | `0.125` | 🎚️ | set threshold |
| `ratio` | float | 1 → 20 | `2` | 🎚️ | set ratio |
| `attack` | float | 0.01 → 2000 | `20` | 🎚️ | set attack |
| `release` | float | 0.01 → 9000 | `250` | 🎚️ | set release |
| `makeup` | float | 1 → 64 | `1` | 🎚️ | set make up gain |
| `knee` | float | 1 → 8 | `2.82843` | 🎚️ | set knee |
| `link` | integer | 0 → 1 | `average` | 🎚️ | set link type |
| `detection` | integer | 0 → 1 | `rms` | 🎚️ | set detection |
| `level_sc` | float | 0.015625 → 64 | `1` | 🎚️ | set sidechain gain |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acompressor" output.wav
```


---

### acontrast

Simple audio dynamic range compression/expansion filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `contrast` | float | 0 → 100 | `33` | — | set contrast |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acontrast" output.wav
```


---

### acopy

Copy the input audio unchanged to the output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acopy" output.wav
```


---

### acrossfade

Cross fade two input audio streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `nb_samples` | integer | 1 → 214748000.0 | `44100` | — | set number of samples for cross fade duration |
| `ns` | integer | 1 → 214748000.0 | `44100` | — | set number of samples for cross fade duration |
| `duration` | duration | — | `0` | — | set cross fade duration |
| `d` | duration | — | `0` | — | set cross fade duration |
| `overlap` | boolean | — | `true` | — | overlap 1st stream end with 2nd stream start |
| `o` | boolean | — | `true` | — | overlap 1st stream end with 2nd stream start |
| `curve1` | integer | -1 → 22 | `tri` | — | set fade curve type for 1st stream |
| `c1` | integer | -1 → 22 | `tri` | — | set fade curve type for 1st stream |
| `curve2` | integer | -1 → 22 | `tri` | — | set fade curve type for 2nd stream |
| `c2` | integer | -1 → 22 | `tri` | — | set fade curve type for 2nd stream |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acrossfade" output.wav
```


---

### acrossover

Split audio into per-bands streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `split` | string | — | `"500"` | — | set split frequencies |
| `order` | integer | 0 → 9 | `4th` | — | set filter order |
| `level` | float | 0 → 1 | `1` | — | set input gain |
| `gain` | string | — | `"1.f"` | — | set output bands gain |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acrossover" output.wav
```


---

### acrusher

Reduce audio bit resolution.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set level in |
| `level_out` | float | 0.015625 → 64 | `1` | 🎚️ | set level out |
| `bits` | float | 1 → 64 | `8` | 🎚️ | set bit reduction |
| `mix` | float | 0 → 1 | `0.5` | 🎚️ | set mix |
| `mode` | integer | 0 → 1 | `lin` | 🎚️ | set mode |
| `dc` | float | 0.25 → 4 | `1` | 🎚️ | set DC |
| `aa` | float | 0 → 1 | `0.5` | 🎚️ | set anti-aliasing |
| `samples` | float | 1 → 250 | `1` | 🎚️ | set sample reduction |
| `lfo` | boolean | — | `false` | 🎚️ | enable LFO |
| `lforange` | float | 1 → 250 | `20` | 🎚️ | set LFO depth |
| `lforate` | float | 0.01 → 200 | `0.3` | 🎚️ | set LFO rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acrusher" output.wav
```


---

### acue

Delay filtering to match a cue.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `cue` | integer | ≥ 0 | `0` | — | cue unix timestamp in microseconds |
| `preroll` | duration | — | `0` | — | preroll duration in seconds |
| `buffer` | duration | — | `0` | — | buffer duration in seconds |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "acue" output.wav
```


---

### adeclick

Remove impulsive noise from input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `window` | float | 10 → 100 | `55` | — | set window size |
| `w` | float | 10 → 100 | `55` | — | set window size |
| `overlap` | float | 50 → 95 | `75` | — | set window overlap |
| `o` | float | 50 → 95 | `75` | — | set window overlap |
| `arorder` | float | 0 → 25 | `2` | — | set autoregression order |
| `a` | float | 0 → 25 | `2` | — | set autoregression order |
| `threshold` | float | 1 → 100 | `2` | — | set threshold |
| `t` | float | 1 → 100 | `2` | — | set threshold |
| `burst` | float | 0 → 10 | `2` | — | set burst fusion |
| `b` | float | 0 → 10 | `2` | — | set burst fusion |
| `method` | integer | 0 → 1 | `add` | — | set overlap method |
| `m` | integer | 0 → 1 | `add` | — | set overlap method |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adeclick" output.wav
```


---

### adeclip

Remove clipping from input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `window` | float | 10 → 100 | `55` | — | set window size |
| `w` | float | 10 → 100 | `55` | — | set window size |
| `overlap` | float | 50 → 95 | `75` | — | set window overlap |
| `o` | float | 50 → 95 | `75` | — | set window overlap |
| `arorder` | float | 0 → 25 | `8` | — | set autoregression order |
| `a` | float | 0 → 25 | `8` | — | set autoregression order |
| `threshold` | float | 1 → 100 | `10` | — | set threshold |
| `t` | float | 1 → 100 | `10` | — | set threshold |
| `hsize` | integer | 100 → 9999 | `1000` | — | set histogram size |
| `n` | integer | 100 → 9999 | `1000` | — | set histogram size |
| `method` | integer | 0 → 1 | `add` | — | set overlap method |
| `m` | integer | 0 → 1 | `add` | — | set overlap method |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adeclip" output.wav
```


---

### adecorrelate

Apply decorrelation to input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `stages` | integer | 1 → 16 | `6` | — | set filtering stages |
| `seed` | integer | ≥ -1 | `-1` | — | set random seed |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adecorrelate" output.wav
```


---

### adelay

Delay one or more audio channels.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `delays` | string | — | — | 🎚️ | set list of delays for each channel |
| `all` | boolean | — | `false` | — | use last available delay for remained channels |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adelay" output.wav
```


---

### adenorm

Remedy denormals by adding extremely low-level noise.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level` | float | -451 → -90 | `-351` | 🎚️ | set level |
| `type` | integer | 0 → 3 | `dc` | 🎚️ | set type |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adenorm" output.wav
```


---

### aderivative

Compute derivative of input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aderivative" output.wav
```


---

### adrawgraph

Draw a graph using input audio metadata.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `m1` | string | — | `""` | — | set 1st metadata key |
| `fg1` | string | — | `"0xffff0000"` | — | set 1st foreground color expression |
| `m2` | string | — | `""` | — | set 2nd metadata key |
| `fg2` | string | — | `"0xff00ff00"` | — | set 2nd foreground color expression |
| `m3` | string | — | `""` | — | set 3rd metadata key |
| `fg3` | string | — | `"0xffff00ff"` | — | set 3rd foreground color expression |
| `m4` | string | — | `""` | — | set 4th metadata key |
| `fg4` | string | — | `"0xffffff00"` | — | set 4th foreground color expression |
| `bg` | color | — | `"white"` | — | set background color |
| `min` | float | — | `-1` | — | set minimal value |
| `max` | float | — | `1` | — | set maximal value |
| `mode` | integer | 0 → 2 | `line` | — | set graph mode |
| `slide` | integer | 0 → 4 | `frame` | — | set slide mode |
| `size` | image_size | — | `"900x256"` | — | set graph size |
| `s` | image_size | — | `"900x256"` | — | set graph size |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adrawgraph" output.wav
```


---

### adrc

Audio Spectral Dynamic Range Controller.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `transfer` | string | — | `"p"` | 🎚️ | set the transfer expression |
| `attack` | float | 1 → 1000 | `50` | 🎚️ | set the attack |
| `release` | float | 5 → 2000 | `100` | 🎚️ | set the release |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adrc" output.wav
```


---

### adynamicequalizer

Apply Dynamic Equalization of input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `threshold` | float | 0 → 100 | `0` | 🎚️ | set detection threshold |
| `dfrequency` | float | 2 → 1000000.0 | `1000` | 🎚️ | set detection frequency |
| `dqfactor` | float | 0.001 → 1000 | `1` | 🎚️ | set detection Q factor |
| `tfrequency` | float | 2 → 1000000.0 | `1000` | 🎚️ | set target frequency |
| `tqfactor` | float | 0.001 → 1000 | `1` | 🎚️ | set target Q factor |
| `attack` | float | 0.01 → 2000 | `20` | 🎚️ | set detection attack duration |
| `release` | float | 0.01 → 2000 | `200` | 🎚️ | set detection release duration |
| `ratio` | float | 0 → 30 | `1` | 🎚️ | set ratio factor |
| `makeup` | float | 0 → 1000 | `0` | 🎚️ | set makeup gain |
| `range` | float | 1 → 2000 | `50` | 🎚️ | set max gain |
| `mode` | integer | -1 → 3 | `cutbelow` | 🎚️ | set mode |
| `dftype` | integer | 0 → 3 | `bandpass` | 🎚️ | set detection filter type |
| `tftype` | integer | 0 → 2 | `bell` | 🎚️ | set target filter type |
| `auto` | integer | 1 → 4 | `off` | 🎚️ | set auto threshold |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adynamicequalizer" output.wav
```


---

### adynamicsmooth

Apply Dynamic Smoothing of input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sensitivity` | float | 0 → 1000000.0 | `2` | 🎚️ | set smooth sensitivity |
| `basefreq` | float | 2 → 1000000.0 | `22050` | 🎚️ | set base frequency |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "adynamicsmooth" output.wav
```


---

### aecho

Add echoing to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `in_gain` | float | 0 → 1 | `0.6` | — | set signal input gain |
| `out_gain` | float | 0 → 1 | `0.3` | — | set signal output gain |
| `delays` | string | — | `"1000"` | — | set list of signal delays |
| `decays` | string | — | `"0.5"` | — | set list of signal decays |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aecho" output.wav
```


---

### aemphasis

Audio emphasis.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0 → 64 | `1` | 🎚️ | set input gain |
| `level_out` | float | 0 → 64 | `1` | 🎚️ | set output gain |
| `mode` | integer | 0 → 1 | `reproduction` | 🎚️ | set filter mode |
| `type` | integer | 0 → 8 | `cd` | 🎚️ | set filter type |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aemphasis" output.wav
```


---

### aeval

Filter audio signal according to a specified expression.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `exprs` | string | — | — | — | set the '\|'-separated list of channels expressions |
| `channel_layout` | string | — | — | — | set channel layout |
| `c` | string | — | — | — | set channel layout |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aeval" output.wav
```


---

### aevalsrc

Generate an audio signal generated by an expression.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `exprs` | string | — | — | — | set the '\|'-separated list of channels expressions |
| `nb_samples` | integer | ≥ 0 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 0 | `1024` | — | set the number of samples per requested frame |
| `sample_rate` | string | — | `"44100"` | — | set the sample rate |
| `s` | string | — | `"44100"` | — | set the sample rate |
| `duration` | duration | — | `-1e-06` | — | set audio duration |
| `d` | duration | — | `-1e-06` | — | set audio duration |
| `channel_layout` | string | — | — | — | set channel layout |
| `c` | string | — | — | — | set channel layout |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aevalsrc" output.wav
```


---

### aexciter

Enhance high frequency part of audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0 → 64 | `1` | 🎚️ | set level in |
| `level_out` | float | 0 → 64 | `1` | 🎚️ | set level out |
| `amount` | float | 0 → 64 | `1` | 🎚️ | set amount |
| `drive` | float | 0.1 → 10 | `8.5` | 🎚️ | set harmonics |
| `blend` | float | -10 → 10 | `0` | 🎚️ | set blend harmonics |
| `freq` | float | 2000 → 12000 | `7500` | 🎚️ | set scope |
| `ceil` | float | 9999 → 20000 | `9999` | 🎚️ | set ceiling |
| `listen` | boolean | — | `false` | 🎚️ | enable listen mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aexciter" output.wav
```


---

### afade

Fade in/out input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `type` | integer | 0 → 1 | `in` | 🎚️ | set the fade direction |
| `t` | integer | 0 → 1 | `in` | 🎚️ | set the fade direction |
| `start_sample` | integer | ≥ 0 | `0` | 🎚️ | set number of first sample to start fading |
| `ss` | integer | ≥ 0 | `0` | 🎚️ | set number of first sample to start fading |
| `nb_samples` | integer | ≥ 1 | `44100` | 🎚️ | set number of samples for fade duration |
| `ns` | integer | ≥ 1 | `44100` | 🎚️ | set number of samples for fade duration |
| `start_time` | duration | — | `0` | 🎚️ | set time to start fading |
| `st` | duration | — | `0` | 🎚️ | set time to start fading |
| `duration` | duration | — | `0` | 🎚️ | set fade duration |
| `d` | duration | — | `0` | 🎚️ | set fade duration |
| `curve` | integer | -1 → 22 | `tri` | 🎚️ | set fade curve type |
| `c` | integer | -1 → 22 | `tri` | 🎚️ | set fade curve type |
| `silence` | float | 0 → 1 | `0` | 🎚️ | set the silence gain |
| `unity` | float | 0 → 1 | `1` | 🎚️ | set the unity gain |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afade" output.wav
```


---

### afdelaysrc

Generate a Fractional delay FIR coefficients.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `delay` | float | 0 → 32767 | `0` | — | set fractional delay |
| `d` | float | 0 → 32767 | `0` | — | set fractional delay |
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `taps` | integer | 0 → 32768 | `0` | — | set number of taps for delay filter |
| `t` | integer | 0 → 32768 | `0` | — | set number of taps for delay filter |
| `channel_layout` | channel_layout | — | `"stereo"` | — | set channel layout |
| `c` | channel_layout | — | `"stereo"` | — | set channel layout |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afdelaysrc" output.wav
```


---

### afftdn

Denoise audio samples using FFT.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `noise_reduction` | float | 0.01 → 97 | `12` | 🎚️ | set the noise reduction |
| `nr` | float | 0.01 → 97 | `12` | 🎚️ | set the noise reduction |
| `noise_floor` | float | -80 → -20 | `-50` | 🎚️ | set the noise floor |
| `nf` | float | -80 → -20 | `-50` | 🎚️ | set the noise floor |
| `noise_type` | integer | 0 → 3 | `white` | — | set the noise type |
| `nt` | integer | 0 → 3 | `white` | — | set the noise type |
| `band_noise` | string | — | — | — | set the custom bands noise |
| `bn` | string | — | — | — | set the custom bands noise |
| `residual_floor` | float | -80 → -20 | `-38` | 🎚️ | set the residual floor |
| `rf` | float | -80 → -20 | `-38` | 🎚️ | set the residual floor |
| `track_noise` | boolean | — | `false` | 🎚️ | track noise |
| `tn` | boolean | — | `false` | 🎚️ | track noise |
| `track_residual` | boolean | — | `false` | 🎚️ | track residual |
| `tr` | boolean | — | `false` | 🎚️ | track residual |
| `output_mode` | integer | 0 → 2 | `output` | 🎚️ | set output mode |
| `om` | integer | 0 → 2 | `output` | 🎚️ | set output mode |
| `adaptivity` | float | 0 → 1 | `0.5` | 🎚️ | set adaptivity factor |
| `ad` | float | 0 → 1 | `0.5` | 🎚️ | set adaptivity factor |
| `floor_offset` | float | -2 → 2 | `1` | 🎚️ | set noise floor offset factor |
| `fo` | float | -2 → 2 | `1` | 🎚️ | set noise floor offset factor |
| `noise_link` | integer | 0 → 3 | `min` | 🎚️ | set the noise floor link |
| `nl` | integer | 0 → 3 | `min` | 🎚️ | set the noise floor link |
| `band_multiplier` | float | 0.2 → 5 | `1.25` | — | set band multiplier |
| `bm` | float | 0.2 → 5 | `1.25` | — | set band multiplier |
| `sample_noise` | integer | 0 → 2 | `none` | 🎚️ | set sample noise mode |
| `sn` | integer | 0 → 2 | `none` | 🎚️ | set sample noise mode |
| `gain_smooth` | integer | 0 → 50 | `0` | 🎚️ | set gain smooth radius |
| `gs` | integer | 0 → 50 | `0` | 🎚️ | set gain smooth radius |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afftdn" output.wav
```


---

### afftfilt

Apply arbitrary expressions to samples in frequency domain.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `real` | string | — | `"re"` | — | set channels real expressions |
| `imag` | string | — | `"im"` | — | set channels imaginary expressions |
| `win_size` | integer | 16 → 131072 | `4096` | — | set window size |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `overlap` | float | 0 → 1 | `0.75` | — | set window overlap |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afftfilt" output.wav
```


---

### afir

Apply Finite Impulse Response filter with supplied coefficients in additional stream(s).

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `dry` | float | 0 → 10 | `1` | 🎚️ | set dry gain |
| `wet` | float | 0 → 10 | `1` | 🎚️ | set wet gain |
| `length` | float | 0 → 1 | `1` | — | set IR length |
| `gtype` | integer | -1 → 4 | `peak` | — | set IR auto gain type |
| `irnorm` | float | -1 → 2 | `1` | — | set IR norm |
| `irlink` | boolean | — | `true` | — | set IR link |
| `irgain` | float | 0 → 1 | `1` | — | set IR gain |
| `irfmt` | integer | 0 → 1 | `input` | — | set IR format |
| `maxir` | float | 0.1 → 60 | `30` | — | set max IR length |
| `response` | boolean | — | `false` | — | show IR frequency response |
| `channel` | integer | 0 → 1024 | `0` | — | set IR channel to display frequency response |
| `size` | image_size | — | `"hd720"` | — | set video size |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `minp` | integer | 1 → 65536 | `8192` | — | set min partition size |
| `maxp` | integer | 8 → 65536 | `8192` | — | set max partition size |
| `nbirs` | integer | 1 → 32 | `1` | — | set number of input IRs |
| `ir` | integer | 0 → 31 | `0` | 🎚️ | select IR |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |
| `irload` | integer | 0 → 1 | `init` | — | set IR loading type |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afir" output.wav
```


---

### afireqsrc

Generate a FIR equalizer coefficients audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `preset` | integer | -1 → 17 | `flat` | — | set equalizer preset |
| `p` | integer | -1 → 17 | `flat` | — | set equalizer preset |
| `gains` | string | — | `"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"` | — | set gain values per band |
| `g` | string | — | `"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"` | — | set gain values per band |
| `bands` | string | — | `"25 40 63 100 160 250 400 630 1000 1600 2500 4000 6300 10000 16000 24000"` | — | set central frequency values per band |
| `b` | string | — | `"25 40 63 100 160 250 400 630 1000 1600 2500 4000 6300 10000 16000 24000"` | — | set central frequency values per band |
| `taps` | integer | 16 → 65535 | `4096` | — | set number of taps |
| `t` | integer | 16 → 65535 | `4096` | — | set number of taps |
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `interp` | integer | 0 → 1 | `linear` | — | set the interpolation |
| `i` | integer | 0 → 1 | `linear` | — | set the interpolation |
| `phase` | integer | 0 → 1 | `min` | — | set the phase |
| `h` | integer | 0 → 1 | `min` | — | set the phase |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afireqsrc" output.wav
```


---

### afirsrc

Generate a FIR coefficients audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `taps` | integer | 9 → 65535 | `1025` | — | set number of taps |
| `t` | integer | 9 → 65535 | `1025` | — | set number of taps |
| `frequency` | string | — | `"0 1"` | — | set frequency points |
| `f` | string | — | `"0 1"` | — | set frequency points |
| `magnitude` | string | — | `"1 1"` | — | set magnitude values |
| `m` | string | — | `"1 1"` | — | set magnitude values |
| `phase` | string | — | `"0 0"` | — | set phase values |
| `p` | string | — | `"0 0"` | — | set phase values |
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `win_func` | integer | 0 → 20 | `blackman` | — | set window function |
| `w` | integer | 0 → 20 | `blackman` | — | set window function |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afirsrc" output.wav
```


---

### aformat

Convert the input audio to one of the specified formats.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aformat" output.wav
```


---

### afreqshift

Apply frequency shifting to input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `shift` | float | ≥ -2147480000.0 | `0` | 🎚️ | set frequency shift |
| `level` | float | 0 → 1 | `1` | 🎚️ | set output level |
| `order` | integer | 1 → 16 | `8` | 🎚️ | set filter order |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afreqshift" output.wav
```


---

### afwtdn

Denoise audio stream using Wavelets.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sigma` | float | 0 → 1 | `0` | 🎚️ | set noise sigma |
| `levels` | integer | 1 → 12 | `10` | — | set number of wavelet levels |
| `wavet` | integer | 0 → 6 | `sym10` | — | set wavelet type |
| `percent` | float | 0 → 100 | `85` | 🎚️ | set percent of full denoising |
| `profile` | boolean | — | `false` | 🎚️ | profile noise |
| `adaptive` | boolean | — | `false` | 🎚️ | adaptive profiling of noise |
| `samples` | integer | 512 → 65536 | `8192` | — | set frame size in number of samples |
| `softness` | float | 0 → 10 | `1` | 🎚️ | set thresholding softness |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "afwtdn" output.wav
```


---

### agate

Audio gate.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input level |
| `mode` | integer | 0 → 1 | `downward` | 🎚️ | set mode |
| `range` | float | 0 → 1 | `0.06125` | 🎚️ | set max gain reduction |
| `threshold` | float | 0 → 1 | `0.125` | 🎚️ | set threshold |
| `ratio` | float | 1 → 9000 | `2` | 🎚️ | set ratio |
| `attack` | float | 0.01 → 9000 | `20` | 🎚️ | set attack |
| `release` | float | 0.01 → 9000 | `250` | 🎚️ | set release |
| `makeup` | float | 1 → 64 | `1` | 🎚️ | set makeup gain |
| `knee` | float | 1 → 8 | `2.82843` | 🎚️ | set knee |
| `detection` | integer | 0 → 1 | `rms` | 🎚️ | set detection |
| `link` | integer | 0 → 1 | `average` | 🎚️ | set link |
| `level_sc` | float | 0.015625 → 64 | `1` | 🎚️ | set sidechain gain |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "agate" output.wav
```


---

### agraphmonitor

Show various filtergraph stats.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"hd720"` | — | set monitor size |
| `s` | image_size | — | `"hd720"` | — | set monitor size |
| `opacity` | float | 0 → 1 | `0.9` | 🎚️ | set video opacity |
| `o` | float | 0 → 1 | `0.9` | 🎚️ | set video opacity |
| `mode` | flags | — | `0` | 🎚️ | set mode |
| `m` | flags | — | `0` | 🎚️ | set mode |
| `flags` | flags | — | `all+queue` | 🎚️ | set flags |
| `f` | flags | — | `all+queue` | 🎚️ | set flags |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "agraphmonitor" output.wav
```


---

### ahistogram

Convert input audio to histogram video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `dmode` | integer | 0 → 1 | `single` | — | set method to display channels |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `size` | image_size | — | `"hd720"` | — | set video size |
| `s` | image_size | — | `"hd720"` | — | set video size |
| `scale` | integer | 0 → 4 | `log` | — | set display scale |
| `ascale` | integer | 0 → 1 | `log` | — | set amplitude scale |
| `acount` | integer | -1 → 100 | `1` | — | how much frames to accumulate |
| `rheight` | float | 0 → 1 | `0.1` | — | set histogram ratio of window height |
| `slide` | integer | 0 → 1 | `replace` | — | set sonogram sliding |
| `hmode` | integer | 0 → 1 | `abs` | — | set histograms mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "ahistogram" output.wav
```


---

### aiir

Apply Infinite Impulse Response filter with supplied coefficients.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `zeros` | string | — | `"1+0i 1-0i"` | — | set B/numerator/zeros/reflection coefficients |
| `z` | string | — | `"1+0i 1-0i"` | — | set B/numerator/zeros/reflection coefficients |
| `poles` | string | — | `"1+0i 1-0i"` | — | set A/denominator/poles/ladder coefficients |
| `p` | string | — | `"1+0i 1-0i"` | — | set A/denominator/poles/ladder coefficients |
| `gains` | string | — | `"1|1"` | — | set channels gains |
| `k` | string | — | `"1|1"` | — | set channels gains |
| `dry` | float | 0 → 1 | `1` | — | set dry gain |
| `wet` | float | 0 → 1 | `1` | — | set wet gain |
| `format` | integer | -2 → 4 | `zp` | — | set coefficients format |
| `f` | integer | -2 → 4 | `zp` | — | set coefficients format |
| `process` | integer | 0 → 2 | `s` | — | set kind of processing |
| `r` | integer | 0 → 2 | `s` | — | set kind of processing |
| `precision` | integer | 0 → 3 | `dbl` | — | set filtering precision |
| `e` | integer | 0 → 3 | `dbl` | — | set precision |
| `normalize` | boolean | — | `true` | — | normalize coefficients |
| `n` | boolean | — | `true` | — | normalize coefficients |
| `mix` | float | 0 → 1 | `1` | — | set mix |
| `response` | boolean | — | `false` | — | show IR frequency response |
| `channel` | integer | 0 → 1024 | `0` | — | set IR channel to display frequency response |
| `size` | image_size | — | `"hd720"` | — | set video size |
| `rate` | video_rate | — | `"25"` | — | set video rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aiir" output.wav
```


---

### aintegral

Compute integral of input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aintegral" output.wav
```


---

### ainterleave

Temporally interleave audio inputs.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `nb_inputs` | integer | ≥ 1 | `2` | — | set number of inputs |
| `n` | integer | ≥ 1 | `2` | — | set number of inputs |
| `duration` | integer | 0 → 2 | `longest` | — | how to determine the end-of-stream |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "ainterleave" output.wav
```


---

### alatency

Report audio filtering latency.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "alatency" output.wav
```


---

### alimiter

Audio lookahead limiter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input level |
| `level_out` | float | 0.015625 → 64 | `1` | 🎚️ | set output level |
| `limit` | float | 0.0625 → 1 | `1` | 🎚️ | set limit |
| `attack` | float | 0.1 → 80 | `5` | 🎚️ | set attack |
| `release` | float | 1 → 8000 | `50` | 🎚️ | set release |
| `asc` | boolean | — | `false` | 🎚️ | enable asc |
| `asc_level` | float | 0 → 1 | `0.5` | 🎚️ | set asc level |
| `level` | boolean | — | `true` | 🎚️ | auto level |
| `latency` | boolean | — | `false` | 🎚️ | compensate delay |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "alimiter" output.wav
```


---

### allpass

Apply a two-pole all-pass filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `order` | integer | 1 → 2 | `2` | 🎚️ | set filter order |
| `o` | integer | 1 → 2 | `2` | 🎚️ | set filter order |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "allpass" output.wav
```


---

### aloop

Loop audio samples.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `loop` | integer | ≥ -1 | `0` | — | number of loops |
| `size` | integer | ≥ 0 | `0` | — | max number of samples to loop |
| `start` | integer | ≥ -1 | `0` | — | set the loop start sample |
| `time` | duration | — | `INT64_MAX` | — | set the loop start time |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aloop" output.wav
```


---

### amerge

Merge two or more audio streams into a single multi-channel stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `inputs` | integer | 1 → 64 | `2` | — | specify the number of inputs |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "amerge" output.wav
```


---

### ametadata

Manipulate audio frame metadata.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `mode` | integer | 0 → 4 | `select` | — | set a mode of operation |
| `key` | string | — | — | — | set metadata key |
| `value` | string | — | — | — | set metadata value |
| `function` | integer | 0 → 6 | `same_str` | — | function for comparing values |
| `expr` | string | — | — | — | set expression for expr function |
| `file` | string | — | — | — | set file where to print metadata information |
| `direct` | boolean | — | `false` | — | reduce buffering when printing to user-set file or pipe |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "ametadata" output.wav
```


---

### amix

Audio mixing.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `inputs` | integer | 1 → 32767 | `2` | — | Number of inputs. |
| `duration` | integer | 0 → 2 | `longest` | — | How to determine the end-of-stream. |
| `dropout_transition` | float | ≥ 0 | `2` | — | Transition time, in seconds, for volume renormalization when an input stream ends. |
| `weights` | string | — | `"1 1"` | 🎚️ | Set weight for each input. |
| `normalize` | boolean | — | `true` | 🎚️ | Scale inputs |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "amix" output.wav
```


---

### amultiply

Multiply two audio streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "amultiply" output.wav
```


---

### anequalizer

Apply high-order audio parametric multi band equalizer.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `params` | string | — | `""` | — |  |
| `curves` | boolean | — | `false` | — | draw frequency response curves |
| `size` | image_size | — | `"hd720"` | — | set video size |
| `mgain` | float | -900 → 900 | `60` | — | set max gain |
| `fscale` | integer | 0 → 1 | `log` | — | set frequency scale |
| `colors` | string | — | `"red|green|blue|yellow|orange|lime|pink|magenta|brown"` | — | set channels curves colors |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anequalizer" output.wav
```


---

### anlmdn

Reduce broadband noise from stream using Non-Local Means.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `strength` | float | 1e-05 → 10000 | `1e-05` | 🎚️ | set denoising strength |
| `s` | float | 1e-05 → 10000 | `1e-05` | 🎚️ | set denoising strength |
| `patch` | duration | — | `0.002` | 🎚️ | set patch duration |
| `p` | duration | — | `0.002` | 🎚️ | set patch duration |
| `research` | duration | — | `0.006` | 🎚️ | set research duration |
| `r` | duration | — | `0.006` | 🎚️ | set research duration |
| `output` | integer | 0 → 2 | `o` | 🎚️ | set output mode |
| `o` | integer | 0 → 2 | `o` | 🎚️ | set output mode |
| `smooth` | float | 1 → 1000 | `11` | 🎚️ | set smooth factor |
| `m` | float | 1 → 1000 | `11` | 🎚️ | set smooth factor |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anlmdn" output.wav
```


---

### anlmf

Apply Normalized Least-Mean-Fourth algorithm to first audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `order` | integer | 1 → 32767 | `256` | — | set the filter order |
| `mu` | float | 0 → 2 | `0.75` | 🎚️ | set the filter mu |
| `eps` | float | 0 → 1 | `1` | 🎚️ | set the filter eps |
| `leakage` | float | 0 → 1 | `0` | 🎚️ | set the filter leakage |
| `out_mode` | integer | 0 → 4 | `o` | 🎚️ | set output mode |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anlmf" output.wav
```


---

### anlms

Apply Normalized Least-Mean-Squares algorithm to first audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `order` | integer | 1 → 32767 | `256` | — | set the filter order |
| `mu` | float | 0 → 2 | `0.75` | 🎚️ | set the filter mu |
| `eps` | float | 0 → 1 | `1` | 🎚️ | set the filter eps |
| `leakage` | float | 0 → 1 | `0` | 🎚️ | set the filter leakage |
| `out_mode` | integer | 0 → 4 | `o` | 🎚️ | set output mode |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anlms" output.wav
```


---

### anoisesrc

Generate a noise audio signal.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 15 | `48000` | — | set sample rate |
| `r` | integer | ≥ 15 | `48000` | — | set sample rate |
| `amplitude` | float | 0 → 1 | `1` | — | set amplitude |
| `a` | float | 0 → 1 | `1` | — | set amplitude |
| `duration` | duration | — | `0` | — | set duration |
| `d` | duration | — | `0` | — | set duration |
| `color` | integer | 0 → 5 | `white` | — | set noise color |
| `colour` | integer | 0 → 5 | `white` | — | set noise color |
| `c` | integer | 0 → 5 | `white` | — | set noise color |
| `seed` | integer | ≥ -1 | `-1` | — | set random seed |
| `s` | integer | ≥ -1 | `-1` | — | set random seed |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `density` | float | 0 → 1 | `0.05` | — | set density |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anoisesrc" output.wav
```


---

### anull

Pass the source unchanged to the output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anull" output.wav
```


---

### anullsink

Do absolutely nothing with the input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anullsink" output.wav
```


---

### anullsrc

Null audio source, return empty audio frames.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `channel_layout` | channel_layout | — | `"stereo"` | — | set channel_layout |
| `cl` | channel_layout | — | `"stereo"` | — | set channel_layout |
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `nb_samples` | integer | 1 → 65535 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | 1 → 65535 | `1024` | — | set the number of samples per requested frame |
| `duration` | duration | — | `-1e-06` | — | set the audio duration |
| `d` | duration | — | `-1e-06` | — | set the audio duration |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "anullsrc" output.wav
```


---

### apad

Pad audio with silence.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `packet_size` | integer | ≥ 0 | `4096` | — | set silence packet size |
| `pad_len` | integer | ≥ -1 | `-1` | — | set number of samples of silence to add |
| `whole_len` | integer | ≥ -1 | `-1` | — | set minimum target number of samples in the audio stream |
| `pad_dur` | duration | — | `-1e-06` | — | set duration of silence to add |
| `whole_dur` | duration | — | `-1e-06` | — | set minimum target duration in the audio stream |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "apad" output.wav
```


---

### aperms

Set permissions for the output audio frame.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `mode` | integer | 0 → 4 | `none` | 🎚️ | select permissions mode |
| `seed` | integer | ≥ -1 | `-1` | — | set the seed for the random mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aperms" output.wav
```


---

### aphasemeter

Convert input audio to phase meter video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `size` | image_size | — | `"800x400"` | — | set video size |
| `s` | image_size | — | `"800x400"` | — | set video size |
| `rc` | integer | 0 → 255 | `2` | — | set red contrast |
| `gc` | integer | 0 → 255 | `7` | — | set green contrast |
| `bc` | integer | 0 → 255 | `1` | — | set blue contrast |
| `mpc` | string | — | `"none"` | — | set median phase color |
| `video` | boolean | — | `true` | — | set video output |
| `phasing` | boolean | — | `false` | — | set mono and out-of-phase detection output |
| `tolerance` | float | 0 → 1 | `0` | — | set phase tolerance for mono detection |
| `t` | float | 0 → 1 | `0` | — | set phase tolerance for mono detection |
| `angle` | float | 90 → 180 | `170` | — | set angle threshold for out-of-phase detection |
| `a` | float | 90 → 180 | `170` | — | set angle threshold for out-of-phase detection |
| `duration` | duration | — | `2` | — | set minimum mono or out-of-phase duration in seconds |
| `d` | duration | — | `2` | — | set minimum mono or out-of-phase duration in seconds |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aphasemeter" output.wav
```


---

### aphaser

Add a phasing effect to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `in_gain` | float | 0 → 1 | `0.4` | — | set input gain |
| `out_gain` | float | 0 → 1000000000.0 | `0.74` | — | set output gain |
| `delay` | float | 0 → 5 | `3` | — | set delay in milliseconds |
| `decay` | float | 0 → 0.99 | `0.4` | — | set decay |
| `speed` | float | 0.1 → 2 | `0.5` | — | set modulation speed |
| `type` | integer | 0 → 1 | `triangular` | — | set modulation type |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aphaser" output.wav
```


---

### aphaseshift

Apply phase shifting to input audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `shift` | float | -1 → 1 | `0` | 🎚️ | set phase shift |
| `level` | float | 0 → 1 | `1` | 🎚️ | set output level |
| `order` | integer | 1 → 16 | `8` | 🎚️ | set filter order |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aphaseshift" output.wav
```


---

### apsnr

Measure Audio Peak Signal-to-Noise Ratio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "apsnr" output.wav
```


---

### apsyclip

Audio Psychoacoustic Clipper.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input level |
| `level_out` | float | 0.015625 → 64 | `1` | 🎚️ | set output level |
| `clip` | float | 0.015625 → 1 | `1` | 🎚️ | set clip level |
| `diff` | boolean | — | `false` | 🎚️ | enable difference |
| `adaptive` | float | 0 → 1 | `0.5` | 🎚️ | set adaptive distortion |
| `iterations` | integer | 1 → 20 | `10` | 🎚️ | set iterations |
| `level` | boolean | — | `false` | 🎚️ | set auto level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "apsyclip" output.wav
```


---

### apulsator

Audio pulsator.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | — | set input gain |
| `level_out` | float | 0.015625 → 64 | `1` | — | set output gain |
| `mode` | integer | 0 → 4 | `sine` | — | set mode |
| `amount` | float | 0 → 1 | `1` | — | set modulation |
| `offset_l` | float | 0 → 1 | `0` | — | set offset L |
| `offset_r` | float | 0 → 1 | `0.5` | — | set offset R |
| `width` | float | 0 → 2 | `1` | — | set pulse width |
| `timing` | integer | 0 → 2 | `hz` | — | set timing |
| `bpm` | float | 30 → 300 | `120` | — | set BPM |
| `ms` | integer | 10 → 2000 | `500` | — | set ms |
| `hz` | float | 0.01 → 100 | `2` | — | set frequency |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "apulsator" output.wav
```


---

### arealtime

Slow down filtering to match realtime.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `limit` | duration | — | `2` | 🎚️ | sleep time limit |
| `speed` | float | — | `1` | 🎚️ | speed factor |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "arealtime" output.wav
```


---

### aresample

Resample audio data.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 0 | `0` | — |  |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aresample" output.wav
```


---

### areverse

Reverse an audio clip.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "areverse" output.wav
```


---

### arls

Apply Recursive Least Squares algorithm to first audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `order` | integer | 1 → 32767 | `16` | — | set the filter order |
| `lambda` | float | 0 → 1 | `1` | 🎚️ | set the filter lambda |
| `delta` | float | 0 → 32767 | `2` | — | set the filter delta |
| `out_mode` | integer | 0 → 4 | `o` | 🎚️ | set output mode |
| `precision` | integer | 0 → 2 | `auto` | — | set processing precision |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "arls" output.wav
```


---

### arnndn

Reduce noise from speech using Recurrent Neural Networks.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `model` | string | — | — | 🎚️ | set model name |
| `m` | string | — | — | 🎚️ | set model name |
| `mix` | float | -1 → 1 | `1` | 🎚️ | set output vs input mix |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "arnndn" output.wav
```


---

### asdr

Measure Audio Signal-to-Distortion Ratio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asdr" output.wav
```


---

### asegment

Segment audio stream.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `timestamps` | string | — | — | — | timestamps of input at which to split input |
| `samples` | string | — | — | — | samples at which to split input |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asegment" output.wav
```


---

### aselect

Select audio frames to pass in output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `expr` | string | — | `"1"` | — | set an expression to use for selecting frames |
| `e` | string | — | `"1"` | — | set an expression to use for selecting frames |
| `outputs` | integer | ≥ 1 | `1` | — | set the number of outputs |
| `n` | integer | ≥ 1 | `1` | — | set the number of outputs |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aselect" output.wav
```


---

### asendcmd

Send commands to filters.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `commands` | string | — | — | — | set commands |
| `c` | string | — | — | — | set commands |
| `filename` | string | — | — | — | set commands file |
| `f` | string | — | — | — | set commands file |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asendcmd" output.wav
```


---

### asetnsamples

Set the number of samples for each output audio frames.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `nb_out_samples` | integer | ≥ 1 | `1024` | 🎚️ | set the number of per-frame output samples |
| `n` | integer | ≥ 1 | `1024` | 🎚️ | set the number of per-frame output samples |
| `pad` | boolean | — | `true` | 🎚️ | pad last frame with zeros |
| `p` | boolean | — | `true` | 🎚️ | pad last frame with zeros |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asetnsamples" output.wav
```


---

### asetpts

Set PTS for the output audio frame.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `expr` | string | — | `"PTS"` | 🎚️ | Expression determining the frame timestamp |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asetpts" output.wav
```


---

### asetrate

Change the sample rate without altering the data.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 1 | `44100` | — | set the sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set the sample rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asetrate" output.wav
```


---

### asettb

Set timebase for the audio output link.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `expr` | string | — | `"intb"` | — | set expression determining the output timebase |
| `tb` | string | — | `"intb"` | — | set expression determining the output timebase |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asettb" output.wav
```


---

### ashowinfo

Show textual information for each audio frame.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "ashowinfo" output.wav
```


---

### asidedata

Manipulate audio frame side data.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `mode` | integer | 0 → 1 | `select` | — | set a mode of operation |
| `type` | integer | ≥ -1 | `-1` | — | set side data type |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asidedata" output.wav
```


---

### asisdr

Measure Audio Scale-Invariant Signal-to-Distortion Ratio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asisdr" output.wav
```


---

### asoftclip

Audio Soft Clipper.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `type` | integer | -1 → 7 | `tanh` | 🎚️ | set softclip type |
| `threshold` | float | 1e-06 → 1 | `1` | 🎚️ | set softclip threshold |
| `output` | float | 1e-06 → 16 | `1` | 🎚️ | set softclip output gain |
| `param` | float | 0.01 → 3 | `1` | 🎚️ | set softclip parameter |
| `oversample` | integer | 1 → 64 | `1` | 🎚️ | set oversample factor |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asoftclip" output.wav
```


---

### aspectralstats

Show frequency domain statistics about audio frames.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `win_size` | integer | 32 → 65536 | `2048` | — | set the window size |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `overlap` | float | 0 → 1 | `0.5` | — | set window overlap |
| `measure` | flags | — | `all+mean+variance+centroid+spread+skewness+kurtosis+entropy+flatness+crest+flux+slope+decrease+rolloff` | — | select the parameters which are measured |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "aspectralstats" output.wav
```


---

### asplit

Pass on the audio input to N audio outputs.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `outputs` | integer | ≥ 1 | `2` | — | set number of outputs |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asplit" output.wav
```


---

### astats

Show time domain statistics about audio frames.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `length` | float | 0 → 10 | `0.05` | — | set the window length |
| `metadata` | boolean | — | `false` | — | inject metadata in the filtergraph |
| `reset` | integer | ≥ 0 | `0` | — | Set the number of frames over which cumulative stats are calculated before being reset |
| `measure_perchannel` | flags | — | `all+Bit_depth+Crest_factor+DC_offset+Dynamic_range+Entropy+Flat_factor+Max_difference+Max_level+Mean_difference+Min_difference+Min_level+Noise_floor+Noise_floor_count+Number_of_Infs+Number_of_NaNs+Number_of_denormals+Number_of_samples+Peak_count+Peak_level+RMS_difference+RMS_level+RMS_peak+RMS_trough+Zero_crossings+Zero_crossings_rate+Abs_Peak_count` | — | Select the parameters which are measured per channel |
| `measure_overall` | flags | — | `all+Bit_depth+Crest_factor+DC_offset+Dynamic_range+Entropy+Flat_factor+Max_difference+Max_level+Mean_difference+Min_difference+Min_level+Noise_floor+Noise_floor_count+Number_of_Infs+Number_of_NaNs+Number_of_denormals+Number_of_samples+Peak_count+Peak_level+RMS_difference+RMS_level+RMS_peak+RMS_trough+Zero_crossings+Zero_crossings_rate+Abs_Peak_count` | — | Select the parameters which are measured overall |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "astats" output.wav
```


---

### asubboost

Boost subwoofer frequencies.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `dry` | float | 0 → 1 | `1` | 🎚️ | set dry gain |
| `wet` | float | 0 → 1 | `1` | 🎚️ | set wet gain |
| `boost` | float | 1 → 12 | `2` | 🎚️ | set max boost |
| `decay` | float | 0 → 1 | `0` | 🎚️ | set decay |
| `feedback` | float | 0 → 1 | `0.9` | 🎚️ | set feedback |
| `cutoff` | float | 50 → 900 | `100` | 🎚️ | set cutoff |
| `slope` | float | 0.0001 → 1 | `0.5` | 🎚️ | set slope |
| `delay` | float | 1 → 100 | `20` | 🎚️ | set delay |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asubboost" output.wav
```


---

### asubcut

Cut subwoofer frequencies.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `cutoff` | float | 2 → 200 | `20` | 🎚️ | set cutoff frequency |
| `order` | integer | 3 → 20 | `10` | 🎚️ | set filter order |
| `level` | float | 0 → 1 | `1` | 🎚️ | set input level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asubcut" output.wav
```


---

### asupercut

Cut super frequencies.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `cutoff` | float | 20000 → 192000 | `20000` | 🎚️ | set cutoff frequency |
| `order` | integer | 3 → 20 | `10` | 🎚️ | set filter order |
| `level` | float | 0 → 1 | `1` | 🎚️ | set input level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asupercut" output.wav
```


---

### asuperpass

Apply high order Butterworth band-pass filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `centerf` | float | 2 → 999999 | `1000` | 🎚️ | set center frequency |
| `order` | integer | 4 → 20 | `4` | 🎚️ | set filter order |
| `qfactor` | float | 0.01 → 100 | `1` | 🎚️ | set Q-factor |
| `level` | float | 0 → 2 | `1` | 🎚️ | set input level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asuperpass" output.wav
```


---

### asuperstop

Apply high order Butterworth band-stop filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `centerf` | float | 2 → 999999 | `1000` | 🎚️ | set center frequency |
| `order` | integer | 4 → 20 | `4` | 🎚️ | set filter order |
| `qfactor` | float | 0.01 → 100 | `1` | 🎚️ | set Q-factor |
| `level` | float | 0 → 2 | `1` | 🎚️ | set input level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "asuperstop" output.wav
```


---

### atempo

Adjust audio tempo.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `tempo` | float | 0.5 → 100 | `1` | 🎚️ | set tempo scale factor |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "atempo" output.wav
```


---

### atilt

Apply spectral tilt to audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `freq` | float | 20 → 192000 | `10000` | 🎚️ | set central frequency |
| `slope` | float | -1 → 1 | `0` | 🎚️ | set filter slope |
| `width` | float | 100 → 10000 | `1000` | 🎚️ | set filter width |
| `order` | integer | 2 → 30 | `5` | 🎚️ | set filter order |
| `level` | float | 0 → 4 | `1` | 🎚️ | set input level |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "atilt" output.wav
```


---

### atrim

Pick one continuous section from the input, drop the rest.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `start` | duration | — | `INT64_MAX` | — | Timestamp of the first frame that should be passed |
| `starti` | duration | — | `INT64_MAX` | — | Timestamp of the first frame that should be passed |
| `end` | duration | — | `INT64_MAX` | — | Timestamp of the first frame that should be dropped again |
| `endi` | duration | — | `INT64_MAX` | — | Timestamp of the first frame that should be dropped again |
| `start_pts` | integer | — | `I64_MIN` | — | Timestamp of the first frame that should be  passed |
| `end_pts` | integer | — | `I64_MIN` | — | Timestamp of the first frame that should be dropped again |
| `duration` | duration | — | `0` | — | Maximum duration of the output |
| `durationi` | duration | — | `0` | — | Maximum duration of the output |
| `start_sample` | integer | ≥ -1 | `-1` | — | Number of the first audio sample that should be passed to the output |
| `end_sample` | integer | ≥ 0 | `I64_MAX` | — | Number of the first audio sample that should be dropped again |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "atrim" output.wav
```


---

### avectorscope

Convert input audio to vectorscope video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `mode` | integer | 0 → 2 | `lissajous` | 🎚️ | set mode |
| `m` | integer | 0 → 2 | `lissajous` | 🎚️ | set mode |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `size` | image_size | — | `"400x400"` | — | set video size |
| `s` | image_size | — | `"400x400"` | — | set video size |
| `rc` | integer | 0 → 255 | `40` | 🎚️ | set red contrast |
| `gc` | integer | 0 → 255 | `160` | 🎚️ | set green contrast |
| `bc` | integer | 0 → 255 | `80` | 🎚️ | set blue contrast |
| `ac` | integer | 0 → 255 | `255` | 🎚️ | set alpha contrast |
| `rf` | integer | 0 → 255 | `15` | 🎚️ | set red fade |
| `gf` | integer | 0 → 255 | `10` | 🎚️ | set green fade |
| `bf` | integer | 0 → 255 | `5` | 🎚️ | set blue fade |
| `af` | integer | 0 → 255 | `5` | 🎚️ | set alpha fade |
| `zoom` | float | 0 → 10 | `1` | 🎚️ | set zoom factor |
| `draw` | integer | 0 → 2 | `dot` | 🎚️ | set draw mode |
| `scale` | integer | 0 → 3 | `lin` | 🎚️ | set amplitude scale mode |
| `swap` | boolean | — | `true` | 🎚️ | swap x axis with y axis |
| `mirror` | integer | 0 → 3 | `none` | 🎚️ | mirror axis |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "avectorscope" output.wav
```


---

### avsynctest

Generate an Audio Video Sync Test.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"hd720"` | — | set frame size |
| `s` | image_size | — | `"hd720"` | — | set frame size |
| `framerate` | video_rate | — | `"30"` | — | set frame rate |
| `fr` | video_rate | — | `"30"` | — | set frame rate |
| `samplerate` | integer | 8000 → 384000 | `44100` | — | set sample rate |
| `sr` | integer | 8000 → 384000 | `44100` | — | set sample rate |
| `amplitude` | float | 0 → 1 | `0.7` | 🎚️ | set beep amplitude |
| `a` | float | 0 → 1 | `0.7` | 🎚️ | set beep amplitude |
| `period` | integer | 1 → 99 | `3` | — | set beep period |
| `p` | integer | 1 → 99 | `3` | — | set beep period |
| `delay` | integer | -30 → 30 | `0` | 🎚️ | set flash delay |
| `dl` | integer | -30 → 30 | `0` | 🎚️ | set flash delay |
| `cycle` | boolean | — | `false` | 🎚️ | set delay cycle |
| `c` | boolean | — | `false` | 🎚️ | set delay cycle |
| `duration` | duration | — | `0` | — | set duration |
| `d` | duration | — | `0` | — | set duration |
| `fg` | color | — | `"white"` | — | set foreground color |
| `bg` | color | — | `"black"` | — | set background color |
| `ag` | color | — | `"gray"` | — | set additional color |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "avsynctest" output.wav
```


---

### axcorrelate

Cross-correlate two audio streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | integer | 2 → 131072 | `256` | — | set the segment size |
| `algo` | integer | 0 → 2 | `best` | — | set the algorithm |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "axcorrelate" output.wav
```


---

### azmq

Receive commands through ZMQ and broker them to filters.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `bind_address` | string | — | `"tcp://*:5555"` | — | set bind address |
| `b` | string | — | `"tcp://*:5555"` | — | set bind address |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "azmq" output.wav
```


---

### bandpass

Apply a two-pole Butterworth band-pass filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `csg` | boolean | — | `false` | 🎚️ | use constant skirt gain |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "bandpass" output.wav
```


---

### bandreject

Apply a two-pole Butterworth band-reject filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "bandreject" output.wav
```


---

### bass

Boost or cut lower frequencies.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `100` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `100` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "bass" output.wav
```


---

### biquad

Apply a biquad IIR filter with the given coefficients.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `a0` | float | — | `1` | 🎚️ |  |
| `a1` | float | — | `0` | 🎚️ |  |
| `a2` | float | — | `0` | 🎚️ |  |
| `b0` | float | — | `0` | 🎚️ |  |
| `b1` | float | — | `0` | 🎚️ |  |
| `b2` | float | — | `0` | 🎚️ |  |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "biquad" output.wav
```


---

### channelmap

Remap audio channels.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `map` | string | — | — | — | A comma-separated list of input channel numbers in output order. |
| `channel_layout` | channel_layout | — | — | — | Output channel layout. |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "channelmap" output.wav
```


---

### channelsplit

Split audio into per-channel streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `channel_layout` | channel_layout | — | `"stereo"` | — | Input channel layout. |
| `channels` | string | — | `"all"` | — | Channels to extract. |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "channelsplit" output.wav
```


---

### chorus

Add a chorus effect to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `in_gain` | float | 0 → 1 | `0.4` | — | set input gain |
| `out_gain` | float | 0 → 1 | `0.4` | — | set output gain |
| `delays` | string | — | — | — | set delays |
| `decays` | string | — | — | — | set decays |
| `speeds` | string | — | — | — | set speeds |
| `depths` | string | — | — | — | set depths |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "chorus" output.wav
```


---

### compand

Compress or expand audio dynamic range.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `attacks` | string | — | `"0"` | — | set time over which increase of volume is determined |
| `decays` | string | — | `"0.8"` | — | set time over which decrease of volume is determined |
| `points` | string | — | `"-70/-70|-60/-20|1/0"` | — | set points of transfer function |
| `gain` | float | -900 → 900 | `0` | — | set output gain |
| `volume` | float | -900 → 0 | `0` | — | set initial volume |
| `delay` | float | 0 → 20 | `0` | — | set delay for samples before sending them to volume adjuster |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "compand" output.wav
```


---

### compensationdelay

Audio Compensation Delay Line.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `mm` | integer | 0 → 10 | `0` | 🎚️ | set mm distance |
| `cm` | integer | 0 → 100 | `0` | 🎚️ | set cm distance |
| `m` | integer | 0 → 100 | `0` | 🎚️ | set meter distance |
| `dry` | float | 0 → 1 | `0` | 🎚️ | set dry amount |
| `wet` | float | 0 → 1 | `1` | 🎚️ | set wet amount |
| `temp` | integer | -50 → 50 | `20` | 🎚️ | set temperature °C |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "compensationdelay" output.wav
```


---

### crossfeed

Apply headphone crossfeed filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `strength` | float | 0 → 1 | `0.2` | 🎚️ | set crossfeed strength |
| `range` | float | 0 → 1 | `0.5` | 🎚️ | set soundstage wideness |
| `slope` | float | 0.01 → 1 | `0.5` | 🎚️ | set curve slope |
| `level_in` | float | 0 → 1 | `0.9` | 🎚️ | set level in |
| `level_out` | float | 0 → 1 | `1` | 🎚️ | set level out |
| `block_size` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "crossfeed" output.wav
```


---

### crystalizer

Simple audio noise sharpening filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `i` | float | -10 → 10 | `2` | 🎚️ | set intensity |
| `c` | boolean | — | `true` | 🎚️ | enable clipping |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "crystalizer" output.wav
```


---

### dcshift

Apply a DC shift to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `shift` | float | -1 → 1 | `0` | — | set DC shift |
| `limitergain` | float | 0 → 1 | `0` | — | set limiter gain |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "dcshift" output.wav
```


---

### deesser

Apply de-essing to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `i` | float | 0 → 1 | `0` | — | set intensity |
| `m` | float | 0 → 1 | `0.5` | — | set max deessing |
| `f` | float | 0 → 1 | `0.5` | — | set frequency |
| `s` | integer | 0 → 2 | `o` | — | set output mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "deesser" output.wav
```


---

### dialoguenhance

Audio Dialogue Enhancement.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `original` | float | 0 → 1 | `1` | 🎚️ | set original center factor |
| `enhance` | float | 0 → 3 | `1` | 🎚️ | set dialogue enhance factor |
| `voice` | float | 2 → 32 | `2` | 🎚️ | set voice detection factor |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "dialoguenhance" output.wav
```


---

### drmeter

Measure audio dynamic range.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `length` | float | 0.01 → 10 | `3` | — | set the window length |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "drmeter" output.wav
```


---

### dynaudnorm

Dynamic Audio Normalizer.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `framelen` | integer | 10 → 8000 | `500` | 🎚️ | set the frame length in msec |
| `f` | integer | 10 → 8000 | `500` | 🎚️ | set the frame length in msec |
| `gausssize` | integer | 3 → 301 | `31` | 🎚️ | set the filter size |
| `g` | integer | 3 → 301 | `31` | 🎚️ | set the filter size |
| `peak` | float | 0 → 1 | `0.95` | 🎚️ | set the peak value |
| `p` | float | 0 → 1 | `0.95` | 🎚️ | set the peak value |
| `maxgain` | float | 1 → 100 | `10` | 🎚️ | set the max amplification |
| `m` | float | 1 → 100 | `10` | 🎚️ | set the max amplification |
| `targetrms` | float | 0 → 1 | `0` | 🎚️ | set the target RMS |
| `r` | float | 0 → 1 | `0` | 🎚️ | set the target RMS |
| `coupling` | boolean | — | `true` | 🎚️ | set channel coupling |
| `n` | boolean | — | `true` | 🎚️ | set channel coupling |
| `correctdc` | boolean | — | `false` | 🎚️ | set DC correction |
| `c` | boolean | — | `false` | 🎚️ | set DC correction |
| `altboundary` | boolean | — | `false` | 🎚️ | set alternative boundary mode |
| `b` | boolean | — | `false` | 🎚️ | set alternative boundary mode |
| `compress` | float | 0 → 30 | `0` | 🎚️ | set the compress factor |
| `s` | float | 0 → 30 | `0` | 🎚️ | set the compress factor |
| `threshold` | float | 0 → 1 | `0` | 🎚️ | set the threshold value |
| `t` | float | 0 → 1 | `0` | 🎚️ | set the threshold value |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `h` | string | — | `"all"` | 🎚️ | set channels to filter |
| `overlap` | float | 0 → 1 | `0` | 🎚️ | set the frame overlap |
| `o` | float | 0 → 1 | `0` | 🎚️ | set the frame overlap |
| `curve` | string | — | — | 🎚️ | set the custom peak mapping curve |
| `v` | string | — | — | 🎚️ | set the custom peak mapping curve |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "dynaudnorm" output.wav
```


---

### earwax

Widen the stereo image.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "earwax" output.wav
```


---

### ebur128

EBU R128 scanner.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | dynamic |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `video` | boolean | — | `false` | — | set video output |
| `size` | image_size | — | `"640x480"` | — | set video size |
| `meter` | integer | 9 → 18 | `9` | — | set scale meter (+9 to +18) |
| `framelog` | integer | — | `-1` | — | force frame logging level |
| `metadata` | boolean | — | `false` | — | inject metadata in the filtergraph |
| `peak` | flags | — | `0` | — | set peak mode |
| `dualmono` | boolean | — | `false` | — | treat mono input files as dual-mono |
| `panlaw` | float | -10 → 0 | `-3.0103` | — | set a specific pan law for dual-mono files |
| `target` | integer | -23 → 0 | `-23` | — | set a specific target level in LUFS (-23 to 0) |
| `gauge` | integer | 0 → 1 | `momentary` | — | set gauge display type |
| `scale` | integer | 0 → 1 | `absolute` | — | sets display method for the stats |
| `integrated` | float | — | `0` | — | integrated loudness (LUFS) |
| `range` | float | — | `0` | — | loudness range (LU) |
| `lra_low` | float | — | `0` | — | LRA low (LUFS) |
| `lra_high` | float | — | `0` | — | LRA high (LUFS) |
| `sample_peak` | float | — | `0` | — | sample peak (dBFS) |
| `true_peak` | float | — | `0` | — | true peak (dBFS) |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "ebur128" output.wav
```


---

### equalizer

Apply two-pole peaking equalization (EQ) filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `0` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `0` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `1` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `1` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "equalizer" output.wav
```


---

### extrastereo

Increase difference between stereo audio channels.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `m` | float | -10 → 10 | `2.5` | 🎚️ | set the difference coefficient |
| `c` | boolean | — | `true` | 🎚️ | enable clipping |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "extrastereo" output.wav
```


---

### firequalizer

Finite Impulse Response Equalizer.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `gain` | string | — | `"gain_interpolate(f` | 🎚️ | set gain curve |
| `gain_entry` | string | — | — | 🎚️ | set gain entry |
| `delay` | float | 0 → 10000000000.0 | `0.01` | — | set delay |
| `accuracy` | float | 0 → 10000000000.0 | `5` | — | set accuracy |
| `wfunc` | integer | 0 → 9 | `hann` | — | set window function |
| `fixed` | boolean | — | `false` | — | set fixed frame samples |
| `multi` | boolean | — | `false` | — | set multi channels mode |
| `zero_phase` | boolean | — | `false` | — | set zero phase mode |
| `scale` | integer | 0 → 3 | `linlog` | — | set gain scale |
| `dumpfile` | string | — | — | — | set dump file |
| `dumpscale` | integer | 0 → 3 | `linlog` | — | set dump scale |
| `fft2` | boolean | — | `false` | — | set 2-channels fft |
| `min_phase` | boolean | — | `false` | — | set minimum phase mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "firequalizer" output.wav
```


---

### flanger

Apply a flanging effect to the audio.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `delay` | float | 0 → 30 | `0` | — | base delay in milliseconds |
| `depth` | float | 0 → 10 | `2` | — | added swept delay in milliseconds |
| `regen` | float | -95 → 95 | `0` | — | percentage regeneration (delayed signal feedback) |
| `width` | float | 0 → 100 | `71` | — | percentage of delayed signal mixed with original |
| `speed` | float | 0.1 → 10 | `0.5` | — | sweeps per second (Hz) |
| `shape` | integer | 0 → 1 | `sinusoidal` | — | swept wave shape |
| `phase` | float | 0 → 100 | `25` | — | swept wave percentage phase-shift for multi-channel |
| `interp` | integer | 0 → 1 | `linear` | — | delay-line interpolation |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "flanger" output.wav
```


---

### haas

Apply Haas Stereo Enhancer.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | — | set level in |
| `level_out` | float | 0.015625 → 64 | `1` | — | set level out |
| `side_gain` | float | 0.015625 → 64 | `1` | — | set side gain |
| `middle_source` | integer | 0 → 3 | `mid` | — | set middle source |
| `middle_phase` | boolean | — | `false` | — | set middle phase |
| `left_delay` | float | 0 → 40 | `2.05` | — | set left delay |
| `left_balance` | float | -1 → 1 | `-1` | — | set left balance |
| `left_gain` | float | 0.015625 → 64 | `1` | — | set left gain |
| `left_phase` | boolean | — | `false` | — | set left phase |
| `right_delay` | float | 0 → 40 | `2.12` | — | set right delay |
| `right_balance` | float | -1 → 1 | `1` | — | set right balance |
| `right_gain` | float | 0.015625 → 64 | `1` | — | set right gain |
| `right_phase` | boolean | — | `true` | — | set right phase |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "haas" output.wav
```


---

### hdcd

Apply High Definition Compatible Digital (HDCD) decoding.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `disable_autoconvert` | boolean | — | `true` | — | Disable any format conversion or resampling in the filter graph. |
| `process_stereo` | boolean | — | `true` | — | Process stereo channels together. Only apply target_gain when both channels match. |
| `cdt_ms` | integer | 100 → 60000 | `2000` | — | Code detect timer period in ms. |
| `force_pe` | boolean | — | `false` | — | Always extend peaks above -3dBFS even when PE is not signaled. |
| `analyze_mode` | integer | 0 → 4 | `off` | — | Replace audio with solid tone and signal some processing aspect in the amplitude. |
| `bits_per_sample` | integer | 16 → 24 | `16` | — | Valid bits per sample (location of the true LSB). |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "hdcd" output.wav
```


---

### headphone

Apply headphone binaural spatialization with HRTFs in additional streams.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `map` | string | — | — | — | set channels convolution mappings |
| `gain` | float | -20 → 40 | `0` | — | set gain in dB |
| `lfe` | float | -20 → 40 | `0` | — | set lfe gain in dB |
| `type` | integer | 0 → 1 | `freq` | — | set processing |
| `size` | integer | 1024 → 96000 | `1024` | — | set frame size |
| `hrir` | integer | 0 → 1 | `stereo` | — | set hrir format |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "headphone" output.wav
```


---

### highpass

Apply a high-pass filter with 3dB point frequency.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "highpass" output.wav
```


---

### highshelf

Apply a high shelf filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "highshelf" output.wav
```


---

### hilbert

Generate a Hilbert transform FIR coefficients.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `taps` | integer | 11 → 65535 | `22051` | — | set number of taps |
| `t` | integer | 11 → 65535 | `22051` | — | set number of taps |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `win_func` | integer | 0 → 20 | `blackman` | — | set window function |
| `w` | integer | 0 → 20 | `blackman` | — | set window function |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "hilbert" output.wav
```


---

### join

Join multiple audio streams into multi-channel output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | dynamic |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `inputs` | integer | ≥ 1 | `2` | — | Number of input streams. |
| `channel_layout` | channel_layout | — | `"stereo"` | — | Channel layout of the output stream. |
| `map` | string | — | — | — | A comma-separated list of channels maps in the format 'input_stream.input_channel-output_channel. |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "join" output.wav
```


---

### loudnorm

EBU R128 loudness normalization

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `I` | float | -70 → -5 | `-24` | — | set integrated loudness target |
| `i` | float | -70 → -5 | `-24` | — | set integrated loudness target |
| `LRA` | float | 1 → 50 | `7` | — | set loudness range target |
| `lra` | float | 1 → 50 | `7` | — | set loudness range target |
| `TP` | float | -9 → 0 | `-2` | — | set maximum true peak |
| `tp` | float | -9 → 0 | `-2` | — | set maximum true peak |
| `measured_I` | float | -99 → 0 | `0` | — | measured IL of input file |
| `measured_i` | float | -99 → 0 | `0` | — | measured IL of input file |
| `measured_LRA` | float | 0 → 99 | `0` | — | measured LRA of input file |
| `measured_lra` | float | 0 → 99 | `0` | — | measured LRA of input file |
| `measured_TP` | float | -99 → 99 | `99` | — | measured true peak of input file |
| `measured_tp` | float | -99 → 99 | `99` | — | measured true peak of input file |
| `measured_thresh` | float | -99 → 0 | `-70` | — | measured threshold of input file |
| `offset` | float | -99 → 99 | `0` | — | set offset gain |
| `linear` | boolean | — | `true` | — | normalize linearly if possible |
| `dual_mono` | boolean | — | `false` | — | treat mono input as dual-mono |
| `print_format` | integer | 0 → 2 | `none` | — | set print format for stats |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "loudnorm" output.wav
```


---

### lowpass

Apply a low-pass filter with 3dB point frequency.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `500` | 🎚️ | set frequency |
| `f` | float | 0 → 999999 | `500` | 🎚️ | set frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.707` | 🎚️ | set width |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "lowpass" output.wav
```


---

### lowshelf

Apply a low shelf filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `100` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `100` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "lowshelf" output.wav
```


---

### mcompand

Multiband Compress or expand audio dynamic range.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `args` | string | — | `"0.005,0.1 6 -47/-40,-34/-34,-17/-33 100 | 0.003,0.05 6 -47/-40,-34/-34,-17/-33 400 | 0.000625,0.0125 6 -47/-40,-34/-34,-15/-33 1600 | 0.0001,0.025 6 -47/-40,-34/-34,-31/-31,-0/-30 6400 | 0,0.025 6 -38/-31,-28/-28,-0/-25 22000"` | — | set parameters for each band |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "mcompand" output.wav
```


---

### pan

Remix channels with coefficients (panning).

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "pan" output.wav
```


---

### replaygain

ReplayGain scanner.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `track_gain` | float | — | `0` | — | track gain (dB) |
| `track_peak` | float | — | `0` | — | track peak |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "replaygain" output.wav
```


---

### rubberband

Apply time-stretching and pitch-shifting.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `tempo` | float | 0.01 → 100 | `1` | 🎚️ | set tempo scale factor |
| `pitch` | float | 0.01 → 100 | `1` | 🎚️ | set pitch scale factor |
| `transients` | integer | ≥ 0 | `crisp` | — | set transients |
| `detector` | integer | ≥ 0 | `compound` | — | set detector |
| `phase` | integer | ≥ 0 | `laminar` | — | set phase |
| `window` | integer | ≥ 0 | `standard` | — | set window |
| `smoothing` | integer | ≥ 0 | `off` | — | set smoothing |
| `formant` | integer | ≥ 0 | `shifted` | — | set formant |
| `pitchq` | integer | ≥ 0 | `speed` | — | set pitch quality |
| `channels` | integer | ≥ 0 | `apart` | — | set channels |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "rubberband" output.wav
```


---

### showcqt

Convert input audio to a CQT (Constant/Clamped Q Transform) spectrum video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"1920x1080"` | — | set video size |
| `s` | image_size | — | `"1920x1080"` | — | set video size |
| `fps` | video_rate | — | `"25"` | — | set video rate |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `bar_h` | integer | ≥ -1 | `-1` | — | set bargraph height |
| `axis_h` | integer | ≥ -1 | `-1` | — | set axis height |
| `sono_h` | integer | ≥ -1 | `-1` | — | set sonogram height |
| `fullhd` | boolean | — | `true` | — | set fullhd size |
| `sono_v` | string | — | `"16"` | — | set sonogram volume |
| `volume` | string | — | `"16"` | — | set sonogram volume |
| `bar_v` | string | — | `"sono_v"` | — | set bargraph volume |
| `volume2` | string | — | `"sono_v"` | — | set bargraph volume |
| `sono_g` | float | 1 → 7 | `3` | — | set sonogram gamma |
| `gamma` | float | 1 → 7 | `3` | — | set sonogram gamma |
| `bar_g` | float | 1 → 7 | `1` | — | set bargraph gamma |
| `gamma2` | float | 1 → 7 | `1` | — | set bargraph gamma |
| `bar_t` | float | 0 → 1 | `1` | — | set bar transparency |
| `timeclamp` | float | 0.002 → 1 | `0.17` | — | set timeclamp |
| `tc` | float | 0.002 → 1 | `0.17` | — | set timeclamp |
| `attack` | float | 0 → 1 | `0` | — | set attack time |
| `basefreq` | float | 10 → 100000 | `20.0152` | — | set base frequency |
| `endfreq` | float | 10 → 100000 | `20495.6` | — | set end frequency |
| `coeffclamp` | float | 0.1 → 10 | `1` | — | set coeffclamp |
| `tlength` | string | — | `"384*tc/(384+tc*f` | — | set tlength |
| `count` | integer | 1 → 30 | `6` | — | set transform count |
| `fcount` | integer | 0 → 10 | `0` | — | set frequency count |
| `fontfile` | string | — | — | — | set axis font file |
| `font` | string | — | — | — | set axis font |
| `fontcolor` | string | — | `"st(0, (midi(f` | — | set font color |
| `axisfile` | string | — | — | — | set axis image |
| `axis` | boolean | — | `true` | — | draw axis |
| `text` | boolean | — | `true` | — | draw axis |
| `csp` | integer | ≥ 0 | `unspecified` | — | set color space |
| `cscheme` | string | — | `"1|0.5|0|0|0.5|1"` | — | set color scheme |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showcqt" output.wav
```


---

### showcwt

Convert input audio to a CWT (Continuous Wavelet Transform) spectrum video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"640x512"` | — | set video size |
| `s` | image_size | — | `"640x512"` | — | set video size |
| `rate` | string | — | `"25"` | — | set video rate |
| `r` | string | — | `"25"` | — | set video rate |
| `scale` | integer | 0 → 8 | `linear` | — | set frequency scale |
| `iscale` | integer | 0 → 4 | `log` | — | set intensity scale |
| `min` | float | 1 → 192000 | `20` | — | set minimum frequency |
| `max` | float | 1 → 192000 | `20000` | — | set maximum frequency |
| `imin` | float | 0 → 1 | `0` | — | set minimum intensity |
| `imax` | float | 0 → 1 | `1` | — | set maximum intensity |
| `logb` | float | 0 → 1 | `0.0001` | — | set logarithmic basis |
| `deviation` | float | 0 → 100 | `1` | — | set frequency deviation |
| `pps` | integer | 1 → 1024 | `64` | — | set pixels per second |
| `mode` | integer | 0 → 4 | `magnitude` | — | set output mode |
| `slide` | integer | 0 → 2 | `replace` | — | set slide mode |
| `direction` | integer | 0 → 3 | `lr` | — | set direction mode |
| `bar` | float | 0 → 1 | `0` | — | set bargraph ratio |
| `rotation` | float | -1 → 1 | `0` | — | set color rotation |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showcwt" output.wav
```


---

### showfreqs

Convert input audio to a frequencies video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"1024x512"` | — | set video size |
| `s` | image_size | — | `"1024x512"` | — | set video size |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `mode` | integer | 0 → 2 | `bar` | — | set display mode |
| `ascale` | integer | 0 → 3 | `log` | — | set amplitude scale |
| `fscale` | integer | 0 → 2 | `lin` | — | set frequency scale |
| `win_size` | integer | 16 → 65536 | `2048` | — | set window size |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `overlap` | float | 0 → 1 | `1` | — | set window overlap |
| `averaging` | integer | ≥ 0 | `1` | — | set time averaging |
| `colors` | string | — | `"red|green|blue|yellow|orange|lime|pink|magenta|brown"` | — | set channels colors |
| `cmode` | integer | 0 → 1 | `combined` | — | set channel mode |
| `minamp` | float | ≤ 1e-06 | `1e-06` | — | set minimum amplitude |
| `data` | integer | 0 → 2 | `magnitude` | — | set data mode |
| `channels` | string | — | `"all"` | — | set channels to draw |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showfreqs" output.wav
```


---

### showspatial

Convert input audio to a spatial video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"512x512"` | — | set video size |
| `s` | image_size | — | `"512x512"` | — | set video size |
| `win_size` | integer | 1024 → 65536 | `4096` | — | set window size |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showspatial" output.wav
```


---

### showspectrum

Convert input audio to a spectrum video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"640x512"` | — | set video size |
| `s` | image_size | — | `"640x512"` | — | set video size |
| `slide` | integer | 0 → 4 | `replace` | — | set sliding mode |
| `mode` | integer | 0 → 1 | `combined` | — | set channel display mode |
| `color` | integer | 0 → 14 | `channel` | — | set channel coloring |
| `scale` | integer | 0 → 5 | `sqrt` | — | set display scale |
| `fscale` | integer | 0 → 1 | `lin` | — | set frequency scale |
| `saturation` | float | -10 → 10 | `1` | — | color saturation multiplier |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `orientation` | integer | 0 → 1 | `vertical` | — | set orientation |
| `overlap` | float | 0 → 1 | `0` | — | set window overlap |
| `gain` | float | 0 → 128 | `1` | — | set scale gain |
| `data` | integer | 0 → 2 | `magnitude` | — | set data mode |
| `rotation` | float | -1 → 1 | `0` | — | color rotation |
| `start` | integer | ≥ 0 | `0` | — | start frequency |
| `stop` | integer | ≥ 0 | `0` | — | stop frequency |
| `fps` | string | — | `"auto"` | — | set video rate |
| `legend` | boolean | — | `false` | — | draw legend |
| `drange` | float | 10 → 200 | `120` | — | set dynamic range in dBFS |
| `limit` | float | -100 → 100 | `0` | — | set upper limit in dBFS |
| `opacity` | float | 0 → 10 | `1` | — | set opacity strength |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showspectrum" output.wav
```


---

### showspectrumpic

Convert input audio to a spectrum video output single picture.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"4096x2048"` | — | set video size |
| `s` | image_size | — | `"4096x2048"` | — | set video size |
| `mode` | integer | 0 → 1 | `combined` | — | set channel display mode |
| `color` | integer | 0 → 14 | `intensity` | — | set channel coloring |
| `scale` | integer | 0 → 5 | `log` | — | set display scale |
| `fscale` | integer | 0 → 1 | `lin` | — | set frequency scale |
| `saturation` | float | -10 → 10 | `1` | — | color saturation multiplier |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `orientation` | integer | 0 → 1 | `vertical` | — | set orientation |
| `gain` | float | 0 → 128 | `1` | — | set scale gain |
| `legend` | boolean | — | `true` | — | draw legend |
| `rotation` | float | -1 → 1 | `0` | — | color rotation |
| `start` | integer | ≥ 0 | `0` | — | start frequency |
| `stop` | integer | ≥ 0 | `0` | — | stop frequency |
| `drange` | float | 10 → 200 | `120` | — | set dynamic range in dBFS |
| `limit` | float | -100 → 100 | `0` | — | set upper limit in dBFS |
| `opacity` | float | 0 → 10 | `1` | — | set opacity strength |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showspectrumpic" output.wav
```


---

### showvolume

Convert input audio volume to video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `b` | integer | 0 → 5 | `1` | — | set border width |
| `w` | integer | 80 → 8192 | `400` | — | set channel width |
| `h` | integer | 1 → 900 | `20` | — | set channel height |
| `f` | float | 0 → 1 | `0.95` | — | set fade |
| `c` | string | — | `"PEAK*255+floor((1-PEAK` | — | set volume color expression |
| `t` | boolean | — | `true` | — | display channel names |
| `v` | boolean | — | `true` | — | display volume value |
| `dm` | float | 0 → 9000 | `0` | — | duration for max value display |
| `dmc` | color | — | `"orange"` | — | set color of the max value line |
| `o` | integer | 0 → 1 | `h` | — | set orientation |
| `s` | integer | 0 → 5 | `0` | — | set step size |
| `p` | float | 0 → 1 | `0` | — | set background opacity |
| `m` | integer | 0 → 1 | `p` | — | set mode |
| `ds` | integer | 0 → 1 | `lin` | — | set display scale |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showvolume" output.wav
```


---

### showwaves

Convert input audio to a video output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"600x240"` | — | set video size |
| `s` | image_size | — | `"600x240"` | — | set video size |
| `mode` | integer | 0 → 3 | `point` | — | select display mode |
| `n` | rational | ≥ 0 | `0/1` | — | set how many samples to show in the same point |
| `rate` | video_rate | — | `"25"` | — | set video rate |
| `r` | video_rate | — | `"25"` | — | set video rate |
| `split_channels` | boolean | — | `false` | — | draw channels separately |
| `colors` | string | — | `"red|green|blue|yellow|orange|lime|pink|magenta|brown"` | — | set channels colors |
| `scale` | integer | 0 → 3 | `lin` | — | set amplitude scale |
| `draw` | integer | 0 → 1 | `scale` | — | set draw mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showwaves" output.wav
```


---

### showwavespic

Convert input audio to a video output single picture.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `size` | image_size | — | `"600x240"` | — | set video size |
| `s` | image_size | — | `"600x240"` | — | set video size |
| `split_channels` | boolean | — | `false` | — | draw channels separately |
| `colors` | string | — | `"red|green|blue|yellow|orange|lime|pink|magenta|brown"` | — | set channels colors |
| `scale` | integer | 0 → 3 | `lin` | — | set amplitude scale |
| `draw` | integer | 0 → 1 | `scale` | — | set draw mode |
| `filter` | integer | 0 → 1 | `average` | — | set filter mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "showwavespic" output.wav
```


---

### sidechaincompress

Sidechain compressor.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input gain |
| `mode` | integer | 0 → 1 | `downward` | 🎚️ | set mode |
| `threshold` | float | 0.000976563 → 1 | `0.125` | 🎚️ | set threshold |
| `ratio` | float | 1 → 20 | `2` | 🎚️ | set ratio |
| `attack` | float | 0.01 → 2000 | `20` | 🎚️ | set attack |
| `release` | float | 0.01 → 9000 | `250` | 🎚️ | set release |
| `makeup` | float | 1 → 64 | `1` | 🎚️ | set make up gain |
| `knee` | float | 1 → 8 | `2.82843` | 🎚️ | set knee |
| `link` | integer | 0 → 1 | `average` | 🎚️ | set link type |
| `detection` | integer | 0 → 1 | `rms` | 🎚️ | set detection |
| `level_sc` | float | 0.015625 → 64 | `1` | 🎚️ | set sidechain gain |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "sidechaincompress" output.wav
```


---

### sidechaingate

Audio sidechain gate.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 2 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set input level |
| `mode` | integer | 0 → 1 | `downward` | 🎚️ | set mode |
| `range` | float | 0 → 1 | `0.06125` | 🎚️ | set max gain reduction |
| `threshold` | float | 0 → 1 | `0.125` | 🎚️ | set threshold |
| `ratio` | float | 1 → 9000 | `2` | 🎚️ | set ratio |
| `attack` | float | 0.01 → 9000 | `20` | 🎚️ | set attack |
| `release` | float | 0.01 → 9000 | `250` | 🎚️ | set release |
| `makeup` | float | 1 → 64 | `1` | 🎚️ | set makeup gain |
| `knee` | float | 1 → 8 | `2.82843` | 🎚️ | set knee |
| `detection` | integer | 0 → 1 | `rms` | 🎚️ | set detection |
| `link` | integer | 0 → 1 | `average` | 🎚️ | set link |
| `level_sc` | float | 0.015625 → 64 | `1` | 🎚️ | set sidechain gain |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "sidechaingate" output.wav
```


---

### silencedetect

Detect silence.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `n` | float | ≥ 0 | `0.001` | — | set noise tolerance |
| `noise` | float | ≥ 0 | `0.001` | — | set noise tolerance |
| `d` | duration | — | `2` | — | set minimum duration in seconds |
| `duration` | duration | — | `2` | — | set minimum duration in seconds |
| `mono` | boolean | — | `false` | — | check each channel separately |
| `m` | boolean | — | `false` | — | check each channel separately |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "silencedetect" output.wav
```


---

### silenceremove

Remove silence.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `start_periods` | integer | 0 → 9000 | `0` | — | set periods of silence parts to skip from start |
| `start_duration` | duration | — | `0` | — | set start duration of non-silence part |
| `start_threshold` | float | ≥ 0 | `0` | 🎚️ | set threshold for start silence detection |
| `start_silence` | duration | — | `0` | — | set start duration of silence part to keep |
| `start_mode` | integer | 0 → 1 | `any` | 🎚️ | set which channel will trigger trimming from start |
| `stop_periods` | integer | -9000 → 9000 | `0` | — | set periods of silence parts to skip from end |
| `stop_duration` | duration | — | `0` | — | set stop duration of silence part |
| `stop_threshold` | float | ≥ 0 | `0` | 🎚️ | set threshold for stop silence detection |
| `stop_silence` | duration | — | `0` | — | set stop duration of silence part to keep |
| `stop_mode` | integer | 0 → 1 | `all` | 🎚️ | set which channel will trigger trimming from end |
| `detection` | integer | 0 → 5 | `rms` | — | set how silence is detected |
| `window` | duration | — | `0.02` | — | set duration of window for silence detection |
| `timestamp` | integer | 0 → 1 | `write` | — | set how every output frame timestamp is processed |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "silenceremove" output.wav
```


---

### sinc

Generate a sinc kaiser-windowed low-pass, high-pass, band-pass, or band-reject FIR coefficients.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 1 | `44100` | — | set sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set sample rate |
| `nb_samples` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `n` | integer | ≥ 1 | `1024` | — | set the number of samples per requested frame |
| `hp` | float | ≥ 0 | `0` | — | set high-pass filter frequency |
| `lp` | float | ≥ 0 | `0` | — | set low-pass filter frequency |
| `phase` | float | 0 → 100 | `50` | — | set filter phase response |
| `beta` | float | -1 → 256 | `-1` | — | set kaiser window beta |
| `att` | float | 40 → 180 | `120` | — | set stop-band attenuation |
| `round` | boolean | — | `false` | — | enable rounding |
| `hptaps` | integer | 0 → 32768 | `0` | — | set number of taps for high-pass filter |
| `lptaps` | integer | 0 → 32768 | `0` | — | set number of taps for low-pass filter |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "sinc" output.wav
```


---

### sine

Generate sine wave audio signal.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | ≥ 0 | `440` | — | set the sine frequency |
| `f` | float | ≥ 0 | `440` | — | set the sine frequency |
| `beep_factor` | float | ≥ 0 | `0` | — | set the beep frequency factor |
| `b` | float | ≥ 0 | `0` | — | set the beep frequency factor |
| `sample_rate` | integer | ≥ 1 | `44100` | — | set the sample rate |
| `r` | integer | ≥ 1 | `44100` | — | set the sample rate |
| `duration` | duration | — | `0` | — | set the audio duration |
| `d` | duration | — | `0` | — | set the audio duration |
| `samples_per_frame` | string | — | `"1024"` | — | set the number of samples per frame |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "sine" output.wav
```


---

### sofalizer

SOFAlizer (Spatially Oriented Format for Acoustics).

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sofa` | string | — | — | — | sofa filename |
| `gain` | float | -20 → 40 | `0` | — | set gain in dB |
| `rotation` | float | -360 → 360 | `0` | — | set rotation |
| `elevation` | float | -90 → 90 | `0` | — | set elevation |
| `radius` | float | 0 → 5 | `1` | — | set radius |
| `type` | integer | 0 → 1 | `freq` | — | set processing |
| `speakers` | string | — | — | — | set speaker custom positions |
| `lfegain` | float | -20 → 40 | `0` | — | set lfe gain |
| `framesize` | integer | 1024 → 96000 | `1024` | — | set frame size |
| `normalize` | boolean | — | `true` | — | normalize IRs |
| `interpolate` | boolean | — | `false` | — | interpolate IRs from neighbors |
| `minphase` | boolean | — | `false` | — | minphase IRs |
| `anglestep` | float | 0.01 → 10 | `0.5` | — | set neighbor search angle step |
| `radstep` | float | 0.01 → 1 | `0.01` | — | set neighbor search radius step |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "sofalizer" output.wav
```


---

### spectrumsynth

Convert input spectrum videos to audio output.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `sample_rate` | integer | ≥ 15 | `44100` | — | set sample rate |
| `channels` | integer | 1 → 8 | `1` | — | set channels |
| `scale` | integer | 0 → 1 | `log` | — | set input amplitude scale |
| `slide` | integer | 0 → 3 | `fullframe` | — | set input sliding mode |
| `win_func` | integer | 0 → 20 | `rect` | — | set window function |
| `overlap` | float | 0 → 1 | `1` | — | set window overlap |
| `orientation` | integer | 0 → 1 | `vertical` | — | set orientation |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "spectrumsynth" output.wav
```


---

### speechnorm

Speech Normalizer.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `peak` | float | 0 → 1 | `0.95` | 🎚️ | set the peak value |
| `p` | float | 0 → 1 | `0.95` | 🎚️ | set the peak value |
| `expansion` | float | 1 → 50 | `2` | 🎚️ | set the max expansion factor |
| `e` | float | 1 → 50 | `2` | 🎚️ | set the max expansion factor |
| `compression` | float | 1 → 50 | `2` | 🎚️ | set the max compression factor |
| `c` | float | 1 → 50 | `2` | 🎚️ | set the max compression factor |
| `threshold` | float | 0 → 1 | `0` | 🎚️ | set the threshold value |
| `t` | float | 0 → 1 | `0` | 🎚️ | set the threshold value |
| `raise` | float | 0 → 1 | `0.001` | 🎚️ | set the expansion raising amount |
| `r` | float | 0 → 1 | `0.001` | 🎚️ | set the expansion raising amount |
| `fall` | float | 0 → 1 | `0.001` | 🎚️ | set the compression raising amount |
| `f` | float | 0 → 1 | `0.001` | 🎚️ | set the compression raising amount |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `h` | string | — | `"all"` | 🎚️ | set channels to filter |
| `invert` | boolean | — | `false` | 🎚️ | set inverted filtering |
| `i` | boolean | — | `false` | 🎚️ | set inverted filtering |
| `link` | boolean | — | `false` | 🎚️ | set linked channels filtering |
| `l` | boolean | — | `false` | 🎚️ | set linked channels filtering |
| `rms` | float | 0 → 1 | `0` | 🎚️ | set the RMS value |
| `m` | float | 0 → 1 | `0` | 🎚️ | set the RMS value |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "speechnorm" output.wav
```


---

### stereotools

Apply various stereo tools.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `level_in` | float | 0.015625 → 64 | `1` | 🎚️ | set level in |
| `level_out` | float | 0.015625 → 64 | `1` | 🎚️ | set level out |
| `balance_in` | float | -1 → 1 | `0` | 🎚️ | set balance in |
| `balance_out` | float | -1 → 1 | `0` | 🎚️ | set balance out |
| `softclip` | boolean | — | `false` | 🎚️ | enable softclip |
| `mutel` | boolean | — | `false` | 🎚️ | mute L |
| `muter` | boolean | — | `false` | 🎚️ | mute R |
| `phasel` | boolean | — | `false` | 🎚️ | phase L |
| `phaser` | boolean | — | `false` | 🎚️ | phase R |
| `mode` | integer | 0 → 10 | `lr>lr` | 🎚️ | set stereo mode |
| `slev` | float | 0.015625 → 64 | `1` | 🎚️ | set side level |
| `sbal` | float | -1 → 1 | `0` | 🎚️ | set side balance |
| `mlev` | float | 0.015625 → 64 | `1` | 🎚️ | set middle level |
| `mpan` | float | -1 → 1 | `0` | 🎚️ | set middle pan |
| `base` | float | -1 → 1 | `0` | 🎚️ | set stereo base |
| `delay` | float | -20 → 20 | `0` | 🎚️ | set delay |
| `sclevel` | float | 1 → 100 | `1` | 🎚️ | set S/C level |
| `phase` | float | 0 → 360 | `0` | 🎚️ | set stereo phase |
| `bmode_in` | integer | 0 → 2 | `balance` | 🎚️ | set balance in mode |
| `bmode_out` | integer | 0 → 2 | `balance` | 🎚️ | set balance out mode |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "stereotools" output.wav
```


---

### stereowiden

Apply stereo widening effect.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `delay` | float | 1 → 100 | `20` | — | set delay time |
| `feedback` | float | 0 → 0.9 | `0.3` | 🎚️ | set feedback gain |
| `crossfeed` | float | 0 → 0.8 | `0.3` | 🎚️ | set cross feed |
| `drymix` | float | 0 → 1 | `0.8` | 🎚️ | set dry-mix |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "stereowiden" output.wav
```


---

### superequalizer

Apply 18 band equalization filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "superequalizer" output.wav
```


---

### surround

Apply audio surround upmix filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `chl_out` | channel_layout | — | `"5.1"` | — | set output channel layout |
| `chl_in` | channel_layout | — | `"stereo"` | — | set input channel layout |
| `level_in` | float | 0 → 10 | `1` | 🎚️ | set input level |
| `level_out` | float | 0 → 10 | `1` | 🎚️ | set output level |
| `lfe` | boolean | — | `true` | 🎚️ | output LFE |
| `lfe_low` | integer | 0 → 256 | `128` | — | LFE low cut off |
| `lfe_high` | integer | 0 → 512 | `256` | — | LFE high cut off |
| `lfe_mode` | integer | 0 → 1 | `add` | 🎚️ | set LFE channel mode |
| `smooth` | float | 0 → 1 | `0` | 🎚️ | set temporal smoothness strength |
| `angle` | float | 0 → 360 | `90` | 🎚️ | set soundfield transform angle |
| `focus` | float | -1 → 1 | `0` | 🎚️ | set soundfield transform focus |
| `fc_in` | float | 0 → 10 | `1` | 🎚️ | set front center channel input level |
| `fc_out` | float | 0 → 10 | `1` | 🎚️ | set front center channel output level |
| `fl_in` | float | 0 → 10 | `1` | 🎚️ | set front left channel input level |
| `fl_out` | float | 0 → 10 | `1` | 🎚️ | set front left channel output level |
| `fr_in` | float | 0 → 10 | `1` | 🎚️ | set front right channel input level |
| `fr_out` | float | 0 → 10 | `1` | 🎚️ | set front right channel output level |
| `sl_in` | float | 0 → 10 | `1` | 🎚️ | set side left channel input level |
| `sl_out` | float | 0 → 10 | `1` | 🎚️ | set side left channel output level |
| `sr_in` | float | 0 → 10 | `1` | 🎚️ | set side right channel input level |
| `sr_out` | float | 0 → 10 | `1` | 🎚️ | set side right channel output level |
| `bl_in` | float | 0 → 10 | `1` | 🎚️ | set back left channel input level |
| `bl_out` | float | 0 → 10 | `1` | 🎚️ | set back left channel output level |
| `br_in` | float | 0 → 10 | `1` | 🎚️ | set back right channel input level |
| `br_out` | float | 0 → 10 | `1` | 🎚️ | set back right channel output level |
| `bc_in` | float | 0 → 10 | `1` | 🎚️ | set back center channel input level |
| `bc_out` | float | 0 → 10 | `1` | 🎚️ | set back center channel output level |
| `lfe_in` | float | 0 → 10 | `1` | 🎚️ | set lfe channel input level |
| `lfe_out` | float | 0 → 10 | `1` | 🎚️ | set lfe channel output level |
| `allx` | float | -1 → 15 | `-1` | 🎚️ | set all channel's x spread |
| `ally` | float | -1 → 15 | `-1` | 🎚️ | set all channel's y spread |
| `fcx` | float | 0.06 → 15 | `0.5` | 🎚️ | set front center channel x spread |
| `flx` | float | 0.06 → 15 | `0.5` | 🎚️ | set front left channel x spread |
| `frx` | float | 0.06 → 15 | `0.5` | 🎚️ | set front right channel x spread |
| `blx` | float | 0.06 → 15 | `0.5` | 🎚️ | set back left channel x spread |
| `brx` | float | 0.06 → 15 | `0.5` | 🎚️ | set back right channel x spread |
| `slx` | float | 0.06 → 15 | `0.5` | 🎚️ | set side left channel x spread |
| `srx` | float | 0.06 → 15 | `0.5` | 🎚️ | set side right channel x spread |
| `bcx` | float | 0.06 → 15 | `0.5` | 🎚️ | set back center channel x spread |
| `fcy` | float | 0.06 → 15 | `0.5` | 🎚️ | set front center channel y spread |
| `fly` | float | 0.06 → 15 | `0.5` | 🎚️ | set front left channel y spread |
| `fry` | float | 0.06 → 15 | `0.5` | 🎚️ | set front right channel y spread |
| `bly` | float | 0.06 → 15 | `0.5` | 🎚️ | set back left channel y spread |
| `bry` | float | 0.06 → 15 | `0.5` | 🎚️ | set back right channel y spread |
| `sly` | float | 0.06 → 15 | `0.5` | 🎚️ | set side left channel y spread |
| `sry` | float | 0.06 → 15 | `0.5` | 🎚️ | set side right channel y spread |
| `bcy` | float | 0.06 → 15 | `0.5` | 🎚️ | set back center channel y spread |
| `win_size` | integer | 1024 → 65536 | `4096` | — | set window size |
| `win_func` | integer | 0 → 20 | `hann` | — | set window function |
| `overlap` | float | 0 → 1 | `0.5` | 🎚️ | set window overlap |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "surround" output.wav
```


---

### tiltshelf

Apply a tilt shelf filter.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "tiltshelf" output.wav
```


---

### treble

Boost or cut upper frequencies.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✓ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `frequency` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `f` | float | 0 → 999999 | `3000` | 🎚️ | set central frequency |
| `width_type` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `t` | integer | 1 → 5 | `q` | 🎚️ | set filter-width type |
| `width` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `w` | float | 0 → 99999 | `0.5` | 🎚️ | set width |
| `gain` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `g` | float | -900 → 900 | `0` | 🎚️ | set gain |
| `poles` | integer | 1 → 2 | `2` | — | set number of poles |
| `p` | integer | 1 → 2 | `2` | — | set number of poles |
| `mix` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `m` | float | 0 → 1 | `1` | 🎚️ | set mix |
| `channels` | string | — | `"all"` | 🎚️ | set channels to filter |
| `c` | string | — | `"all"` | 🎚️ | set channels to filter |
| `normalize` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `n` | boolean | — | `false` | 🎚️ | normalize coefficients |
| `transform` | integer | 0 → 6 | `di` | — | set transform type |
| `a` | integer | 0 → 6 | `di` | — | set transform type |
| `precision` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `r` | integer | -1 → 3 | `auto` | — | set filtering precision |
| `blocksize` | integer | 0 → 32768 | `0` | — | set the block size |
| `b` | integer | 0 → 32768 | `0` | — | set the block size |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "treble" output.wav
```


---

### tremolo

Apply tremolo effect.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `f` | float | 0.1 → 20000 | `5` | — | set frequency in hertz |
| `d` | float | 0 → 1 | `0.5` | — | set depth as percentage |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "tremolo" output.wav
```


---

### vibrato

Apply vibrato effect.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `f` | float | 0.1 → 20000 | `5` | — | set frequency in hertz |
| `d` | float | 0 → 1 | `0.5` | — | set depth as percentage |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "vibrato" output.wav
```


---

### virtualbass

Audio Virtual Bass.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `cutoff` | float | 100 → 500 | `250` | — | set virtual bass cutoff |
| `strength` | float | 0.5 → 3 | `3` | 🎚️ | set virtual bass strength |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "virtualbass" output.wav
```


---

### volume

Change input volume.

| Property | Value |
|----------|-------|
| Timeline Support | ✓ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

#### Parameters

| Parameter | Type | Range | Default | Auto | Description |
|-----------|------|-------|---------|------|-------------|
| `volume` | string | — | `"1.0"` | 🎚️ | set volume adjustment expression |
| `precision` | integer | 0 → 2 | `float` | — | select mathematical precision |
| `eval` | integer | 0 → 1 | `once` | — | specify when to evaluate expressions |
| `replaygain` | integer | 0 → 3 | `drop` | — | Apply replaygain side data when present |
| `replaygain_preamp` | float | -15 → 15 | `0` | — | Apply replaygain pre-amplification |
| `replaygain_noclip` | boolean | — | `true` | — | Apply replaygain clipping prevention |

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "volume" output.wav
```


---

### volumedetect

Detect audio volume.

| Property | Value |
|----------|-------|
| Timeline Support | ✗ |
| Slice Threading | ✗ |
| Command Support | ✗ |
| Inputs | 1 |
| Outputs | 1 |

*This filter has no configurable parameters.*

**Basic Usage:**
```bash
ffmpeg -i input.wav -af "volumedetect" output.wav
```


---

## Automation & Modulation Notes

### Which Parameters Can Be Automated?

Parameters marked with 🎚️ support runtime changes via FFmpeg's command system. This is indicated by the 'T' flag in FFmpeg's help output.

### FFmpeg's sendcmd Filter

You can automate parameter changes over time using the `sendcmd` filter:

```bash
# Commands file format (cmds.txt):
# <time> <target> <command> <arg>
0.0 volume volume 0.5;
1.0 volume volume 1.0;
2.0 volume volume 0.25;

# Apply automation:
ffmpeg -i input.wav -af "sendcmd=f=cmds.txt,volume@v" output.wav
```

### Real-time Control via Named Pipes (Advanced)

```bash
# Create a named pipe
mkfifo /tmp/ffmpeg_cmds

# Run FFmpeg reading commands from pipe
ffmpeg -i input.wav -af "sendcmd=f=/tmp/ffmpeg_cmds,volume" output.wav &

# Send commands in real-time
echo "0 volume volume 0.5" > /tmp/ffmpeg_cmds
```

### Units and Value Formats

FFmpeg accepts values in their native units—no normalization needed:

| Parameter Type | Input Format | Example |
|----------------|--------------|---------|
| Volume/Gain | Linear or dB | `1.5` or `3dB` |
| Frequency | Hz | `1000` or `1k` |
| Time | Seconds or timestamp | `0.5` or `00:00:00.500` |
| Percentage | Decimal (0-1) | `0.75` for 75% |

### Building Your Own Automation

When building a GUI automation system:

1. **Store values in native units** (dB, Hz, etc.)—FFmpeg handles them directly
2. **Use the 'automatable' flag** from this database to determine which knobs to expose
3. **Generate sendcmd-compatible output** for time-based automation
4. **Consider the filter's command_support flag** for filters that support runtime changes

---

## Appendix: Automatable Parameters Quick Reference


| Filter | Automatable Parameters |
|--------|------------------------|
| a3dscope | `fov`, `roll`, `pitch`, `yaw`, `xzoom`, `yzoom`, `zzoom`, `xpos`, `ypos`, `zpos` |
| aap | `mu`, `delta`, `out_mode` |
| acompressor | `level_in`, `mode`, `threshold`, `ratio`, `attack`, `release`, `makeup`, `knee`, `link`, `detection`, `level_sc`, `mix` |
| acrusher | `level_in`, `level_out`, `bits`, `mix`, `mode`, `dc`, `aa`, `samples`, `lfo`, `lforange`, `lforate` |
| adelay | `delays` |
| adenorm | `level`, `type` |
| adrc | `transfer`, `attack`, `release`, `channels` |
| adynamicequalizer | `threshold`, `dfrequency`, `dqfactor`, `tfrequency`, `tqfactor`, `attack`, `release`, `ratio`, `makeup`, `range`, `mode`, `dftype`, `tftype`, `auto` |
| adynamicsmooth | `sensitivity`, `basefreq` |
| aemphasis | `level_in`, `level_out`, `mode`, `type` |
| aexciter | `level_in`, `level_out`, `amount`, `drive`, `blend`, `freq`, `ceil`, `listen` |
| afade | `type`, `t`, `start_sample`, `ss`, `nb_samples`, `ns`, `start_time`, `st`, `duration`, `d`, `curve`, `c`, `silence`, `unity` |
| afftdn | `noise_reduction`, `nr`, `noise_floor`, `nf`, `residual_floor`, `rf`, `track_noise`, `tn`, `track_residual`, `tr`, `output_mode`, `om`, `adaptivity`, `ad`, `floor_offset`, `fo`, `noise_link`, `nl`, `sample_noise`, `sn`, `gain_smooth`, `gs` |
| afir | `dry`, `wet`, `ir` |
| afreqshift | `shift`, `level`, `order` |
| afwtdn | `sigma`, `percent`, `profile`, `adaptive`, `softness` |
| agate | `level_in`, `mode`, `range`, `threshold`, `ratio`, `attack`, `release`, `makeup`, `knee`, `detection`, `link`, `level_sc` |
| agraphmonitor | `opacity`, `o`, `mode`, `m`, `flags`, `f` |
| alimiter | `level_in`, `level_out`, `limit`, `attack`, `release`, `asc`, `asc_level`, `level`, `latency` |
| allpass | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `mix`, `m`, `channels`, `c`, `normalize`, `n`, `order`, `o` |
| amix | `weights`, `normalize` |
| anlmdn | `strength`, `s`, `patch`, `p`, `research`, `r`, `output`, `o`, `smooth`, `m` |
| anlmf | `mu`, `eps`, `leakage`, `out_mode` |
| anlms | `mu`, `eps`, `leakage`, `out_mode` |
| aperms | `mode` |
| aphaseshift | `shift`, `level`, `order` |
| apsyclip | `level_in`, `level_out`, `clip`, `diff`, `adaptive`, `iterations`, `level` |
| arealtime | `limit`, `speed` |
| arls | `lambda`, `out_mode` |
| arnndn | `model`, `m`, `mix` |
| asetnsamples | `nb_out_samples`, `n`, `pad`, `p` |
| asetpts | `expr` |
| asoftclip | `type`, `threshold`, `output`, `param`, `oversample` |
| asubboost | `dry`, `wet`, `boost`, `decay`, `feedback`, `cutoff`, `slope`, `delay`, `channels` |
| asubcut | `cutoff`, `order`, `level` |
| asupercut | `cutoff`, `order`, `level` |
| asuperpass | `centerf`, `order`, `qfactor`, `level` |
| asuperstop | `centerf`, `order`, `qfactor`, `level` |
| atempo | `tempo` |
| atilt | `freq`, `slope`, `width`, `order`, `level` |
| avectorscope | `mode`, `m`, `rc`, `gc`, `bc`, `ac`, `rf`, `gf`, `bf`, `af`, `zoom`, `draw`, `scale`, `swap`, `mirror` |
| avsynctest | `amplitude`, `a`, `delay`, `dl`, `cycle`, `c` |
| bandpass | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `csg`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| bandreject | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| bass | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| biquad | `a0`, `a1`, `a2`, `b0`, `b1`, `b2`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| compensationdelay | `mm`, `cm`, `m`, `dry`, `wet`, `temp` |
| crossfeed | `strength`, `range`, `slope`, `level_in`, `level_out` |
| crystalizer | `i`, `c` |
| dialoguenhance | `original`, `enhance`, `voice` |
| dynaudnorm | `framelen`, `f`, `gausssize`, `g`, `peak`, `p`, `maxgain`, `m`, `targetrms`, `r`, `coupling`, `n`, `correctdc`, `c`, `altboundary`, `b`, `compress`, `s`, `threshold`, `t`, `channels`, `h`, `overlap`, `o`, `curve`, `v` |
| equalizer | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| extrastereo | `m`, `c` |
| firequalizer | `gain`, `gain_entry` |
| highpass | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| highshelf | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| lowpass | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| lowshelf | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| rubberband | `tempo`, `pitch` |
| sidechaincompress | `level_in`, `mode`, `threshold`, `ratio`, `attack`, `release`, `makeup`, `knee`, `link`, `detection`, `level_sc`, `mix` |
| sidechaingate | `level_in`, `mode`, `range`, `threshold`, `ratio`, `attack`, `release`, `makeup`, `knee`, `detection`, `link`, `level_sc` |
| silenceremove | `start_threshold`, `start_mode`, `stop_threshold`, `stop_mode` |
| speechnorm | `peak`, `p`, `expansion`, `e`, `compression`, `c`, `threshold`, `t`, `raise`, `r`, `fall`, `f`, `channels`, `h`, `invert`, `i`, `link`, `l`, `rms`, `m` |
| stereotools | `level_in`, `level_out`, `balance_in`, `balance_out`, `softclip`, `mutel`, `muter`, `phasel`, `phaser`, `mode`, `slev`, `sbal`, `mlev`, `mpan`, `base`, `delay`, `sclevel`, `phase`, `bmode_in`, `bmode_out` |
| stereowiden | `feedback`, `crossfeed`, `drymix` |
| surround | `level_in`, `level_out`, `lfe`, `lfe_mode`, `smooth`, `angle`, `focus`, `fc_in`, `fc_out`, `fl_in`, `fl_out`, `fr_in`, `fr_out`, `sl_in`, `sl_out`, `sr_in`, `sr_out`, `bl_in`, `bl_out`, `br_in`, `br_out`, `bc_in`, `bc_out`, `lfe_in`, `lfe_out`, `allx`, `ally`, `fcx`, `flx`, `frx`, `blx`, `brx`, `slx`, `srx`, `bcx`, `fcy`, `fly`, `fry`, `bly`, `bry`, `sly`, `sry`, `bcy`, `overlap` |
| tiltshelf | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| treble | `frequency`, `f`, `width_type`, `t`, `width`, `w`, `gain`, `g`, `mix`, `m`, `channels`, `c`, `normalize`, `n` |
| virtualbass | `strength` |
| volume | `volume` |

---

*Generated by ffmpeg-filters-to-markdown.py*
