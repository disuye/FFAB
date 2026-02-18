#include "FilterChain.h"
#include "BaseFilter.h"
#include "InputFilter.h"
#include "OutputFilter.h"
#include "AudioInputFilter.h"
#include "AuxOutputFilter.h"
#include "MultiOutputFilter.h"
#include "SmartAuxReturn.h"
#include "CustomFFmpegFilter.h"
#include "ChannelEqFilter.h"

#include "ff-adelay.h"
#include "ff-aecho.h"
#include "ff-atempo.h"
#include "ff-atrim.h"
#include "ff-afade.h"
#include "ff-acrossfade.h"
#include "ff-aloop.h"
#include "ff-areverse.h"
#include "ff-apad.h"
#include "ff-asetpts.h"
#include "ff-aformat.h"
#include "ff-aresample.h"
#include "ff-asetrate.h"
#include "ff-dcshift.h"
#include "ff-anull.h"
#include "ff-silenceremove.h"
#include "ff-volume.h"
#include "ff-showwavespic.h"
#include "ff-afir.h"
#include "ff-amerge.h"
#include "ff-sidechaincompress.h"
#include "ff-sidechaingate.h"
#include "ff-amix.h"
#include "ff-acrossfade.h"
#include "ff-asplit.h"
#include "ff-acompressor.h"
#include "ff-alimiter.h"
#include "ff-agate.h"
#include "ff-dynaudnorm.h"
#include "ff-compand.h"
#include "ff-mcompand.h"
#include "ff-apsyclip.h"
#include "ff-asoftclip.h"
#include "ff-chorus.h"
#include "ff-flanger.h"
#include "ff-aphaser.h"
#include "ff-tremolo.h"
#include "ff-vibrato.h"
#include "ff-afreqshift.h"
#include "ff-aphaseshift.h"
#include "ff-highpass.h"
#include "ff-lowpass.h"
#include "ff-bandpass.h"
#include "ff-bandreject.h"
#include "ff-equalizer.h"
#include "ff-bass.h"
#include "ff-treble.h"
#include "ff-highshelf.h"
#include "ff-lowshelf.h"
#include "ff-tiltshelf.h"
#include "ff-allpass.h"
#include "ff-crossfeed.h"
#include "ff-extrastereo.h"
#include "ff-stereowiden.h"
#include "ff-earwax.h"
#include "ff-haas.h"
#include "ff-stereotools.h"
#include "ff-aexciter.h"
#include "ff-crystalizer.h"
#include "ff-asubboost.h"
#include "ff-virtualbass.h"
#include "ff-acrusher.h"
#include "ff-apulsator.h"
#include "ff-afftdn.h"
#include "ff-adeclick.h"
#include "ff-adeclip.h"
#include "ff-deesser.h"
#include "ff-adrc.h"
#include "ff-adynamicequalizer.h"
#include "ff-bs2b.h"
#include "ff-loudnorm.h"
#include "ff-speechnorm.h"
#include "ff-dialoguenhance.h"
#include "ff-acontrast.h"
#include "ff-adecorrelate.h"
#include "ff-atilt.h"
#include "ff-asubcut.h"
#include "ff-asupercut.h"
#include "ff-asuperpass.h"
#include "ff-asuperstop.h"
#include "ff-biquad.h"
#include "ff-compensationdelay.h"
#include "ff-astats.h"
#include "ff-channelmap.h"
#include "ff-channelsplit.h"
#include "ff-anequalizer.h"
#include "ff-acopy.h"
#include "ff-adenorm.h"
#include "ff-volumedetect.h"
#include "ff-drmeter.h"
#include "ff-silencedetect.h"
#include "ff-replaygain.h"
#include "ff-aemphasis.h"
#include "ff-hdcd.h"
#include "ff-aderivative.h"
#include "ff-aintegral.h"
#include "ff-adynamicsmooth.h"
#include "ff-ashowinfo.h"
#include "ff-acue.h"
#include "ff-apsnr.h"
#include "ff-asdr.h"
#include "ff-asisdr.h"
#include "ff-axcorrelate.h"
#include "ff-pan.h"
#include "ff-join.h"
#include "ff-surround.h"
#include "ff-sofalizer.h"
#include "ff-acrossover.h"
#include "ff-superequalizer.h"
#include "ff-firequalizer.h"
#include "ff-rubberband.h"
#include "ff-anullsink.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QSettings>
#include <algorithm>

// ========== LogSettings Implementation ==========

QString LogSettings::buildFlags() const {
    QStringList args;
    args << "-y";
    if (!showBanner) args << "-hide_banner";
    args << "-loglevel" << logLevel;
    if (showStats) {
        args << "-stats";
    } else {
        args << "-nostats";
    }
    return args.join(" ");
}

LogSettings LogSettings::preview() {
    return LogSettings();
}

LogSettings LogSettings::fromQSettings() {
    QSettings settings;
    LogSettings s;
    s.showBanner = settings.value("log/showBanner", false).toBool();
    s.logLevel = settings.value("log/logLevel", "error").toString();
    s.showStats = settings.value("log/showStats", true).toBool();
    return s;
}

// Static empty chain for out-of-bounds access
const std::vector<std::shared_ptr<BaseFilter>> FilterChain::s_emptyChain;

FilterChain::FilterChain() {
    inputFilter = new InputFilter();
    outputFilter = new OutputFilter();
    
    // First middle filter will be #1
    m_nextFilterId = 1;
    
    filters.push_back(std::shared_ptr<BaseFilter>(inputFilter));
    filters.push_back(std::shared_ptr<BaseFilter>(outputFilter));
}

FilterChain::~FilterChain() {}

void FilterChain::assignFilterId(std::shared_ptr<BaseFilter> filter) {
    if (filter && filter->getFilterId() < 0) {
        filter->setFilterId(m_nextFilterId++);
    }
}

void FilterChain::addFilter(std::shared_ptr<BaseFilter> filter, int position) {
    if (!filter) return;
    
    // Assign unique ID if not already assigned
    assignFilterId(filter);
    
    if (position < 0) position = filters.size() - 1;
    position = std::max(1, std::min(position, (int)filters.size() - 1));
    filters.insert(filters.begin() + position, filter);
}

void FilterChain::removeFilter(int position) {
    if (position <= 0 || position >= (int)filters.size() - 1) return;
    
    // Note: MultiOutputFilter's sub-chains are automatically cleaned up
    // when the filter is destroyed (shared_ptr magic)
    filters.erase(filters.begin() + position);
}

void FilterChain::moveFilter(int fromPos, int toPos) {
    if (fromPos < 1 || fromPos >= (int)filters.size() - 1 ||
        toPos < 1 || toPos >= (int)filters.size() - 1) return;
    if (fromPos == toPos) return;
    
    auto filter = filters[fromPos];
    filters.erase(filters.begin() + fromPos);
    filters.insert(filters.begin() + toPos, filter);
}

std::shared_ptr<BaseFilter> FilterChain::getFilter(int position) const {
    if (position >= 0 && position < (int)filters.size()) {
        return filters[position];
    }
    return nullptr;
}

// ========== Sub-Chain Operations ==========

MultiOutputFilter* FilterChain::getMultiOutputFilter(int position) const {
    auto filter = getFilter(position);
    if (!filter) return nullptr;
    return dynamic_cast<MultiOutputFilter*>(filter.get());
}

void FilterChain::addFilterToStream(int mainChainPos, int streamIndex, std::shared_ptr<BaseFilter> filter) {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) {
        qWarning() << "FilterChain::addFilterToStream: No MultiOutputFilter at position" << mainChainPos;
        return;
    }
    
    // Assign ID from our global counter
    assignFilterId(filter);
    
    multiOutput->addFilterToStream(streamIndex, filter);
}

void FilterChain::removeFilterFromStream(int mainChainPos, int streamIndex, int filterPos) {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) {
        qWarning() << "FilterChain::removeFilterFromStream: No MultiOutputFilter at position" << mainChainPos;
        return;
    }
    
    multiOutput->removeFilterFromStream(streamIndex, filterPos);
}

void FilterChain::moveFilterInStream(int mainChainPos, int streamIndex, int fromPos, int toPos) {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) {
        qWarning() << "FilterChain::moveFilterInStream: No MultiOutputFilter at position" << mainChainPos;
        return;
    }
    
    multiOutput->moveFilterInStream(streamIndex, fromPos, toPos);
}

std::shared_ptr<BaseFilter> FilterChain::getFilterFromStream(int mainChainPos, int streamIndex, int filterPos) const {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) return nullptr;
    
    return multiOutput->getFilterFromStream(streamIndex, filterPos);
}

const std::vector<std::shared_ptr<BaseFilter>>& FilterChain::getSubChain(int mainChainPos, int streamIndex) const {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) return s_emptyChain;
    
    return multiOutput->getSubChain(streamIndex);
}

int FilterChain::getStreamFilterCount(int mainChainPos, int streamIndex) const {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) return 0;
    
    return multiOutput->getStreamFilterCount(streamIndex);
}

QSet<int> FilterChain::getActiveStreamsForMultiOutput(int mainChainPos) const {
    auto* multiOutput = getMultiOutputFilter(mainChainPos);
    if (!multiOutput) return QSet<int>();
    
    return multiOutput->getActiveStreams();
}

// ========== Utilities ==========

QString FilterChain::getFilterHexLabel(int filterId) const {
    return QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper();
}

// ========== FFmpeg Command Building ==========

