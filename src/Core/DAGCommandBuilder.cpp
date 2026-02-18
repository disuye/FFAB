#include "DAGCommandBuilder.h"
#include "CustomFFmpegFilter.h"
#include "AudioInputFilter.h"
#include "AuxOutputFilter.h"
#include "SmartAuxReturn.h"
#include "MultiOutputFilter.h"
#include "ff-showwavespic.h"
#include "ff-anullsink.h"
#include "ff-afir.h"
#include "ff-sidechaincompress.h"
#include "ff-sidechaingate.h"
#include "ff-acrossfade.h"
#include "ff-amerge.h"
#include "ff-amix.h"
#include "ff-axcorrelate.h"
#include "ff-join.h"
#include "ff-asplit.h"
#include <QRegularExpression>
#include <QStringList>

namespace DAG {

// ================================================================
// Shared helpers (used by main loop and AsplitFilter sub-chain loop)
// ================================================================

// Classify multi-input filters
static bool isMultiInputFilter(BaseFilter* f) {
    return dynamic_cast<FFAfir*>(f) ||
           dynamic_cast<FFSidechaincompress*>(f) ||
           dynamic_cast<FFSidechaingate*>(f) ||
           dynamic_cast<FFAcrossfade*>(f) ||
           dynamic_cast<FFAmerge*>(f) ||
           dynamic_cast<FFAmix*>(f) ||
           dynamic_cast<FFAxcorrelate*>(f) ||
           dynamic_cast<FFJoin*>(f);
}

// Strip trailing output label [xxx] after last = or :
static void stripTrailingOutputLabel(QString& filterStr) {
    int lastParam = qMax(filterStr.lastIndexOf('='), filterStr.lastIndexOf(':'));
    if (lastParam != -1) {
        int outputLabelStart = filterStr.indexOf('[', lastParam);
        if (outputLabelStart != -1) {
            filterStr = filterStr.left(outputLabelStart);
        }
    }
}

QString DAGCommandBuilder::buildFilterFlags(
    const FilterGraph& graph,
    const QList<int>& mutedPositions,
    const QMap<int, int>& nodeIdToChainPosition,
    std::function<QString(int)> hexLabelFunc)
{
    const auto& nodes = graph.nodes();
    if (nodes.size() <= 2) return "";  // Only INPUT + OUTPUT

    auto topoOrder = graph.topologicalOrder();
    int firstNodeId = topoOrder.front();
    int lastNodeId  = topoOrder.back();

    // ================================================================
    // Helper: check if there are non-muted filters after currentIdx
    // that produce output (including aux, waveform, asplit).
    // Replicates FilterChain.cpp hasFiltersAfter lambda.
    // ================================================================
    auto hasFiltersAfter = [&](size_t currentIdx) -> bool {
        for (size_t j = currentIdx + 1; j < topoOrder.size(); ++j) {
            int nid = topoOrder[j];
            if (nid == lastNodeId) continue;

            int chainPos = nodeIdToChainPosition.value(nid, -1);
            if (mutedPositions.contains(chainPos)) continue;

            const FilterNode* n = graph.findNode(nid);
            if (!n) continue;
            BaseFilter* f = n->filter.get();

            if (dynamic_cast<AuxOutputFilter*>(f) ||
                dynamic_cast<FFShowwavespic*>(f) ||
                dynamic_cast<AsplitFilter*>(f)) {
                return true;
            }

            if (dynamic_cast<AudioInputFilter*>(f)) {
                continue;
            }

            if (!f->buildFFmpegFlags().isEmpty()) {
                return true;
            }
        }
        return false;
    };

    // ================================================================
    // State variables
    // ================================================================
    QStringList filterStrs;
    QString mainChainInput = "[0:a]";

    QMap<int, QString> sidechainOutputs;
    AudioInputFilter* currentAudioInput = nullptr;

    // ================================================================
    // Pre-processing pass: count aux outputs and waveforms
    // ================================================================
    int totalAuxOutputs = 0;
    int totalWaveforms = 0;
    int lastAuxOutputIdx = -1;
    int lastWaveformIdx = -1;
    bool hasFiltersAfterBranchOutputs = false;

    for (size_t i = 0; i < topoOrder.size(); ++i) {
        int nid = topoOrder[i];
        if (nid == firstNodeId || nid == lastNodeId) continue;

        int chainPos = nodeIdToChainPosition.value(nid, -1);
        if (mutedPositions.contains(chainPos)) continue;

        const FilterNode* n = graph.findNode(nid);
        if (!n) continue;

        if (dynamic_cast<AuxOutputFilter*>(n->filter.get())) {
            totalAuxOutputs++;
            lastAuxOutputIdx = static_cast<int>(i);
        }
        if (dynamic_cast<FFShowwavespic*>(n->filter.get())) {
            totalWaveforms++;
            lastWaveformIdx = static_cast<int>(i);
        }
    }

    int lastBranchOutputIdx = qMax(lastAuxOutputIdx, lastWaveformIdx);

    if (lastBranchOutputIdx >= 0) {
        for (size_t i = lastBranchOutputIdx + 1; i < topoOrder.size(); ++i) {
            int nid = topoOrder[i];
            if (nid == lastNodeId) continue;

            int chainPos = nodeIdToChainPosition.value(nid, -1);
            if (mutedPositions.contains(chainPos)) continue;

            const FilterNode* n = graph.findNode(nid);
            if (!n) continue;
            BaseFilter* f = n->filter.get();

            if ((!f->buildFFmpegFlags().isEmpty() &&
                 !dynamic_cast<AuxOutputFilter*>(f) &&
                 !dynamic_cast<FFShowwavespic*>(f)) ||
                dynamic_cast<AsplitFilter*>(f)) {
                hasFiltersAfterBranchOutputs = true;
                break;
            }
        }
    }

    int auxOutputsProcessed = 0;
    int waveformsProcessed = 0;

    // ================================================================
    // Main processing loop
    // ================================================================
    for (size_t i = 0; i < topoOrder.size(); ++i) {
        int nodeId = topoOrder[i];
        if (nodeId == firstNodeId || nodeId == lastNodeId) continue;

        int chainPos = nodeIdToChainPosition.value(nodeId, -1);
        if (mutedPositions.contains(chainPos)) {
            // Muted: invisible, don't clear sidechain state
            continue;
        }

        const FilterNode* node = graph.findNode(nodeId);
        if (!node) continue;
        BaseFilter* rawFilter = node->filter.get();

        // ============================================================
        // CASE 1: AuxOutputFilter — asplit branch
        // ============================================================
        if (auto* auxOutput = dynamic_cast<AuxOutputFilter*>(rawFilter)) {
            auxOutputsProcessed++;
            bool isLastBranchOutput = (auxOutputsProcessed == totalAuxOutputs &&
                                       waveformsProcessed == totalWaveforms);

            int filterId = rawFilter->getFilterId();
            QString filterIdLabel = hexLabelFunc(filterId);
            QString auxBranchLabel = QString("[%1-auxOut]").arg(filterIdLabel);

            QString mainContinueLabel;
            if (isLastBranchOutput && !hasFiltersAfterBranchOutputs) {
                mainContinueLabel = "[out]";
            } else {
                mainContinueLabel = QString("[%1]").arg(filterIdLabel);
            }

            QString asplitCmd = QString("%1asplit=2%2%3")
                .arg(mainChainInput)
                .arg(mainContinueLabel)
                .arg(auxBranchLabel);

            filterStrs.append(asplitCmd);
            mainChainInput = mainContinueLabel;
            continue;
        }

        // ============================================================
        // CASE 2: FFShowwavespic — asplit + showwavespic branch
        // ============================================================
        if (auto* waveform = dynamic_cast<FFShowwavespic*>(rawFilter)) {
            waveformsProcessed++;
            bool isLastBranchOutput = (auxOutputsProcessed == totalAuxOutputs &&
                                       waveformsProcessed == totalWaveforms);

            int filterId = rawFilter->getFilterId();
            QString filterIdLabel = hexLabelFunc(filterId);
            QString waveformBranchLabel = QString("[%1-waveform]").arg(filterIdLabel);

            QString mainContinueLabel;
            if (isLastBranchOutput && !hasFiltersAfterBranchOutputs) {
                mainContinueLabel = "[out]";
            } else {
                mainContinueLabel = QString("[%1]").arg(filterIdLabel);
            }

            QString waveformInputLabel = QString("[%1-wfin]").arg(filterIdLabel);
            QString asplitCmd = QString("%1asplit=2%2%3")
                .arg(mainChainInput)
                .arg(mainContinueLabel)
                .arg(waveformInputLabel);
            filterStrs.append(asplitCmd);

            QString waveformFilterCmd = QString("%1%2%3")
                .arg(waveformInputLabel)
                .arg(waveform->buildFFmpegFlags())
                .arg(waveformBranchLabel);
            filterStrs.append(waveformFilterCmd);

            mainChainInput = mainContinueLabel;
            continue;
        }

        // ============================================================
        // CASE 3: AsplitFilter — split into parallel streams with sub-chains
        // ============================================================
        if (auto* asplitFilter = dynamic_cast<AsplitFilter*>(rawFilter)) {
            int filterId = rawFilter->getFilterId();
            QString hexId = hexLabelFunc(filterId);
            int numSplits = asplitFilter->numSplits();
            int totalStreams = numSplits + 1;  // thru + parallel streams

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

            // Process each sub-chain, collect outputs for mixing
            QStringList mixInputs;
            QStringList mixWeights;

            // Thru (stream 0) goes directly to mix
            mixInputs << thruLabel;
            mixWeights << QString::number(asplitFilter->getStreamWeight(0), 'f', 4);

            for (int s = 1; s <= numSplits; s++) {
                QString currentInput_sub = streamLabels[s - 1];
                bool isStreamMuted = asplitFilter->isStreamMuted(s);

                if (isStreamMuted) {
                    // Muted stream: skip sub-chain, use raw stream with weight 0
                    mixInputs << currentInput_sub;
                    mixWeights << "0";
                } else {
                    const auto& subChain = asplitFilter->getSubChain(s);

                    if (subChain.empty()) {
                        // No processing on this stream, goes directly to mix
                        mixInputs << currentInput_sub;
                    } else {
                        // Process sub-chain filters with sidechain awareness
                        AudioInputFilter* subCurrentAudioInput = nullptr;
                        QMap<int, QString> subSidechainOutputs;

                        for (size_t j = 0; j < subChain.size(); j++) {
                            const auto& subFilter = subChain[j];

                            // AudioInputFilter — establishes sidechain context
                            if (auto* audioInput = dynamic_cast<AudioInputFilter*>(subFilter.get())) {
                                subCurrentAudioInput = audioInput;
                                int idx = audioInput->getInputIndex();
                                subSidechainOutputs[idx] = QString("[%1:a]").arg(idx);
                                continue;
                            }

                            QString subFilterStr = subFilter->buildFFmpegFlags();
                            if (subFilterStr.isEmpty()) continue;

                            int subFilterId = subFilter->getFilterId();
                            QString subHexId = hexLabelFunc(subFilterId);
                            QString subOutputLabel = QString("[%1_s%2_%3]")
                                .arg(hexId).arg(s).arg(subHexId);

                            bool subIsAnalysisTwoInput = subFilter->isAnalysisTwoInputFilter();
                            bool subIsMultiInput = isMultiInputFilter(subFilter.get());
                            bool subIsInsertMode = (subCurrentAudioInput != nullptr &&
                                                    !subIsAnalysisTwoInput &&
                                                    !subIsMultiInput);

                            if (subIsInsertMode) {
                                // INSERT MODE: wire filter onto the sidechain path
                                int idx = subCurrentAudioInput->getInputIndex();
                                QString inputLabel = subSidechainOutputs[idx];

                                bool hasInputLabels = subFilterStr.contains(QRegularExpression("^\\["));
                                if (!hasInputLabels) {
                                    subFilterStr = inputLabel + subFilterStr;
                                }

                                stripTrailingOutputLabel(subFilterStr);
                                subFilterStr += subOutputLabel;
                                subSidechainOutputs[idx] = subOutputLabel;

                                filterStrs.append(subFilterStr);
                            }
                            else if (subIsAnalysisTwoInput) {
                                // ANALYSIS TWO-INPUT: [subChainStream][sidechain]filter[output]
                                QString scLabel;
                                if (!subSidechainOutputs.isEmpty()) {
                                    QList<int> scKeys = subSidechainOutputs.keys();
                                    std::sort(scKeys.begin(), scKeys.end());
                                    scLabel = subSidechainOutputs[scKeys.last()];
                                } else if (!sidechainOutputs.isEmpty()) {
                                    // Fall back to main chain sidechain state
                                    QList<int> scKeys = sidechainOutputs.keys();
                                    std::sort(scKeys.begin(), scKeys.end());
                                    scLabel = sidechainOutputs[scKeys.last()];
                                } else {
                                    scLabel = "[1:a]";
                                }

                                subFilterStr = currentInput_sub + scLabel + subFilterStr + subOutputLabel;
                                filterStrs.append(subFilterStr);
                                currentInput_sub = subOutputLabel;
                            }
                            else if (subIsMultiInput) {
                                // MULTI-INPUT in sub-chain
                                if (subFilter->handlesOwnInputRouting()) {
                                    // Sub-case A: filter manages its own input routing (AFIR with amix/apad)
                                    // Replace [0:a] inline, replace [N:a] with sidechain outputs
                                    static QRegularExpression startsWithMainInput(R"(^\[0:a\])");
                                    if (startsWithMainInput.match(subFilterStr).hasMatch()) {
                                        subFilterStr.replace(QRegularExpression(R"(^\[0:a\])"), currentInput_sub);
                                    } else if (!subFilterStr.startsWith('[')) {
                                        subFilterStr = currentInput_sub + subFilterStr;
                                    }

                                    for (auto it = subSidechainOutputs.constBegin();
                                         it != subSidechainOutputs.constEnd(); ++it) {
                                        int idx = it.key();
                                        QString rawLabel = QString("[%1:a]").arg(idx);
                                        QString processedLabel = it.value();
                                        if (processedLabel != rawLabel) {
                                            subFilterStr.replace(rawLabel, processedLabel);
                                        }
                                    }
                                } else {
                                    // Sub-case B: standard multi-input — strip and rebuild input labels
                                    static QRegularExpression leadingInputLabels(R"(^(\[\d+:a\])+)");
                                    QRegularExpressionMatch inputLabelMatch = leadingInputLabels.match(subFilterStr);

                                    if (inputLabelMatch.hasMatch()) {
                                        subFilterStr = subFilterStr.mid(inputLabelMatch.capturedLength());
                                        QString inputPrefix = currentInput_sub;
                                        QList<int> scKeys = subSidechainOutputs.keys();
                                        std::sort(scKeys.begin(), scKeys.end());
                                        for (int key : scKeys) {
                                            inputPrefix += subSidechainOutputs[key];
                                        }
                                        subFilterStr = inputPrefix + subFilterStr;
                                    } else {
                                        for (auto it = subSidechainOutputs.constBegin();
                                             it != subSidechainOutputs.constEnd(); ++it) {
                                            int idx = it.key();
                                            QString rawLabel = QString("[%1:a]").arg(idx);
                                            QString processedLabel = it.value();
                                            if (processedLabel != rawLabel) {
                                                subFilterStr.replace(rawLabel, processedLabel);
                                            }
                                        }
                                        if (!subFilterStr.contains(currentInput_sub)) {
                                            subFilterStr.replace("[0:a]", currentInput_sub);
                                        }
                                    }
                                }

                                stripTrailingOutputLabel(subFilterStr);
                                subFilterStr += subOutputLabel;
                                filterStrs.append(subFilterStr);
                                currentInput_sub = subOutputLabel;

                                subCurrentAudioInput = nullptr;
                                subSidechainOutputs.clear();
                            }
                            else {
                                // NORMAL FILTER: wire onto sub-chain stream
                                bool hasInputLabels = subFilterStr.contains(QRegularExpression("^\\["));
                                if (!hasInputLabels) {
                                    subFilterStr = currentInput_sub + subFilterStr;
                                } else {
                                    subFilterStr.replace("[0:a]", currentInput_sub);
                                }

                                stripTrailingOutputLabel(subFilterStr);
                                subFilterStr += subOutputLabel;
                                filterStrs.append(subFilterStr);
                                currentInput_sub = subOutputLabel;
                            }
                        }
                        mixInputs << currentInput_sub;  // Final sub-chain output
                    }
                    mixWeights << QString::number(asplitFilter->getStreamWeight(s), 'f', 4);
                }
            }

            // Generate amix recombination (if auto amix enabled)
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
                // Manual routing: thru stream becomes main chain
                mainChainInput = thruLabel;
            }

            // Clear sidechain state after asplit block
            currentAudioInput = nullptr;
            sidechainOutputs.clear();
            continue;
        }

        // ============================================================
        // CASE 4: AudioInputFilter — establish sidechain context
        // ============================================================
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(rawFilter)) {
            currentAudioInput = audioInput;
            int idx = audioInput->getInputIndex();
            sidechainOutputs[idx] = QString("[%1:a]").arg(idx);
            continue;
        }

