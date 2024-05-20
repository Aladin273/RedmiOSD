#pragma once

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTimer>
#include <QHotkey>

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class QKeySequenceEdit;
class QSpinBox;
class QAction;
class QMenu;
class QTimer;
class QProcess;
QT_END_NAMESPACE

struct Presets
{
    QMap<QString, QStringList> argsMap;
    QMap<QString, QString> shorcutsMap;
    QString defaultPreset;
    QString lastPreset;
    int32_t updateRate;
    bool liveEdit;
    bool showTray;
    bool showOverlay;
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
    
    void defaultComboBoxChanged(const QString& text);
    void updateRateTimeSpinBoxChanged(int value);

    void liveEditCheckBoxToggled(bool checked);
    void overlayCheckBoxToggled(bool checked);
    void trayCheckBoxToggled(bool checked);

    void presetsButtonClicked();
    void silenceButtonClicked();
    void turboButtonClicked();

    void silenceKeySequenceFinished();
    void turboKeySequenceFinished();

private:
    void readPresets(const QString& filePath);
    void writePresets(const QString& filePath);
    
    void applyPreset(const QStringList& args);
    void updatePresets();

    void showOSD(const QString& message);

    void createWindow();
    void createTray();
    void createShortcuts();

    QSystemTrayIcon* m_trayIcon;
    
    QLabel* m_activeLabel;
    QComboBox* m_defaultComboBox;
    
    QSpinBox* m_updateRateSpinBox;
    QCheckBox* m_liveEditCheckBox;
    QCheckBox* m_overlayCheckBox;
    QCheckBox* m_trayCheckBox;

    QPushButton* m_presetsButton;

    QPushButton* m_silenceButton;
    QPushButton* m_turboButton;
    
    QKeySequenceEdit* m_silenceKeySequence;
    QKeySequenceEdit* m_turboKeySequence;
    
    QHotkey m_silenceShortcut;
    QHotkey m_turboShortcut;

    QTimer m_updateTimer;

    Presets m_presets;
    QString m_filePath = "Presets.json";
};