QString FilterChain::buildCompleteCommand(const QString& inputFile, const QString& outputFile,
                                         const LogSettings& logSettings) const {
    QString command = logSettings.buildFlags() + " ";
    
    if (inputFilter) {
        QString inputFlags = inputFilter->buildFFmpegFlags();
        inputFlags.replace("{INPUT}", inputFile);
        command += inputFlags + " ";
    }
    
    int audioInputCount = getRequiredAudioInputCount();
    if (audioInputCount > 0) {
        qDebug() << "WARNING: Filter chain requires" << audioInputCount 
                 << "sidechain input(s) but buildCompleteCommand doesn't support sidechain files yet";
    }
    
    // Check if chain ends with a sink (no audio output)
    bool chainEndsWithSink = endsWithSinkFilter();
    
    // Check for video passthrough mode
    bool videoPassthrough = outputFilter && outputFilter->isVideoPassthrough();
    
    QString filterFlags = buildFilterFlags();
    if (!filterFlags.isEmpty()) {
        command += "-filter_complex \"" + filterFlags + "\" ";
        
        // Video passthrough: map video/subtitle streams before audio
        if (videoPassthrough) {
            command += outputFilter->buildOutputMappingFlags() + " ";
        }
        
        // Only add audio output mapping if chain doesn't end with sink
        if (!chainEndsWithSink) {
            command += "-map \"[out]\" ";
        }
    } else if (videoPassthrough) {
        // No filter graph but video passthrough — still need video mapping
        command += outputFilter->buildOutputMappingFlags() + " ";
    }
    
    // Only add audio output file if chain doesn't end with sink
    if (!chainEndsWithSink) {
        if (outputFilter) command += outputFilter->buildFFmpegFlags() + " ";
        command += QString("\"%1\"").arg(outputFile);
    }
    
    // ========== APPEND AUX OUTPUT MAPPINGS ==========
    int auxIndex = 1;
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (auto* auxOut = dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);
            
            // Build output file path
            QString auxOutputPath = buildAuxOutputPath(inputFile, auxOut, auxIndex);
            
            // Append output mapping
            command += QString(" -map \"%1\" %2 \"%3\"")
                .arg(auxBranchLabel)
                .arg(auxOut->getCodecFlags())
                .arg(auxOutputPath);
            
            auxIndex++;
        }
    }
    
    // ========== APPEND WAVEFORM IMAGE OUTPUT MAPPINGS ==========
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);
            
            // Build output file path
            QString waveformOutputPath = buildWaveformOutputPath(inputFile, waveform);
            
            // Append video output mapping
            command += QString(" -map \"%1\" %2 \"%3\"")
                .arg(waveformBranchLabel)
                .arg(waveform->getVideoCodecFlags())
                .arg(waveformOutputPath);
        }
    }
    
    return command.trimmed();
}


QString FilterChain::buildFilterFlags() const {
    // Delegate to the version with muted positions, passing an empty list
    return buildFilterFlags(QList<int>());
}


QString FilterChain::buildCompleteCommand(const QString& inputFile, const QString& outputFile,
                                         const QList<int>& mutedPositions,
                                         const LogSettings& logSettings) const {
    QString command = logSettings.buildFlags() + " ";
    
    if (inputFilter) {
        QString inputFlags = inputFilter->buildFFmpegFlags();
        inputFlags.replace("{INPUT}", inputFile);
        command += inputFlags + " ";
    }
    
    // Check if chain ends with a sink (no audio output)
    bool chainEndsWithSink = endsWithSinkFilter(mutedPositions);
    
    // Check for video passthrough mode
    bool videoPassthrough = outputFilter && outputFilter->isVideoPassthrough();
    
    QString filterFlags = buildFilterFlags(mutedPositions);
    if (!filterFlags.isEmpty()) {
        command += "-filter_complex \"" + filterFlags + "\" ";
        
        // Video passthrough: map video/subtitle streams before audio
        if (videoPassthrough) {
            command += outputFilter->buildOutputMappingFlags() + " ";
        }
        
        // Only add audio output mapping if chain doesn't end with sink
        if (!chainEndsWithSink) {
            command += "-map \"[out]\" "; // quotes for View Command, stripped by QProcess
        }
    } else if (videoPassthrough) {
        // No filter graph but video passthrough — still need video mapping
        command += outputFilter->buildOutputMappingFlags() + " ";
    }
    
    // Only add audio output file if chain doesn't end with sink
    if (!chainEndsWithSink) {
        if (outputFilter) command += outputFilter->buildFFmpegFlags() + " ";
        command += QString("\"%1\"").arg(outputFile);
    }
    
    // ========== APPEND AUX OUTPUT MAPPINGS ==========
    int auxIndex = 1;
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
            continue;
        }
        
        if (auto* auxOut = dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);
            
            // Build output file path
            QString auxOutputPath = buildAuxOutputPath(inputFile, auxOut, auxIndex);
            
            // Append output mapping
            command += QString(" -map \"%1\" %2 \"%3\"") // quotes for View Command, stripped by QProcess
                .arg(auxBranchLabel)
                .arg(auxOut->getCodecFlags())
                .arg(auxOutputPath);
            
            auxIndex++;
        }
    }
    
    // ========== APPEND WAVEFORM IMAGE OUTPUT MAPPINGS ==========
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
            continue;
        }
        
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);
            
            // Build output file path
            QString waveformOutputPath = buildWaveformOutputPath(inputFile, waveform);
            
            // Append video output mapping
            command += QString(" -map \"%1\" %2 \"%3\"")
                .arg(waveformBranchLabel)
                .arg(waveform->getVideoCodecFlags())
                .arg(waveformOutputPath);
        }
    }
    
    return command.trimmed();
}

