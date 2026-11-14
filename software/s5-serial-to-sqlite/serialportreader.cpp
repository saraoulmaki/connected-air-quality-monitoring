/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "serialportreader.h"
#include "qdebug.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QtEndian>
#include <QSqlDatabase>
#include <QSqlQuery>
 #include <QDate>
 #include <QFile>

#include <aes.hpp>

SerialPortReader::SerialPortReader(QSerialPort *serialPort, QObject *parent) :
    QObject(parent),
    m_serialPort(serialPort),
    m_standardOutput(stdout)
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortReader::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortReader::handleError);
    connect(&m_timer, &QTimer::timeout, this, &SerialPortReader::handleTimeout);

    //m_timer.start(5000);
}

void SerialPortReader::handleReadyRead()
{
    m_readData.append(m_serialPort->readAll());
    extract();

}

void SerialPortReader::extract(){

    //Initialisation des variables

        int index;

        QByteArray en_tete = QByteArray::fromHex("aa55aa55");
        QByteArray trame;

        uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
        uint8_t iv[16];

        uint8_t payload[16];

        //Extraction des donnees

        index = m_readData.indexOf(en_tete);

        if (index == -1) {
            //qDebug() << "En-tête non trouvé";
            return;
        }

        if (m_readData.size() >= index + 28) {

            trame = m_readData.mid(index, 28);

            qDebug() << "\n-----------------------Extraction Trame-------------------------\n";

            qDebug() << "Trame complète : \n" << trame.toHex();

            m_readData.remove(0, index + 28);

            QByteArray header = trame.mid(0,4);
            quint32 myHeather = qFromLittleEndian<quint32>(header);
            qDebug() << "\nheader int = " << myHeather;

            QByteArray timestamp = trame.mid(4,4);
            quint32 myTimestamp = qFromLittleEndian<quint32>(timestamp);
            qDebug() << "\ntimestamp int = " << myTimestamp;

            QByteArray NodeID = trame.mid(8,4);
            quint32 myNodeID = qFromLittleEndian<quint32>(NodeID);
            qDebug() << "\nNodeID int  = " << myNodeID;

            QByteArray FrameCounter = trame.mid(12,4);
            myFrameCounter = qFromLittleEndian<quint32>(FrameCounter);

            QByteArray NodeID2 = trame.mid(16,2);
            myNodeID2 = qFromLittleEndian<quint16>(NodeID2);

            QByteArray len = trame.mid(18,1);
            mylen = qFromLittleEndian<quint8>(len);

            QByteArray FrameType = trame.mid(19,1);
            myFrameType = qFromLittleEndian<quint8>(FrameType);

            QByteArray Data = trame.mid(20,8);
            //quint32 myData = qFromLittleEndian<quint32>(Data);

            QByteArray Temperature = trame.mid(20,2);
            myTemperature = qFromLittleEndian<qint16>(Temperature);

            QByteArray Humidity = trame.mid(22,2);
            myHumidity = qFromLittleEndian<quint16>(Humidity);


            if(myNodeID == myNodeID2){

                qDebug() << "\n-----Mode Dechiffre-----";

                qDebug() << "\nFrameCounter = " << myFrameCounter;
                qDebug() << "\nNodeID2 = " << myNodeID2;
                qDebug() << "\nlen int = " << mylen;
                qDebug() << "\nFrameType = " << myFrameType;
                qDebug() << "\nTemperature = " << myTemperature;
                qDebug() << "\nHumidity = " << myHumidity;

            }

            //Dechiffrement de myData si chiffree

            else{

                qDebug() << "\n-----Mode Chiffre-----";

                //Vecteur init

                memcpy(iv,&myTimestamp,4);
                memcpy(&iv[4],&myNodeID,4);
                memcpy(&iv[8],&myTimestamp,4);
                memcpy(&iv[12],&myNodeID,4);

                //Affichage vecteur init

                /*

                for(int i = 0;i<16;i++){
                    qDebug() << "vecteur_init[" << i << "] = " << iv[i];
                }

                */

                //Payload

                memcpy(payload,FrameCounter,4);
                memcpy(&payload[4],NodeID2,2);
                memcpy(&payload[6],len,1);
                memcpy(&payload[7],FrameType,1);
                memcpy(&payload[8],Data,8);

                //qDebug() << "Payload chiffrée : \n" << QByteArray((char*)payload, 16).toHex() << "\n";

                struct AES_ctx ctx;

                AES_init_ctx_iv(&ctx, key, iv);
                AES_CBC_decrypt_buffer(&ctx, payload, 16);

                //qDebug() << "Payload dechiffree : " << QByteArray((char*)payload, 16).toHex() << "\n";

                memcpy(&myFrameCounter,&payload,4);
                memcpy(&myNodeID2,&payload[4],2);
                memcpy(&mylen,&payload[6],1);
                memcpy(&myFrameType,&payload[7],1);
                memcpy(&myTemperature,&payload[8],2);
                memcpy(&myHumidity,&payload[10],2);

                qDebug() << "\nFrameCounter = " << myFrameCounter;
                qDebug() << "\nNodeID2 = " << myNodeID2;
                qDebug() << "\nlen = " << mylen;
                qDebug() << "\nFrameType = " << myFrameType;
                qDebug() << "\nTemperature = " << myTemperature;
                qDebug() << "\nHumidity = " << myHumidity;

            }

            SQL();

        }

        else {
            //qDebug() << "Données incomplètes";
            return;
        }

}

