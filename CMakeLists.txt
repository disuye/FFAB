cmake_minimum_required(VERSION 3.16)
project(FFAB LANGUAGES CXX)

add_definitions(-DQT_NO_DEBUG_OUTPUT) # Disable all QDebug commands

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)

set(CMAKE_AUTOMOC_MOC_OPTIONS "")
set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS 
    src/UI/MainWindow.h 
    src/UI/FilterChainWidget.h 
    src/UI/FilterParamsPanel.h
    src/UI/BatchAdminPanel.h
    src/UI/FileListWidget.h
    src/UI/WaveformPreviewWidget.h
    src/UI/LogWidget.h
    src/UI/PresetManager.h
    src/FFmpeg/FFmpegRunner.h
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia Network Svg Concurrent)

add_executable(FFAB
    src/main.cpp

    # Core
    src/Core/AppConfig.h
    src/Core/AudioFileScanner.h
    src/Core/AudioFileScanner.cpp
    src/Core/BatchProcessor.h
    src/Core/BatchProcessor.cpp
    src/Core/FFmpegRunner.h
    src/Core/FFmpegRunner.cpp
    src/Core/FilterChain.h
    src/Core/FilterChain.cpp
    src/Core/OperationPreview.h
    src/Core/OperationPreview.cpp
    src/Core/FFmpegDetector.h
    src/Core/FFmpegDetector.cpp
    src/Core/PreviewGenerator.h
    src/Core/PreviewGenerator.cpp
    src/Core/Preferences.h
    src/Core/Preferences.cpp
    src/Core/FFmpegSyntax.h
    src/Core/FFmpegSyntax.cpp
    src/Core/JobListBuilder.h
    src/Core/JobListBuilder.cpp
    src/Core/LogFileWriter.h
    src/Core/LogFileWriter.cpp

    # Filters (Structural)
    src/Filters/BaseFilter.h
    src/Filters/BaseFilter.cpp
    src/Filters/MultiOutputFilter.h
    src/Filters/MultiOutputFilter.cpp
    src/Filters/InputFilter.h
    src/Filters/InputFilter.cpp
    src/Filters/OutputFilter.h
    src/Filters/OutputFilter.cpp
    src/Filters/AudioInputFilter.h
    src/Filters/AudioInputFilter.cpp
    src/Filters/CustomFFmpegFilter.h
    src/Filters/CustomFFmpegFilter.cpp
    src/UI/FilterMenuBuilder.h
    src/UI/FilterMenuBuilder.cpp
    
    # Filters (FFmpeg Built-in)
    src/Filters/ff-volume.h
    src/Filters/ff-volume.cpp
    src/Filters/ff-showwavespic.h
    src/Filters/ff-showwavespic.cpp
    src/Filters/ff-afir.h
    src/Filters/ff-afir.cpp
    src/Filters/ff-amerge.h
    src/Filters/ff-amerge.cpp
    src/Filters/ff-sidechaincompress.h
    src/Filters/ff-sidechaincompress.cpp
    src/Filters/ff-sidechaingate.h
    src/Filters/ff-sidechaingate.cpp
    src/Filters/ff-amix.h
    src/Filters/ff-amix.cpp
    src/Filters/ff-acrossfade.h
    src/Filters/ff-acrossfade.cpp
    src/Filters/ff-asplit.h
    src/Filters/ff-asplit.cpp
    src/Filters/ff-adelay.cpp
    src/Filters/ff-adelay.h
    src/Filters/ff-aecho.cpp
    src/Filters/ff-aecho.h
    src/Filters/ff-atempo.cpp
    src/Filters/ff-atempo.h
    src/Filters/ff-atrim.cpp
    src/Filters/ff-atrim.h
    src/Filters/ff-afade.cpp
    src/Filters/ff-afade.h
    src/Filters/ff-acrossfade.cpp
    src/Filters/ff-acrossfade.h
    src/Filters/ff-aloop.cpp
    src/Filters/ff-aloop.h
    src/Filters/ff-areverse.cpp
    src/Filters/ff-areverse.h
    src/Filters/ff-apad.cpp
    src/Filters/ff-apad.h
    src/Filters/ff-asetpts.cpp
    src/Filters/ff-asetpts.h
    src/Filters/ff-aformat.cpp
    src/Filters/ff-aformat.h
    src/Filters/ff-aresample.cpp
    src/Filters/ff-aresample.h
    src/Filters/ff-asetrate.cpp
    src/Filters/ff-asetrate.h
    src/Filters/ff-dcshift.cpp
    src/Filters/ff-dcshift.h
    src/Filters/ff-anull.cpp
    src/Filters/ff-anull.h
    src/Filters/ff-silenceremove.cpp
    src/Filters/ff-silenceremove.h
    src/Filters/ff-acompressor.cpp
    src/Filters/ff-acompressor.h
    src/Filters/ff-afreqshift.cpp
    src/Filters/ff-afreqshift.h
    src/Filters/ff-agate.cpp
    src/Filters/ff-agate.h
    src/Filters/ff-alimiter.cpp
    src/Filters/ff-alimiter.h
    src/Filters/ff-aphaser.cpp
    src/Filters/ff-aphaser.h
    src/Filters/ff-aphaseshift.cpp
    src/Filters/ff-aphaseshift.h
    src/Filters/ff-apsyclip.cpp
    src/Filters/ff-apsyclip.h
    src/Filters/ff-asoftclip.cpp
    src/Filters/ff-asoftclip.h
    src/Filters/ff-chorus.cpp
    src/Filters/ff-chorus.h
    src/Filters/ff-compand.cpp
    src/Filters/ff-compand.h
    src/Filters/ff-dynaudnorm.cpp
    src/Filters/ff-dynaudnorm.h
    src/Filters/ff-flanger.cpp
    src/Filters/ff-flanger.h
    src/Filters/ff-mcompand.cpp
    src/Filters/ff-mcompand.h
    src/Filters/ff-tremolo.cpp
    src/Filters/ff-tremolo.h
    src/Filters/ff-vibrato.cpp
    src/Filters/ff-vibrato.h
    src/Filters/ff-highpass.h
    src/Filters/ff-highpass.cpp
    src/Filters/ff-lowpass.h
    src/Filters/ff-lowpass.cpp
    src/Filters/ff-bandpass.h
    src/Filters/ff-bandpass.cpp
    src/Filters/ff-bandreject.h
    src/Filters/ff-bandreject.cpp
    src/Filters/ff-equalizer.h
    src/Filters/ff-equalizer.cpp
    src/Filters/ff-bass.h
    src/Filters/ff-bass.cpp
    src/Filters/ff-treble.h
    src/Filters/ff-treble.cpp
    src/Filters/ff-highshelf.h
    src/Filters/ff-highshelf.cpp
    src/Filters/ff-lowshelf.h
    src/Filters/ff-lowshelf.cpp
    src/Filters/ff-tiltshelf.h
    src/Filters/ff-tiltshelf.cpp
    src/Filters/ff-allpass.h
    src/Filters/ff-allpass.cpp
    src/Filters/ff-crossfeed.h
    src/Filters/ff-crossfeed.cpp
    src/Filters/ff-extrastereo.h
    src/Filters/ff-extrastereo.cpp
    src/Filters/ff-stereowiden.h
    src/Filters/ff-stereowiden.cpp
    src/Filters/ff-earwax.h
    src/Filters/ff-earwax.cpp
    src/Filters/ff-haas.h
    src/Filters/ff-haas.cpp
    src/Filters/ff-stereotools.h
    src/Filters/ff-stereotools.cpp
    src/Filters/ff-aexciter.h
    src/Filters/ff-aexciter.cpp
    src/Filters/ff-crystalizer.h
    src/Filters/ff-crystalizer.cpp
    src/Filters/ff-asubboost.h
    src/Filters/ff-asubboost.cpp
    src/Filters/ff-virtualbass.h
    src/Filters/ff-virtualbass.cpp
    src/Filters/ff-acrusher.h
    src/Filters/ff-acrusher.cpp
    src/Filters/ff-apulsator.h
    src/Filters/ff-apulsator.cpp
    src/Filters/ff-afftdn.h
    src/Filters/ff-afftdn.cpp
    src/Filters/ff-adeclick.h
    src/Filters/ff-adeclick.cpp
    src/Filters/ff-adeclip.h
    src/Filters/ff-adeclip.cpp
    src/Filters/ff-deesser.h
    src/Filters/ff-deesser.cpp
    src/Filters/ff-adrc.h
    src/Filters/ff-adrc.cpp
    src/Filters/ff-adynamicequalizer.h
    src/Filters/ff-adynamicequalizer.cpp
    src/Filters/ff-bs2b.h
    src/Filters/ff-bs2b.cpp
    src/Filters/ff-loudnorm.cpp
    src/Filters/ff-loudnorm.h
    src/Filters/ff-speechnorm.cpp
    src/Filters/ff-speechnorm.h
    src/Filters/ff-dialoguenhance.cpp
    src/Filters/ff-dialoguenhance.h
    src/Filters/ff-acontrast.cpp
    src/Filters/ff-acontrast.h
    src/Filters/ff-adecorrelate.cpp
    src/Filters/ff-adecorrelate.h
    src/Filters/ff-atilt.cpp
    src/Filters/ff-atilt.h
    src/Filters/ff-asubcut.cpp
    src/Filters/ff-asubcut.h
    src/Filters/ff-asupercut.cpp
    src/Filters/ff-asupercut.h
    src/Filters/ff-asuperpass.cpp
    src/Filters/ff-asuperpass.h
    src/Filters/ff-asuperstop.cpp
    src/Filters/ff-asuperstop.h
    src/Filters/ff-biquad.cpp
    src/Filters/ff-biquad.h
    src/Filters/ff-compensationdelay.cpp
    src/Filters/ff-compensationdelay.h
    src/Filters/ff-astats.cpp
    src/Filters/ff-astats.h
    src/Filters/ff-channelmap.cpp
    src/Filters/ff-channelmap.h
    src/Filters/ff-channelsplit.cpp
    src/Filters/ff-channelsplit.h
    src/Filters/ff-anequalizer.cpp
    src/Filters/ff-anequalizer.h
    src/Filters/ff-acopy.cpp
    src/Filters/ff-acopy.h
    src/Filters/ff-adenorm.cpp
    src/Filters/ff-adenorm.h
    src/Filters/ff-volumedetect.cpp
    src/Filters/ff-volumedetect.h
    src/Filters/ff-drmeter.cpp
    src/Filters/ff-drmeter.h
    src/Filters/ff-silencedetect.cpp
    src/Filters/ff-silencedetect.h
    src/Filters/ff-replaygain.cpp
    src/Filters/ff-replaygain.h
    src/Filters/ff-aemphasis.cpp
    src/Filters/ff-aemphasis.h
    src/Filters/ff-hdcd.cpp
    src/Filters/ff-hdcd.h
    src/Filters/ff-aderivative.cpp
    src/Filters/ff-aderivative.h
    src/Filters/ff-aintegral.cpp
    src/Filters/ff-aintegral.h
    src/Filters/ff-adynamicsmooth.cpp
    src/Filters/ff-adynamicsmooth.h
    src/Filters/ff-ashowinfo.cpp
    src/Filters/ff-ashowinfo.h
    src/Filters/ff-acue.cpp
    src/Filters/ff-acue.h
    src/Filters/ff-pan.cpp
    src/Filters/ff-pan.h
    src/Filters/ff-join.cpp
    src/Filters/ff-join.h
    src/Filters/ff-surround.cpp
    src/Filters/ff-surround.h
    src/Filters/ff-sofalizer.cpp
    src/Filters/ff-sofalizer.h
    src/Filters/ff-acrossover.cpp
    src/Filters/ff-acrossover.h
    src/Filters/ff-superequalizer.cpp
    src/Filters/ff-superequalizer.h
    src/Filters/ff-firequalizer.cpp
    src/Filters/ff-firequalizer.h
    src/Filters/ff-apsnr.cpp
    src/Filters/ff-apsnr.h
    src/Filters/ff-asdr.cpp
    src/Filters/ff-asdr.h
    src/Filters/ff-asisdr.cpp
    src/Filters/ff-asisdr.h
    src/Filters/ff-axcorrelate.cpp
    src/Filters/ff-axcorrelate.h
    src/Filters/ff-rubberband.cpp
    src/Filters/ff-rubberband.h
    src/Filters/ff-anullsink.cpp
    src/Filters/ff-anullsink.h

    # Custom & Smart Filters
    src/Filters/SmartAuxReturn.h
    src/Filters/SmartAuxReturn.cpp
    src/Filters/AuxOutputFilter.h
    src/Filters/AuxOutputFilter.cpp

    #PROTOTYPE
    src/Filters/ChannelEqFilter.cpp
    src/Filters/ChannelEqFilter.h
    src/UI/ChannelEqWidget.cpp
    src/UI/ChannelEqWidget.h

    # FFmpeg
    src/FFmpeg/FFmpegCommandBuilder.h
    src/FFmpeg/FFmpegCommandBuilder.cpp
    src/FFmpeg/FFmpegPresets.h
    src/FFmpeg/FFmpegPresets.cpp
    
    # UI
    src/UI/MainWindow.h
    src/UI/MainWindow.cpp
    src/UI/FilterChainWidget.h
    src/UI/FilterChainWidget.cpp
    src/UI/AsplitRowWidget.h
    src/UI/AsplitRowWidget.cpp
    src/UI/FilterParamsPanel.h
    src/UI/FilterParamsPanel.cpp
    src/UI/FileListWidget.h
    src/UI/FileListWidget.cpp
    src/UI/CollapsibleHelpSection.h
    src/UI/CollapsibleHelpSection.cpp
    src/UI/WaveformPreviewWidget.h
    src/UI/WaveformPreviewWidget.cpp
    src/UI/CommandViewWindow.h
    src/UI/CommandViewWindow.cpp
    src/UI/LogViewWindow.h
    src/UI/LogViewWindow.cpp
    src/UI/LogWidget.h
    src/UI/LogWidget.cpp
    src/UI/PresetManager.h
    src/UI/PresetManager.cpp
    src/UI/InputPanel.cpp
    src/UI/InputPanel.h
    src/UI/OutputSettingsPanel.cpp
    src/UI/OutputSettingsPanel.h
    src/UI/SettingsDialog.h
    src/UI/SettingsDialog.cpp
    src/UI/BatchSettingsWindow.h
    src/UI/BatchSettingsWindow.cpp
    src/UI/BatchConfirmDialog.h
    src/UI/BatchConfirmDialog.cpp
    src/UI/RegionPreviewWindow.h
    src/UI/RegionPreviewWindow.cpp
    src/UI/FFmpegSetupDialog.h
    src/UI/FFmpegSetupDialog.cpp
    
    # Utils
    src/Utils/KeyCommands.h
    src/Utils/Metadata.h
    src/Utils/Metadata.cpp
    src/Utils/PresetStorage.h
    src/Utils/PresetStorage.cpp
    src/Utils/Logger.h
    src/Utils/Logger.cpp
    src/Utils/SnapSlider.h
)