QString FilterChain::buildFilterFlags(const QList<int>& mutedPositions) const {
    if (filters.size() <= 2) return "";

    // Phase A: DAG path for linear chains
    if (isLinearChain(mutedPositions)) {
        return buildFilterFlagsDAG(mutedPositions);
    }
    
    // Check if chain ends with a sink filter (no audio output)
    bool chainEndsWithSink = endsWithSinkFilter(mutedPositions);
    
    // Helper lambda to check if there are any filters (including aux outputs, waveforms, asplit) after position i
    auto hasFiltersAfter = [&](size_t currentPos) -> bool {
        for (size_t j = currentPos + 1; j < filters.size() - 1; ++j) {
            if (mutedPositions.contains(static_cast<int>(j))) {
                continue;  // Skip muted filters
            }
            
            auto* filter = filters[j].get();
            
            // AuxOutput, Waveform, and Asplit count as filters after
            if (dynamic_cast<AuxOutputFilter*>(filter) ||
                dynamic_cast<FFShowwavespic*>(filter) ||
                dynamic_cast<AsplitFilter*>(filter)) {
                return true;
            }
            
            // AudioInputFilter doesn't produce filter output, skip it
            if (dynamic_cast<AudioInputFilter*>(filter)) {
                continue;
            }
            
            // Any other non-muted filter with actual output
            if (!filter->buildFFmpegFlags().isEmpty()) {
                return true;
            }
        }
        return false;
    };
    
    QStringList filterStrs;
    QString mainChainInput = "[0:a]";
    
    QMap<int, QString> sidechainOutputs;
    AudioInputFilter* currentAudioInput = nullptr;
    
    // Count total non-muted aux outputs and waveforms, track last positions
    int totalAuxOutputs = 0;
    int totalWaveforms = 0;
    int lastAuxOutputPos = -1;
    int lastWaveformPos = -1;
    bool hasFiltersAfterBranchOutputs = false;
    
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) continue;
        
        if (dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            totalAuxOutputs++;
            lastAuxOutputPos = i;
        }
        if (dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            totalWaveforms++;
            lastWaveformPos = i;
        }
    }
    
    // Find the last branch output position (aux or waveform)
    int lastBranchOutputPos = qMax(lastAuxOutputPos, lastWaveformPos);
    
    // Check if there are any non-branch-output filters after the last branch output
    if (lastBranchOutputPos >= 0) {
        for (size_t i = lastBranchOutputPos + 1; i < filters.size() - 1; ++i) {
            if (mutedPositions.contains(static_cast<int>(i))) continue;
            // Check for regular filters OR asplit (which has empty buildFFmpegFlags)
            if ((!filters[i]->buildFFmpegFlags().isEmpty() && 
                !dynamic_cast<AuxOutputFilter*>(filters[i].get()) &&
                !dynamic_cast<FFShowwavespic*>(filters[i].get())) ||
                dynamic_cast<AsplitFilter*>(filters[i].get())) {
                hasFiltersAfterBranchOutputs = true;
                break;
            }
        }
    }
    
    int auxOutputsProcessed = 0;
    int waveformsProcessed = 0;
    
    // Count NON-aux-output, NON-waveform filters with non-empty flags
    int filterCount = 0;
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (!mutedPositions.contains(static_cast<int>(i)) && 
            !filters[i]->buildFFmpegFlags().isEmpty() &&
            !dynamic_cast<AuxOutputFilter*>(filters[i].get()) &&
            !dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            filterCount++;
        }
    }
    
    int processedFilters = 0;
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
    // Muted filter should be invisible - don't clear sidechain state
    // so it can pass through to the next filter that needs it
    continue;
}
        
        // HANDLE AUX OUTPUT FILTER - Insert asplit to branch stream
        if (auto* auxOutput = dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            auxOutputsProcessed++;
            bool isLastBranchOutput = (auxOutputsProcessed == totalAuxOutputs && 
                                       waveformsProcessed == totalWaveforms);
            
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);
            
            QString mainContinueLabel;
            // If this is the last branch output AND there are no regular filters after it
            if (isLastBranchOutput && !hasFiltersAfterBranchOutputs) {
                // Last branch output and nothing after: main chain continues to [out]
                mainContinueLabel = "[out]";
            } else {
                // Not last, or has filters after: main chain continues to intermediate label
                mainContinueLabel = QString("[%1]").arg(filterIdLabel);
            }
            
            // Generate asplit: input → [continue] + [auxOut]
            QString asplitCmd = QString("%1asplit=2%2%3")
                .arg(mainChainInput)
                .arg(mainContinueLabel)
                .arg(auxBranchLabel);
            
            filterStrs.append(asplitCmd);
            
            // Main chain continues from the main branch
            mainChainInput = mainContinueLabel;
            continue;
        }
        
        // HANDLE WAVEFORM IMAGE FILTER - Insert asplit to branch stream for video output
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            waveformsProcessed++;
            bool isLastBranchOutput = (auxOutputsProcessed == totalAuxOutputs && 
                                       waveformsProcessed == totalWaveforms);
            
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);
            
            QString mainContinueLabel;
            // If this is the last branch output AND there are no regular filters after it
            if (isLastBranchOutput && !hasFiltersAfterBranchOutputs) {
                mainContinueLabel = "[out]";
            } else {
                mainContinueLabel = QString("[%1]").arg(filterIdLabel);
            }
            
            // Generate asplit: input → [continue] + [waveform_in]
            // Then showwavespic filter: [waveform_in] → showwavespic → [waveform]
            QString waveformInputLabel = QString("[%1-wfin]").arg(filterIdLabel);
            QString asplitCmd = QString("%1asplit=2%2%3")
                .arg(mainChainInput)
                .arg(mainContinueLabel)
                .arg(waveformInputLabel);
            filterStrs.append(asplitCmd);
            
            // Add the showwavespic filter
            QString waveformFilterCmd = QString("%1%2%3")
                .arg(waveformInputLabel)
                .arg(waveform->buildFFmpegFlags())
                .arg(waveformBranchLabel);
            filterStrs.append(waveformFilterCmd);
            
            // Main chain continues from the main branch
            mainChainInput = mainContinueLabel;
            continue;
        }
        
        // HANDLE ASPLIT FILTER - Split into parallel streams with sub-chains
        if (auto* asplitFilter = dynamic_cast<AsplitFilter*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString hexId = getFilterHexLabel(filterId);
            int numSplits = asplitFilter->numSplits();
            int totalStreams = numSplits + 1;  // thru + parallel streams
            
            // Determine if this is the last filter in the chain
            bool isLastFilter = !hasFiltersAfter(i);
            
            // Generate asplit command: [input]asplit=N[thru][s1][s2]...
            QString thruLabel = QString("[%1_thru]").arg(hexId);
            QString asplitCmd = QString("%1asplit=%2%3")
                .arg(mainChainInput)
                .arg(totalStreams)
                .arg(thruLabel);
            
            QStringList streamLabels;
            for (int s = 1; s <= numSplits; s++) {
                QString label = QString("[%1_s%2]").arg(hexId).arg(s);
                asplitCmd += label;
                streamLabels << label;
            }
            filterStrs.append(asplitCmd);
            
            // Process each sub-chain and collect outputs for mixing
            QStringList mixInputs;
            QStringList mixWeights;
            
            // Thru (stream 0) goes directly to mix
            mixInputs << thruLabel;
            mixWeights << QString::number(asplitFilter->getStreamWeight(0), 'f', 4);
            
            for (int s = 1; s <= numSplits; s++) {
                QString currentInput = streamLabels[s - 1];
                bool isStreamMuted = asplitFilter->isStreamMuted(s);
                
                if (isStreamMuted) {
                    // Muted stream: skip sub-chain processing, use raw stream with weight 0
                    mixInputs << currentInput;
                    mixWeights << "0";
                } else {
                    const auto& subChain = asplitFilter->getSubChain(s);
                    
                    if (subChain.empty()) {
                        // No processing on this stream, goes directly to mix
                        mixInputs << currentInput;
                    } else {
                        // Process sub-chain filters with sidechain awareness
                        AudioInputFilter* subCurrentAudioInput = nullptr;
                        QMap<int, QString> subSidechainOutputs;

                        for (size_t j = 0; j < subChain.size(); j++) {
                            const auto& subFilter = subChain[j];

                            // Handle AudioInputFilter — establishes sidechain context
                            if (auto* audioInput = dynamic_cast<AudioInputFilter*>(subFilter.get())) {
                                subCurrentAudioInput = audioInput;
                                int idx = audioInput->getInputIndex();
                                subSidechainOutputs[idx] = QString("[%1:a]").arg(idx);
                                continue;
                            }

                            QString subFilterStr = subFilter->buildFFmpegFlags();
                            if (subFilterStr.isEmpty()) continue;

                            int subFilterId = subFilter->getFilterId();
                            QString subHexId = getFilterHexLabel(subFilterId);
                            QString subOutputLabel = QString("[%1_s%2_%3]")
                                .arg(hexId).arg(s).arg(subHexId);

                            bool subIsAnalysisTwoInput = subFilter->isAnalysisTwoInputFilter();

                            bool subIsMultiInput = (dynamic_cast<FFAmix*>(subFilter.get()) ||
                                                    dynamic_cast<FFAmerge*>(subFilter.get()) ||
                                                    dynamic_cast<FFAfir*>(subFilter.get()) ||
                                                    dynamic_cast<FFSidechaincompress*>(subFilter.get()) ||
                                                    dynamic_cast<FFSidechaingate*>(subFilter.get()) ||
                                                    dynamic_cast<FFAcrossfade*>(subFilter.get()) ||
                                                    dynamic_cast<FFAxcorrelate*>(subFilter.get()) ||
                                                    dynamic_cast<FFJoin*>(filters[i].get()));

                            bool subIsInsertMode = (subCurrentAudioInput != nullptr &&
                                                    !subIsAnalysisTwoInput &&
                                                    !subIsMultiInput);

                            if (subIsInsertMode) {
                                // INSERT MODE: Wire filter onto the sidechain path
                                int idx = subCurrentAudioInput->getInputIndex();
                                QString inputLabel = subSidechainOutputs[idx];

                                bool hasInputLabels = subFilterStr.contains(QRegularExpression("^\\["));
                                if (!hasInputLabels) {
                                    subFilterStr = inputLabel + subFilterStr;
                                }

                                int lastParam = qMax(subFilterStr.lastIndexOf('='), subFilterStr.lastIndexOf(':'));
                                if (lastParam != -1) {
                                    int outputLabelStart = subFilterStr.indexOf('[', lastParam);
                                    if (outputLabelStart != -1) {
                                        subFilterStr = subFilterStr.left(outputLabelStart);
                                    }
                                }
                                subFilterStr += subOutputLabel;

                                subSidechainOutputs[idx] = subOutputLabel;

                                filterStrs.append(subFilterStr);
                            }
                            else if (subIsAnalysisTwoInput) {
                                // ANALYSIS TWO-INPUT: [subChainStream][sidechain]filterName[output]
                                QString scLabel;
                                if (!subSidechainOutputs.isEmpty()) {
                                    QList<int> scKeys = subSidechainOutputs.keys();
                                    std::sort(scKeys.begin(), scKeys.end());
                                    scLabel = subSidechainOutputs[scKeys.last()];
                                } else if (!sidechainOutputs.isEmpty()) {
                                    QList<int> scKeys = sidechainOutputs.keys();
                                    std::sort(scKeys.begin(), scKeys.end());
                                    scLabel = sidechainOutputs[scKeys.last()];
                                } else {
                                    scLabel = "[1:a]";
                                }

                                subFilterStr = currentInput + scLabel + subFilterStr + subOutputLabel;

                                filterStrs.append(subFilterStr);
                                currentInput = subOutputLabel;
                            }
                            else if (subIsMultiInput) {
                                // MULTI-INPUT in sub-chain
                                static QRegularExpression leadingInputLabels(R"(^(\[\d+:a\])+)");
                                QRegularExpressionMatch inputLabelMatch = leadingInputLabels.match(subFilterStr);

                                if (inputLabelMatch.hasMatch()) {
                                    subFilterStr = subFilterStr.mid(inputLabelMatch.capturedLength());
                                    QString inputPrefix = currentInput;
                                    QList<int> scKeys = subSidechainOutputs.keys();
                                    std::sort(scKeys.begin(), scKeys.end());
                                    for (int key : scKeys) {
                                        inputPrefix += subSidechainOutputs[key];
                                    }
                                    subFilterStr = inputPrefix + subFilterStr;
                                } else {
                                    for (auto it = subSidechainOutputs.constBegin(); it != subSidechainOutputs.constEnd(); ++it) {
                                        int idx = it.key();
                                        QString rawLabel = QString("[%1:a]").arg(idx);
                                        QString processedLabel = it.value();
                                        if (processedLabel != rawLabel) {
                                            subFilterStr.replace(rawLabel, processedLabel);
                                        }
                                    }
                                    if (!subFilterStr.contains(currentInput)) {
                                        subFilterStr.replace("[0:a]", currentInput);
                                    }
                                }

                                int lastParam = qMax(subFilterStr.lastIndexOf('='), subFilterStr.lastIndexOf(':'));
                                if (lastParam != -1) {
                                    int outputLabelStart = subFilterStr.indexOf('[', lastParam);
                                    if (outputLabelStart != -1) {
                                        subFilterStr = subFilterStr.left(outputLabelStart);
                                    }
                                }
                                subFilterStr += subOutputLabel;

                                filterStrs.append(subFilterStr);
                                currentInput = subOutputLabel;

                                subCurrentAudioInput = nullptr;
                                subSidechainOutputs.clear();
                            }
                            else {
                                // NORMAL FILTER: wire onto sub-chain stream
                                bool hasInputLabels = subFilterStr.contains(QRegularExpression("^\\["));
                                if (!hasInputLabels) {
                                    subFilterStr = currentInput + subFilterStr;
                                } else {
                                    subFilterStr.replace("[0:a]", currentInput);
                                }

                                int lastParam = qMax(subFilterStr.lastIndexOf('='), subFilterStr.lastIndexOf(':'));
                                if (lastParam != -1) {
                                    int outputLabelStart = subFilterStr.indexOf('[', lastParam);
                                    if (outputLabelStart != -1) {
                                        subFilterStr = subFilterStr.left(outputLabelStart);
                                    }
                                }
                                subFilterStr += subOutputLabel;

                                filterStrs.append(subFilterStr);
                                currentInput = subOutputLabel;
                            }
                        }
                        mixInputs << currentInput;  // Final sub-chain output goes to mix
                    }
                    mixWeights << QString::number(asplitFilter->getStreamWeight(s), 'f', 4);
                }
            }

            // Generate amix to recombine all streams (if auto amix is enabled)
            if (asplitFilter->useAutoAmix()) {
                QString mixOutput = isLastFilter ? "[out]" : QString("[%1]").arg(hexId);
                QString amixCmd = mixInputs.join("") +
                    QString("amix=inputs=%1:weights=%2:duration=longest:dropout_transition=0:normalize=0%3")
                        .arg(mixInputs.size())
                        .arg(mixWeights.join(" "))
                        .arg(mixOutput);
                filterStrs.append(amixCmd);
                mainChainInput = mixOutput;
            } else {
                // Manual routing mode: thru stream becomes main chain
                mainChainInput = thruLabel;
            }

            currentAudioInput = nullptr;
            sidechainOutputs.clear();
            continue;
        }
        
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filters[i].get())) {
            currentAudioInput = audioInput;
            int idx = audioInput->getInputIndex();
            sidechainOutputs[idx] = QString("[%1:a]").arg(idx);
            continue;
        }
        
        // HANDLE NULL SINK FILTER - Consumes audio, no output
        if (dynamic_cast<FFAnullsink*>(filters[i].get())) {
            QString sinkCmd = QString("%1anullsink").arg(mainChainInput);
            filterStrs.append(sinkCmd);
            // No output label - sink consumes the stream
            // Main chain effectively ends here
            continue;
        }
        
        QString filterStr = filters[i]->buildFFmpegFlags();
        if (filterStr.isEmpty()) continue;
        
        processedFilters++;
        bool isLastFilter = !hasFiltersAfter(i);
        int filterId = filters[i]->getFilterId();

        bool isMultiInputFilter = (dynamic_cast<FFAfir*>(filters[i].get()) ||
                                   dynamic_cast<FFSidechaincompress*>(filters[i].get()) ||
                                   dynamic_cast<FFSidechaingate*>(filters[i].get()) ||
                                   dynamic_cast<FFAcrossfade*>(filters[i].get()) ||
                                   dynamic_cast<FFAmerge*>(filters[i].get()) ||
                                   dynamic_cast<FFAmix*>(filters[i].get()) ||
                                   dynamic_cast<FFAxcorrelate*>(filters[i].get()) ||
                                   dynamic_cast<FFJoin*>(filters[i].get()));

        bool isSmartAuxReturn = (dynamic_cast<SmartAuxReturn*>(filters[i].get()) != nullptr);

        bool isAnalysisTwoInput = filters[i]->isAnalysisTwoInputFilter();

        bool isInInsertMode = (currentAudioInput != nullptr && !isMultiInputFilter && !isSmartAuxReturn && !isAnalysisTwoInput);
        
        if (isInInsertMode) {
            int idx = currentAudioInput->getInputIndex();
            QString inputLabel = sidechainOutputs[idx];
            
            bool hasInputLabels = filterStr.contains(QRegularExpression("^\\["));
            
            if (!hasInputLabels) {
                filterStr = inputLabel + filterStr;
            }
            
            QString outputLabel = QString("[%1]").arg(getFilterHexLabel(filterId));
            
            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(filters[i].get())) {
                manualLabels = customFilter->getManualOutputLabels();
            }
            
            if (!manualLabels) {
                int lastParam = qMax(filterStr.lastIndexOf('='), filterStr.lastIndexOf(':'));
                if (lastParam != -1) {
                    int outputLabelStart = filterStr.indexOf('[', lastParam);
                    if (outputLabelStart != -1) {
                        filterStr = filterStr.left(outputLabelStart);
                    }
                }
                filterStr += outputLabel;
                sidechainOutputs[idx] = outputLabel;
            }
            
            filterStrs.append(filterStr);
        }
        else if (isSmartAuxReturn) {
            auto* smartAux = dynamic_cast<SmartAuxReturn*>(filters[i].get());
            double mixLevel = smartAux ? smartAux->getMixLevel() : 0.5;
            
            QStringList inputStreams;
            inputStreams << mainChainInput;
            
            QStringList weights;
            weights << "1.0";
            
            for (auto it = sidechainOutputs.constBegin(); it != sidechainOutputs.constEnd(); ++it) {
                inputStreams << it.value();
                weights << QString::number(mixLevel);
            }
            
            QString outputLabel = isLastFilter ? "[out]" : QString("[%1]").arg(getFilterHexLabel(filterId));
            
            QString durationStr = "longest";
            if (smartAux) {
                switch (smartAux->getDurationMode()) {
                    case SmartAuxReturn::DurationMode::Longest: durationStr = "longest"; break;
                    case SmartAuxReturn::DurationMode::Shortest: durationStr = "shortest"; break;
                    case SmartAuxReturn::DurationMode::First: durationStr = "first"; break;
                }
            }
            
            double dropoutTransition = smartAux ? smartAux->getDropoutTransition() : 2.0;
            bool normalize = smartAux ? smartAux->getNormalize() : true;
            
            QString amixStr = inputStreams.join("") + 
                             QString("amix=inputs=%1:weights=%2:duration=%3:dropout_transition=%4:normalize=%5")
                                 .arg(inputStreams.size())
                                 .arg(weights.join(" "))
                                 .arg(durationStr)
                                 .arg(dropoutTransition)
                                 .arg(normalize ? 1 : 0) +
                             outputLabel;
            
            filterStrs.append(amixStr);
            mainChainInput = outputLabel;
            currentAudioInput = nullptr;
            sidechainOutputs.clear();
        }
        else if (isMultiInputFilter) {
            // Check if filter handles its own complex internal routing
            // (e.g., AFIR with amix/apad splits main input internally)
            if (filters[i]->handlesOwnInputRouting()) {
                // Filter manages its own input labels internally
                // Just handle mainChainInput and sidechain label substitution
                
                // Check if starts with [0:a] pattern
                static QRegularExpression startsWithMainInput(R"(^\[0:a\])");
                if (startsWithMainInput.match(filterStr).hasMatch()) {
                    // Replace [0:a] with actual mainChainInput
                    filterStr.replace(QRegularExpression(R"(^\[0:a\])"), mainChainInput);
                } else if (!filterStr.startsWith('[')) {
                    // No input label at start - prepend mainChainInput
                    filterStr = mainChainInput + filterStr;
                }
                
                // Replace sidechain references [N:a] with processed outputs if they exist
                for (auto it = sidechainOutputs.constBegin(); it != sidechainOutputs.constEnd(); ++it) {
                    int idx = it.key();
                    QString rawLabel = QString("[%1:a]").arg(idx);
                    QString processedLabel = it.value();
                    if (processedLabel != rawLabel) {
                        filterStr.replace(rawLabel, processedLabel);
                    }
                }
            } else {
                // Original logic for simple multi-input filters like sidechaincompress
                // Multi-input filters like amix/amerge may self-generate input labels
                // (e.g. "[0:a][1:a]amix=...") that don't reflect actual stream routing.
                // Detect and replace them with the correct labels from chain state.
                static QRegularExpression leadingInputLabels(R"(^(\[\d+:a\])+)");
                QRegularExpressionMatch inputLabelMatch = leadingInputLabels.match(filterStr);
                
                if (inputLabelMatch.hasMatch()) {
                    // Filter generated its own input labels — strip and rebuild
                    filterStr = filterStr.mid(inputLabelMatch.capturedLength());
                    
                    // Rebuild: main chain input first, then sidechain outputs in index order
                    QString inputPrefix = mainChainInput;
                    QList<int> scKeys = sidechainOutputs.keys();
                    std::sort(scKeys.begin(), scKeys.end());
                    for (int key : scKeys) {
                        inputPrefix += sidechainOutputs[key];
                    }
                    filterStr = inputPrefix + filterStr;
                } else {
                    // Filter didn't generate input labels (e.g. acrossfade, sidechaincompress)
                    // Use replacement logic for any sidechain labels that were processed
                    for (auto it = sidechainOutputs.constBegin(); it != sidechainOutputs.constEnd(); ++it) {
                        int idx = it.key();
                        QString rawLabel = QString("[%1:a]").arg(idx);
                        QString processedLabel = it.value();
                        if (processedLabel != rawLabel) {
                            filterStr.replace(rawLabel, processedLabel);
                        }
                    }
                    filterStr.replace("[0:a]", mainChainInput);
                }
            }
            
            bool usesCustomOutput = filters[i]->usesCustomOutputStream();
            QString outputLabel;
            if (usesCustomOutput) {
                outputLabel = QString("[%1]").arg(getFilterHexLabel(filterId));
            } else {
                outputLabel = isLastFilter ? "[out]" : QString("[%1]").arg(getFilterHexLabel(filterId));
            }
            
            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(filters[i].get())) {
                manualLabels = customFilter->getManualOutputLabels();
            }
            
            if (!manualLabels) {
                int lastParam = qMax(filterStr.lastIndexOf('='), filterStr.lastIndexOf(':'));
                if (lastParam != -1) {
                    int outputLabelStart = filterStr.indexOf('[', lastParam);
                    if (outputLabelStart != -1) {
                        filterStr = filterStr.left(outputLabelStart);
                    }
                }
                filterStr += outputLabel;
                if (!usesCustomOutput) {
                    mainChainInput = outputLabel;
                }
            }
            
            // Clear consumed sidechain state — prevents stale entries from
            // leaking to subsequent multi-input filters in the chain
            currentAudioInput = nullptr;
            sidechainOutputs.clear();
            filterStrs.append(filterStr);
        }
        else if (isAnalysisTwoInput) {
            // Analysis two-input filter: [mainChain][sidechain]filterName[output]
            // Takes two inputs, passes first through unchanged.

            QString sidechainLabel;
            if (!sidechainOutputs.isEmpty()) {
                QList<int> scKeys = sidechainOutputs.keys();
                std::sort(scKeys.begin(), scKeys.end());
                sidechainLabel = sidechainOutputs[scKeys.last()];
            } else {
                sidechainLabel = "[1:a]";
            }

            filterStr = mainChainInput + sidechainLabel + filterStr;

            QString outputLabel = isLastFilter ? "[out]" : QString("[%1]").arg(getFilterHexLabel(filterId));
            filterStr += outputLabel;

            mainChainInput = outputLabel;

            // Clear sidechain state if it was processed (insert-mode filters
            // produced a unique label that can only be consumed once).
            // Raw [N:a] labels can be reused by FFmpeg, so keep them for
            // stacked analysis filters sharing the same unprocessed sidechain.
            bool sidechainWasProcessed = false;
            for (auto it = sidechainOutputs.constBegin(); it != sidechainOutputs.constEnd(); ++it) {
                QString rawLabel = QString("[%1:a]").arg(it.key());
                if (it.value() != rawLabel) {
                    sidechainWasProcessed = true;
                    break;
                }
            }
            if (sidechainWasProcessed) {
                currentAudioInput = nullptr;
                sidechainOutputs.clear();
            }

            filterStrs.append(filterStr);
        }
        else {
            bool hasInputLabels = filterStr.contains("[0:a]") || filterStr.contains(QRegularExpression("^\\["));

            if (hasInputLabels) {
                filterStr.replace("[0:a]", mainChainInput);
            } else {
                filterStr = mainChainInput + filterStr;
            }

            bool usesCustomOutput = filters[i]->usesCustomOutputStream();
            QString outputLabel;
            if (usesCustomOutput) {
                outputLabel = QString("[%1]").arg(getFilterHexLabel(filterId));
            } else {
                outputLabel = isLastFilter ? "[out]" : QString("[%1]").arg(getFilterHexLabel(filterId));
            }

            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(filters[i].get())) {
                manualLabels = customFilter->getManualOutputLabels();
            }

            if (!manualLabels) {
                int lastParam = qMax(filterStr.lastIndexOf('='), filterStr.lastIndexOf(':'));
                if (lastParam != -1) {
                    int outputLabelStart = filterStr.indexOf('[', lastParam);
                    if (outputLabelStart != -1) {
                        filterStr = filterStr.left(outputLabelStart);
                    }
                }
                filterStr += outputLabel;
                if (!usesCustomOutput && !isLastFilter) {
                    mainChainInput = outputLabel;
                }
            }

            filterStrs.append(filterStr);
        }
    }

    return filterStrs.join(";");
}