void SerialPortReader::SQL(){

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlQuery query;

    db.setDatabaseName("database.db");

    if( QFile::exists("database.db")){

        qDebug()<<"\nDatabase exists\n";

        if(db.open()){

            /*

            qDebug() << "\nDatabase Open\n";

            query.prepare("CREATE TABLE IF NOT EXISTS capteur (FrameCounter int, NodeID2 int,Temperature int,Humidity int);");
            qDebug() << query.exec();

            query.prepare("INSERT INTO capteur (FrameCounter,NodeID2,Temperature,Humidity) VALUES (:FrameCounter,:NodeID2,:Temperature,:Humidity)");

            quint32 var = myFrameCounter;
            quint32 var2 = 0;

            qDebug() << query.exec("SELECT * FROM capteur WHERE FrameCounter = :FrameCounter");
            query.bindValue(":FrameCounter", myFrameCounter);
            qDebug() << query.exec();

            query.value(var2);

            if(var == var2){
                qDebug() << "\nData already exists\n";
            }
            else{
                query.bindValue(":FrameCounter", myFrameCounter);
                query.bindValue(":NodeID2", myNodeID2);
                query.bindValue(":Temperature", myTemperature);
                query.bindValue(":Humidity", myHumidity);
            }



            qDebug() << query.exec();

            //query.exec("DELETE FROM capteur");

            */

            qDebug() << "\nDatabase Open\n";

            query.prepare("CREATE TABLE IF NOT EXISTS capteur (FrameCounter int UNIQUE, NodeID2 int,Temperature int,Humidity int);");
            qDebug() << query.exec();

            query.prepare("INSERT INTO capteur (FrameCounter,NodeID2,Temperature,Humidity) VALUES (:FrameCounter,:NodeID2,:Temperature,:Humidity);");

            query.bindValue(":FrameCounter", myFrameCounter);
            query.bindValue(":NodeID2", myNodeID2);
            query.bindValue(":Temperature", myTemperature);
            query.bindValue(":Humidity", myHumidity);

            qDebug() << query.exec();

            //qDebug() << query.exec("DELETE FROM capteur");

        }

        else{

            qDebug() << "\nDatabase Not Open\n";

        }

    }

    else{

        qDebug()<<"\nDatabase does not exist, creating new database\n";

        db.setDatabaseName("database.db");

    }



}

void SerialPortReader::handleTimeout()
{
    if (m_readData.isEmpty()) {
        m_standardOutput << QObject::tr("No data was currently available "
                                        "for reading from port %1")
                            .arg(m_serialPort->portName())
                         << "\n";
    } else {
        m_standardOutput << QObject::tr("Data successfully received from port %1")
                            .arg(m_serialPort->portName())
                         << "\n";
        m_standardOutput << m_readData << "\n";
    }

    QCoreApplication::quit();
}

void SerialPortReader::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        m_standardOutput << QObject::tr("An I/O error occurred while reading "
                                        "the data from port %1, error: %2")
                            .arg(m_serialPort->portName())
                            .arg(m_serialPort->errorString())
                         << "\n";
        QCoreApplication::exit(1);
    }
}