        // ============================================================
        // CASE 5: FFAnullsink — sink, no output
        // ============================================================
        if (dynamic_cast<FFAnullsink*>(rawFilter)) {
            QString sinkCmd = QString("%1anullsink").arg(mainChainInput);
            filterStrs.append(sinkCmd);
            continue;
        }

        // ============================================================
        // Get filter string. If empty, skip.
        // ============================================================
        QString filterStr = rawFilter->buildFFmpegFlags();
        if (filterStr.isEmpty()) continue;

        bool isLastFilter = !hasFiltersAfter(i);
        int filterId = rawFilter->getFilterId();

        // ============================================================
        // Classify the filter
        // ============================================================
        bool isMultiInput = isMultiInputFilter(rawFilter);
        bool isSmartAuxReturn = (dynamic_cast<SmartAuxReturn*>(rawFilter) != nullptr);
        bool isAnalysisTwoInput = rawFilter->isAnalysisTwoInputFilter();
        bool isInInsertMode = (currentAudioInput != nullptr &&
                               !isMultiInput &&
                               !isSmartAuxReturn &&
                               !isAnalysisTwoInput);

        // ============================================================
        // CASE 6: Insert mode — process sidechain stream
        // ============================================================
        if (isInInsertMode) {
            int idx = currentAudioInput->getInputIndex();
            QString inputLabel = sidechainOutputs[idx];

            bool hasInputLabels = filterStr.contains(QRegularExpression("^\\["));
            if (!hasInputLabels) {
                filterStr = inputLabel + filterStr;
            }

            QString outputLabel = QString("[%1]").arg(hexLabelFunc(filterId));

            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(rawFilter)) {
                manualLabels = customFilter->getManualOutputLabels();
            }

