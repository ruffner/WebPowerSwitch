#ifndef LAUREMOTEPOWERWIDGET_H
#define LAUREMOTEPOWERWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QNetworkRequest>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURemotePowerWidget : public QWidget
{
    Q_OBJECT

public:
    LAURemotePowerWidget(QWidget *parent = 0);
    ~LAURemotePowerWidget();

protected:
    void showEvent(QShowEvent *)
    {
        this->setFixedSize(this->size());
    }

public slots:
    void onConnect();
    void onRadioButtonToggled(bool state);

signals:
    void emitConnected(bool connected);

private:
    void testConnection();
    void getOutletStates();
    void makeAuthorizedRequest(QString url);

    QGroupBox *buttonGroupBox;
    QLineEdit *ipAddressLineEdit;
    QList<QRadioButton *> buttons;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QList<int> pendingStateRequestValues;
    bool pendingRequest, pendingStateRequest, connected;

private slots:
    void managerFinished(QNetworkReply *reply);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURemotePowerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAURemotePowerDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        widget = new LAURemotePowerWidget();

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

        QPushButton *button = new QPushButton("Connect");
        connect(button, SIGNAL(clicked()), widget, SLOT(onConnect()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);

        connect(widget, SIGNAL(emitConnected(bool)), buttonBox, SLOT(setDisabled(bool)));

        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->layout()->addWidget(widget);
        this->layout()->addWidget(buttonBox);
    }

private:
    LAURemotePowerWidget *widget;
};

#endif // LAUREMOTEPOWERWIDGET_H