// ========== DAG Infrastructure (Phase A) ==========

bool FilterChain::isLinearChain(const QList<int>& mutedPositions) const {
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) continue;

        auto* f = filters[i].get();

        if (dynamic_cast<AudioInputFilter*>(f))     return false;
        if (dynamic_cast<MultiOutputFilter*>(f))     return false;
        if (dynamic_cast<AuxOutputFilter*>(f))       return false;
        if (dynamic_cast<FFShowwavespic*>(f))        return false;
        if (dynamic_cast<FFAnullsink*>(f))           return false;
        if (dynamic_cast<SmartAuxReturn*>(f))        return false;
        if (dynamic_cast<FFAfir*>(f))                return false;
        if (dynamic_cast<FFSidechaincompress*>(f))   return false;
        if (dynamic_cast<FFSidechaingate*>(f))       return false;
        if (dynamic_cast<FFAcrossfade*>(f))          return false;
        if (dynamic_cast<FFAmerge*>(f))              return false;
        if (dynamic_cast<FFAmix*>(f))                return false;
        if (dynamic_cast<FFAxcorrelate*>(f))         return false;
        if (dynamic_cast<FFJoin*>(f))                return false;
        if (f->isAnalysisTwoInputFilter())           return false;
        if (f->usesCustomOutputStream())             return false;
    }
    return true;
}

