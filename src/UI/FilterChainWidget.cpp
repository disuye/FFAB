#include "FilterChainWidget.h"
#include "FilterMenuBuilder.h"
#include "RotatedLabel.h"
#include "AsplitRowWidget.h"
#include "Core/FilterChain.h"
#include "Core/AppConfig.h"
#include "Filters/BaseFilter.h"
#include "Filters/MultiOutputFilter.h"
#include "Filters/AudioInputFilter.h"
#include "Filters/ff-afir.h"
#include "Filters/ff-asplit.h"
#include "Filters/ff-sidechaincompress.h"
#include "Filters/ff-sidechaingate.h"
#include "Filters/ff-acrossfade.h"
#include "Filters/ff-amerge.h"
#include "Filters/ff-join.h"
#include "Filters/ff-amix.h"
#include "Filters/SmartAuxReturn.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QRegularExpression>
#include <QSet>
#include <QPainter>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <algorithm>

static QPushButton* createCompactButton(const QString& text) {
    auto btn = new QPushButton(text);
    btn->setFixedSize(20, 20);
    QSizePolicy btnSp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnSp.setRetainSizeWhenHidden(false);
    btn->setSizePolicy(btnSp);
    return btn;
}

// ========== DRAGGABLE FILTER LIST ==========
// Custom QListView subclass that handles drag-and-drop reordering

class DraggableFilterList : public QListView {
public:
    DraggableFilterList(FilterChainWidget* widget, QWidget* parent = nullptr)
        : QListView(parent), chainWidget(widget) {
        setDragEnabled(true);
        setAcceptDrops(true);
        setDropIndicatorShown(false);  // We draw our own
        setDragDropMode(QAbstractItemView::InternalMove);
        setDefaultDropAction(Qt::MoveAction);
    }
    
protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            dragStartPosition = event->pos();
            dragStartIndex = indexAt(event->pos());
        }
        QListView::mousePressEvent(event);
    }
    
    void mouseMoveEvent(QMouseEvent* event) override {
        if (!(event->buttons() & Qt::LeftButton)) {
            QListView::mouseMoveEvent(event);
            return;
        }
        
        if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
            QListView::mouseMoveEvent(event);
            return;
        }
        
        if (!dragStartIndex.isValid()) {
            QListView::mouseMoveEvent(event);
            return;
        }
        
        // Check if this row is draggable
        if (!isRowDraggable(dragStartIndex)) {
            QListView::mouseMoveEvent(event);
            return;
        }
        
        // Start drag
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        
        // Store the model row index
        mimeData->setData("application/x-ffab-filter-row", 
                         QByteArray::number(dragStartIndex.row()));
        drag->setMimeData(mimeData);
        
        // Create a semi-transparent pixmap of the row
        QWidget* rowWidget = indexWidget(dragStartIndex);
        if (rowWidget) {
            QPixmap pixmap(rowWidget->size());
            pixmap.fill(Qt::transparent);
            rowWidget->render(&pixmap);
            
            // Make it semi-transparent
            QPixmap transparent(pixmap.size());
            transparent.fill(Qt::transparent);
            QPainter p(&transparent);
            p.setOpacity(0.95); // drag & drop row opacity
            p.drawPixmap(0, 0, pixmap);
            p.end();
            
            drag->setPixmap(transparent);
            drag->setHotSpot(event->pos() - visualRect(dragStartIndex).topLeft());
        }
        
        isDragging = true;
        drag->exec(Qt::MoveAction);
        isDragging = false;
        dropTargetRow = -1;
        viewport()->update();
    }
    
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasFormat("application/x-ffab-filter-row")) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
    
    void dragMoveEvent(QDragMoveEvent* event) override {
        if (!event->mimeData()->hasFormat("application/x-ffab-filter-row")) {
            event->ignore();
            return;
        }
        
        // Calculate drop target row
        QModelIndex idx = indexAt(event->position().toPoint());
        int newDropTarget = -1;
        
        if (idx.isValid()) {
            // Check if we're in the top or bottom half of the item
            QRect rect = visualRect(idx);
            bool inTopHalf = event->position().toPoint().y() < rect.center().y();
            
            if (inTopHalf) {
                newDropTarget = idx.row();
            } else {
                newDropTarget = idx.row() + 1;
            }
            
            // Validate drop target
            if (!isValidDropTarget(newDropTarget)) {
                newDropTarget = -1;
            }
        }
        
        if (newDropTarget != dropTargetRow) {
            dropTargetRow = newDropTarget;
            viewport()->update();
        }
        
        if (dropTargetRow >= 0) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
    
    void dragLeaveEvent(QDragLeaveEvent* event) override {
        dropTargetRow = -1;
        viewport()->update();
        QListView::dragLeaveEvent(event);
    }
    
    void dropEvent(QDropEvent* event) override {
        if (!event->mimeData()->hasFormat("application/x-ffab-filter-row")) {
            event->ignore();
            return;
        }
        
        if (dropTargetRow < 0) {
            event->ignore();
            return;
        }
        
        int sourceRow = event->mimeData()->data("application/x-ffab-filter-row").toInt();
        
        // Execute the move
        chainWidget->handleDragDrop(sourceRow, dropTargetRow);
        
        dropTargetRow = -1;
        viewport()->update();
        event->acceptProposedAction();
    }
    
    void paintEvent(QPaintEvent* event) override {
        QListView::paintEvent(event);
        
        // Draw drop indicator line
        if (isDragging && dropTargetRow >= 0) {
            QPainter painter(viewport());
            painter.setRenderHint(QPainter::Antialiasing);
            
            // Calculate Y position for the indicator
            int y;
            if (dropTargetRow < model()->rowCount()) {
                QRect rect = visualRect(model()->index(dropTargetRow, 0));
                y = rect.top();
            } else {
                QRect rect = visualRect(model()->index(model()->rowCount() - 1, 0));
                y = rect.bottom();
            }
            
            // Drag & Drop
            QPen pen(QColor(128, 128, 128, 64), 40);  // Position box
            painter.setPen(pen);
            painter.drawLine(0, y, viewport()->width(), y);

            //  Drag & Drop
            // QPen pen2(QColor(128, 128, 128, 20), 30);  // Position line
            // painter.setPen(pen2);
            // painter.drawLine(0, y, viewport()->width(), y);
            
            // Draw small arrows at the ends
            // painter.setBrush(QColor(255, 140, 0));
            // QPolygon leftArrow, rightArrow;
            // leftArrow << QPoint(5, y) << QPoint(15, y - 5) << QPoint(15, y + 5);
            // rightArrow << QPoint(viewport()->width() - 5, y) 
            //           << QPoint(viewport()->width() - 15, y - 5) 
            //           << QPoint(viewport()->width() - 15, y + 5);
            // painter.drawPolygon(leftArrow);
            // painter.drawPolygon(rightArrow);
        }
    }
    
private:
    bool isRowDraggable(const QModelIndex& index) const {
        if (!index.isValid()) return false;
        
        // Can't drag arrows
        bool isArrow = index.data(Qt::UserRole + 1).toBool();
        if (isArrow) return false;
        
        int position = index.data(Qt::UserRole).toInt();
        
        // Can't drag special rows (back button, header, footer)
        if (position == -2 || position == -3 || position == -4) return false;
        
        // For sub-chain filters, allow dragging
        bool isSubChainFilter = index.data(Qt::UserRole + 2).toBool();
        if (isSubChainFilter) return true;
        
        // Main chain: can't drag INPUT (pos 0) or OUTPUT (last)
        if (position == 0) return false;
        
        auto* chain = chainWidget->getFilterChain();
        if (chain && position >= chain->filterCount() - 1) return false;
        
        return true;
    }
    
    bool isValidDropTarget(int targetRow) const {
        if (targetRow < 0) return false;
        
        auto* mdl = model();
        if (!mdl) return false;
        
        // Get source row info
        int sourceRow = dragStartIndex.row();
        if (targetRow == sourceRow || targetRow == sourceRow + 1) {
            return false;  // No-op drop
        }
        
        // Check if in sub-chain view
        if (chainWidget->isInStreamView()) {
            return isValidSubChainDropTarget(targetRow);
        }
        
        // Main chain view
        return isValidMainChainDropTarget(targetRow);
    }
    
    bool isValidMainChainDropTarget(int targetRow) const {
        auto* mdl = model();
        int rowCount = mdl->rowCount();
        
        // Can't drop at row 0 (INPUT row) or row 1 (arrow after INPUT)
        if (targetRow <= 1) return false;
        
        // Can't drop at/after OUTPUT
        // Find OUTPUT row (last non-arrow row)
        for (int i = rowCount - 1; i >= 0; i--) {
            QModelIndex idx = mdl->index(i, 0);
            bool isArrow = idx.data(Qt::UserRole + 1).toBool();
            if (!isArrow) {
                // This is OUTPUT - can't drop at or after it
                if (targetRow >= i) return false;
                break;
            }
        }
        
        return true;
    }
    
    bool isValidSubChainDropTarget(int targetRow) const {
        auto* mdl = model();
        int rowCount = mdl->rowCount();
        
        // Can't drop before first filter position (after header + arrow)
        if (targetRow <= 2) return false;
        
        // Can't drop at footer
        for (int i = rowCount - 1; i >= 0; i--) {
            QModelIndex idx = mdl->index(i, 0);
            int pos = idx.data(Qt::UserRole).toInt();
            if (pos == -4) {
                // Footer row
                if (targetRow >= i) return false;
                break;
            }
        }
        
        return true;
    }
    
    FilterChainWidget* chainWidget;
    QPoint dragStartPosition;
    QModelIndex dragStartIndex;
    int dropTargetRow = -1;
    bool isDragging = false;
};


