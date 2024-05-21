#pragma once

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTimer>
#include <QHotkey>

#include <ryzenadj.h>

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
    QMap<QString, QMap<QString, int32_t>> argsMap;
    QMap<QString, QString> shorcutsMap;
    QString defaultPreset;
    QString lastPreset;
    int32_t updateRate;
    bool startup;
    bool liveEdit;
    bool showTray;
    bool showOverlay;
};

class RedmiOSD : public QDialog
{
    Q_OBJECT

public:
    RedmiOSD();
    virtual ~RedmiOSD();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    
    void defaultComboBoxChanged(const QString& text);
    void updateRateTimeSpinBoxChanged(int value);

    void startupCheckBoxToggled(bool checked);
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
    
    void initPreset();
    void applyPreset(const QMap<QString, int32_t>& args);
    void applyStartup(bool enable);
    void showOSD(const QString& message);
    
    void updatePreset();
    void updateLiveEdit();

    void createWindow();
    void createTray();
    void createShortcuts();

    QString formatToUpper(const QString& text);
    QString formatToLower(const QString& text);

    QSystemTrayIcon* m_trayIcon;
    
    QLabel* m_activeLabel;

    QComboBox* m_defaultComboBox;
    QSpinBox* m_updateRateSpinBox;
    
    QCheckBox* m_startupCheckBox;
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

    QTimer m_updatePresetTimer;
    QTimer m_updateLiveEditTimer;

    Presets m_presets;
    QString m_filePath = "Presets.json";
};
