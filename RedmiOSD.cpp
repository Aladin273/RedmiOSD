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
#include <QDateTime>

RedmiOSD::RedmiOSD()
{
    m_presets = readPresets(m_filePath);

    if (m_presets.defaultPreset != "lastPreset")
    {
        m_presets.lastPreset = m_presets.defaultPreset;
        writePresets(m_filePath);
    }

    QString presetName = m_presets.lastPreset;
    presetName.front() = presetName.front().toUpper();

    applyPreset(m_presets.argsMap[m_presets.lastPreset]);
    showOSD(presetName);

    createWindow();
    createTray();

    setWindowIcon(QIcon("Resources/Main.png"));
    setWindowTitle("RedmiOSD");
    setFixedSize(400, 100);

    m_trayIcon->setIcon(QIcon(QString("Resources/%1.png").arg(presetName)));
    m_trayIcon->setToolTip(presetName);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &RedmiOSD::trayActivated);
    connect(m_silenceButton, &QPushButton::clicked, this, &RedmiOSD::silenceButtonClicked);
    connect(m_turboButton, &QPushButton::clicked, this, &RedmiOSD::turboButtonClicked);
    connect(m_defaultComboBox, &QComboBox::currentTextChanged, this, &RedmiOSD::defaultComboBoxChanged);
    connect(m_shortcutKeySequence, &QKeySequenceEdit::keySequenceChanged, this, &RedmiOSD::shortcutKeySequenceChanged);
    connect(m_trayCheckBox, &QAbstractButton::toggled, this, &RedmiOSD::trayCheckBoxToggled);
    connect(&m_updateTimer, &QTimer::timeout, this, &RedmiOSD::updatePresets);

    m_updateTimer.start(m_updateRate);
}

void RedmiOSD::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible())
        return;
    
    if (m_trayIcon->isVisible()) 
    {
        hide();
        event->ignore();
    }
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
    m_trayIcon->setIcon(QIcon("Resources/Silence.png"));
    m_trayIcon->setToolTip("Silence");

    m_activeLabel->setText("Silence");

    m_presets.lastPreset = "silence";
    writePresets(m_filePath);

    applyPreset(m_presets.argsMap["silence"]);
    showOSD("Silence");
}

void RedmiOSD::turboButtonClicked()
{
    m_trayIcon->setIcon(QIcon("Resources/Turbo.png"));
    m_trayIcon->setToolTip("Turbo");

    m_activeLabel->setText("Turbo");

    m_presets.lastPreset = "turbo";
    writePresets(m_filePath);

    applyPreset(m_presets.argsMap["turbo"]);
    showOSD("Turbo");
}

void RedmiOSD::defaultComboBoxChanged(const QString& text)
{
    QString presetName = text;
    presetName.front() = presetName.front().toLower();

    m_presets.defaultPreset = presetName;
    writePresets(m_filePath);
}

void RedmiOSD::shortcutKeySequenceChanged(const QKeySequence& keySequence)
{
    m_presets.shortCut = keySequence.toString();
    writePresets(m_filePath);
}

void RedmiOSD::trayCheckBoxToggled(bool checked)
{
    m_presets.showTray = checked;
    writePresets(m_filePath);

    m_trayIcon->setVisible(checked);
}

Presets RedmiOSD::readPresets(const QString& filePath)
{
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

    if (!rootObject.contains("defaultPreset"))
    {
        qDebug() << "Default preset not found in JSON.";
        return {};
    }

    if (!rootObject.contains("lastPreset"))
    {
        qDebug() << "Last preset not found in JSON.";
        return {};
    }

    if (!rootObject.contains("presets"))
    {
        qDebug() << "Presets array not found in JSON.";
        return {};
    }
    
    Presets presets;

    presets.defaultPreset = rootObject["defaultPreset"].toString();
    presets.lastPreset = rootObject["lastPreset"].toString();
    presets.showTray = rootObject["showTray"].toBool();
    presets.shortCut = rootObject["shortCut"].toString();

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

        QStringList argsList;

        for (auto it = argsObject.constBegin(); it != argsObject.constEnd(); ++it)
            argsList << QString("--%1=%2").arg(it.key()).arg(it.value().toInt());
        
        presets.argsMap.insert(presetName, argsList);
    }

    return presets;
}