class FilterRowDelegate : public QStyledItemDelegate {
public:
    FilterRowDelegate(FilterChain* chain, FilterChainWidget* widget, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), filterChain(chain), chainWidget(widget) {}
    
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        int position = index.data(Qt::UserRole).toInt();
        bool isArrow = index.data(Qt::UserRole + 1).toBool();
        
        if (position == -2) {
            return QSize(option.rect.width(), 40);
        }
        
        return QSize(option.rect.width(), isArrow ? 20 : 40);
    }
    
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, 
                         const QModelIndex& index) const override {
        int position = index.data(Qt::UserRole).toInt();
        bool isArrow = index.data(Qt::UserRole + 1).toBool();
        bool isSubChainFilter = index.data(Qt::UserRole + 2).toBool();
        int subChainIndex = index.data(Qt::UserRole + 3).toInt();

        auto container = new QWidget(parent);
        container->setAttribute(Qt::WA_StyledBackground, true);
        container->setFixedWidth(372);
        
        auto layout = new QHBoxLayout(container);
        layout->setContentsMargins(4, 1, 4, 1);
        layout->setSpacing(2);
        layout->setSizeConstraint(QLayout::SetDefaultConstraint);

        if (isArrow) {
            layout->setContentsMargins(0, 0, 0, 0);
            auto arrow = new QLabel("↓");
            arrow->setAlignment(Qt::AlignCenter);
            // ARROW ROW STYLE
            arrow->setStyleSheet("QLabel {background: qlineargradient(y1:0, y2:1, stop:0 rgba(128,128,128,0.07), stop:0.5 transparent);}");
            layout->addWidget(arrow);
            return container;
        }
        
        // Back button
        if (position == -2) {
            auto* backButton = new QPushButton("← Back to Main Chain");
            connect(backButton, &QPushButton::clicked, [this]() {
                chainWidget->showMainChain();
            });
            layout->addWidget(backButton);
            container->setFixedHeight(40);
            return container;
        }
        
        // Sub-chain header (INPUT-style row)
        if (position == -3) {
            int multiOutputPos = chainWidget->getCurrentMultiOutputPosition();
            int streamIndex = chainWidget->getCurrentStreamIndex();
            auto* multiOutput = filterChain->getMultiOutputFilter(multiOutputPos);
            
            QString headerText;
            if (multiOutput) {
                QString hexId = QString("%1").arg(multiOutput->getFilterId(), 4, 16, QChar('0')).toUpper();
                headerText = QString("%1 → Stream %2 Sub-Chain").arg(hexId).arg(streamIndex);
            } else {
                headerText = QString("Stream/Sub-Chain %1").arg(streamIndex);
            }
            
            auto* headerLabel = new QLabel(headerText);
            headerLabel->setStyleSheet("background: transparent; border: none;");
            headerLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            layout->addWidget(headerLabel);
            container->setStyleSheet("background-color: rgba(128, 128, 128, 0.05);");
            container->setFixedHeight(40);
            return container;
        }
        
        // Sub-chain footer (OUTPUT-style row)
        if (position == -4) {
            auto* footerLabel = new QLabel("Sub-Chain Output");
            footerLabel->setStyleSheet("background: transparent; border: none;");
            footerLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            layout->addWidget(footerLabel);
            container->setStyleSheet("background-color: rgba(128, 128, 128, 0.05);");
            container->setFixedHeight(40);
            return container;
        }
        
        // Get the filter - either from main chain or sub-chain
        std::shared_ptr<BaseFilter> filter;
        if (isSubChainFilter) {
            int multiOutputPos = chainWidget->getCurrentMultiOutputPosition();
            int streamIndex = chainWidget->getCurrentStreamIndex();
            filter = filterChain->getFilterFromStream(multiOutputPos, streamIndex, subChainIndex);
        } else {
            filter = filterChain->getFilter(position);
        }
        
        if (!filter) return new QWidget(parent);
        
        // Check if this is an asplit filter - use custom widget (only in main chain view)
        if (!isSubChainFilter && filter->filterType() == "asplit") {
            auto* asplitFilter = dynamic_cast<AsplitFilter*>(filter.get());
            if (asplitFilter) {
                auto* asplitWidget = new AsplitRowWidget(asplitFilter, filterChain, position, chainWidget, parent);
                asplitWidget->setFixedWidth(372);
                
                connect(asplitWidget, &AsplitRowWidget::moveUpRequested, 
                        [this, position]() { chainWidget->moveFilterUp(position); });
                connect(asplitWidget, &AsplitRowWidget::moveDownRequested,
                        [this, position]() { chainWidget->moveFilterDown(position); });
                connect(asplitWidget, &AsplitRowWidget::deleteRequested,
                        [this, position]() {
                            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                                chainWidget->deleteFilter(position);
                            } else {
                                chainWidget->confirmDeleteFilter(position);
                            }
                        });
                connect(asplitWidget, &AsplitRowWidget::muteToggled,
                        [this, position]() { chainWidget->handleMuteToggle(position); });
                connect(asplitWidget, &AsplitRowWidget::soloToggled,
                        [this, position]() { chainWidget->handleSoloToggle(position); });
                connect(asplitWidget, &AsplitRowWidget::customOutputToggled,
                        [this, position]() { chainWidget->handleCustomOutputToggle(position); });
                connect(asplitWidget, &AsplitRowWidget::streamButtonClicked,
                        [this, position](int streamIndex) {
                            qDebug() << "Stream button clicked:" << streamIndex << "for asplit at position" << position;
                            chainWidget->showStreamView(position, streamIndex);
                        });
                
                return asplitWidget;
            }
        }
        
        // Standard filter row
        auto muteBtn = createCompactButton("M");
        auto soloBtn = createCompactButton("S");
        auto customOutputBtn = createCompactButton("○");
        auto upBtn = createCompactButton("↑");
        auto downBtn = createCompactButton("↓");
        auto delBtn = createCompactButton("✕");
        auto blankBtn1 = createCompactButton("");
        auto blankBtn2 = createCompactButton("");
        auto blankBtn3 = createCompactButton("");

        int filterId = filter->getFilterId();
        bool isExplicitMute = chainWidget->isFilterExplicitMuteById(filterId);
        bool isImpliedMute = chainWidget->isFilterImpliedMuteById(filterId);
        bool isSoloed = chainWidget->isFilterSoloedById(filterId);
        bool hasCustomOutput = filter->usesCustomOutputStream();
        bool anySoloActive = isSubChainFilter ? chainWidget->isAnySoloActiveInSubChain() : chainWidget->isAnySoloActive();
        
        QString buttonBaseStyle = "QPushButton { border: none; border-radius: 2px; background-color: rgba(128, 128, 128, 0.17); %1 }";
        QString buttonActiveStyle = "QPushButton { border: none; border-radius: 2px; background-color: rgba(128, 128, 128, 0.17); %1 }";

        if (isExplicitMute) {
            muteBtn->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(63, 125, 156, 0.8);"));
            muteBtn->setToolTip("MUTED (click to unmute)");
        } else if (isImpliedMute) {
            muteBtn->setStyleSheet(buttonActiveStyle.arg("color: rgba(63, 125, 156, 0.9); background-color: rgba(63, 125, 156, 0.25); border: 1px solid rgba(63, 125, 156, 0.6);"));
            muteBtn->setToolTip("IMPLIED MUTE (another filter is soloed)\nClick to join solo group");
        } else {
            muteBtn->setStyleSheet(buttonBaseStyle.arg(""));
            muteBtn->setToolTip("Click to mute");
        }
        
        if (isSoloed) {
            soloBtn->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(194, 80, 24, 0.8);"));
            soloBtn->setToolTip("SOLOED (click to unsolo)");
        } else if (anySoloActive) {
            soloBtn->setStyleSheet(buttonBaseStyle.arg("color: rgba(194, 80, 24, 0.6);"));
            soloBtn->setToolTip("Click to add to solo group");
        } else {
            soloBtn->setStyleSheet(buttonBaseStyle.arg(""));
            soloBtn->setToolTip("Click to solo");
        }
        
        if (hasCustomOutput) {
            customOutputBtn->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(67, 139, 33, 0.5);"));
            customOutputBtn->setToolTip("Custom Audio Output: ON");
        } else {
            customOutputBtn->setStyleSheet(buttonBaseStyle.arg(""));
            customOutputBtn->setToolTip("Custom Audio Output: OFF");
        }
    
        upBtn->setStyleSheet(buttonBaseStyle.arg(""));
        downBtn->setStyleSheet(buttonBaseStyle.arg(""));
        delBtn->setStyleSheet(buttonBaseStyle.arg(""));
        delBtn->setToolTip("Press ALT+[✕] to skip delete dialog");
        blankBtn1->setStyleSheet(buttonBaseStyle.arg(""));
        blankBtn2->setStyleSheet(buttonBaseStyle.arg(""));
        blankBtn3->setStyleSheet(buttonBaseStyle.arg(""));

        int effectivePosition = isSubChainFilter ? subChainIndex : position;
        
        connect(muteBtn, &QPushButton::clicked, [this, filterId, isSubChainFilter, effectivePosition]() {
            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                chainWidget->handleMuteAllToggle();
            } else if (isSubChainFilter) {
                chainWidget->handleMuteToggleById(filterId);
            } else {
                chainWidget->handleMuteToggle(effectivePosition);
            }
        });
        
        connect(soloBtn, &QPushButton::clicked, [this, filterId, isSubChainFilter, effectivePosition]() {
            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                chainWidget->handleSoloAllClear();
            } else if (isSubChainFilter) {
                chainWidget->handleSoloToggleById(filterId);
            } else {
                chainWidget->handleSoloToggle(effectivePosition);
            }
        });
        
        connect(customOutputBtn, &QPushButton::clicked, [this, filterId, isSubChainFilter, effectivePosition]() {
            if (isSubChainFilter) {
                chainWidget->handleCustomOutputToggleById(filterId);
            } else {
                chainWidget->handleCustomOutputToggle(effectivePosition);
            }
        });

        connect(upBtn, &QPushButton::clicked, [this, isSubChainFilter, effectivePosition]() {
            chainWidget->moveFilterUp(isSubChainFilter ? -(effectivePosition + 1) : effectivePosition);
        });
        
        connect(downBtn, &QPushButton::clicked, [this, isSubChainFilter, effectivePosition]() {
            chainWidget->moveFilterDown(isSubChainFilter ? -(effectivePosition + 1) : effectivePosition);
        });
        
        connect(delBtn, &QPushButton::clicked, [this, isSubChainFilter, effectivePosition]() {
            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                chainWidget->deleteFilter(isSubChainFilter ? -(effectivePosition + 1) : effectivePosition);
            } else {
                chainWidget->confirmDeleteFilter(isSubChainFilter ? -(effectivePosition + 1) : effectivePosition);
            }
        });
        
        // Filter ID label - add FIRST for proper left-side positioning
        if (filter->getPosition() == BaseFilter::Position::MIDDLE) {
            if (filterId >= 0) {
                QString idHex = FilterColors::hexId(filterId);
                QColor rowBgColor = FilterColors::rowBackground(filterId);
                QColor labelBgColor = FilterColors::labelBackground(filterId);
                QColor textColor = FilterColors::textColor(filterId);
                
                auto idLabel = new RotatedLabel(QString("%1").arg(idHex));
                idLabel->setBackgroundColor(labelBgColor);
                idLabel->setTextColor(textColor);
                idLabel->setStyleSheet("font-size: 10px;");
                idLabel->setAlignment(Qt::AlignCenter);
                layout->addWidget(idLabel, 0);
                
                upBtn->setStyleSheet(buttonActiveStyle.arg(QString("background: %1;").arg(rowBgColor.name(QColor::HexArgb))));
                downBtn->setStyleSheet(buttonActiveStyle.arg(QString("background: %1;").arg(rowBgColor.name(QColor::HexArgb))));
            }
        }

        layout->addWidget(upBtn);
        layout->addWidget(downBtn);
        layout->addWidget(delBtn);

        bool isAudioInputFilter = (filter->filterType() == "audio-input");
        bool isAuxOutputFilter = (filter->filterType() == "aux-output");
        bool isInputOutputFilter = (filter->getPosition() == BaseFilter::Position::INPUT || 
                                    filter->getPosition() == BaseFilter::Position::OUTPUT);
        
        if (isAudioInputFilter || isInputOutputFilter) {
            muteBtn->hide();
            soloBtn->hide();
            customOutputBtn->hide();
            if (isAudioInputFilter) {
                layout->addWidget(blankBtn1);
                layout->addWidget(blankBtn2);
                layout->addWidget(blankBtn3);
            }
        } else {
            layout->addWidget(muteBtn);
            layout->addWidget(soloBtn);
            layout->addWidget(customOutputBtn);
        } if (isAuxOutputFilter) {
            customOutputBtn->hide();
            layout->addWidget(blankBtn3);
        }

        auto nameLabel = new QLabel(filter->displayName());
        nameLabel->setStyleSheet("background: transparent; border: none; padding-left: 6px;");
        layout->addWidget(nameLabel, 1);
        
        // Signal flow label (right side)
        QString flowLabel;
        if (isSubChainFilter) {
            int multiOutputPos = chainWidget->getCurrentMultiOutputPosition();
            int streamIndex = chainWidget->getCurrentStreamIndex();
            flowLabel = chainWidget->getSubChainSignalFlowLabel(multiOutputPos, streamIndex, subChainIndex);
        } else {
            flowLabel = chainWidget->getFilterInputStreamLabel(position);
        }
        if (!flowLabel.isEmpty()) {
            auto* flowLabelWidget = new QLabel(flowLabel);
            flowLabelWidget->setStyleSheet("background: transparent; border: none; color: rgba(128, 128, 128, 0.7); padding-right: 6px;");
            flowLabelWidget->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            layout->addWidget(flowLabelWidget);
        }

        // INPUT and OUTPUT header styling
        bool isInputOrOutput = !isSubChainFilter && (position == 0 || position == filterChain->filterCount() - 1);
        if (isInputOrOutput) {
            muteBtn->hide();
            soloBtn->hide();
            delBtn->hide();
            upBtn->hide();
            downBtn->hide();
            // INPUT / OUTPUT ROW STYLE
            container->setStyleSheet("border: 1px solid rgba(128,128,128,0.1); border-right: none; border-left: none;");
            nameLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        } else {
            // STANDARD ROW STYLE
            container->setStyleSheet("border: 1px solid rgba(128,128,128,0.1); border-right: none; border-left: none;");
            nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        
        container->setFixedHeight(40);
        return container;
    }
    
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override {
        bool isArrow = index.data(Qt::UserRole + 1).toBool();
        if (isArrow) {
            editor->setGeometry(option.rect);
        } else {
            QRect rect = option.rect;
            rect.setHeight(40);
            editor->setGeometry(rect);
        }
    }
    
