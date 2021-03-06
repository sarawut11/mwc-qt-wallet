// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "x_walletconfig_w.h"
#include "ui_x_walletconfig.h"
#include "../control_desktop/messagebox.h"
#include <QFileInfo>
#include <QHostInfo>
#include <QStandardPaths>
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/networkselectiondlg.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"

namespace wnd {

static int scale2Id(double scale) {
    if (scale < 1.15 )
        return 1;
    else if (scale < 1.45)
        return 2;
    else if (scale < 1.8)
        return 3;
    else
        return 4;
}

static double id2scale(int scale) {
    switch(scale) {
    case 1: return 1.0;
    case 2: return 1.3;
    case 3: return 1.6;
    default: return 2.0;
    }
}

static QString calcMWCMW_DOMAIN_DEFAULT_STR() {
    return "default MWC MQS Domain";
}

WalletConfig::WalletConfig(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::WalletConfig)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);
    util = new bridge::Util(this);

    uiScale = scale2Id( walletConfig->getGuiScale() );
    checkSizeButton(uiScale);

    walletLogsEnabled = walletConfig->getWalletLogsEnabled();
    updateLogsStateUI(walletLogsEnabled);

    autoStartMQSEnabled = walletConfig->getAutoStartMQSEnabled();
    autoStartTorEnabled = walletConfig->getAutoStartTorEnabled();
    updateAutoStartStateUI(autoStartMQSEnabled, autoStartTorEnabled);

    notificationWindowsEnabled = walletConfig->getNotificationWindowsEnabled();
    ui->notificationsEnabled->setChecked(notificationWindowsEnabled);

    lockLater = config->getSendLockOutput();
    ui->lockLaterEnabled->setChecked(lockLater);

    currentLogoutTimeout = walletConfig->getLogoutTimeMs() / 1000;
    updateAutoLogoutStateUI(currentLogoutTimeout);
    logoutTimeout = currentLogoutTimeout;

    outputLockingEnabled = walletConfig->isOutputLockingEnabled();
    ui->outputLockingCheck->setChecked(outputLockingEnabled);

    QVector<QString> instanceInfo = config->getCurrentWalletInstance();
    QString fullPath = instanceInfo[1];
    walletInstanceName = instanceInfo[2];
    ui->walletInstanceNameEdit->setText(walletInstanceName);
    ui->walletDataPath->setText(fullPath);

#ifdef Q_OS_DARWIN
    // MacOS doesn't support font scale. Need to hide all the buttons
    ui->fontHolder->hide();
#endif

    ui->progress->initLoader(false);

    mqsHost = walletConfig->getMqsHost();
    inputConfirmationsNumber = walletConfig->getInputConfirmationsNumber();
    changeOutputs = walletConfig->getChangeOutputs();

#ifdef Q_OS_WIN
    // Disable in windows because notification bring the whole QT wallet on the top of other windows.
    // Notications overlap other windows.
    ui->notificationsEnabled->setEnabled(false);
#endif

    setValues(mqsHost, inputConfirmationsNumber, changeOutputs);
    updateButtons();
}

WalletConfig::~WalletConfig()
{
    walletConfig->canApplySettings(false);
    delete ui;
}

void WalletConfig::setValues(
                             const QString & mwcmqHostNorm,
                             int inputConfirmationNumber,
                             int changeOutputs) {
    ui->mwcmqHost->setText( mwcDomainConfig2InputStr( mwcmqHostNorm ) );

    ui->confirmationNumberEdit->setText( QString::number(inputConfirmationNumber) );
    ui->changeOutputsEdit->setText( QString::number(changeOutputs) );

    setFocus();
}

