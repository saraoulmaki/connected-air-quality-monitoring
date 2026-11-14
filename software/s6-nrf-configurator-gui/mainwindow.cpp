#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "serialportreader.h"
#include <QSerialPort>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include <QRandomGenerator>
#include <QSerialPortInfo>
#include <QRegularExpression>
#include <QValidator>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();

    QSerialPortInfo portInfo;

    for (int i = 0; i < serialPortInfos.size(); ++i) {
        portInfo = serialPortInfos[i];
        ui->Port_serie->addItem(portInfo.portName());
    }

    const int serialPortBaudRate = QSerialPort::Baud115200;

    serialPort.setBaudRate(serialPortBaudRate);

    if (!serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open port %1, error: %2";
    }

    serialPortReader = new SerialPortReader(&serialPort);

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

    QByteArray data = "a";

    serialPortReader->handleWrite(data);

}

void MainWindow::on_NodeID_line_textChanged(const QString &arg1)
{
    if(arg1.length() <= 4){
        NodeID = arg1.toInt(nullptr,16);
        ui->NodeID_line->setMaxLength(4);
    }

    qDebug() << "NodeID = " << NodeID;
}


void MainWindow::on_AES_Key_line_textChanged(const QString &arg1)
{
    QRegularExpression hexMatcher("[0-9a-f]{32}");

    QValidator *validator = new QRegularExpressionValidator(hexMatcher, this);

    ui->AES_Key_line->setValidator(validator);

    AES_Key = arg1;

}

void MainWindow::on_Port_serie_currentTextChanged(const QString &arg1)
{

    serialPortName = arg1;
    serialPort.setPortName(serialPortName);

    if (serialPort.isOpen()) {
        serialPort.close();
    }

    if (serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "Port série ouvert : " << serialPortName;
    } else {
        qDebug() << "Échec d'ouverture du port" << serialPortName << ", erreur :" << serialPort.errorString();
    }

    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &portInfo : serialPortInfos) {

        if (portInfo.portName() == serialPortName) {

            description = portInfo.description();
            manufacturer = portInfo.manufacturer();
            serialNumber = portInfo.serialNumber();
            location = portInfo.systemLocation();
            productIdentifier = (portInfo.hasVendorIdentifier() ? QByteArray::number(portInfo.vendorIdentifier(), 16) : "");
            vendorIdentifier = (portInfo.hasProductIdentifier() ? QByteArray::number(portInfo.productIdentifier(), 16) : "");


            ui->label_decription->setText("Description : " + description);
            ui->label_manufacturer->setText("Manufacturer : " + manufacturer);
            ui->label_serialNumber->setText("Serial Number : " + serialNumber);
            ui->label_location->setText("Location : " + location);
            ui->label_productIdentifier->setText("Product Identifier : " + productIdentifier);
            ui->label_vendorIdentifier->setText("Vendor Identifier : " + vendorIdentifier);

        }

    }

}


void MainWindow::on_Generate_clicked()
{

    int i;
    QString randomHex;

    for(i=0;i<32;i++) {
        int n=arc4random() % 16;
        randomHex.append(QString::number(n,16));
    }

    ui->AES_Key_line->setText(randomHex);

}


void MainWindow::on_PB_Lire_clicked()
{
    qDebug() << "NodeID : " << NodeID;
}


void MainWindow::on_PB_Ecrire_clicked()
{

    QByteArray myNodeID;
    QByteArray myAES_Key;

    myNodeID = QString::number(NodeID, 16).toUtf8();
    myAES_Key = AES_Key.toUtf8();

    serialPortReader->handleWrite(myNodeID);
    serialPortReader->handleWrite(myAES_Key);

    qDebug() << "myNodeID : " << myNodeID;

}