QString FilterChain::buildFilterFlagsDAG(const QList<int>& mutedPositions) const {
    DAG::FilterGraph graph;
    graph.buildLinearChain(filters);

    QMap<int, int> nodeIdToPos;
    for (size_t i = 0; i < filters.size(); ++i) {
        nodeIdToPos[filters[i]->getFilterId()] = static_cast<int>(i);
    }

    auto hexFunc = [this](int filterId) -> QString {
        return getFilterHexLabel(filterId);
    };

    return DAG::DAGCommandBuilder::buildFilterFlags(
        graph, mutedPositions, nodeIdToPos, hexFunc);
}

QString FilterChain::buildAuxOutputPath(const QString& inputFile, AuxOutputFilter* auxOut, int auxIndex) const {
    if (!auxOut) return "";
    
    // Extract base filename without extension
    QFileInfo fileInfo(inputFile);
    QString baseName = fileInfo.completeBaseName();  // filename without extension
    QString inputDir = fileInfo.absolutePath();
    
    // Determine output directory
    QString outputDir;
    if (auxOut->getUseCustomOutputFolder() && !auxOut->getOutputFolder().isEmpty()) {
        // Use aux output's custom folder
        outputDir = auxOut->getOutputFolder();
    } else if (outputFilter && !outputFilter->getOutputFolder().isEmpty()) {
        // Use main OUTPUT filter's folder
        outputDir = outputFilter->getOutputFolder();
    } else {
        // Fallback to input directory
        outputDir = inputDir;
    }
    
    // Get suffix from aux output
    QString suffix = auxOut->getFilenameSuffix();
    if (suffix.isEmpty()) {
        suffix = QString("_aux%1").arg(auxIndex);
    }
    
    // Get file extension from aux output
    QString extension = auxOut->getFileExtension();
    
    // Build full path
    return QString("%1/%2%3.%4").arg(outputDir).arg(baseName).arg(suffix).arg(extension);
}

QString FilterChain::buildWaveformOutputPath(const QString& inputFile, FFShowwavespic* waveform) const {
    if (!waveform) return "";
    
    // Extract base filename without extension
    QFileInfo fileInfo(inputFile);
    QString baseName = fileInfo.completeBaseName();
    QString inputDir = fileInfo.absolutePath();
    
    // Determine output directory
    QString outputDir;
    if (waveform->getUseCustomOutputFolder() && !waveform->getOutputFolder().isEmpty()) {
        // Use waveform's custom folder
        outputDir = waveform->getOutputFolder();
    } else if (outputFilter && !outputFilter->getOutputFolder().isEmpty()) {
        // Use main OUTPUT filter's folder
        outputDir = outputFilter->getOutputFolder();
    } else {
        // Fallback to input directory
        outputDir = inputDir;
    }
    
    // Get suffix from waveform filter (includes hex ID: _XXXX-waveform)
    QString suffix = waveform->getFilenameSuffix();
    
    // Get file extension (always png)
    QString extension = waveform->getFileExtension();
    
    // Build full path: <dir>/<basename>_<hexID>-waveform.png
    return QString("%1/%2%3.%4").arg(outputDir).arg(baseName).arg(suffix).arg(extension);
}

bool FilterChain::endsWithSinkFilter(const QList<int>& mutedPositions) const {
    // Find the last non-muted, non-branch filter in the main chain
    // Branch filters: AuxOutputFilter, FFShowwavespic
    // These don't count as "ending" the chain since they branch off
    
    for (int i = static_cast<int>(filters.size()) - 2; i >= 1; --i) {
        if (mutedPositions.contains(i)) {
            continue;
        }
        
        auto* filter = filters[i].get();
        
        // Skip branch output filters - they don't terminate the main chain
        if (dynamic_cast<AuxOutputFilter*>(filter) || 
            dynamic_cast<FFShowwavespic*>(filter)) {
            continue;
        }
        
        // Skip AudioInputFilter - doesn't produce output
        if (dynamic_cast<AudioInputFilter*>(filter)) {
            continue;
        }
        
        // Check if this is a sink filter
        if (dynamic_cast<FFAnullsink*>(filter)) {
            return true;
        }
        
        // Found a regular filter - chain doesn't end with sink
        return false;
    }
    
    return false;
}