            if (!manualLabels) {
                stripTrailingOutputLabel(filterStr);
                filterStr += outputLabel;
                sidechainOutputs[idx] = outputLabel;
            }

            filterStrs.append(filterStr);
        }
        // ============================================================
        // CASE 7: SmartAuxReturn — amix main + sidechains
        // ============================================================
        else if (isSmartAuxReturn) {
            auto* smartAux = dynamic_cast<SmartAuxReturn*>(rawFilter);
            double mixLevel = smartAux ? smartAux->getMixLevel() : 0.5;

            QStringList inputStreams;
            inputStreams << mainChainInput;

            QStringList weights;
            weights << "1.0";

            for (auto it = sidechainOutputs.constBegin();
                 it != sidechainOutputs.constEnd(); ++it) {
                inputStreams << it.value();
                weights << QString::number(mixLevel);
            }

            QString outputLabel = isLastFilter
                ? "[out]"
                : QString("[%1]").arg(hexLabelFunc(filterId));

            QString durationStr = "longest";
            if (smartAux) {
                switch (smartAux->getDurationMode()) {
                    case SmartAuxReturn::DurationMode::Longest:
                        durationStr = "longest"; break;
                    case SmartAuxReturn::DurationMode::Shortest:
                        durationStr = "shortest"; break;
                    case SmartAuxReturn::DurationMode::First:
                        durationStr = "first"; break;
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
        // ============================================================
        // CASE 8: Multi-input filter
        // ============================================================
        else if (isMultiInput) {
            if (rawFilter->handlesOwnInputRouting()) {
                // Sub-case A: filter manages its own input routing (AFIR with amix/apad)
                static QRegularExpression startsWithMainInput(R"(^\[0:a\])");
                if (startsWithMainInput.match(filterStr).hasMatch()) {
                    filterStr.replace(QRegularExpression(R"(^\[0:a\])"), mainChainInput);
                } else if (!filterStr.startsWith('[')) {
                    filterStr = mainChainInput + filterStr;
                }

                for (auto it = sidechainOutputs.constBegin();
                     it != sidechainOutputs.constEnd(); ++it) {
                    int idx = it.key();
                    QString rawLabel = QString("[%1:a]").arg(idx);
                    QString processedLabel = it.value();
                    if (processedLabel != rawLabel) {
                        filterStr.replace(rawLabel, processedLabel);
                    }
                }
            } else {
                // Sub-case B: standard multi-input — strip and rebuild input labels
                static QRegularExpression leadingInputLabels(R"(^(\[\d+:a\])+)");
                QRegularExpressionMatch inputLabelMatch = leadingInputLabels.match(filterStr);

                if (inputLabelMatch.hasMatch()) {
                    filterStr = filterStr.mid(inputLabelMatch.capturedLength());
                    QString inputPrefix = mainChainInput;
                    QList<int> scKeys = sidechainOutputs.keys();
                    std::sort(scKeys.begin(), scKeys.end());
                    for (int key : scKeys) {
                        inputPrefix += sidechainOutputs[key];
                    }
                    filterStr = inputPrefix + filterStr;
                } else {
                    for (auto it = sidechainOutputs.constBegin();
                         it != sidechainOutputs.constEnd(); ++it) {
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

            // Output label handling (same for both sub-cases)
            bool usesCustomOutput = rawFilter->usesCustomOutputStream();
            QString outputLabel;
            if (usesCustomOutput) {
                outputLabel = QString("[%1]").arg(hexLabelFunc(filterId));
            } else {
                outputLabel = isLastFilter
                    ? "[out]"
                    : QString("[%1]").arg(hexLabelFunc(filterId));
            }

            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(rawFilter)) {
                manualLabels = customFilter->getManualOutputLabels();
            }

            if (!manualLabels) {
                stripTrailingOutputLabel(filterStr);
                filterStr += outputLabel;
                if (!usesCustomOutput) {
                    mainChainInput = outputLabel;
                }
            }

            currentAudioInput = nullptr;
            sidechainOutputs.clear();
            filterStrs.append(filterStr);
        }
        // ============================================================
        // CASE 9: Analysis two-input filter (apsnr, asdr, asisdr)
        // ============================================================
        else if (isAnalysisTwoInput) {
            QString sidechainLabel;
            if (!sidechainOutputs.isEmpty()) {
                QList<int> scKeys = sidechainOutputs.keys();
                std::sort(scKeys.begin(), scKeys.end());
                sidechainLabel = sidechainOutputs[scKeys.last()];
            } else {
                sidechainLabel = "[1:a]";
            }

            filterStr = mainChainInput + sidechainLabel + filterStr;

            QString outputLabel = isLastFilter
                ? "[out]"
                : QString("[%1]").arg(hexLabelFunc(filterId));
            filterStr += outputLabel;
            mainChainInput = outputLabel;

            // Clear sidechain only if it was processed (non-raw labels)
            bool sidechainWasProcessed = false;
            for (auto it = sidechainOutputs.constBegin();
                 it != sidechainOutputs.constEnd(); ++it) {
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
        // ============================================================
        // CASE 10: Normal single-input filter
        // ============================================================
        else {
            bool hasInputLabels = filterStr.contains("[0:a]") ||
                                  filterStr.contains(QRegularExpression("^\\["));

            if (hasInputLabels) {
                filterStr.replace("[0:a]", mainChainInput);
            } else {
                filterStr = mainChainInput + filterStr;
            }

            bool usesCustomOutput = rawFilter->usesCustomOutputStream();
            QString outputLabel;
            if (usesCustomOutput) {
                outputLabel = QString("[%1]").arg(hexLabelFunc(filterId));
            } else {
                outputLabel = isLastFilter
                    ? "[out]"
                    : QString("[%1]").arg(hexLabelFunc(filterId));
            }

            bool manualLabels = false;
            if (auto* customFilter = dynamic_cast<CustomFFmpegFilter*>(rawFilter)) {
                manualLabels = customFilter->getManualOutputLabels();
            }

            if (!manualLabels) {
                stripTrailingOutputLabel(filterStr);
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

} // namespace DAG
