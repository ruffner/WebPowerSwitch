#include "lauremotepowerwidget.h"

#include <QLabel>
#include <QDebug>
#include <QSettings>
#include <QGroupBox>
#include <QHBoxLayout>

LAURemotePowerWidget::LAURemotePowerWidget(QWidget *parent) : QWidget(parent), pendingRequest(false), pendingStateRequest(false), connected(false)
{
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(6, 6, 6, 6);

    QGroupBox *groupBox = new QGroupBox("Address");
    groupBox->setLayout(new QHBoxLayout());
    groupBox->layout()->setContentsMargins(6, 6, 6, 6);
    groupBox->layout()->setSpacing(6);
    this->layout()->addWidget(groupBox);

    groupBox->layout()->addWidget(new QLabel("IP Address:"));

    ipAddressLineEdit = new QLineEdit();
    ipAddressLineEdit->setInputMask(QString("000.000.000.000;_"));
    groupBox->layout()->addWidget(ipAddressLineEdit);

    QSettings settings;
    ipAddressLineEdit->setText(settings.value("LAURemotePowerWidget::ipAddressLineEdit", QString("198.165.000.100:2048")).toString());
    connect(ipAddressLineEdit, SIGNAL(editingFinished()), this, SLOT(onConnect()));

    buttonGroupBox = new QGroupBox("Switches");
    buttonGroupBox->setEnabled(false);
    buttonGroupBox->setLayout(new QHBoxLayout());
    buttonGroupBox->layout()->setContentsMargins(6, 6, 6, 6);
    buttonGroupBox->layout()->setSpacing(6);
    this->layout()->addWidget(buttonGroupBox);

    for (int n = 0; n < 8; n++) {
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

LAURemotePowerWidget::~LAURemotePowerWidget()
{

}

void LAURemotePowerWidget::onConnect()
{
    qDebug() << "LAURemotePowerWidget::onConnect()";

    QNetworkConfiguration cfg;
    QNetworkConfigurationManager ncm;
    qDebug() << "default config is " << ncm.defaultConfiguration().isValid() << "valid";
    auto nc = ncm.allConfigurations();
    for (auto &x : nc)
    {
        //qDebug() << "Network test " << x.bearerType();
        //qDebug() << "Network name: " << x.name();

        // FIXME: CHOOSE NETWORK THAT THE POWER SWITCH IS CONNECTED TO
        if (x.name() == "en1")
            cfg = x;
    }
    if (!cfg.isValid()) {
        qDebug() << "no correct network found, try to click 'Connect' again";
    } else {
        auto session = new QNetworkSession(cfg, this);
        session->open();
        testConnection();
    }
}

void LAURemotePowerWidget::managerFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "network request error." << reply->error();
        return;
    }

    QString answer = reply->readAll();

    if (pendingRequest && !connected ){
        qDebug() << "got connection test response:" << answer.size() << " bytes" << endl;
        qDebug() << answer << endl;

        if (answer.compare("\"DLI Controller\"")==0) {
            qDebug() << "CONNECTED";
            connected = true;
            getOutletStates();
            emit emitConnected(connected);
        }
    } else if (pendingRequest && connected) {
        qDebug() << "got toggle response" << answer.size() << "bytes";
        qDebug() << answer;
    } else if (pendingStateRequest && connected) {
        qDebug() << " setting pending state value " << pendingStateRequestValues.front() << answer;
        if (pendingStateRequestValues.front()>0 && pendingStateRequestValues.front() <=8){
            buttons[pendingStateRequestValues.front()-1]->setChecked(answer.contains("true"));
        }
        pendingStateRequestValues.pop_front();
        if (pendingStateRequestValues.size()==0) {
            pendingStateRequest = false;
            buttonGroupBox->setEnabled(true);
        }
    }
    pendingRequest = false;
}

void LAURemotePowerWidget::testConnection()
{
    if( !pendingRequest && !connected){
        makeAuthorizedRequest(QString("http://192.168.0.100/restapi/relay/name/"));
        pendingRequest = true;
    }
}

void LAURemotePowerWidget::getOutletStates()
{
    for (int i=0; i<8; i++) {
        // WAIT TO BE CLEARED BY NETWORK RESPONSE
        //while(pendingStateRequest==true);

        // GET THE STATE OF EACH OUTLET

        QString url("http://192.168.0.100/restapi/relay/outlets/");
        url.append(QString::number(i)).append("/state/");

        makeAuthorizedRequest(url);
        pendingStateRequest = true;
        pendingStateRequestValues.push_back(i+1);
    }
}

void LAURemotePowerWidget::onRadioButtonToggled(bool state)
{
    if (!connected) {
        qDebug() << "\nPLEASE CONNECT\n";
        return;
    }
    for (int n = 0; n < buttons.count(); n++) {
        if (QObject::sender() == buttons.at(n)) {
            qDebug() << "Button" << n + 1 << "toggled" << state;

            QString url(QString("http://192.168.0.100/script.cgi?run=").append((state ? "on" : "off")).append(QString::number(n+1)));
            makeAuthorizedRequest(url);
            pendingRequest = true;
        }
    }
}

void LAURemotePowerWidget::makeAuthorizedRequest(QString url) {
    request.setUrl(url);
    QString concatenated = "admin:1234";
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    manager->get(request);
    qDebug() << "URL: " << url;
}