void WalletConfig::updateButtons() {
    bool sameWithCurrent =
        ui->walletInstanceNameEdit->text().trimmed() == walletInstanceName &&
        getcheckedSizeButton() == uiScale &&
        walletLogsEnabled == ui->logsEnableBtn->isChecked() &&
        mwcDomainInputStr2Config( ui->mwcmqHost->text().trimmed() ) == mqsHost &&
        ui->confirmationNumberEdit->text().trimmed() == QString::number(inputConfirmationsNumber) &&
        ui->changeOutputsEdit->text().trimmed() == QString::number(changeOutputs) &&
        autoStartMQSEnabled == ui->start_mqs->isChecked() &&
        autoStartTorEnabled == ui->start_tor->isChecked() &&
        notificationWindowsEnabled == ui->notificationsEnabled->isChecked() &&
        lockLater == ui->lockLaterEnabled->isChecked() &&
        logoutTimeout == currentLogoutTimeout &&
        outputLockingEnabled == ui->outputLockingCheck->isChecked();

    walletConfig->canApplySettings(!sameWithCurrent);

    bool sameWithDefault =
        getcheckedSizeButton() == scale2Id( walletConfig->getInitGuiScale() ) &&
        true == ui->logsEnableBtn->isChecked() &&
        // 713 directory is skipped intentionally. We don't want to reset it because user is expected to have many such directories
        // ui->mwc713directoryEdit->text().trimmed() == defaultWalletConfig.getDataPath() &&
        mwcDomainInputStr2Config( ui->mwcmqHost->text().trimmed() ) == walletConfig->getDefaultMqsHost() &&
        ui->confirmationNumberEdit->text().trimmed() == QString::number(walletConfig->getDefaultInputConfirmationsNumber()) &&
        ui->changeOutputsEdit->text().trimmed() == QString::number(walletConfig->getDefaultChangeOutputs()) &&
        ui->start_mqs->isChecked() == true &&
        ui->start_tor->isChecked() == true &&
        ui->logout_20->isChecked() == true &&
        ui->outputLockingCheck->isChecked() == false;

    ui->restoreDefault->setEnabled( !sameWithDefault );
    ui->applyButton->setEnabled( !sameWithCurrent );
}

QString WalletConfig::mwcDomainConfig2InputStr(QString mwcDomain) {
    return mwcDomain.isEmpty() ? calcMWCMW_DOMAIN_DEFAULT_STR() : mwcDomain;
}

QString WalletConfig::mwcDomainInputStr2Config(QString mwcDomain) {
    return mwcDomain == calcMWCMW_DOMAIN_DEFAULT_STR() ? "" : mwcDomain;
}

void WalletConfig::on_mwcmqHost_textEdited(const QString &)
{
    updateButtons();
}

void WalletConfig::on_confirmationNumberEdit_textChanged(const QString &)
{
    updateButtons();
}

void WalletConfig::on_changeOutputsEdit_textEdited(const QString &)
{
    updateButtons();
}

void WalletConfig::on_restoreDefault_clicked()
{
    setValues(walletConfig->getMqsHost(),
              walletConfig->getDefaultInputConfirmationsNumber(),
              walletConfig->getDefaultChangeOutputs());

    checkSizeButton( scale2Id(walletConfig->getInitGuiScale()) );

    updateLogsStateUI(true);

    updateAutoStartStateUI(true, true);

    currentLogoutTimeout = 20 * 60;
    updateAutoLogoutStateUI(20 * 60);

    ui->outputLockingCheck->setChecked(false);

    updateButtons();
}

