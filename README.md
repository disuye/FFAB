# FFAB Overview

![FFAB](./readme/ffab-mode-dark.png)

FFAB is a cross-platform, complex audio batch processor, effects rack, command builder & graphical user interface for [FFmpeg](https://ffmpeg.org/download.html).

More info here: [www.disuye.com/ffab](https://www.disuye.com/ffab/)

<!--But only the sound related stuff.

FFAB is for musicians, sound engineers, media archivists, sonic artists, mental experimentalists, or anyone curious enough to collide thousands of files together into one hot mess.-->

# Download & Install

Check [Releases](https://github.com/disuye/FFAB/releases) for the latest DMG/ZIP files:
- macOS (Monterey+ Universal Binary) → FFAB.app
- Linux (x86 / Arm / Ubuntu24 +) → FFAB.AppImage

## Installation Issues

#### macOS
FFAB.app is not notarized by Apple and may be blocked by macOS Gatekeeper. 
- Monterey & Sonoma, right click FFAB.app, click Open, click Confirm
- Sequoia & Tahoe run this Terminal command:<br> ```xattr -cr /Applications/FFAB.app```

#### Linux

Audio preview player may fail on some Linux versions.
- This can be fixed with community help!
- Launch FFAB.AppImage with this Terminal command:<br>```GST_PLUGIN_SYSTEM_PATH=​/usr/lib/$(uname -m)​-linux-gnu/gstreamer-1.0 ./FFAB*.AppImage```

# Build From Source

- Developed on macOS Sequoia Silicon using Qt version 6.10.1
- Run ```/scripts/dummyAGL.sh``` and FFAB can also build with Qt 6.7.3
- Install ```brew install ninja```
- Run ```/scripts/ninja.sh``` for the debug version
- Run ```/scripts/ninja-release.sh``` to build macOS Silicon & Universal
- ```.github/workflows/build-ffab-linux.yml``` for Linux x86_64 & ARM64

### Codebase v1.0.2
```
FFAB/
├── CMakeLists.txt
├── images/
│   ├── AppIcon.icns
│   ├── AppIcon.png
│   ├── fader-h.png
│   ├── fader-v.png
│   ├── log-bg-dark.png
│   ├── log-bg-light.png
│   ├── track-h.png
│   └── track-v.png
├── fonts/
│   └── FiraCode-VariableFont_wght.ttf
├── licenses/
│   ├── FFAB-license.txt
│   ├── FiraCode-OFL.txt
│   ├── GPLv3.txt
│   └── LGPLv3.txt
├── linux/
│   ├── ffab-512.png
│   └── ffab.desktop
└── src/
    ├── Core/
    │   ├── AppConfig.h
    │   ├── AudioFileScanner.h/cpp
    │   ├── BatchProcessor.h/cpp
    │   ├── Connection.h
    │   ├── DAGCommandBuilder.h/cpp <- DAG-based FFmpeg command builder (v1.0)
    │   ├── FFmpegDetector.h/cpp
    │   ├── FFmpegRunner.h/cpp
    │   ├── FFmpegSyntax.h/cpp
    │   ├── FilterChain.h/cpp <- chain manager (routes through DAG) & old logic
    │   ├── FilterGraph.h/cpp
    │   ├── FilterNode.h
    │   ├── JobListBuilder.h/cpp
    │   ├── LogFileWriter.h/cpp
    │   ├── OperationPreview.h/cpp
    │   ├── Port.h
    │   ├── Preferences.h/cpp
    │   ├── PreviewGenerator.h/cpp
    │   └── UpdateChecker.h/cpp
    ├── FFmpeg/
    │   ├── FFmpegCommandBuilder.h/cpp
    │   └── FFmpegPresets.h/cpp
    ├── Filters/
    │   ├── AudioInputFilter.h/cpp
    │   ├── AuxOutputFilter.h/cpp
    │   ├── BaseFilter.h/cpp
    │   ├── ChannelEqFilter.h/cpp
    │   ├── CustomFFmpegFilter.h/cpp
    │   ├── InputFilter.h/cpp
    │   ├── MultiOutputFilter.h/cpp
    │   ├── OutputFilter.h/cpp
    │   ├── SmartAuxReturn.h/cpp
    │   └── ff-*.h/cpp  (118 FFmpeg filter wrappers)
    ├── UI/
    │   ├── AsplitRowWidget.h/cpp
    │   ├── BatchAdminPanel.h/cpp
    │   ├── BatchConfirmDialog.h/cpp
    │   ├── BatchSettingsWindow.h/cpp
    │   ├── ChannelEqWidget.h/cpp
    │   ├── CollapsibleHelpSection.h/cpp
    │   ├── CommandViewWindow.h/cpp
    │   ├── FFmpegSetupDialog.h/cpp
    │   ├── FileListWidget.h/cpp
    │   ├── FilterChainWidget.h/cpp
    │   ├── FilterMenuBuilder.h/cpp
    │   ├── FilterParamsPanel.h/cpp
    │   ├── InputBatchPanel.h/cpp
    │   ├── InputPanel.h/cpp
    │   ├── LogViewWindow.h/cpp
    │   ├── LogWidget.h/cpp
    │   ├── MainWindow.h/cpp
    │   ├── OutputPanel.h/cpp
    │   ├── OutputSettingsPanel.h/cpp
    │   ├── PresetManager.h/cpp
    │   ├── RegionPreviewWindow.h/cpp
    │   ├── RotatedLabel.h
    │   ├── SettingsDialog.h/cpp
    │   └── WaveformPreviewWidget.h/cpp
    ├── Utils/
    │   ├── FrequencySliderHelpers.h
    │   ├── KeyCommands.h
    │   ├── Logger.h/cpp
    │   ├── Metadata.h/cpp
    │   ├── PresetStorage.h/cpp
    │   ├── SnapSlider.h
    │   └── UnicodeSymbols.h
    └── main.cpp
```
# Toolkit

- VSCode
- Claude Code
- Affinity Designer
- Affinity Photo
- Qt
- FFmpeg
- ninja
- png2icns

# FFAB is FREE

<img src="./images/AppIcon.png" width="128px">

FFAB is free / donationware, you can support this project by purchasing music from here:

- https://dan-f.bandcamp.com
- https://dyscopian.bandcamp.com
- https://smplr.bandcamp.com
- https://www.disuye.com
