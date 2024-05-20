#pragma once

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTimer>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QKeySequenceEdit;
class QVBoxLayout;
class QTimer;
class QProcess;
QT_END_NAMESPACE

struct Presets
{
    QMap<QString, QStringList> argsMap;
    QString defaultPreset;
    QString lastPreset;
    bool showTray;
    QString shortCut;
};

class RedmiOSD : public QDialog
{
    Q_OBJECT

public:
    RedmiOSD();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

    void silenceButtonClicked();
    void turboButtonClicked();
    void defaultComboBoxChanged(const QString& text);
    void shortcutKeySequenceChanged(const QKeySequence& keySequence);
    void trayCheckBoxToggled(bool checked);

private:
    Presets readPresets(const QString& filePath);
    void writePresets(const QString& filePath);
    
    void applyPreset(const QStringList& args);
    void updatePresets();

    void showOSD(const QString& message);

    void createWindow();
    void createTray();

    QSystemTrayIcon* m_trayIcon;

    QLabel* m_activeLabel;
    QPushButton* m_silenceButton;
    QPushButton* m_turboButton;
    QComboBox* m_defaultComboBox;
    QKeySequenceEdit* m_shortcutKeySequence;
    QCheckBox* m_trayCheckBox;

    Presets m_presets;
    QString m_filePath = "Presets.json";
    
    QTimer m_updateTimer;
    int32_t m_updateRate = 5000;
};