QString FilterChain::buildCompleteCommand(const QString& inputFile,
                                         const QStringList& sidechainFiles,
                                         const QString& outputFile,
                                         const LogSettings& logSettings) const {
    QList<int> emptyMuted;
    return buildCompleteCommand(inputFile, sidechainFiles, outputFile, emptyMuted, logSettings);
}

QString FilterChain::buildCompleteCommand(const QString& inputFile,
                                         const QStringList& sidechainFiles,
                                         const QString& outputFile,
                                         const QList<int>& mutedPositions,
                                         const LogSettings& logSettings) const {
    QString command = logSettings.buildFlags() + " ";
    
    if (inputFilter) {
        QString inputFlags = inputFilter->buildFFmpegFlags();
        inputFlags.replace("{INPUT}", inputFile);
        command += inputFlags + " ";
    }
    
    int audioInputCount = getRequiredAudioInputCount();
    for (int i = 0; i < audioInputCount; ++i) {
        if (i < sidechainFiles.size() && !sidechainFiles[i].isEmpty()) {
            command += QString("-i \"%1\" ").arg(sidechainFiles[i]);
        } else {
            qDebug() << "WARNING: AudioInput filter" << (i+1) << "has no files, using silent input";
            command += "-f lavfi -i anullsrc=duration=1 ";
        }
    }
    
    // Check if chain ends with a sink (no audio output)
    bool chainEndsWithSink = endsWithSinkFilter(mutedPositions);
    
    // Check for video passthrough mode
    bool videoPassthrough = outputFilter && outputFilter->isVideoPassthrough();
    
    QString filterFlags = mutedPositions.isEmpty() ? buildFilterFlags() : buildFilterFlags(mutedPositions);
    if (!filterFlags.isEmpty()) {
        command += "-filter_complex \"" + filterFlags + "\" ";
        
        // Video passthrough: map video/subtitle streams before audio
        if (videoPassthrough) {
            command += outputFilter->buildOutputMappingFlags() + " ";
        }
        
        // Only add audio output mapping if chain doesn't end with sink
        if (!chainEndsWithSink) {
            command += "-map \"[out]\" ";
        }
    } else if (videoPassthrough) {
        // No filter graph but video passthrough — still need video mapping
        command += outputFilter->buildOutputMappingFlags() + " ";
    }
    
    // Only add audio output file if chain doesn't end with sink
    if (!chainEndsWithSink) {
        if (outputFilter) command += outputFilter->buildFFmpegFlags() + " ";
        command += QString("\"%1\"").arg(outputFile);
    }
    qDebug() << "Filter count:" << filters.size();
    qDebug() << "Muted positions:" << mutedPositions;
    
    int auxIndex = 1;
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        qDebug() << "Checking filter at position" << i << "- Type:" << filters[i]->filterType();
        
        if (mutedPositions.contains(static_cast<int>(i))) {
            qDebug() << "  Filter is muted, skipping";
            continue;
        }
        
        if (auto* auxOut = dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);
            
            qDebug() << "  Found AuxOutput filter! ID:" << filterId << "Label:" << auxBranchLabel;
            
            // Build output file path
            QString auxOutputPath = buildAuxOutputPath(inputFile, auxOut, auxIndex);
            qDebug() << "  Aux output path:" << auxOutputPath;
            
            QString codecFlags = auxOut->getCodecFlags();
            qDebug() << "  Codec flags:" << codecFlags;
            
            // Append output mapping
            command += QString(" -map \"%1\" %2 \"%3\"")
                .arg(auxBranchLabel)
                .arg(codecFlags)
                .arg(auxOutputPath);
            
            qDebug() << "  Added aux mapping to command";
            
            auxIndex++;
        }
    }
    
    // ========== APPEND WAVEFORM IMAGE OUTPUT MAPPINGS ==========
    qDebug() << "Building waveform output mappings...";
    
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
            continue;
        }
        
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);
            
            qDebug() << "  Found Waveform filter! ID:" << filterId << "Label:" << waveformBranchLabel;
            
            // Build output file path
            QString waveformOutputPath = buildWaveformOutputPath(inputFile, waveform);
            qDebug() << "  Waveform output path:" << waveformOutputPath;
            
            QString videoFlags = waveform->getVideoCodecFlags();
            qDebug() << "  Video codec flags:" << videoFlags;
            
            // Append video output mapping
            command += QString(" -map \"%1\" %2 \"%3\"")
                .arg(waveformBranchLabel)
                .arg(videoFlags)
                .arg(waveformOutputPath);
            
            qDebug() << "  Added waveform mapping to command";
        }
    }
    
    qDebug() << "Final command length:" << command.length();
    
    return command.trimmed();
}

// ========== Preview Command Generation ==========

QString FilterChain::buildPreviewCommand(const QString& inputFile,
                                        const QStringList& sidechainFiles,
                                        const QString& outputFile,
                                        const QList<int>& mutedPositions,
                                        const LogSettings& logSettings) const {
    // Preview uses hardcoded safe flags by default.
    // When preview logging is enabled, user's LogSettings are passed in
    // so analysis filters can produce output to the log file.
    QString command = "-y " + logSettings.buildFlags() + " ";
    
    if (inputFilter) {
        QString inputFlags = inputFilter->buildFFmpegFlags();
        inputFlags.replace("{INPUT}", inputFile);
        command += inputFlags + " ";
    }
    
    int audioInputCount = getRequiredAudioInputCount();
    for (int i = 0; i < audioInputCount; ++i) {
        if (i < sidechainFiles.size() && !sidechainFiles[i].isEmpty()) {
            command += QString("-i \"%1\" ").arg(sidechainFiles[i]);
        } else {
            command += "-f lavfi -i anullsrc=duration=1 ";
        }
    }
    
    // Check if chain ends with a sink (no audio output)
    bool chainEndsWithSink = endsWithSinkFilter(mutedPositions);
    
    // NOTE: Video passthrough is intentionally skipped for preview.
    // Preview renders audio-only to a temp WAV for playback — no video mapping.
    
    QString filterFlags = mutedPositions.isEmpty() ? buildFilterFlags() : buildFilterFlags(mutedPositions);
    if (!filterFlags.isEmpty()) {
        command += "-filter_complex \"" + filterFlags + "\" ";
        
        // Only add audio output mapping if chain doesn't end with sink
        if (!chainEndsWithSink) {
            command += "-map \"[out]\" ";
        }
    }
    
    // Only add main audio output if chain doesn't end with sink
    if (!chainEndsWithSink) {
        if (outputFilter) command += outputFilter->buildFFmpegFlags() + " ";
        command += QString("\"%1\"").arg(outputFile);
    }
    
    // ========== DISCARD AUX OUTPUT STREAMS ==========
    // For preview, we need to consume aux output streams but not write files
    // Use FFmpeg's null muxer: -map [label] -f null -
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
            continue;
        }
        
        if (auto* auxOut = dynamic_cast<AuxOutputFilter*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);
            
            // Discard aux stream to null muxer (use "-" which FFmpeg treats specially)
            command += QString(" -map %1 -f null -").arg(auxBranchLabel);
        }
    }
    
    // ========== DISCARD WAVEFORM OUTPUT STREAMS ==========
    // For preview, we need to consume waveform output streams but not write files
    for (size_t i = 1; i < filters.size() - 1; ++i) {
        if (mutedPositions.contains(static_cast<int>(i))) {
            continue;
        }
        
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(filters[i].get())) {
            int filterId = filters[i]->getFilterId();
            QString filterIdLabel = getFilterHexLabel(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);
            
            // Discard waveform stream to null muxer
            command += QString(" -map %1 -f null -").arg(waveformBranchLabel);
        }
    }
    
    return command.trimmed();
}

QString FilterChain::formatCommandForDisplay(const QString& command, const QString& ffmpegPath) {
    // Just prepend ffmpeg path - let the Command View Window handle formatting
    return ffmpegPath + " " + command;
}

QStringList FilterChain::parseCommandToArgs(const QString& command) {
    // Parse command string into QStringList for QProcess
    // This handles quoted strings properly
    QStringList args;
    QString currentArg;
    bool inQuotes = false;
    
    for (int i = 0; i < command.length(); ++i) {
        QChar c = command[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
            // Don't include the quotes in the argument
        } else if (c == ' ' && !inQuotes) {
            if (!currentArg.isEmpty()) {
                args << currentArg;
                currentArg.clear();
            }
        } else {
            currentArg += c;
        }
    }
    
    // Add final argument if any
    if (!currentArg.isEmpty()) {
        args << currentArg;
    }
    
    return args;
}

// ========== Serialization ==========