void RedmiOSD::writePresets(const QString& filePath)
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

    rootObject["defaultPreset"] = m_presets.defaultPreset;
    rootObject["lastPreset"] = m_presets.lastPreset;
    rootObject["showTray"] = m_presets.showTray;
    rootObject["shortCut"] = m_presets.shortCut;

    jsonDoc.setObject(rootObject);

    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    if (!file.isOpen())
    {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    file.write(jsonDoc.toJson());
    file.close();

    qDebug() << "File updated successfully:" << filePath;
}

void RedmiOSD::applyPreset(const QStringList& args)
{
    qDebug() << "\nApplied with ryzenadj at" << QDateTime::currentDateTime().toString();
    
    for (const auto& arg : args)
        qDebug() << arg;
    
    QProcess* process = new QProcess();
    process->start("Tools/ryzenadj.exe", args);

    connect(process, &QProcess::finished, process, &QProcess::deleteLater);
}

void RedmiOSD::updatePresets()
{
    m_presets = readPresets(m_filePath);
    applyPreset(m_presets.argsMap[m_presets.lastPreset]);
}

void RedmiOSD::showOSD(const QString& message)
{
    QDialog* dialog = new QDialog();
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setAttribute(Qt::WA_TransparentForMouseEvents);
    dialog->setFixedSize(200, 200);

    QLabel* label = new QLabel(message, dialog);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24pt;");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    dialog->setLayout(layout);

    dialog->setStyleSheet("background-color: rgba(0, 0, 0, 150); color: white;");

    dialog->show();
    QTimer::singleShot(1000, dialog, &QDialog::deleteLater);
}

void RedmiOSD::createWindow()
{
    QString lastPresetName = m_presets.lastPreset;
    QString defaultPresetName = m_presets.defaultPreset;
    lastPresetName.front() = lastPresetName.front().toUpper();
    defaultPresetName.front() = defaultPresetName.front().toUpper();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout* horizontalLayout1 = new QHBoxLayout;
    QHBoxLayout* horizontalLayout2 = new QHBoxLayout;

    m_activeLabel = new QLabel(lastPresetName);
    m_activeLabel->setStyleSheet("font-weight: bold;");

    m_defaultComboBox = new QComboBox();
    m_defaultComboBox->addItem("Silence");
    m_defaultComboBox->addItem("Turbo");
    m_defaultComboBox->addItem("LastPreset");
    m_defaultComboBox->setCurrentText(defaultPresetName);

    m_trayCheckBox = new QCheckBox("Show Tray");
    m_trayCheckBox->setChecked(m_presets.showTray);

    m_silenceButton = new QPushButton("Silence");
    m_turboButton = new QPushButton("Turbo");

    m_shortcutKeySequence = new QKeySequenceEdit();
    m_shortcutKeySequence->setKeySequence(m_presets.shortCut);

    horizontalLayout1->addWidget(new QLabel("Active Preset :"));
    horizontalLayout1->addWidget(m_activeLabel);
    horizontalLayout1->addWidget(new QLabel("Default Preset :"));
    horizontalLayout1->addWidget(m_defaultComboBox);
    horizontalLayout1->addWidget(m_trayCheckBox);

    horizontalLayout2->addWidget(m_silenceButton);
    horizontalLayout2->addWidget(m_turboButton);
    horizontalLayout2->addWidget(m_shortcutKeySequence);

    mainLayout->addLayout(horizontalLayout1);
    mainLayout->addLayout(horizontalLayout2);
}

void RedmiOSD::createTray()
{
    QAction* settingsAction = new QAction(QIcon("Resources/Main.png"), "Settings", this);
    connect(settingsAction, &QAction::triggered, this, &QWidget::show);

    QAction* quitAction = new QAction(QIcon("Resources/Quit.png"), "Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(trayIconMenu);
}