private:
    FilterChain* filterChain;
    FilterChainWidget* chainWidget;
};

// ========== FILTER CHAIN WIDGET IMPLEMENTATION ==========

FilterChainWidget::FilterChainWidget(FilterChain* chain, QWidget* parent)
    : QWidget(parent), filterChain(chain) {
    setupUI();
    refreshChain();
}

FilterChainWidget::~FilterChainWidget() = default;

void FilterChainWidget::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Use custom draggable list view
    listView = new DraggableFilterList(this);
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    listView->setAutoFillBackground(false);
    listView->viewport()->setAutoFillBackground(false);
    listView->setFrameShape(QFrame::HLine);
    listView->setStyleSheet("QListView { border-radius: 4px; border: 1px solid rgba(0,0,0,0.05); border-left: 0px; border-top: 0px; background: qlineargradient(y1:0, y2:1, stop:0 rgba(0,0,0,0.05), stop:0.01 transparent, stop:0.99 transparent, stop:1 rgba(0,0,0,0.05)); }");
    
    model = new QStandardItemModel(this);
    listView->setModel(model);
    listView->setItemDelegate(new FilterRowDelegate(filterChain, this, this));
    
    connect(listView, &QListView::clicked, this, &FilterChainWidget::onListItemClicked);
    
    mainLayout->addWidget(listView);
    
    addFilterButton = new QPushButton("+ Add Filter");
    addFilterButton->setStyleSheet("QPushButton { font-family: 'Fira Code', monospace; font-size: 12px; font-weight: light; }");
    connect(addFilterButton, &QPushButton::clicked, this, &FilterChainWidget::onAddFilterClicked);
    addFilterButton->setToolTip("Use keys A or ` to open filter menu\nDrag filters to reorder");
    
    mainLayout->addWidget(addFilterButton);
}

