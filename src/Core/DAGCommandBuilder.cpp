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
        // CASE 3: AsplitFilter — safety guard (Phase C)
        // useDAGPath() should have excluded chains with AsplitFilter.
        // ============================================================
        if (dynamic_cast<AsplitFilter*>(rawFilter)) {
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
        bool isMultiInputFilter = (dynamic_cast<FFAfir*>(rawFilter) ||
                                   dynamic_cast<FFSidechaincompress*>(rawFilter) ||
                                   dynamic_cast<FFSidechaingate*>(rawFilter) ||
                                   dynamic_cast<FFAcrossfade*>(rawFilter) ||
                                   dynamic_cast<FFAmerge*>(rawFilter) ||
                                   dynamic_cast<FFAmix*>(rawFilter) ||
                                   dynamic_cast<FFAxcorrelate*>(rawFilter) ||
                                   dynamic_cast<FFJoin*>(rawFilter));

        bool isSmartAuxReturn = (dynamic_cast<SmartAuxReturn*>(rawFilter) != nullptr);
        bool isAnalysisTwoInput = rawFilter->isAnalysisTwoInputFilter();
        bool isInInsertMode = (currentAudioInput != nullptr &&
                               !isMultiInputFilter &&
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
        else if (isMultiInputFilter) {
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

} // namespace DAG
