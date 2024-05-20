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
    bool showTray;
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
    void trayCheckBoxToggled(bool checked);
    
    void silenceButtonClicked();
    void turboButtonClicked();

    void silenceKeySequenceChanged();
    void turboKeySequenceChanged();

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
    QCheckBox* m_trayCheckBox;

    QPushButton* m_silenceButton;
    QPushButton* m_turboButton;
    
    QKeySequenceEdit* m_silenceKeySequence;
    QKeySequenceEdit* m_turboKeySequence;
    
    QHotkey m_silenceShortcut;
    QHotkey m_turboShortcut;

    Presets m_presets;
    QString m_filePath = "Presets.json";
    
    QTimer m_updateTimer;
    int32_t m_updateRate = 5000;
};