target_include_directories(FFAB PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Filters
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FFmpeg
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
)

# Explicit resource embedding
qt6_add_resources(FFAB "app_resources"
    PREFIX "/"
    FILES
        AppIcon.icns
        AppIcon.png
        fonts/FiraCode-VariableFont_wght.ttf
        images/log-bg-light.png
        images/log-bg-dark.png
        images/fader-v.png
        images/fader-h.png
        images/track-v.png
        images/track-h.png
        licenses/FiraCode-OFL.txt
        licenses/LGPLv3.txt
        licenses/GPLv3.txt
        licenses/FFAB-license.txt
)

target_link_libraries(FFAB PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::Network Qt6::Svg Qt6::Concurrent)

# -----------------------------
# Detect version from MainWindow.h
# -----------------------------
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/AppConfig.h" VERSION_LINE REGEX "^#define VERSION_STR")
if(VERSION_LINE)
    string(REGEX MATCH "\"([^\"]+)\"" VERSION_STR "${VERSION_LINE}")
    string(REPLACE "\"" "" VERSION_STR "${CMAKE_MATCH_1}")
else()
    message(WARNING "VERSION_STR not found in AppConfig.h – falling back to 6.6.6")
    set(VERSION_STR "6.6.6")
endif()
message(STATUS "Detected application version: ${VERSION_STR}")


if(APPLE)
    set_target_properties(FFAB PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.disuye.ffab"
        MACOSX_BUNDLE_BUNDLE_NAME "FFAB"
        MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_STR}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_STR}"
        MACOSX_BUNDLE_ICON_FILE AppIcon.icns
        )
        
        set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/AppIcon.icns")
        set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        target_sources(FFAB PRIVATE ${app_icon_macos})
        
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")

    qt_finalize_executable(FFAB)

elseif(UNIX AND NOT APPLE)
    set_target_properties(FFAB PROPERTIES
        BUILD_RPATH_USE_ORIGIN TRUE
        INSTALL_RPATH "$ORIGIN/lib"
    )

    install(TARGETS FFAB DESTINATION bin)
    install(FILES linux/ffab.desktop DESTINATION share/applications)
    install(FILES linux/ffab-512.png DESTINATION share/icons/hicolor/512x512/apps RENAME ffab.png)

else()
    message(STATUS "Unsupported platform – default executable only")
endif()
