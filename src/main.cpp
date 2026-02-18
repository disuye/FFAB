#include <QApplication>
#include <QFontDatabase>
#include "UI/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("FFAB");
    app.setApplicationVersion(VERSION_STR);
    app.setApplicationDisplayName("FFmpeg Audio Batch");

    // Register embedded Fira Code font with Qt
    QFontDatabase::addApplicationFont(":/fonts/FiraCode-VariableFont_wght.ttf");

     // Global Fira Code styling
    QString styleSheet =
        "QWidget {"
        "    font-family: 'Fira Code';"
        "    font-size: 12px;"
        "    font-weight: 300;"
        "}"
        "QPushButton {"
        "    font-family: 'Fira Code';"
        "    font-size: 11px;"
        "    font-weight: 300;"
        "}"
        "QListView {"
        "    border: none;"
        "}"
        "QListView::item:hover {"
        "    background-color: transparent;"
        "}"
        "QListView::item:selected {"
        "    background-color: rgba(128, 128, 128, 0.1);"
        "}"
        "QScrollBar:vertical {"
        "    width: 4px;"
        "    background: transparent;"
        "}"
        "QScrollBar:horizontal {"
        "    height: 4px;"
        "    background: transparent;"
        "}"        
        "QScrollBar::handle:vertical,"
        "QScrollBar::handle:horizontal {"
        "    background: rgba(128, 128, 128, 0.2);"
        "    border-radius: 2px;"
        "}"
        "QScrollBar::handle:vertical:hover,"
        "QScrollBar::handle:horizontal:hover {"        
        "    background: rgba(128, 128, 128, 0.6);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    height: 0px;"
        "}"        
        "QSlider::groove:vertical {"
        "   background-image: url(:/images/track-v.png);"
        "   background-position: center;"
        "   background-repeat: repeat-y;"
        "   width: 40px;"
        "   margin: 20px 20px;"
        "}"
        "QSlider::groove:horizontal {"
        "   background-image: url(:/images/track-h.png);"
        "   background-position: center;"
        "   background-repeat: repeat-x;"
        "   height: 40px;"
        "   margin: 20px 20px;"
        "}"
        "QSlider::handle:vertical {"
        "   image: url(:/images/fader-v.png);"
        "   width: 40px;"
        "   margin: -20px 0px;"
        "}"
        "QSlider::handle:horizontal {"
        "   image: url(:/images/fader-h.png);"
        "   height: 40px;"
        "   margin: 0 -20px;"
        "}"
        "#filterParamsPaneTarget QLabel {"
        "   min-width: 44px;"
        "}"
        "#filterParamsPaneTarget QDoubleSpinBox,"
        "#filterParamsPaneTarget QComboBox,"
        "#auxOutputFilterTarget QComboBox,"
        "#outputFilterTarget QComboBox,"        
        "#filterParamsPaneTarget QSpinBox,"        
        "#ffAfirTarget QDoubleSpinBox,"
        "#ffAsplitTarget QSpinBox {"
        "   min-width: 88px;"
        "   max-width: 300px;"
        "}"
        "#superEqualizerTarget QDoubleSpinBox {"
        "   min-width: 44px;"
        "   min-width: 44px;"        
        "   max-width: 44px;"
        "}";
        app.setStyleSheet(styleSheet);

    MainWindow window;
    window.show();
    
    return app.exec();
}
