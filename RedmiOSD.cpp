#include "RedmiOSD.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
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
#include <QDesktopServices>
#include <QSettings>

RedmiOSD::RedmiOSD()
{
    readPresets(m_filePath);
    
    initPreset();

    createWindow();
    createTray();
    createShortcuts();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &RedmiOSD::trayActivated);
    
    connect(m_defaultComboBox, &QComboBox::currentTextChanged, this, &RedmiOSD::defaultComboBoxChanged);
    connect(m_updateRateSpinBox, &QSpinBox::valueChanged, this, &RedmiOSD::updateRateTimeSpinBoxChanged);

    connect(m_startupCheckBox, &QAbstractButton::toggled, this, &RedmiOSD::startupCheckBoxToggled);
    connect(m_liveEditCheckBox, &QAbstractButton::toggled, this, &RedmiOSD::liveEditCheckBoxToggled);
    connect(m_overlayCheckBox, &QAbstractButton::toggled, this, &RedmiOSD::overlayCheckBoxToggled);
    connect(m_trayCheckBox, &QAbstractButton::toggled, this, &RedmiOSD::trayCheckBoxToggled);
    
    connect(m_presetsButton, &QPushButton::clicked, this, &RedmiOSD::presetsButtonClicked);
    connect(m_silenceButton, &QPushButton::clicked, this, &RedmiOSD::silenceButtonClicked);
    connect(m_turboButton, &QPushButton::clicked, this, &RedmiOSD::turboButtonClicked);

    connect(m_silenceKeySequence, &QKeySequenceEdit::editingFinished, this, &RedmiOSD::silenceKeySequenceFinished);
    connect(m_turboKeySequence, &QKeySequenceEdit::editingFinished, this, &RedmiOSD::turboKeySequenceFinished);

    connect(&m_silenceShortcut, &QHotkey::activated, this, &RedmiOSD::silenceButtonClicked);
    connect(&m_turboShortcut, &QHotkey::activated, this, &RedmiOSD::turboButtonClicked);
    
    connect(&m_updatePresetTimer, &QTimer::timeout, this, &RedmiOSD::updatePreset);
    connect(&m_updateLiveEditTimer, &QTimer::timeout, this, &RedmiOSD::updateLiveEdit);

    applyPreset(m_presets.argsMap[m_presets.lastPreset]);
    applyStartup(m_presets.startup);

    if (m_presets.showOverlay)
        showOSD(formatToUpper(m_presets.lastPreset));

    if (m_presets.showTray)
        m_trayIcon->show();

    if (m_presets.liveEdit) 
        m_updateLiveEditTimer.start(1000);
    
    m_updatePresetTimer.start(m_presets.updateRate);
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

void RedmiOSD::defaultComboBoxChanged(const QString& text)
{
    m_presets.defaultPreset = formatToLower(text);
    writePresets(m_filePath);
}

void RedmiOSD::updateRateTimeSpinBoxChanged(int value)
{
    m_presets.updateRate = value;
    writePresets(m_filePath);

    m_updatePresetTimer.stop();
    m_updatePresetTimer.start(m_presets.updateRate);
}

void RedmiOSD::startupCheckBoxToggled(bool checked)
{
    m_presets.startup = checked;
    writePresets(m_filePath);

    applyStartup(checked);
}

void RedmiOSD::liveEditCheckBoxToggled(bool checked)
{
    readPresets(m_filePath);
    
    m_presets.liveEdit = checked;
    writePresets(m_filePath);

    if (checked)
        m_updateLiveEditTimer.start(1000);
    else
        m_updateLiveEditTimer.stop();
}

void RedmiOSD::overlayCheckBoxToggled(bool checked)
{
    m_presets.showOverlay = checked;
    writePresets(m_filePath);
}

void RedmiOSD::trayCheckBoxToggled(bool checked)
{
    m_presets.showTray = checked;
    writePresets(m_filePath);

    m_trayIcon->setIcon(QIcon(QString("Resources/%1.png").arg(formatToUpper(m_presets.lastPreset))));
    m_trayIcon->setToolTip(formatToUpper(m_presets.lastPreset));
    m_trayIcon->setVisible(checked);
}