// return true if no chnages need to be made.
// false - need to be made or was made and wallet need to be restarted
bool WalletConfig::applyChanges() {

    util::TimeoutLockObject to("WalletConfig");

    QString newWalletInstanceName = ui->walletInstanceNameEdit->text().trimmed();
    if (newWalletInstanceName.isEmpty()) {
        control::MessageBox::messageText(this, "Input", "Please specify non empty wallet instance name");
        ui->walletInstanceNameEdit->setFocus();
        return false;
    }

    QString mwcmqHost = mwcDomainInputStr2Config(ui->mwcmqHost->text().trimmed());
    if (!mwcmqHost.isEmpty()) {
        // Checking the host
        QHostInfo host = QHostInfo::fromName(mwcmqHost);
        if (host.error() != QHostInfo::NoError) {
            control::MessageBox::messageText(this, "Input",
                                             "Host " + mwcmqHost + " is not reachable.\n" + host.errorString());
            ui->mwcmqHost->setFocus();
            return false;
        }
    }

    bool ok = false;
    int confirmations = ui->confirmationNumberEdit->text().trimmed().toInt(&ok);
    if (!ok || confirmations <= 0 || confirmations > 10) {
        control::MessageBox::messageText(this, "Input",
                                         "Please input the number of confirmations in the range from 1 to 10");
        ui->confirmationNumberEdit->setFocus();
        return false;
    }

    int changeOutputs = ui->changeOutputsEdit->text().trimmed().toInt(&ok);
    if (!ok || changeOutputs <= 0 || changeOutputs >= 100) {
        control::MessageBox::messageText(this, "Input",
                                         "Please input the change output number in the range from 1 to 100");
        ui->changeOutputsEdit->setFocus();
        return false;
    }

    if (!(confirmations == inputConfirmationsNumber && changeOutputs == this->changeOutputs)) {
        walletConfig->setSendCoinsParams(confirmations, changeOutputs);
        inputConfirmationsNumber = confirmations;
        this->changeOutputs = changeOutputs;
    }

    bool need2updateLogEnabled = (walletLogsEnabled != ui->logsEnableBtn->isChecked());
    if (need2updateLogEnabled) {
        bool needCleanupLogs = false;
        if (!ui->logsEnableBtn->isChecked()) {
            needCleanupLogs = (control::MessageBox::questionText(this, "Wallet Logs",
                   "You just disabled the logs. Log files location:\n~/mwc-qt-wallet/logs\n"
                   "Please note, the logs can contain private information about your transactions and accounts.\n"
                   "Do you want to clean up existing logs from your wallet?",
                   "Clean up", "Keep the logs",
                   "Delete my logs data that I have",
                   "Let's keep the logs that I already have even they has information about my past operation and wallet balance",
                   true, false) ==
                                    core::WndManager::RETURN_CODE::BTN1);
        } else {
            control::MessageBox::messageText(this, "Wallet Logs",
                    "You just enabled the logs. Log files location:\n~/mwc-qt-wallet/logs\n"
                    "Please note, the logs can contain private infromation about your transactions and accounts.");
        }
        walletConfig->updateWalletLogsEnabled(ui->logsEnableBtn->isChecked(), needCleanupLogs);
    }

    walletLogsEnabled = ui->logsEnableBtn->isChecked();

    bool need2updateGuiSize = (getcheckedSizeButton() != uiScale);

    if (need2updateGuiSize) {
        walletConfig->updateGuiScale(id2scale(getcheckedSizeButton()));
    }

    bool need2updateAutoStartMQSEnabled = (autoStartMQSEnabled != ui->start_mqs->isChecked());
    if (need2updateAutoStartMQSEnabled) {
        walletConfig->updateAutoStartMQSEnabled(ui->start_mqs->isChecked());
    }
    autoStartMQSEnabled = ui->start_mqs->isChecked();

    // Check if Foreign API configured correctly
    if ( ui->start_tor->isChecked() ) {
        // Checking how foign API is doing
        bool foreignIsOk = true;
        bool resetForeignForTor = false;
        if ( !config->hasForeignApi() ) {
            foreignIsOk = false;
            resetForeignForTor =
                    core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionHTML(this, "Tor Configuration",
                                "Tor requires Foreign API to be running. "
                                "Do you want configure Foreign API to match Tor expectations?",
                                "Disable Tor",
                                "Configure API",
                                "Don't change foreign API settings and disable the Tor",
                                "Continue and change Foreign API settings, I want Tor to run",
                                false, true);
        }
        else if ( config->hasTls() ) {
            foreignIsOk = false;
            resetForeignForTor =
                    core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionHTML(this, "Tor Configuration",
                                "Tor requires Foreign API to be running without TLS. "
                                "Do you want configure Foreign API to match Tor expectations?",
                                "Disable Tor",
                                "Configure API",
                                "Don't change Foreign API settings and disable Tor",
                                "Continue and change Foreign API settings. I want Tor to run",
                                false, true);
        }
        if (resetForeignForTor) {
            foreignIsOk = true;
            need2updateGuiSize = true;
            config->saveForeignApiConfig(true,
                    "127.0.0.1:3415", "", "");
        }
        if (!foreignIsOk) {
            ui->start_tor->setCheckState(Qt::Unchecked);
        }
    }

    bool need2updateAutoStartTorEnabled = (autoStartTorEnabled != ui->start_tor->isChecked());
    if (need2updateAutoStartTorEnabled) {
        walletConfig->updateAutoStartTorEnabled(ui->start_tor->isChecked());
    }
    autoStartTorEnabled = ui->start_tor->isChecked();

    bool notificationsEnabled = ui->notificationsEnabled->isChecked();
    if (notificationsEnabled != notificationWindowsEnabled) {
        walletConfig->setNotificationWindowsEnabled(notificationsEnabled);
        notificationWindowsEnabled = notificationsEnabled;
    }

    bool lockLaterVal = ui->lockLaterEnabled->isChecked();
    if (lockLaterVal != lockLater) {
        lockLater = lockLaterVal;
        config->setSendLockOutput(ui->lockLaterEnabled->isChecked());
    }

    bool lockingEnabled = ui->outputLockingCheck->isChecked();
    if (lockingEnabled != outputLockingEnabled) {
        walletConfig->setOutputLockingEnabled(lockingEnabled);
        outputLockingEnabled = lockingEnabled;
    }

    if (logoutTimeout != currentLogoutTimeout) {
        if (walletConfig->updateTimeoutValue(currentLogoutTimeout))
            logoutTimeout = currentLogoutTimeout;
    }

    if (newWalletInstanceName != walletInstanceName) {
        config->updateActiveInstanceName(newWalletInstanceName);
        walletInstanceName = newWalletInstanceName;
    }

    if (mwcmqHost != mqsHost) {
        ui->progress->show();
        if (walletConfig->updateWalletConfig( mwcmqHost, "", need2updateGuiSize)) {
            return false;
        }
    }

    if (need2updateGuiSize) {
        ui->progress->show();
        // Restating the wallet
        walletConfig->restartQtWallet();
        return false;   // need to be restarted. Just want to cancell caller of caller changes state operation
    }

    ui->progress->hide();

    updateButtons();
    return true; // We are good. Changes was applied
}