void FilterChain::saveToJSON(const QString& filepath) const {
    QJsonObject json = toJSON();
    QJsonDocument doc(json);
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

bool FilterChain::loadFromJSON(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return fromJSON(doc.object());
}

QJsonObject FilterChain::toJSON() const {
    QJsonObject root;
    QJsonArray filtersArray;
    
    for (const auto& filter : filters) {
        if (!filter) continue;
        QJsonObject filterObj;
        filter->toJSON(filterObj);
        filterObj["filter_id"] = filter->getFilterId();
        filterObj["use_custom_output"] = filter->usesCustomOutputStream();
        filtersArray.append(filterObj);
    }
    
    root["filters"] = filtersArray;
    root["next_filter_id"] = m_nextFilterId;
    return root;
}

bool FilterChain::fromJSON(const QJsonObject& json) {
    m_nextFilterId = json.contains("next_filter_id") ? json.value("next_filter_id").toInt() : 0;
    
    QJsonArray filtersArray = json["filters"].toArray();
    
    // Remove all middle filters
    while (filters.size() > 2) {
        filters.erase(filters.begin() + 1);
    }
    
    for (int i = 0; i < filtersArray.size(); ++i) {
        QJsonObject filterObj = filtersArray[i].toObject();
        QString type = filterObj["type"].toString();
        auto filter = createFilterByType(type);
        
        if (filter) {
            if (filterObj.contains("filter_id")) {
                filter->setFilterId(filterObj["filter_id"].toInt());
            }
            if (filterObj.contains("use_custom_output")) {
                filter->setCustomOutputStream(filterObj["use_custom_output"].toBool());
            }
            
            filter->fromJSON(filterObj);
            
            // For MultiOutputFilter, deserialize sub-chains
            if (auto* multiOutput = dynamic_cast<MultiOutputFilter*>(filter.get())) {
                if (filterObj.contains("stream_chains")) {
                    QJsonArray streamsArray = filterObj["stream_chains"].toArray();
                    
                    for (int streamIdx = 0; streamIdx < streamsArray.size() && streamIdx < MultiOutputFilter::MAX_STREAMS - 1; ++streamIdx) {
                        QJsonArray chainArray = streamsArray[streamIdx].toArray();
                        int actualStreamIdx = streamIdx + 1;
                        
                        for (int filterIdx = 0; filterIdx < chainArray.size(); ++filterIdx) {
                            QJsonObject subFilterObj = chainArray[filterIdx].toObject();
                            QString subType = subFilterObj["type"].toString();
                            auto subFilter = createFilterByType(subType);
                            
                            if (subFilter) {
                                if (subFilterObj.contains("filter_id")) {
                                    subFilter->setFilterId(subFilterObj["filter_id"].toInt());
                                }
                                if (subFilterObj.contains("use_custom_output")) {
                                    subFilter->setCustomOutputStream(subFilterObj["use_custom_output"].toBool());
                                }
                                subFilter->fromJSON(subFilterObj);
                                multiOutput->addFilterToStream(actualStreamIdx, subFilter);
                            }
                        }
                    }
                }
            }
            
            if (type != "input" && type != "output") {
                addFilter(filter);
            }
        }
    }
    
    // Ensure m_nextFilterId is higher than any loaded filter ID
    for (const auto& filter : filters) {
        if (filter && filter->getFilterId() >= m_nextFilterId) {
            m_nextFilterId = filter->getFilterId() + 1;
        }
        // Also check sub-chains
        if (auto* multiOutput = dynamic_cast<MultiOutputFilter*>(filter.get())) {
            for (int s = 1; s < MultiOutputFilter::MAX_STREAMS; ++s) {
                for (const auto& subFilter : multiOutput->getSubChain(s)) {
                    if (subFilter && subFilter->getFilterId() >= m_nextFilterId) {
                        m_nextFilterId = subFilter->getFilterId() + 1;
                    }
                }
            }
        }
    }
    
    return true;
}

std::shared_ptr<BaseFilter> FilterChain::createFilterByType(const QString& type) {
    if (type == "input") {
        return std::make_shared<InputFilter>();
    } else if (type == "output") {
        return std::make_shared<OutputFilter>();
    } else if (type == "audio-input") {
        return std::make_shared<AudioInputFilter>();
    } else if (type == "ff-volume") {
        return std::make_shared<FFVolume>();
    } else if (type == "ff-showwavespic") {
        return std::make_shared<FFShowwavespic>();
    } else if (type == "ff-afir") {
        return std::make_shared<FFAfir>();
    } else if (type == "ff-amerge") {
        return std::make_shared<FFAmerge>();
    } else if (type == "ff-sidechaincompress") {
        return std::make_shared<FFSidechaincompress>();
    } else if (type == "ff-sidechaingate") {
        return std::make_shared<FFSidechaingate>();
    } else if (type == "ff-amix") {
        return std::make_shared<FFAmix>();
    } else if (type == "ff-acrossfade") {
        return std::make_shared<FFAcrossfade>();
    } else if (type == "asplit") {
        return std::make_shared<AsplitFilter>();
    } else if (type == "SmartAuxReturn") {
        return std::make_shared<SmartAuxReturn>();
    } else if (type == "aux-output") {
        return std::make_shared<AuxOutputFilter>();
    } else if (type == "custom") {
        return std::make_shared<CustomFFmpegFilter>();
    } else if (type == "ff-adelay") {
        return std::make_shared<FFAdelay>();
    } else if (type == "ff-aecho") {
        return std::make_shared<FFAecho>();
    } else if (type == "ff-atempo") {
        return std::make_shared<FFAtempo>();
    } else if (type == "ff-atrim") {
        return std::make_shared<FFAtrim>();
    } else if (type == "ff-afade") {
        return std::make_shared<FFAfade>();
    } else if (type == "ff-aloop") {
        return std::make_shared<FFAloop>();
    } else if (type == "ff-areverse") {
        return std::make_shared<FFAreverse>();
    } else if (type == "ff-apad") {
        return std::make_shared<FFApad>();
    } else if (type == "ff-asetpts") {
        return std::make_shared<FFAsetpts>();
    } else if (type == "ff-aformat") {
        return std::make_shared<FFAformat>();
    } else if (type == "ff-aresample") {
        return std::make_shared<FFAresample>();
    } else if (type == "ff-asetrate") {
        return std::make_shared<FFAsetrate>();
    } else if (type == "ff-dcshift") {
        return std::make_shared<FFDcshift>();
    } else if (type == "ff-anull") {
        return std::make_shared<FFAnull>();
    } else if (type == "ff-silenceremove") {
        return std::make_shared<FFSilenceremove>();
    } else if (type == "ff-acompressor") {
        return std::make_shared<FFAcompressor>();
    } else if (type == "ff-alimiter") {
        return std::make_shared<FFAlimiter>();
    } else if (type == "ff-agate") {
        return std::make_shared<FFAgate>();
    } else if (type == "ff-dynaudnorm") {
        return std::make_shared<FFDynaudnorm>();
    } else if (type == "ff-compand") {
        return std::make_shared<FFCompand>();
    } else if (type == "ff-mcompand") {
        return std::make_shared<FFMcompand>();
    } else if (type == "ff-apsyclip") {
        return std::make_shared<FFApsyclip>();
    } else if (type == "ff-asoftclip") {
        return std::make_shared<FFAsoftclip>();
    } else if (type == "ff-chorus") {
        return std::make_shared<FFChorus>();
    } else if (type == "ff-flanger") {
        return std::make_shared<FFFlanger>();
    } else if (type == "ff-aphaser") {
        return std::make_shared<FFAphaser>();
    } else if (type == "ff-tremolo") {
        return std::make_shared<FFTremolo>();
    } else if (type == "ff-vibrato") {
        return std::make_shared<FFVibrato>();
    } else if (type == "ff-afreqshift") {
        return std::make_shared<FFAfreqshift>();
    } else if (type == "ff-aphaseshift") {
        return std::make_shared<FFAphaseshift>();
    } else if (type == "ff-highpass") {
        return std::make_shared<FFHighpass>();
    } else if (type == "ff-lowpass") {
        return std::make_shared<FFLowpass>();
    } else if (type == "ff-bandpass") {
        return std::make_shared<FFBandpass>();
    } else if (type == "ff-bandreject") {
        return std::make_shared<FFBandreject>();
    } else if (type == "ff-equalizer") {
        return std::make_shared<FFEqualizer>();
    } else if (type == "ff-bass") {
        return std::make_shared<FFBass>();
    } else if (type == "ff-treble") {
        return std::make_shared<FFTreble>();
    } else if (type == "ff-highshelf") {
        return std::make_shared<FFHighshelf>();
    } else if (type == "ff-lowshelf") {
        return std::make_shared<FFLowshelf>();
    } else if (type == "ff-tiltshelf") {
        return std::make_shared<FFTiltshelf>();
    } else if (type == "ff-allpass") {
        return std::make_shared<FFAllpass>();
    } else if (type == "ff-crossfeed") {
        return std::make_shared<FFCrossfeed>();
    } else if (type == "ff-extrastereo") {
        return std::make_shared<FFExtrastereo>();
    } else if (type == "ff-stereowiden") {
        return std::make_shared<FFStereowiden>();
    } else if (type == "ff-earwax") {
        return std::make_shared<FFEarwax>();
    } else if (type == "ff-haas") {
        return std::make_shared<FFHaas>();
    } else if (type == "ff-stereotools") {
        return std::make_shared<FFStereotools>();
    } else if (type == "ff-aexciter") {
        return std::make_shared<FFAexciter>();
    } else if (type == "ff-crystalizer") {
        return std::make_shared<FFCrystalizer>();
    } else if (type == "ff-asubboost") {
        return std::make_shared<FFAsubboost>();
    } else if (type == "ff-virtualbass") {
        return std::make_shared<FFVirtualbass>();
    } else if (type == "ff-acrusher") {
        return std::make_shared<FFAcrusher>();
    } else if (type == "ff-apulsator") {
        return std::make_shared<FFApulsator>();
    } else if (type == "ff-afftdn") {
        return std::make_shared<FFAfftdn>();
    } else if (type == "ff-adeclick") {
        return std::make_shared<FFAdeclick>();
    } else if (type == "ff-adeclip") {
        return std::make_shared<FFAdeclip>();
    } else if (type == "ff-deesser") {
        return std::make_shared<FFDeesser>();
    } else if (type == "ff-adrc") {
        return std::make_shared<FFAdrc>();
    } else if (type == "ff-adynamicequalizer") {
        return std::make_shared<FFAdynamicequalizer>();
    } else if (type == "ff-bs2b") {
        return std::make_shared<FFBs2b>();
    } else if (type == "ff-loudnorm") {
        return std::make_shared<FFLoudnorm>();
    } else if (type == "ff-speechnorm") {
        return std::make_shared<FFSpeechnorm>();
    } else if (type == "ff-dialoguenhance") {
        return std::make_shared<FFDialoguenhance>();
    } else if (type == "ff-acontrast") {
        return std::make_shared<FFAcontrast>();
    } else if (type == "ff-adecorrelate") {
        return std::make_shared<FFAdecorrelate>();
    } else if (type == "ff-atilt") {
        return std::make_shared<FFAtilt>();
    } else if (type == "ff-asubcut") {
        return std::make_shared<FFAsubcut>();
    } else if (type == "ff-asupercut") {
        return std::make_shared<FFAsupercut>();
    } else if (type == "ff-asuperpass") {
        return std::make_shared<FFAsuperpass>();
    } else if (type == "ff-asuperstop") {
        return std::make_shared<FFAsuperstop>();
    } else if (type == "ff-biquad") {
        return std::make_shared<FFBiquad>();
    } else if (type == "ff-compensationdelay") {
        return std::make_shared<FFCompensationdelay>();
    } else if (type == "ff-astats") {
        return std::make_shared<FFAstats>();
    } else if (type == "ff-channelmap") {
        return std::make_shared<FFChannelmap>();
    } else if (type == "ff-channelsplit") {
        return std::make_shared<FFChannelsplit>();
    } else if (type == "ff-anequalizer") {
        return std::make_shared<FFAnequalizer>();
    } else if (type == "ff-acopy") {
        return std::make_shared<FFAcopy>();
    } else if (type == "ff-adenorm") {
        return std::make_shared<FFAdenorm>();
    } else if (type == "ff-volumedetect") {
        return std::make_shared<FFVolumedetect>();
    } else if (type == "ff-drmeter") {
        return std::make_shared<FFDrmeter>();
    } else if (type == "ff-silencedetect") {
        return std::make_shared<FFSilencedetect>();
    } else if (type == "ff-replaygain") {
        return std::make_shared<FFReplaygain>();
    } else if (type == "ff-aemphasis") {
        return std::make_shared<FFAemphasis>();
    } else if (type == "ff-hdcd") {
        return std::make_shared<FFHdcd>();
    } else if (type == "ff-aderivative") {
        return std::make_shared<FFAderivative>();
    } else if (type == "ff-aintegral") {
        return std::make_shared<FFAintegral>();
    } else if (type == "ff-adynamicsmooth") {
        return std::make_shared<FFAdynamicsmooth>();
    } else if (type == "ff-ashowinfo") {
        return std::make_shared<FFAshowinfo>();
    } else if (type == "ff-acue") {
        return std::make_shared<FFAcue>();
    } else if (type == "ff-pan") {
        return std::make_shared<FFPan>();
    } else if (type == "ff-join") {
        return std::make_shared<FFJoin>();
    } else if (type == "ff-surround") {
        return std::make_shared<FFSurround>();
    } else if (type == "ff-sofalizer") {
        return std::make_shared<FFSofalizer>();
    } else if (type == "ff-acrossover") {
        return std::make_shared<FFAcrossover>();
    } else if (type == "ff-superequalizer") {
        return std::make_shared<FFSuperequalizer>();
    } else if (type == "ff-firequalizer") {
        return std::make_shared<FFFirequalizer>();
    } else if (type == "ff-apsnr") {
        return std::make_shared<FFApsnr>();
    } else if (type == "ff-asdr") {
        return std::make_shared<FFAsdr>();
    } else if (type == "ff-asisdr") {
        return std::make_shared<FFAsisdr>();
    } else if (type == "ff-axcorrelate") {
        return std::make_shared<FFAxcorrelate>();
    } else if (type == "ff-rubberband") {
        return std::make_shared<FFRubberband>();
    } else if (type == "ff-anullsink") {
        return std::make_shared<FFAnullsink>();
    } else if (type == "channel-eq") {
    return std::make_shared<ChannelEqFilter>(); // PROTOTYPE
    } 
    return nullptr;
}

// ========== Audio Input Management ==========

void FilterChain::updateAudioInputIndices() {
    int currentIndex = 1;

    for (auto& filter : filters) {
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filter.get())) {
            audioInput->setInputIndex(currentIndex);
            currentIndex++;
        }
        // Also assign indices to AudioInputFilters inside sub-chains
        if (auto* multiOut = dynamic_cast<MultiOutputFilter*>(filter.get())) {
            for (int s = 1; s < MultiOutputFilter::MAX_STREAMS; s++) {
                for (auto& subFilter : multiOut->getSubChain(s)) {
                    if (auto* audioInput = dynamic_cast<AudioInputFilter*>(subFilter.get())) {
                        audioInput->setInputIndex(currentIndex);
                        currentIndex++;
                    }
                }
            }
        }
    }
}