void FilterChainWidget::refreshChain() {
    model->clear();
    
    if (!filterChain) return;
    
    int filterCount = filterChain->filterCount();
    if (filterCount == 0) return;
    
    // SUB-CHAIN VIEW
    if (currentViewMode == StreamView && currentMultiOutputPos >= 0 && currentStreamIndex > 0) {
        // Back button row
        auto backItem = new QStandardItem();
        backItem->setData(-2, Qt::UserRole);
        backItem->setData(false, Qt::UserRole + 1);
        backItem->setFlags(Qt::ItemIsEnabled);
        model->appendRow(backItem);
        
        addSubChainHeader();
        addArrowSeparator();
        
        // Get filters from the multi-output filter's stream
        const auto& subChain = filterChain->getSubChain(currentMultiOutputPos, currentStreamIndex);
        int streamFilterCount = subChain.size();
        
        for (int i = 0; i < streamFilterCount; ++i) {
            addSubChainFilterRow(subChain[i], i);
        }
        
        // Only add arrow before footer if there are filters
        if (streamFilterCount > 0) {
            addArrowSeparator();
        }
        addSubChainFooter();
        
        addFilterButton->setText(QString("+ Add Filter"));
        
        for (int i = 0; i < model->rowCount(); ++i) {
            listView->openPersistentEditor(model->index(i, 0));
        }
        
        return;
    }
    
    // MAIN CHAIN VIEW
    bool addedAnyFilters = false;
    
    addFilterRow(0);  // INPUT
    addArrowSeparator();
    
    for (int i = 1; i < filterCount - 1; ++i) {
        addFilterRow(i);
        addedAnyFilters = true;
    }
    
    if (addedAnyFilters) {
        addArrowSeparator();
    }
    
    if (filterCount > 1) {
        addFilterRow(filterCount - 1);  // OUTPUT
    }
    
    addFilterButton->setText("+ Add Filter");
    
    for (int i = 0; i < model->rowCount(); ++i) {
        listView->openPersistentEditor(model->index(i, 0));
    }
    
    updateAsplitButtonStates();
}

void FilterChainWidget::addFilterRow(int position) {
    auto filter = filterChain->getFilter(position);
    if (!filter) return;
    
    auto item = new QStandardItem();
    item->setData(position, Qt::UserRole);
    item->setData(false, Qt::UserRole + 1);  // Not an arrow
    item->setData(false, Qt::UserRole + 2);  // Not a sub-chain filter
    item->setData(0, Qt::UserRole + 3);      // Sub-chain index (unused)
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    
    model->appendRow(item);
}

void FilterChainWidget::addSubChainFilterRow(std::shared_ptr<BaseFilter> filter, int indexInSubChain) {
    if (!filter) return;
    
    auto item = new QStandardItem();
    item->setData(-1, Qt::UserRole);          // Position -1 = not in main chain
    item->setData(false, Qt::UserRole + 1);   // Not an arrow
    item->setData(true, Qt::UserRole + 2);    // IS a sub-chain filter
    item->setData(indexInSubChain, Qt::UserRole + 3);  // Index within sub-chain
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    
    model->appendRow(item);
}

