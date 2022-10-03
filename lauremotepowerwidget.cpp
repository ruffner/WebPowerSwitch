#include "lauremotepowerwidget.h"

#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QSettings>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFormLayout>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURemotePowerWidget::LAURemotePowerWidget(int N, QWidget *parent) : QWidget(parent), pendingRequest(false), pendingStateRequest(false), connected(false)
{
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(6, 6, 6, 6);

    ipAddressGroupBox = new QGroupBox("Address");
    ipAddressGroupBox->setLayout(new QHBoxLayout());
    ipAddressGroupBox->layout()->setContentsMargins(0, 0, 0, 0);
    ipAddressGroupBox->layout()->setSpacing(6);
    this->layout()->addWidget(ipAddressGroupBox);

    QString ipRange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
    // You may want to use QRegularExpression for new code with Qt 5 (not mandatory).
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);

    ipAddressLineEdit = new QLineEdit();
    ipAddressLineEdit->setValidator(ipValidator);
    ipAddressLineEdit->setInputMask("000.000.000.000");
    ipAddressLineEdit->setCursorPosition(0);

    portComboBox = new QComboBox();
    portComboBox->addItems(networkPorts());

    connectButton = new QPushButton("Connect");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(onConnect()));

    ipAddressGroupBox->layout()->addWidget(new QLabel("Port:"));
    ipAddressGroupBox->layout()->addWidget(portComboBox);
    ipAddressGroupBox->layout()->addWidget(new QLabel("IP Address:"));
    ipAddressGroupBox->layout()->addWidget(ipAddressLineEdit);
    ipAddressGroupBox->layout()->addWidget(connectButton);

    QSettings settings;
    ipAddressLineEdit->setText(settings.value("LAURemotePowerWidget::ipAddressLineEdit", QString("192.168.0.100")).toString());
    portComboBox->setCurrentText(settings.value("LAURemotePowerWidget::portComboBox", portComboBox->currentText()).toString());

    buttonGroupBox = new QGroupBox("Switches");
    buttonGroupBox->setEnabled(false);
    buttonGroupBox->setLayout(new QHBoxLayout());
    buttonGroupBox->layout()->setContentsMargins(6, 6, 6, 6);
    buttonGroupBox->layout()->setSpacing(6);
    this->layout()->addWidget(buttonGroupBox);

    for (int n = 0; n < N; n++) {
        QGroupBox *switchBox = new QGroupBox(QString("%1").arg(n + 1));
        switchBox->setLayout(new QVBoxLayout());
        switchBox->layout()->setContentsMargins(6, 6, 6, 6);
        buttonGroupBox->layout()->addWidget(switchBox);

        QRadioButton *onButton = new QRadioButton("On");
        onButton->setChecked(false);
        switchBox->layout()->addWidget(onButton);

        QRadioButton *offButton = new QRadioButton("Off");
        offButton->setChecked(true);
        switchBox->layout()->addWidget(offButton);

        connect(onButton, SIGNAL(toggled(bool)), this, SLOT(onRadioButtonToggled(bool)));
        buttons << onButton;
    }

    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(managerFinished(QNetworkReply*)));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURemotePowerWidget::~LAURemotePowerWidget()
{
    QSettings settings;
    settings.setValue("LAURemotePowerWidget::ipAddressLineEdit", ipAddressLineEdit->text());
    settings.setValue("LAURemotePowerWidget::portComboBox", portComboBox->currentText());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QStringList LAURemotePowerWidget::networkPorts()
{
    QStringList strings;
    QNetworkConfiguration cfg;
    QNetworkConfigurationManager ncm;

    auto nc = ncm.allConfigurations();
    for (auto &x : nc){
        strings << x.name();
    }
    return(strings);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURemotePowerWidget::onConnect()
{
    qDebug() << "LAURemotePowerWidget::onConnect()";

    QNetworkConfiguration cfg;
    QNetworkConfigurationManager ncm;
    qDebug() << "default config is " << ncm.defaultConfiguration().isValid() << "valid";
    auto nc = ncm.allConfigurations();
    for (auto &x : nc)
    {
        qDebug() << "Network test " << x.bearerType();
        qDebug() << "Network name: " << x.name();

        // FIXME: CHOOSE NETWORK THAT THE POWER SWITCH IS CONNECTED TO
        if (x.name() == portComboBox->currentText()){
            cfg = x;
        }
    }

    if (!cfg.isValid()) {
        qDebug() << "no correct network found, try to click 'Connect' again";
    } else {
        auto session = new QNetworkSession(cfg, this);
        session->open();
        testConnection();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURemotePowerWidget::managerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "network request error." << reply->error();
        return;
    }

    QString answer = reply->readAll();
    if (pendingRequest && !connected ){
        qDebug() << "got connection test response:" << answer.size() << " bytes" << answer;
        if (answer.compare("\"DLI Controller\"")==0) {
            qDebug() << "CONNECTED";
            connected = true;
            emit emitConnected(connected);

            QString url = QString("http://%1/restapi/relay/outlets/%2/transient_state/").arg(ipAddressLineEdit->text()).arg(0);
            pendingStateRequest = true;
            pendingStateRequestValues << 0;
            makeAuthorizedRequest(url);
        }
    } else if (pendingRequest && connected) {
        qDebug() << "got toggle response" << answer.size() << "bytes" << answer;
    } else if (pendingStateRequest && connected) {
        int index = pendingStateRequestValues.takeFirst();
        if (index < buttons.count()){
            buttons[index]->setChecked(answer.contains("true"));
        }
        qDebug() << "got toggle response" << index << answer.size() << "bytes" << answer;

        if (index < buttons.count()-1){
            QString url = QString("http://%1/restapi/relay/outlets/%2/transient_state/").arg(ipAddressLineEdit->text()).arg(index + 1);
            pendingStateRequest = true;
            pendingStateRequestValues << (index + 1);
            makeAuthorizedRequest(url);
        } else {
            pendingStateRequest = false;
            buttonGroupBox->setEnabled(true);
            ipAddressGroupBox->setEnabled(false);
        }
    } else {
        qDebug() << "Unhandled message.";
    }
    pendingRequest = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURemotePowerWidget::testConnection()
{
    if(!pendingRequest && !connected){
        QString string = QString("http://%1/restapi/relay/name/").arg(ipAddressLineEdit->text());
        makeAuthorizedRequest(string);
        pendingRequest = true;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURemotePowerWidget::onRadioButtonToggled(bool state)
{
    if (!connected) {
        qDebug() << "\nPLEASE CONNECT\n";
        return;
    }

    for (int n = 0; n < buttons.count(); n++) {
        if (QObject::sender() == buttons.at(n)) {
            if (buttons.at(n)->isEnabled()){
                qDebug() << "Button" << n << "toggled" << state;
                QString url(QString("http://%1/script.cgi?run=").arg(ipAddressLineEdit->text()).append((state ? "on" : "off")).append(QString::number(n+1)));
                pendingRequest = true;
                makeAuthorizedRequest(url);
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURemotePowerWidget::makeAuthorizedRequest(QString url)
{
    QString concatenated = "admin:1234";
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    request.setUrl(url);
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    manager->get(request);

    qDebug() << "URL: " << url;
}
