#include "RedmiOSD.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QProcess>
#include <QStringList>
#include <QKeySequenceEdit>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

RedmiOSD::RedmiOSD()
{
    createWindow();
    createTray();

    connect(silenceButton, &QPushButton::clicked, this, &RedmiOSD::silenceButtonClicked);
    connect(turboButton, &QPushButton::clicked, this, &RedmiOSD::turboButtonClicked);
    connect(shortcutKeySequence, &QKeySequenceEdit::keySequenceChanged, this, &RedmiOSD::shortcutKeySequenceChanged);
    connect(showTrayCheckBox, &QAbstractButton::toggled, trayIcon, &QSystemTrayIcon::setVisible);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &RedmiOSD::trayActivated);

    setWindowIcon(QIcon(":/Resources/main.png"));
    setWindowTitle("RedmiOSD");
    setFixedSize(400, 100);

    auto presets = readPresets("Presets.json");
    auto activePreset = readActivePreset("Presets.json");
    applyPreset(presets[activePreset]);
    showOSD(activePreset.toUpper());

    QIcon icon(QString(":/Resources/%1.png").arg(activePreset));
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(activePreset.toUpper());
    trayIcon->show();

    connect(&updateTimer, &QTimer::timeout, this, &RedmiOSD::updatePreset);
    updateTimer.start(1000);
}

void RedmiOSD::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible())
        return;
    
    if (trayIcon->isVisible()) 
    {
        hide();
        event->ignore();
    }
}

void RedmiOSD::showOSD(const QString& message)
{
    QDialog* dialog = new QDialog();
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setAttribute(Qt::WA_TransparentForMouseEvents);
    dialog->setFixedSize(200, 200);

    QLabel* label = new QLabel(message);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24pt;");

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);
    dialog->setLayout(layout);

    dialog->setStyleSheet("background-color: rgba(0, 0, 0, 150); color: white;");

    dialog->show();
    QTimer::singleShot(1000, dialog, &QDialog::close);
}

void RedmiOSD::updatePreset()
{
    auto presets = readPresets("Presets.json");
    applyPreset(presets[readActivePreset("Presets.json")]);
}

void RedmiOSD::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) 
    {
        case QSystemTrayIcon::Trigger:
            break;
        case QSystemTrayIcon::DoubleClick:
            setVisible(!isVisible());
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
            break;
    }
}

void RedmiOSD::silenceButtonClicked()
{
    const auto presets = readPresets("Presets.json");
    applyPreset(presets["silence"]);
    writeActivePreset("Presets.json", "silence");
    showOSD("SILENCE");

    QIcon icon(":/Resources/silence.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("SILENCE");
}

void RedmiOSD::turboButtonClicked()
{
    const auto presets = readPresets("Presets.json");
    applyPreset(presets["turbo"]);
    writeActivePreset("Presets.json", "turbo");
    showOSD("TURBO");

    QIcon icon(":/Resources/turbo.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("TURBO");
}

void RedmiOSD::shortcutKeySequenceChanged(const QKeySequence& keySequence)
{
    // TODO
}

void RedmiOSD::createWindow()
{   
    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;

    silenceButton = new QPushButton("Silence");
    turboButton = new QPushButton("Turbo");
    
    shortcutKeySequence = new QKeySequenceEdit();
    //shortcutKeySequence->setKeySequence(Qt::Key::Key_Delete);
    
    showTrayCheckBox = new QCheckBox("Show Tray");
    showTrayCheckBox->setChecked(true);

    horizontalLayout->addWidget(silenceButton);
    horizontalLayout->addWidget(turboButton);
    horizontalLayout->addWidget(shortcutKeySequence);
    horizontalLayout->addWidget(showTrayCheckBox);
    
    mainLayout->addLayout(horizontalLayout);
}

void RedmiOSD::createTray()
{
    QAction* settingsAction = new QAction("Settings", this);
    connect(settingsAction, &QAction::triggered, this, &QWidget::showNormal);

    QAction*  quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

QMap<QString, QJsonObject> RedmiOSD::readPresets(const QString& filePath)
{
    QMap<QString, QJsonObject> presetsMap;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        qDebug() << "Failed to open file:" << file.errorString();
        return {};
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) 
    {
        qDebug() << "Invalid JSON data.";
        return {};
    }

    QJsonObject rootObject = jsonDoc.object();
    if (!rootObject.contains("presets")) 
    {
        qDebug() << "Presets array not found in JSON.";
        return {};
    }

    QJsonArray presetsArray = rootObject["presets"].toArray();
    for (const QJsonValue& presetValue : presetsArray) 
    {
        if (!presetValue.isObject()) 
        {
            qDebug() << "Invalid preset format.";
            continue;
        }

        QJsonObject presetObject = presetValue.toObject();
        QString presetName = presetObject["name"].toString();
        QJsonObject argsObject = presetObject["args"].toObject();
        
        presetsMap.insert(presetName, argsObject);
    }

    return presetsMap;
}

void RedmiOSD::applyPreset(const QJsonObject& args) 
{
    qDebug() << "\nApplied with ryzenadj :";

    QStringList argsList;
    for (auto it = args.constBegin(); it != args.constEnd(); ++it) 
    {
        qDebug() << it.key() << " : " << it.value().toInt();
        argsList << QString("--%1=%2").arg(it.key()).arg(it.value().toInt());
    }

    QProcess* process = new QProcess();

    QObject::connect(process, &QProcess::finished, process, &QProcess::deleteLater);
    process->start("Tools/ryzenadj.exe", argsList);
}

QString RedmiOSD::readActivePreset(const QString& filePath) 
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        qDebug() << "Failed to open file:" << file.errorString();
        return "";
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) 
    {
        qDebug() << "Invalid JSON data.";
        return "";
    }

    QJsonObject rootObject = jsonDoc.object();
    if (!rootObject.contains("active")) 
    {
        qDebug() << "Active preset not found in JSON.";
        return "";
    }

    return rootObject["active"].toString();
}

void RedmiOSD::writeActivePreset(const QString& filePath, const QString& presetName) 
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) 
    {
        qDebug() << "Failed to open file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) 
    {
        qDebug() << "Invalid JSON data.";
        return;
    }

    QJsonObject rootObject = jsonDoc.object();
    rootObject["active"] = presetName;

    jsonDoc.setObject(rootObject);

    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    if (!file.isOpen())
    {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    qint64 bytesWritten = file.write(jsonDoc.toJson());
    if (bytesWritten == -1)
    {
        qDebug() << "Failed to write JSON data to file:" << file.errorString();
        return;
    }

    qDebug() << "Active preset updated successfully:" << presetName;
    file.close();
}