void RedmiOSD::presetsButtonClicked()
{
    QDesktopServices::openUrl(QUrl(m_filePath));
}

void RedmiOSD::silenceButtonClicked()
{
    m_presets.lastPreset = "silence";
    writePresets(m_filePath);

    applyPreset(m_presets.argsMap["silence"]);

    if (m_presets.showOverlay)
        showOSD("Silence");

    m_activeLabel->setText("Silence");

    m_trayIcon->setIcon(QIcon("Resources/Silence.png"));
    m_trayIcon->setToolTip("Silence");
}

void RedmiOSD::turboButtonClicked()
{
    m_presets.lastPreset = "turbo";
    writePresets(m_filePath);

    applyPreset(m_presets.argsMap["turbo"]);

    if (m_presets.showOverlay)
        showOSD("Turbo");

    m_activeLabel->setText("Turbo");

    m_trayIcon->setIcon(QIcon("Resources/Turbo.png"));
    m_trayIcon->setToolTip("Turbo");
}

void RedmiOSD::silenceKeySequenceFinished()
{
    m_presets.shorcutsMap["silence"] = m_silenceKeySequence->keySequence().toString();
    writePresets(m_filePath);
    
    m_silenceShortcut.setShortcut(QKeySequence(m_presets.shorcutsMap["silence"]), true);
    m_silenceKeySequence->clearFocus();
}

void RedmiOSD::turboKeySequenceFinished()
{
    m_presets.shorcutsMap["turbo"] = m_turboKeySequence->keySequence().toString();
    writePresets(m_filePath);

    m_turboShortcut.setShortcut(QKeySequence(m_presets.shorcutsMap["turbo"]), true);
    m_turboKeySequence->clearFocus();
}

void RedmiOSD::readPresets(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
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

    if (!rootObject.contains("defaultPreset"))
    {
        qDebug() << "Default preset not found in JSON.";
        return;
    }

    if (!rootObject.contains("lastPreset"))
    {
        qDebug() << "Last preset not found in JSON.";
        return;
    }

    if (!rootObject.contains("presets"))
    {
        qDebug() << "Presets array not found in JSON.";
        return;
    }

    m_presets.defaultPreset = rootObject["defaultPreset"].toString();
    m_presets.lastPreset = rootObject["lastPreset"].toString();
    m_presets.updateRate = rootObject["updateRate"].toInt();
    m_presets.startup = rootObject["startup"].toBool();
    m_presets.liveEdit = rootObject["liveEdit"].toBool();
    m_presets.showTray = rootObject["showTray"].toBool();
    m_presets.showOverlay = rootObject["showOverlay"].toBool();

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
        QString shortcut = presetObject["shortcut"].toString();
        
        QJsonObject argsObject = presetObject["args"].toObject();
        QStringList argsList;

        for (auto it = argsObject.constBegin(); it != argsObject.constEnd(); ++it)
            argsList << QString("--%1=%2").arg(it.key()).arg(it.value().toInt());
        
        m_presets.argsMap.insert(presetName, argsList);
        m_presets.shorcutsMap.insert(presetName, shortcut);
    }

    qDebug() << "File read successfully:" << filePath;
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

    QJsonArray presetsArray = rootObject["presets"].toArray();
    for (QJsonValueRef presetValue : presetsArray)
    {
        if (!presetValue.isObject())
        {
            qDebug() << "Invalid preset format.";
            continue;
        }

        QJsonObject presetObject = presetValue.toObject();
        QString presetName = presetObject["name"].toString();
        
        presetObject["shortcut"] = m_presets.shorcutsMap[presetName];
        presetValue = presetObject;
    }

    rootObject["presets"] = presetsArray;
    rootObject["defaultPreset"] = m_presets.defaultPreset;
    rootObject["lastPreset"] = m_presets.lastPreset;
    rootObject["updateRate"] = m_presets.updateRate;
    rootObject["startup"] = m_presets.startup;
    rootObject["liveEdit"] = m_presets.liveEdit;
    rootObject["showTray"] = m_presets.showTray;
    rootObject["showOverlay"] = m_presets.showOverlay;

    jsonDoc.setObject(rootObject);

    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    if (!file.isOpen())
    {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    file.write(jsonDoc.toJson());
    file.close();

    qDebug() << "File write successfully:" << filePath;
}