void FilterChainWidget::addArrowSeparator() {
    auto item = new QStandardItem();
    item->setData(-1, Qt::UserRole);
    item->setData(true, Qt::UserRole + 1);  // Is an arrow
    item->setData(false, Qt::UserRole + 2);
    item->setFlags(Qt::ItemIsEnabled);
    
    model->appendRow(item);
}

void FilterChainWidget::addSubChainHeader() {
    auto item = new QStandardItem();
    item->setData(-3, Qt::UserRole);
    item->setData(false, Qt::UserRole + 1);
    item->setData(false, Qt::UserRole + 2);
    item->setFlags(Qt::ItemIsEnabled);
    
    model->appendRow(item);
}

void FilterChainWidget::addSubChainFooter() {
    auto item = new QStandardItem();
    item->setData(-4, Qt::UserRole);
    item->setData(false, Qt::UserRole + 1);
    item->setData(false, Qt::UserRole + 2);
    item->setFlags(Qt::ItemIsEnabled);
    
    model->appendRow(item);
}

// ========== DRAG AND DROP HANDLER ==========

void FilterChainWidget::handleDragDrop(int sourceModelRow, int targetModelRow) {
    // Convert model rows to filter positions
    auto getFilterInfo = [this](int row) -> std::tuple<int, bool, int> {
        // Returns (position, isSubChainFilter, subChainIndex)
        QModelIndex idx = model->index(row, 0);
        if (!idx.isValid()) return {-1, false, -1};
        
        bool isArrow = idx.data(Qt::UserRole + 1).toBool();
        if (isArrow) return {-1, false, -1};
        
        bool isSubChainFilter = idx.data(Qt::UserRole + 2).toBool();
        if (isSubChainFilter) {
            int subChainIndex = idx.data(Qt::UserRole + 3).toInt();
            return {-1, true, subChainIndex};
        }
        
        int position = idx.data(Qt::UserRole).toInt();
        return {position, false, -1};
    };
    
    auto [sourcePos, sourceIsSub, sourceSubIdx] = getFilterInfo(sourceModelRow);
    
    // Find target filter position by scanning rows
    int targetFilterPos = -1;
    bool targetIsSub = false;
    
    // Walk backwards from targetModelRow to find nearest filter row
    for (int r = targetModelRow - 1; r >= 0; r--) {
        auto [pos, isSub, subIdx] = getFilterInfo(r);
        if (isSub) {
            targetFilterPos = subIdx + 1;  // Insert after this sub-chain filter
            targetIsSub = true;
            break;
        } else if (pos >= 0) {
            targetFilterPos = pos + 1;  // Insert after this main chain filter
            break;
        } else if (pos == -3) {
            // Sub-chain header: drop target is position 0 (top of sub-chain)
            targetFilterPos = 0;
            targetIsSub = true;
            break;
        }
    }
    
    if (targetFilterPos < 0) {
        qDebug() << "Invalid drop target";
        return;
    }
    
    // Perform the move
    if (currentViewMode == StreamView && sourceIsSub && targetIsSub) {
        // Sub-chain move
        int from = sourceSubIdx;
        int to = targetFilterPos;
        if (to > from) to--;  // Adjust for removal
        
        qDebug() << "Sub-chain drag from" << from << "to" << to;
        filterChain->moveFilterInStream(currentMultiOutputPos, currentStreamIndex, from, to);
    } else if (currentViewMode == MainChainView && !sourceIsSub) {
        // Main chain move
        int from = sourcePos;
        int to = targetFilterPos;
        if (to > from) to--;  // Adjust for removal
        
        qDebug() << "Main chain drag from" << from << "to" << to;
        filterChain->moveFilter(from, to);
    } else {
        qDebug() << "Mixed drag not supported";
        return;
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::moveFilterUp(int position) {
    if (position < 0) {
        int subChainIndex = -(position + 1);
        if (subChainIndex <= 0) return;
        
        filterChain->moveFilterInStream(currentMultiOutputPos, currentStreamIndex, 
                                        subChainIndex, subChainIndex - 1);
        refreshChain();
        emit chainModified();
        return;
    }
    
    if (position <= 1) return;
    
    filterChain->moveFilter(position, position - 1);
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::moveFilterDown(int position) {
    if (position < 0) {
        int subChainIndex = -(position + 1);
        int chainSize = filterChain->getStreamFilterCount(currentMultiOutputPos, currentStreamIndex);
        if (subChainIndex >= chainSize - 1) return;
        
        filterChain->moveFilterInStream(currentMultiOutputPos, currentStreamIndex,
                                        subChainIndex, subChainIndex + 1);
        refreshChain();
        emit chainModified();
        return;
    }
    
    int filterCount = filterChain->filterCount();
    if (position >= filterCount - 2) return;
    
    filterChain->moveFilter(position, position + 1);
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::deleteFilter(int position) {
    if (position < 0) {
        int subChainIndex = -(position + 1);
        
        auto filter = filterChain->getFilterFromStream(currentMultiOutputPos, currentStreamIndex, subChainIndex);
        if (filter) {
            int filterId = filter->getFilterId();
            explicitMuteStates.remove(filterId);
            soloStates.remove(filterId);
            impliedMuteSet.remove(filterId);
        }
        
        filterChain->removeFilterFromStream(currentMultiOutputPos, currentStreamIndex, subChainIndex);
        recomputeImpliedMutes();
        refreshChain();
        emit chainModified();
        return;
    }
    
    if (position <= 0 || position >= filterChain->filterCount() - 1) return;
    
    auto filter = filterChain->getFilter(position);
    if (filter) {
        int filterId = filter->getFilterId();
        explicitMuteStates.remove(filterId);
        soloStates.remove(filterId);
        impliedMuteSet.remove(filterId);
    }
    
    filterChain->removeFilter(position);
    recomputeImpliedMutes();
    
    if (currentSelection == position) {
        currentSelection = 0;
        emit filterSelected(0);
    } else if (currentSelection > position) {
        currentSelection--;
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::confirmDeleteFilter(int position) {
    std::shared_ptr<BaseFilter> filter;
    
    if (position < 0) {
        int subChainIndex = -(position + 1);
        filter = filterChain->getFilterFromStream(currentMultiOutputPos, currentStreamIndex, subChainIndex);
    } else {
        filter = filterChain->getFilter(position);
    }
    
    if (!filter) return;
    
    QString filterName = filter->displayName();
    int filterId = filter->getFilterId();
    QString idStr = QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Filter",
        QString("Delete \"%1 ID %2\"?").arg(filterName, idStr),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        deleteFilter(position);
    }
}

void FilterChainWidget::handleMuteToggle(int position) {
    std::shared_ptr<BaseFilter> filter;
    
    if (position < 0 && currentViewMode == StreamView) {
        return;
    }
    
    filter = filterChain->getFilter(position);
    if (!filter) return;
    
    int filterId = filter->getFilterId();
    bool isExplicitMute = explicitMuteStates.value(filterId, false);
    bool isSoloed = soloStates.value(filterId, false);
    bool isImpliedMute = impliedMuteSet.contains(filterId);
    
    if (isSoloed) {
        soloStates.remove(filterId);
        explicitMuteStates[filterId] = true;
        recomputeImpliedMutes();
    } else if (isExplicitMute) {
        explicitMuteStates.remove(filterId);
        recomputeImpliedMutes();
    } else if (isImpliedMute) {
        soloStates[filterId] = true;
        recomputeImpliedMutes();
    } else {
        explicitMuteStates[filterId] = true;
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::handleMuteAllToggle() {
    if (currentViewMode == StreamView && currentMultiOutputPos >= 0) {
        QSet<int> subChainIds = getSubChainFilterIds();
        
        bool hadSolos = false;
        for (int id : subChainIds) {
            if (soloStates.value(id, false)) {
                soloStates.remove(id);
                hadSolos = true;
            }
        }
        if (hadSolos) {
            recomputeSubChainImpliedMutes();
            refreshChain();
            emit chainModified();
            return;
        }
        
        bool anyUnmuted = false;
        for (int id : subChainIds) {
            if (!explicitMuteStates.value(id, false)) {
                anyUnmuted = true;
                break;
            }
        }
        
        for (int id : subChainIds) {
            if (anyUnmuted) {
                explicitMuteStates[id] = true;
            } else {
                explicitMuteStates.remove(id);
            }
        }
        
        refreshChain();
        emit chainModified();
        return;
    }
    
    if (!soloStates.isEmpty()) {
        soloStates.clear();
        explicitMuteStates.clear();
        impliedMuteSet.clear();
        refreshChain();
        emit chainModified();
        return;
    }
    
    bool anyUnmuted = false;
    for (int i = 1; i < filterChain->filterCount() - 1; ++i) {
        auto filter = filterChain->getFilter(i);
        if (filter && filter->filterType() != "audio-input" && 
            !explicitMuteStates.value(filter->getFilterId(), false)) {
            anyUnmuted = true;
            break;
        }
    }
    
    explicitMuteStates.clear();
    if (anyUnmuted) {
        for (int i = 1; i < filterChain->filterCount() - 1; ++i) {
            auto filter = filterChain->getFilter(i);
            if (filter && filter->filterType() != "audio-input") {
                explicitMuteStates[filter->getFilterId()] = true;
            }
        }
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::handleSoloAllClear() {
    if (soloStates.isEmpty()) return;
    
    soloStates.clear();
    impliedMuteSet.clear();
    refreshChain();
}

void FilterChainWidget::handleSoloToggle(int position) {
    auto filter = filterChain->getFilter(position);
    if (!filter) return;
    
    int filterId = filter->getFilterId();
    bool isExplicitMute = explicitMuteStates.value(filterId, false);
    bool isSoloed = soloStates.value(filterId, false);
    
    if (isSoloed) {
        soloStates.remove(filterId);
        recomputeImpliedMutes();
    } else {
        if (isExplicitMute) {
            explicitMuteStates.remove(filterId);
        }
        soloStates[filterId] = true;
        recomputeImpliedMutes();
    }
    
    refreshChain();
    emit chainModified();
}

QSet<int> FilterChainWidget::computeSoloProtectedFilters() const {
    QSet<int> protectedIds;
    
    if (soloStates.isEmpty()) return protectedIds;
    
    for (auto it = soloStates.begin(); it != soloStates.end(); ++it) {
        if (it.value()) {
            protectedIds.insert(it.key());
        }
    }
    
    return protectedIds;
}

void FilterChainWidget::recomputeImpliedMutes() {
    impliedMuteSet.clear();
    
    if (soloStates.isEmpty()) return;
    
    QSet<int> protectedIds = computeSoloProtectedFilters();
    
    for (int i = 0; i < filterChain->filterCount(); ++i) {
        auto filter = filterChain->getFilter(i);
        if (filter) {
            int filterId = filter->getFilterId();
            if (!protectedIds.contains(filterId)) {
                impliedMuteSet.insert(filterId);
            }
        }
    }
}

void FilterChainWidget::handleCustomOutputToggle(int position) {
    auto filter = filterChain->getFilter(position);
    if (!filter) return;
    
    bool currentlyCustom = filter->usesCustomOutputStream();
    filter->setCustomOutputStream(!currentlyCustom);
    
    refreshChain();
    emit chainModified();
}

// ========== Filter ID-based handlers ==========

void FilterChainWidget::handleMuteToggleById(int filterId) {
    bool isExplicitMute = explicitMuteStates.value(filterId, false);
    bool isSoloed = soloStates.value(filterId, false);
    bool isImpliedMute = impliedMuteSet.contains(filterId);
    
    if (isSoloed) {
        soloStates.remove(filterId);
        explicitMuteStates[filterId] = true;
        recomputeSubChainImpliedMutes();
    } else if (isExplicitMute) {
        explicitMuteStates.remove(filterId);
        recomputeSubChainImpliedMutes();
    } else if (isImpliedMute) {
        soloStates[filterId] = true;
        recomputeSubChainImpliedMutes();
    } else {
        explicitMuteStates[filterId] = true;
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::handleSoloToggleById(int filterId) {
    bool isExplicitMute = explicitMuteStates.value(filterId, false);
    bool isSoloed = soloStates.value(filterId, false);
    
    if (isSoloed) {
        soloStates.remove(filterId);
        recomputeSubChainImpliedMutes();
    } else {
        if (isExplicitMute) {
            explicitMuteStates.remove(filterId);
        }
        soloStates[filterId] = true;
        recomputeSubChainImpliedMutes();
    }
    
    refreshChain();
    emit chainModified();
}

void FilterChainWidget::handleCustomOutputToggleById(int filterId) {
    if (currentViewMode != StreamView || currentMultiOutputPos < 0) return;
    
    auto* multiOutput = filterChain->getMultiOutputFilter(currentMultiOutputPos);
    if (!multiOutput) return;
    
    auto& subChain = multiOutput->getSubChain(currentStreamIndex);
    for (auto& filter : subChain) {
        if (filter && filter->getFilterId() == filterId) {
            bool currentlyCustom = filter->usesCustomOutputStream();
            filter->setCustomOutputStream(!currentlyCustom);
            break;
        }
    }
    
    refreshChain();
    emit chainModified();
}

QSet<int> FilterChainWidget::getSubChainFilterIds() const {
    QSet<int> ids;
    
    if (currentViewMode != StreamView || currentMultiOutputPos < 0) return ids;
    
    auto* multiOutput = filterChain->getMultiOutputFilter(currentMultiOutputPos);
    if (!multiOutput) return ids;
    
    auto& subChain = multiOutput->getSubChain(currentStreamIndex);
    for (const auto& filter : subChain) {
        if (filter) {
            ids.insert(filter->getFilterId());
        }
    }
    
    return ids;
}

bool FilterChainWidget::isAnySoloActiveInSubChain() const {
    if (currentViewMode != StreamView) return isAnySoloActive();
    
    QSet<int> subChainIds = getSubChainFilterIds();
    
    for (auto it = soloStates.begin(); it != soloStates.end(); ++it) {
        if (it.value() && subChainIds.contains(it.key())) {
            return true;
        }
    }
    return false;
}

void FilterChainWidget::recomputeSubChainImpliedMutes() {
    if (currentViewMode != StreamView) {
        recomputeImpliedMutes();
        return;
    }
    
    QSet<int> subChainIds = getSubChainFilterIds();
    
    for (int id : subChainIds) {
        impliedMuteSet.remove(id);
    }
    
    QSet<int> soloedInSubChain;
    for (int id : subChainIds) {
        if (soloStates.value(id, false)) {
            soloedInSubChain.insert(id);
        }
    }
    
    if (soloedInSubChain.isEmpty()) return;
    
    for (int id : subChainIds) {
        if (!soloedInSubChain.contains(id)) {
            impliedMuteSet.insert(id);
        }
    }
}

QString FilterChainWidget::getSubChainSignalFlowLabel(int multiOutputPos, int streamIndex, int indexInSubChain) const {
    auto* multiOutput = filterChain->getMultiOutputFilter(multiOutputPos);
    if (!multiOutput) return "";
    
    auto& subChain = multiOutput->getSubChain(streamIndex);
    if (indexInSubChain < 0 || indexInSubChain >= static_cast<int>(subChain.size())) return "";
    
    auto targetFilter = subChain[indexInSubChain];
    if (!targetFilter) return "";
    
    if (auto* audioInput = dynamic_cast<AudioInputFilter*>(targetFilter.get())) {
        return QString("→ [%1:a]").arg(audioInput->getInputIndex());
    }
    
    if (auto* afir = dynamic_cast<FFAfir*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(afir->getSidechainInputIndex());
    }
    if (auto* sc = dynamic_cast<FFSidechaincompress*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(sc->getSidechainInputIndex());
    }
    if (auto* sg = dynamic_cast<FFSidechaingate*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(sg->getSidechainInputIndex());
    }
    if (auto* acf = dynamic_cast<FFAcrossfade*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(acf->getSidechainInputIndex());
    }
    if (targetFilter->isAnalysisTwoInputFilter()) {
        // Find the most recent AudioInputFilter in main chain before the asplit
        int scIndex = 1;
        for (int k = multiOutputPos - 1; k >= 1; k--) {
            auto mainFilter = filterChain->getFilter(k);
            if (auto* ai = dynamic_cast<AudioInputFilter*>(mainFilter.get())) {
                scIndex = ai->getInputIndex();
                break;
            }
        }
        return QString("← [%1:a]").arg(scIndex);
    }
    if (dynamic_cast<FFAmix*>(targetFilter.get()) || dynamic_cast<FFAmerge*>(targetFilter.get()) || dynamic_cast<FFJoin*>(targetFilter.get())) {
        return "← [ ∀ ]";
    }
    if (dynamic_cast<SmartAuxReturn*>(targetFilter.get())) {
        return "← [∀∀∀]";
    }

    if (indexInSubChain == 0) {
        return "";
    }
    
    bool hasAudioInputBefore = false;
    for (int j = 0; j < indexInSubChain; j++) {
        if (dynamic_cast<AudioInputFilter*>(subChain[j].get())) {
            hasAudioInputBefore = true;
            break;
        }
    }
    
    if (!hasAudioInputBefore) {
        return "";
    }
    
    bool hasMultiInputAfter = false;
    for (size_t j = indexInSubChain + 1; j < subChain.size(); j++) {
        if (dynamic_cast<FFAfir*>(subChain[j].get()) ||
            dynamic_cast<FFSidechaincompress*>(subChain[j].get()) ||
            dynamic_cast<FFSidechaingate*>(subChain[j].get()) ||
            dynamic_cast<FFAcrossfade*>(subChain[j].get()) ||
            dynamic_cast<FFAmix*>(subChain[j].get()) ||
            dynamic_cast<FFAmerge*>(subChain[j].get()) ||
            dynamic_cast<SmartAuxReturn*>(subChain[j].get()) ||
            dynamic_cast<FFJoin*>(subChain[j].get()) ||
            subChain[j]->isAnalysisTwoInputFilter()) {
            hasMultiInputAfter = true;
            break;
        }
    }
    
    if (!hasMultiInputAfter) {
        return "";
    }
    
    return "↓";
}

void FilterChainWidget::onListItemClicked(const QModelIndex& index) {
    if (!index.isValid()) return;
    
    bool isArrow = index.data(Qt::UserRole + 1).toBool();
    if (isArrow) return;
    
    int position = index.data(Qt::UserRole).toInt();
    bool isSubChainFilter = index.data(Qt::UserRole + 2).toBool();
    
    if (position == -3 || position == -4) return;
    
    if (isSubChainFilter) {
        int subChainIndex = index.data(Qt::UserRole + 3).toInt();
        auto filter = filterChain->getFilterFromStream(currentMultiOutputPos, currentStreamIndex, subChainIndex);
        if (filter) {
            currentSelection = -1;
            emit subChainFilterSelected(filter->getFilterId());
        }
    } else {
        currentSelection = position;
        emit filterSelected(position);
    }
}

void FilterChainWidget::onAddFilterClicked() {
    if (addFilterMenuOpen) return;
    addFilterMenuOpen = true;

    QMenu* menu = FilterMenuBuilder::createFilterMenu(this);

    if (currentViewMode == StreamView) {
        for (QAction* action : menu->actions()) {
            QString filterType = action->data().toString();
            if (filterType == "asplit" || filterType == "channelsplit" || 
                filterType == "asegment" || filterType == "acrossover") {
                action->setEnabled(false);
                action->setToolTip("Multi-output filters cannot be nested inside sub-chains");
            }
            if (action->menu()) {
                for (QAction* subAction : action->menu()->actions()) {
                    QString subType = subAction->data().toString();
                    if (subType == "asplit" || subType == "channelsplit" || 
                        subType == "asegment" || subType == "acrossover") {
                        subAction->setEnabled(false);
                        subAction->setToolTip("Multi-output filters cannot be nested inside sub-chains");
                    }
                }
            }
        }
    }
    
    connect(menu, &QMenu::triggered, [this](QAction* action) {
        QString filterType = action->data().toString();
        
        auto filter = FilterChain::createFilterByType(filterType);
        if (filter) {
            if (currentViewMode == StreamView && currentMultiOutputPos >= 0 && currentStreamIndex > 0) {
                filterChain->addFilterToStream(currentMultiOutputPos, currentStreamIndex, filter);
                qDebug() << "Added filter to stream" << currentStreamIndex 
                         << "of multi-output at position" << currentMultiOutputPos;
            } else {
                filterChain->addFilter(filter);
            }
            
            refreshChain();
            emit chainModified();
        }
    });
    
    menu->exec(QCursor::pos());
    delete menu;
    addFilterMenuOpen = false;
}

// ========== State Query Methods ==========

bool FilterChainWidget::isFilterMuted(int position) const {
    auto filter = filterChain->getFilter(position);
    if (!filter) return false;
    int filterId = filter->getFilterId();
    return explicitMuteStates.value(filterId, false) || impliedMuteSet.contains(filterId);
}

bool FilterChainWidget::isFilterSoloed(int position) const {
    auto filter = filterChain->getFilter(position);
    if (!filter) return false;
    return soloStates.value(filter->getFilterId(), false);
}

bool FilterChainWidget::isFilterImpliedMute(int position) const {
    auto filter = filterChain->getFilter(position);
    if (!filter) return false;
    int filterId = filter->getFilterId();
    return impliedMuteSet.contains(filterId) && 
           !explicitMuteStates.value(filterId, false) &&
           !soloStates.value(filterId, false);
}

bool FilterChainWidget::isFilterExplicitMute(int position) const {
    auto filter = filterChain->getFilter(position);
    if (!filter) return false;
    return explicitMuteStates.value(filter->getFilterId(), false);
}

bool FilterChainWidget::isFilterMutedById(int filterId) const {
    return explicitMuteStates.value(filterId, false) || impliedMuteSet.contains(filterId);
}

bool FilterChainWidget::isFilterSoloedById(int filterId) const {
    return soloStates.value(filterId, false);
}

bool FilterChainWidget::isFilterImpliedMuteById(int filterId) const {
    return impliedMuteSet.contains(filterId) && 
           !explicitMuteStates.value(filterId, false) &&
           !soloStates.value(filterId, false);
}

bool FilterChainWidget::isFilterExplicitMuteById(int filterId) const {
    return explicitMuteStates.value(filterId, false);
}

bool FilterChainWidget::isAnySoloActive() const {
    for (auto it = soloStates.begin(); it != soloStates.end(); ++it) {
        if (it.value()) return true;
    }
    return false;
}

QList<int> FilterChainWidget::getMutedFilterPositions() const {
    QList<int> mutedPositions;
    for (int pos = 0; pos < filterChain->filterCount(); ++pos) {
        auto filter = filterChain->getFilter(pos);
        if (filter) {
            int filterId = filter->getFilterId();
            bool explicitMuted = explicitMuteStates.value(filterId, false);
            bool impliedMuted = impliedMuteSet.contains(filterId);
            bool soloed = soloStates.value(filterId, false);
            
            if (!soloed && (explicitMuted || impliedMuted)) {
                mutedPositions.append(pos);
            }
        }
    }
    return mutedPositions;
}

QList<int> FilterChainWidget::getMutedFilterIds() const {
    QList<int> filterIds;
    for (auto it = explicitMuteStates.begin(); it != explicitMuteStates.end(); ++it) {
        if (it.value()) {
            filterIds.append(it.key());
        }
    }
    return filterIds;
}

QList<int> FilterChainWidget::getSoloFilterIds() const {
    QList<int> filterIds;
    for (auto it = soloStates.begin(); it != soloStates.end(); ++it) {
        if (it.value()) {
            filterIds.append(it.key());
        }
    }
    return filterIds;
}

void FilterChainWidget::setMutedFilterIds(const QList<int>& filterIds) {
    explicitMuteStates.clear();
    for (int filterId : filterIds) {
        explicitMuteStates[filterId] = true;
    }
    recomputeImpliedMutes();
    refreshChain();
}

void FilterChainWidget::setSoloFilterIds(const QList<int>& filterIds) {
    soloStates.clear();
    for (int filterId : filterIds) {
        soloStates[filterId] = true;
    }
    recomputeImpliedMutes();
    refreshChain();
}

QString FilterChainWidget::getFilterInputStreamLabel(int targetPosition) const {
    if (targetPosition <= 0 || targetPosition >= filterChain->filterCount() - 1) {
        return "";
    }
    
    QString mainChainInput = "[0:a]";
    QMap<int, QString> sidechainOutputs;
    AudioInputFilter* currentAudioInput = nullptr;
    
    for (int i = 1; i < targetPosition; ++i) {
        auto filter = filterChain->getFilter(i);
        if (!filter) continue;
        
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filter.get())) {
            currentAudioInput = audioInput;
            int idx = audioInput->getInputIndex();
            sidechainOutputs[idx] = QString("[%1:a]").arg(idx);
            continue;
        }
        
        if (filter->buildFFmpegFlags().isEmpty()) continue;
        
        bool isMultiInputOrAux = (dynamic_cast<FFAfir*>(filter.get()) ||
                                  dynamic_cast<FFSidechaincompress*>(filter.get()) ||
                                  dynamic_cast<FFSidechaingate*>(filter.get()) ||
                                  dynamic_cast<FFAcrossfade*>(filter.get()) ||
                                  dynamic_cast<FFAmerge*>(filter.get()) ||
                                  dynamic_cast<FFAmix*>(filter.get()) ||
                                  dynamic_cast<FFJoin*>(filter.get()) ||
                                  dynamic_cast<SmartAuxReturn*>(filter.get()));
        
        if (isMultiInputOrAux) {
            currentAudioInput = nullptr;
            sidechainOutputs.clear();
            
            if (!filter->usesCustomOutputStream()) {
                int filterId = filter->getFilterId();
                mainChainInput = QString("[%1]").arg(
                    QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper()
                );
            }
        } else if (currentAudioInput) {
            int idx = currentAudioInput->getInputIndex();
            int filterId = filter->getFilterId();
            sidechainOutputs[idx] = QString("[%1]").arg(
                QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper()
            );
        } else {
            if (!filter->usesCustomOutputStream()) {
                int filterId = filter->getFilterId();
                mainChainInput = QString("[%1]").arg(
                    QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper()
                );
            }
        }
    }
    
    auto targetFilter = filterChain->getFilter(targetPosition);
    if (!targetFilter) return "";

    if (auto* audioInput = dynamic_cast<AudioInputFilter*>(targetFilter.get())) {
        return QString("→ [%1:a]").arg(audioInput->getInputIndex());
    }

    if (auto* afir = dynamic_cast<FFAfir*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(afir->getSidechainInputIndex());
    }
    if (auto* sc = dynamic_cast<FFSidechaincompress*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(sc->getSidechainInputIndex());
    }
    if (auto* sg = dynamic_cast<FFSidechaingate*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(sg->getSidechainInputIndex());
    }
    if (auto* acf = dynamic_cast<FFAcrossfade*>(targetFilter.get())) {
        return QString("← [%1:a]").arg(acf->getSidechainInputIndex());
    }
    if (targetFilter->isAnalysisTwoInputFilter()) {
        if (currentAudioInput) {
            return QString("← [%1:a]").arg(currentAudioInput->getInputIndex());
        }
        return "← [1:a]";
    }

    if (dynamic_cast<FFAmix*>(targetFilter.get()) || dynamic_cast<FFAmerge*>(targetFilter.get()) || dynamic_cast<FFJoin*>(targetFilter.get())) {
        return "← [ ∀ ]";
    }

    if (dynamic_cast<SmartAuxReturn*>(targetFilter.get())) {
        return "← [∀∀∀]";
    }

    if (currentAudioInput) {
        return "↓";
    } else {
        return "";
    }
}

// ========== Stream View Switching ==========

void FilterChainWidget::showStreamView(int multiOutputPos, int streamIndex) {
    if (streamIndex < 1 || streamIndex > 7) {
        qDebug() << "Invalid stream index:" << streamIndex;
        return;
    }
    
    auto* multiOutput = filterChain->getMultiOutputFilter(multiOutputPos);
    if (!multiOutput) {
        qDebug() << "No MultiOutputFilter at position" << multiOutputPos;
        return;
    }
    
    currentViewMode = StreamView;
    currentStreamIndex = streamIndex;
    currentMultiOutputPos = multiOutputPos;
    
    refreshChain();
    
    qDebug() << "Switched to stream view:" << streamIndex << "for multi-output at" << multiOutputPos;
}

void FilterChainWidget::showMainChain() {
    currentViewMode = MainChainView;
    currentStreamIndex = 0;
    currentMultiOutputPos = -1;
    
    refreshChain();
    
    qDebug() << "Switched to main chain view";
}

void FilterChainWidget::updateAsplitButtonStates() {
    for (int pos = 0; pos < filterChain->filterCount(); pos++) {
        auto* multiOutput = filterChain->getMultiOutputFilter(pos);
        if (!multiOutput) continue;
        
        for (int row = 0; row < model->rowCount(); row++) {
            QModelIndex idx = model->index(row, 0);
            int position = idx.data(Qt::UserRole).toInt();
            
            if (position == pos) {
                QWidget* widget = listView->indexWidget(idx);
                
                if (auto* asplitWidget = qobject_cast<AsplitRowWidget*>(widget)) {
                    asplitWidget->updateButtonStates();
                    asplitWidget->updateControlButtonStates();
                }
                break;
            }
        }
    }
}