void FilterChain::updateMultiInputFilterIndices() {
    qDebug() << "=== Updating Multi-Input Filter Indices ===";
    
    for (size_t i = 0; i < filters.size(); ++i) {
        auto& filter = filters[i];
        
        int audioInputsBefore = 0;
        for (size_t j = 0; j < i; ++j) {
            if (dynamic_cast<AudioInputFilter*>(filters[j].get())) {
                audioInputsBefore++;
            }
        }
        
        int sidechainIndex = (audioInputsBefore > 0) ? audioInputsBefore : 1;
        
        if (auto* afir = dynamic_cast<FFAfir*>(filter.get())) {
            afir->setSidechainInputIndex(sidechainIndex);
            qDebug() << "  Position" << i << "- AFIR: sidechainIndex =" << sidechainIndex;
        }
        else if (auto* sidechain = dynamic_cast<FFSidechaincompress*>(filter.get())) {
            sidechain->setSidechainInputIndex(sidechainIndex);
            qDebug() << "  Position" << i << "- sidechaincompress: sidechainIndex =" << sidechainIndex;
        }
        else if (auto* gate = dynamic_cast<FFSidechaingate*>(filter.get())) {
            gate->setSidechainInputIndex(sidechainIndex);
            qDebug() << "  Position" << i << "- sidechaingate: sidechainIndex =" << sidechainIndex;
        }
        else if (auto* crossfade = dynamic_cast<FFAcrossfade*>(filter.get())) {
            crossfade->setSidechainInputIndex(sidechainIndex);
            qDebug() << "  Position" << i << "- acrossfade: sidechainIndex =" << sidechainIndex;
        }
    }
    
    qDebug() << "===========================================";
}

int FilterChain::getRequiredAudioInputCount() const {
    int count = 0;
    for (const auto& filter : filters) {
        if (dynamic_cast<const AudioInputFilter*>(filter.get())) {
            count++;
        }
        // Also check sub-chains in MultiOutputFilters (e.g., AsplitFilter)
        if (auto* multiOut = dynamic_cast<const MultiOutputFilter*>(filter.get())) {
            for (int s = 1; s < MultiOutputFilter::MAX_STREAMS; s++) {
                for (const auto& subFilter : multiOut->getSubChain(s)) {
                    if (dynamic_cast<const AudioInputFilter*>(subFilter.get())) {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

std::vector<int> FilterChain::getAudioInputIndices() const {
    std::vector<int> indices;
    for (const auto& filter : filters) {
        if (const auto* audioInput = dynamic_cast<const AudioInputFilter*>(filter.get())) {
            indices.push_back(audioInput->getInputIndex());
        }
        // Also check sub-chains in MultiOutputFilters (e.g., AsplitFilter)
        if (auto* multiOut = dynamic_cast<const MultiOutputFilter*>(filter.get())) {
            for (int s = 1; s < MultiOutputFilter::MAX_STREAMS; s++) {
                for (const auto& subFilter : multiOut->getSubChain(s)) {
                    if (const auto* audioInput = dynamic_cast<const AudioInputFilter*>(subFilter.get())) {
                        indices.push_back(audioInput->getInputIndex());
                    }
                }
            }
        }
    }
    return indices;
}

std::vector<AudioInputFilter*> FilterChain::getAllAudioInputFilters() const {
    // Collect all AudioInputFilters, then sort by their input index
    std::vector<std::pair<int, AudioInputFilter*>> indexedFilters;
    
    for (const auto& filter : filters) {
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filter.get())) {
            indexedFilters.push_back({audioInput->getInputIndex(), audioInput});
        }
        // Also check sub-chains in MultiOutputFilters (e.g., AsplitFilter)
        if (auto* multiOut = dynamic_cast<MultiOutputFilter*>(filter.get())) {
            for (int s = 1; s < MultiOutputFilter::MAX_STREAMS; s++) {
                for (const auto& subFilter : multiOut->getSubChain(s)) {
                    if (auto* audioInput = dynamic_cast<AudioInputFilter*>(subFilter.get())) {
                        indexedFilters.push_back({audioInput->getInputIndex(), audioInput});
                    }
                }
            }
        }
    }
    
    // Sort by input index
    std::sort(indexedFilters.begin(), indexedFilters.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    
    // Extract just the pointers
    std::vector<AudioInputFilter*> result;
    result.reserve(indexedFilters.size());
    for (const auto& pair : indexedFilters) {
        result.push_back(pair.second);
    }
    return result;
}