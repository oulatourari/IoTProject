#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    w = new ChartWindow();

    client = new QMqttClient(this);
    client->setPort(1883);

    client->setHostname(ui->lineEdit_Host->text());
    client->setClientId(ui->lineEdit_Client->text());

    client->setUsername(ui->lineEdit_Username->text());
    client->setPassword(ui->lineEdit_Password->text());

    connect(client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(ui->pushButton_Connect, &QPushButton::clicked, this, &MainWindow::on_buttonConnect_clicked);
    connect(ui->pushButton_Subscribe, &QPushButton::clicked, this, &MainWindow::on_buttonSubscribe_clicked);
    connect(client, &QMqttClient::connected, this, &MainWindow::connectedSlot);
    connect(client, &QMqttClient::disconnected, this, &MainWindow::disconnectedSlot);
    connect(client, &QMqttClient::usernameChanged, this, &MainWindow::usernameChangedSlot);
    connect(client, &QMqttClient::passwordChanged, this, &MainWindow::passwordChangedSlot);
    connect(client, &QMqttClient::errorChanged, this, &MainWindow::updateLogErrorChanged);
    connect(w, &QDialog::finished, this, &MainWindow::disconnectionSlot);
    connect(client, &QMqttClient::messageReceived, this, &MainWindow::messageReceivedSlot);
    connect(client, &QMqttClient::messageReceived, w, &ChartWindow::messageReceivedSlot);
}

void MainWindow::updateLogStateChange()
{
    QString state;
    switch (client->state()){
    case 0: state = "Disconnected";
            break;
    case 1: state = "Connecting";
            break;
    default : state = "Connected";
               break;
    }

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> State Change : ")
                    + state
                    + QLatin1Char('\n');
    ui->textEdit_Log->insertPlainText(content);
}

void MainWindow::on_buttonConnect_clicked(){

    if(client->state() == QMqttClient::Disconnected){

        client->setHostname(ui->lineEdit_Host->text());
        client->setClientId(ui->lineEdit_Client->text());

        client->setUsername(ui->lineEdit_Username->text());
        client->setPassword(ui->lineEdit_Password->text());

        client->connectToHost();

        ui->lineEdit_Username->setEnabled(false);
        ui->lineEdit_Password->setEnabled(false);
        ui->lineEdit_Host->setEnabled(false);
        ui->lineEdit_Client->setEnabled(false);
        ui->pushButton_Connect->setText("Disconnect");
    }
    else{
        ui->lineEdit_Password->setEnabled(true);
        ui->lineEdit_Username->setEnabled(true);
        ui->lineEdit_Host->setEnabled(true);
        ui->lineEdit_Client->setEnabled(true);
        ui->pushButton_Connect->setText("Connect");
        client->disconnectFromHost();
    }
}

void MainWindow::on_buttonSubscribe_clicked(){
    auto sub = client->subscribe(ui->lineEdit_Topic->text());
    if(!sub){
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
    else{

        QTextCursor newCursor = ui->textEdit_Log->textCursor();
        newCursor.movePosition(QTextCursor::End);
        ui->textEdit_Log->setTextCursor(newCursor);

        QString txt = QDateTime::currentDateTime().toString()
                + "::> Subscribing to :"
                + ui->lineEdit_Topic->text()
                + QLatin1Char('\n');
        ui->textEdit_Log->insertPlainText(txt);
        //ui->textEdit_Log->
    }
}

void MainWindow::connectedSlot(){
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Connected")
                    + QLatin1Char('\n');

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    ui->textEdit_Log->setTextColor("green");
    ui->textEdit_Log->insertPlainText(content);
    ui->textEdit_Log->setTextColor("black");
    w->show();
    w->raise();
}

void MainWindow::disconnectedSlot(){
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Disconnected")
                    + QLatin1Char('\n');

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    ui->textEdit_Log->setTextColor("red");
    ui->textEdit_Log->insertPlainText(content);
    ui->textEdit_Log->setTextColor("black");
    w->close();
}

void MainWindow::disconnectionSlot(){
    ui->lineEdit_Password->setEnabled(true);
    ui->lineEdit_Username->setEnabled(true);
    ui->lineEdit_Host->setEnabled(true);
    ui->lineEdit_Client->setEnabled(true);
    ui->pushButton_Connect->setText("Connect");
    client->disconnectFromHost();
}

void MainWindow::usernameChangedSlot(){
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Username Changed to :")
                    + client->username()
                    + QLatin1Char('\n');

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    ui->textEdit_Log->insertPlainText(content);
}

void MainWindow::passwordChangedSlot(){
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Password Changed to :")
                    + client->password()
                    + QLatin1Char('\n');

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    ui->textEdit_Log->insertPlainText(content);
}

void MainWindow::updateLogErrorChanged(){

    QString error;
    switch (client->error()){
        case 0 : error = "NoError";
            break;
        case 1 : error = "InvalidProtocolError";
            break;
        case 2 : error = "IdRejected";
            break;
        case 3 : error = "ServerUnaivalable";
            break;
        case 4 : error = "BadUsernameOrPassword";
            break;
        case 5 : error = "NotAuthorized";
            break;
        case 256 : error = "TransportInvalid";
            break;
        case 257 : error = "ProtocolViolation";
            break;
        case 258 : error = "UnknownError";
            break;
        default : error = "mqtt5SpecificError";
            break;
    }

    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Error : ")
                    + error
                    + QLatin1Char('\n');
    ui->textEdit_Log->setTextColor("red");
    ui->textEdit_Log->insertPlainText(content);
    ui->textEdit_Log->setTextColor("black");

    ui->lineEdit_Username->setEnabled(true);
    ui->lineEdit_Password->setEnabled(true);
    ui->pushButton_Connect->setText("Connect");
}

void MainWindow::messageReceivedSlot(const QByteArray &message, const QMqttTopicName &topic){

    QString msgstr = QString::fromStdString(message.toStdString());

    ui->textEdit_Log->setTextColor("blue");
    QTextCursor newCursor = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String("::> Received message from topic : ")
                    + topic.name()
                    + QLatin1String("\n")
                    + QLatin1String("Message :")
                    + QLatin1String("\n")
                    + QLatin1String("\n");


    ui->textEdit_Log->insertPlainText(content);

    QTextCursor newCursor2 = ui->textEdit_Log->textCursor();
    newCursor.movePosition(QTextCursor::End);
    ui->textEdit_Log->setTextCursor(newCursor);

    ui->textEdit_Log->setTextColor("black");
    ui->textEdit_Log->insertPlainText(msgstr + QLatin1Char('\n'));

    ui->textEdit_Log->insertPlainText("\n");

}

MainWindow::~MainWindow()
{
    delete w;
    delete ui;
}
