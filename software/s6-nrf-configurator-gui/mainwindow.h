#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QSerialPort>
#include "serialportreader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_pushButton_clicked();

    void on_NodeID_line_textChanged(const QString &arg1);

    void on_AES_Key_line_textChanged(const QString &arg1);

    void on_Generate_clicked();

    void on_Port_serie_currentTextChanged(const QString &arg1);

    void on_PB_Lire_clicked();

    void on_PB_Ecrire_clicked();

private:
    Ui::MainWindow *ui;
    SerialPortReader *serialPortReader;
    QSerialPort serialPort;
    uint32_t NodeID;
    QString AES_Key;
    QString serialPortName;
    QString description, manufacturer, serialNumber, location, vendorIdentifier, productIdentifier;

};
#endif // MAINWINDOW_H