void RedmiOSD::initPreset()
{
    if (m_presets.defaultPreset != m_presets.lastPreset && m_presets.defaultPreset != "lastPreset")
    {
        m_presets.lastPreset = m_presets.defaultPreset;
        writePresets(m_filePath);
    }
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

void RedmiOSD::applyStartup(bool enable)
{
    if (enable)
    {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.setValue("RedmiOSD", QCoreApplication::applicationFilePath());
    }
    else
    {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.remove("RedmiOSD");
    }
}

void RedmiOSD::showOSD(const QString& message)
{
    QDialog* dialog = new QDialog();
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    dialog->setAttribute(Qt::WA_TransparentForMouseEvents);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
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

void RedmiOSD::updatePreset()
{
    applyPreset(m_presets.argsMap[m_presets.lastPreset]);
}

void RedmiOSD::updateLiveEdit()
{
    readPresets(m_filePath);

    m_activeLabel->setText(formatToUpper(m_presets.lastPreset));
    m_defaultComboBox->setCurrentText(formatToUpper(m_presets.defaultPreset));
    m_updateRateSpinBox->setValue(m_presets.updateRate);
    m_startupCheckBox->setChecked(m_presets.startup);
    m_liveEditCheckBox->setChecked(m_presets.liveEdit);
    m_overlayCheckBox->setChecked(m_presets.showOverlay);
    m_trayCheckBox->setChecked(m_presets.showTray);

    m_silenceKeySequence->setKeySequence(m_presets.shorcutsMap["silence"]);
    m_turboKeySequence->setKeySequence(m_presets.shorcutsMap["turbo"]);

    m_trayIcon->setIcon(QIcon(QString("Resources/%1.png").arg(formatToUpper(m_presets.lastPreset))));
    m_trayIcon->setToolTip(formatToUpper(m_presets.lastPreset));
    m_trayIcon->setVisible(m_presets.showTray);
}

void RedmiOSD::createWindow()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* horizontalLayout1 = new QHBoxLayout;
    QHBoxLayout* horizontalLayout2 = new QHBoxLayout;
    QHBoxLayout* horizontalLayout3 = new QHBoxLayout;
    QHBoxLayout* horizontalLayout4 = new QHBoxLayout;

    m_activeLabel = new QLabel(formatToUpper(m_presets.lastPreset));
    m_activeLabel->setStyleSheet("font-weight: bold;");

    m_defaultComboBox = new QComboBox();
    m_defaultComboBox->addItem("Silence");
    m_defaultComboBox->addItem("Turbo");
    m_defaultComboBox->addItem("LastPreset");
    m_defaultComboBox->setCurrentText(formatToUpper(m_presets.defaultPreset));

    m_updateRateSpinBox = new QSpinBox();
    m_updateRateSpinBox->setMinimum(0);
    m_updateRateSpinBox->setMaximum(60000);
    m_updateRateSpinBox->setValue(m_presets.updateRate);

    m_startupCheckBox = new QCheckBox("Startup");
    m_startupCheckBox->setChecked(m_presets.startup);
    m_startupCheckBox->setFixedSize(96, 24);

    m_liveEditCheckBox = new QCheckBox("Live Edit");
    m_liveEditCheckBox->setChecked(m_presets.liveEdit);
    m_liveEditCheckBox->setFixedSize(96, 24);

    m_overlayCheckBox = new QCheckBox("Show Overlay");
    m_overlayCheckBox->setChecked(m_presets.showOverlay);
    m_overlayCheckBox->setFixedSize(96, 24);

    m_trayCheckBox = new QCheckBox("Show Tray");
    m_trayCheckBox->setChecked(m_presets.showTray);
    m_trayCheckBox->setFixedSize(96, 24);

    m_presetsButton = new QPushButton("Presets.json");
    m_presetsButton->setFixedSize(78, 24);

    m_silenceButton = new QPushButton("Silence");
    m_silenceButton->setFixedSize(96, 24);

    m_turboButton = new QPushButton("Turbo");
    m_turboButton->setFixedSize(96, 24);

    m_silenceKeySequence = new QKeySequenceEdit();
    m_silenceKeySequence->setKeySequence(m_presets.shorcutsMap["silence"]);
    m_silenceKeySequence->setClearButtonEnabled(true);
    m_silenceKeySequence->setMaximumSequenceLength(1);

    m_turboKeySequence = new QKeySequenceEdit();
    m_turboKeySequence->setKeySequence(m_presets.shorcutsMap["turbo"]);
    m_turboKeySequence->setClearButtonEnabled(true);
    m_turboKeySequence->setMaximumSequenceLength(1);

    horizontalLayout1->addWidget(new QLabel("Active Preset :"));
    horizontalLayout1->addWidget(m_activeLabel);
    horizontalLayout1->addWidget(new QLabel("Default Preset :"));
    horizontalLayout1->addWidget(m_defaultComboBox);
    horizontalLayout1->addWidget(m_startupCheckBox);

    horizontalLayout2->addWidget(new QLabel("Update Rate (ms) :"));
    horizontalLayout2->addWidget(m_updateRateSpinBox);
    horizontalLayout2->addWidget(m_presetsButton);
    horizontalLayout2->addWidget(m_liveEditCheckBox);

    QLabel* silenceLabel = new QLabel();
    silenceLabel->setFixedSize(24, 24);
    silenceLabel->setStyleSheet("QLabel { background-color : transparent; image: url(Resources/Silence.png); }");
    horizontalLayout3->addWidget(silenceLabel);
    horizontalLayout3->addWidget(m_silenceButton);
    horizontalLayout3->addWidget(m_silenceKeySequence);
    horizontalLayout3->addWidget(m_overlayCheckBox);

    QLabel* turboLabel = new QLabel();
    turboLabel->setFixedSize(24, 24);
    turboLabel->setStyleSheet("QLabel { background-color : transparent; image: url(Resources/Turbo.png); }");
    horizontalLayout4->addWidget(turboLabel);
    horizontalLayout4->addWidget(m_turboButton);
    horizontalLayout4->addWidget(m_turboKeySequence);
    horizontalLayout4->addWidget(m_trayCheckBox);

    mainLayout->addLayout(horizontalLayout1);
    mainLayout->addSpacerItem(new QSpacerItem(0, 5));
    mainLayout->addLayout(horizontalLayout2);
    mainLayout->addSpacerItem(new QSpacerItem(0, 5));
    mainLayout->addLayout(horizontalLayout3);
    mainLayout->addSpacerItem(new QSpacerItem(0, 5));
    mainLayout->addLayout(horizontalLayout4);

    setWindowIcon(QIcon("Resources/Default.png"));
    setWindowTitle("RedmiOSD");
    setFixedSize(415, 150);
    setLayout(mainLayout);
}

void RedmiOSD::createTray()
{
    QAction* settingsAction = new QAction(QIcon("Resources/Default.png"), "Settings", this);
    connect(settingsAction, &QAction::triggered, this, &QWidget::show);

    QAction* quitAction = new QAction(QIcon("Resources/Quit.png"), "Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(trayIconMenu);

    m_trayIcon->setIcon(QIcon(QString("Resources/%1.png").arg(formatToUpper(m_presets.lastPreset))));
    m_trayIcon->setToolTip(formatToUpper(m_presets.lastPreset));
}

void RedmiOSD::createShortcuts()
{
    m_silenceShortcut.setShortcut(QKeySequence(m_presets.shorcutsMap["silence"]), true);
    m_turboShortcut.setShortcut(QKeySequence(m_presets.shorcutsMap["turbo"]), true);
}

QString RedmiOSD::formatToUpper(const QString& text)
{
    QString formatedText = text;
    formatedText.front() = formatedText.front().toUpper();

    return formatedText;
}

QString RedmiOSD::formatToLower(const QString& text)
{
    QString formatedText = text;
    formatedText.front() = formatedText.front().toLower();

    return formatedText;
}