void WalletConfig::on_applyButton_clicked()
{
    applyChanges();
}

// Id match the control names: 1..4
void WalletConfig::checkSizeButton(int szId) {
    ui->fontSz1->setChecked( szId == 1 );
    ui->fontSz2->setChecked( szId == 2 );
    ui->fontSz3->setChecked( szId == 3 );
    ui->fontSz4->setChecked( szId == 4 );

    updateButtons();
}

int WalletConfig::getcheckedSizeButton() const {
    if (ui->fontSz1->isChecked())
        return 1;
    if (ui->fontSz2->isChecked())
        return 2;
    if (ui->fontSz3->isChecked())
        return 3;

    return 4;
}

void WalletConfig::on_fontSz1_clicked()
{
    checkSizeButton(1);
}

void WalletConfig::on_fontSz2_clicked()
{
    checkSizeButton(2);
}

void WalletConfig::on_fontSz3_clicked()
{
    checkSizeButton(3);
}

void WalletConfig::on_fontSz4_clicked()
{
    checkSizeButton(4);
}

void WalletConfig::on_logsEnableBtn_clicked()
{
    // control has 'checked' property, it is expected to be switchable
    updateLogsStateUI( ui->logsEnableBtn->isChecked() );
    updateButtons();
}

void WalletConfig::updateLogsStateUI(bool enabled) {
    ui->logsEnableBtn->setText( enabled ? "Enabled" : "Disabled" );
    ui->logsEnableBtn->setChecked(enabled);
}

void WalletConfig::on_logout_3_clicked()
{
    currentLogoutTimeout = 3 * 60;
    updateButtons();
}

void WalletConfig::on_logout_5_clicked()
{
    currentLogoutTimeout = 5 * 60;
    updateButtons();
}

void WalletConfig::on_logout_10_clicked()
{
    currentLogoutTimeout = 10 * 60;
    updateButtons();
}

void WalletConfig::on_logout_20_clicked()
{
    currentLogoutTimeout = 20 * 60;
    updateButtons();
}

void WalletConfig::on_logout_30_clicked()
{
    currentLogoutTimeout = 30 * 60;
    updateButtons();
}

void WalletConfig::on_logout_never_clicked()
{
    currentLogoutTimeout = -1;
    updateButtons();
}

void WalletConfig::on_start_mqs_clicked()
{
    updateButtons();
}

void WalletConfig::on_start_tor_clicked()
{
    updateButtons();
}

void WalletConfig::on_walletInstanceNameEdit_textChanged(const QString &)
{
    updateButtons();
}

void WalletConfig::updateAutoStartStateUI(bool isAutoStartMQS, bool isAutoStartTor) {
    ui->start_mqs->setChecked(isAutoStartMQS);
    ui->start_tor->setChecked(isAutoStartTor);
}

void WalletConfig::updateAutoLogoutStateUI(int64_t time) {
    if (time < 0)
        ui->logout_never->setChecked(true);
    else if(time < 4 * 60)
        ui->logout_3->setChecked(true);
    else if(time < 7.5 * 60)
        ui->logout_5->setChecked(true);
    else if(time < 15 * 60)
        ui->logout_10->setChecked(true);
    else if(time < 25 * 60)
        ui->logout_20->setChecked(true);
    else
        ui->logout_30->setChecked(true);
}

void WalletConfig::on_outputLockingCheck_stateChanged(int check)
{
    Q_UNUSED(check);
    updateButtons();
}

void WalletConfig::on_notificationsEnabled_clicked() {
    updateButtons();
}

void WalletConfig::on_lockLaterEnabled_clicked()
{
    updateButtons();
}

}


