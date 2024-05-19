#pragma once

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTimer>

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
QT_END_NAMESPACE

class RedmiOSD : public QDialog
{
    Q_OBJECT

public:
    RedmiOSD();

protected:
    void closeEvent(QCloseEvent *event) override;
    
    void showOSD(const QString& message);
    void updatePreset();

private slots:
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void silenceButtonClicked();
    void turboButtonClicked();
    void shortcutKeySequenceChanged(const QKeySequence& keySequence);

private:
    void createWindow();
    void createTray();

    QMap<QString, QJsonObject> readPresets(const QString& presetsPath);
    void applyPreset(const QJsonObject& args);

    QString readActivePreset(const QString& filePath);
    void writeActivePreset(const QString& filePath, const QString& presetName);

    QPushButton* silenceButton;
    QPushButton* turboButton;
    QKeySequenceEdit* shortcutKeySequence;
    QCheckBox *showTrayCheckBox;

    QTimer updateTimer;
    QSystemTrayIcon *trayIcon;
};
