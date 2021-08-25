#include "comunard.h"
#include "ui_comunard.h"

ComunArd::ComunArd(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ComunArd)
{
    ui->setupUi(this);

    QSerialPort1 = new QSerialPort(this);
    QSerialPort1->setPortName("COM3");
    QSerialPort1->setBaudRate(9600);
    QSerialPort1->open(QSerialPort::ReadWrite);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &ComunArd::OnQSerialPort1Rx);
}

ComunArd::~ComunArd()
{
    delete ui;
    delete QSerialPort1;
}

void ComunArd::OnQSerialPort1Rx()
{
    int count;
    quint8 *buf;

    count=QSerialPort1->bytesAvailable();
    if(count<=0){
        return;
    }
    buf =new quint8[count];
    QSerialPort1->read((char *)buf,count);

    Decodificar(count,buf);

    delete[] buf;

}

void ComunArd::Decodificar(int ind, quint8 *buf)
{
    static int caracter=0;
    static uint8_t nBytes=0, cks=0, indLectura=0;
    int indRecepcion=0;

    while (indRecepcion!=ind) {
        switch(caracter){
            case 0:
                if(buf[indRecepcion]=='U'){
                    caracter=1;
                }
            break;
            case 1:
                if(buf[indRecepcion]=='N')
                    caracter=2;
                else {
                    indRecepcion--;
                    caracter=0;
                }
           break;
           case 2:
               if(buf[indRecepcion]=='E')
                   caracter=3;
               else {
                   indRecepcion--;
                   caracter=0;
               }
          break;
          case 3:
            if(buf[indRecepcion]=='R')
                caracter=4;
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 4:
            nBytes=buf[indRecepcion];
            indiceRX=nBytes;
            caracter=5;
          break;
          case 5:
            if(buf[indRecepcion]==0x00)
                caracter=6;
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 6:
            if(buf[indRecepcion]==':'){
                cks= 'U'^'N'^'E'^'R'^nBytes^0x00^':';
                caracter=7;
            }
            else {
                indRecepcion--;
                caracter=0;
            }
          break;
          case 7:
            if(nBytes>1){
                cks^=buf[indRecepcion];
                RX[indLectura++]=buf[indRecepcion]; //guardo datos del buffer en RX para luego ver que llego
            }

            nBytes--;
            if(nBytes==0){
                caracter=0;
                if(cks==buf[indRecepcion]){
                    RecibirComando(indLectura+1-indiceRX);
                }
            }
          break;
        }
        indRecepcion++;
    }
}

void ComunArd::RecibirComando(uint8_t ind){
    switch(RX[ind++]){
        case 0xF0:
            ui->lineEdit->setText("estoy Vivoo Funciona");
        break;
    }
}

void ComunArd::EnviarComando(uint8_t length, uint8_t cmd, uint8_t payload[]){
    uint8_t cks;
    static uint8_t indTX=0, testte = 0;
    // QString strhex;
    QSerialPort1->read((char *)TX, 6 + TX[indTX-testte]);
    TX[indTX++] = 'U';
    TX[indTX++] = 'N';
    TX[indTX++] = 'E';
    TX[indTX++] = 'R';
    TX[indTX++] = length; //4 pasa a 5
    TX[indTX++] = 0x00; //5 pasa a 6
    TX[indTX++] = ':'; //6 pasa a 7
    testte=3;
    switch (cmd) {
        case 0xC0: //Enciende o apaga un led //length = 4 + U N E R '4' 0 : 0XC0 1 2
            TX[indTX++] = cmd; //7 pasa a 8
            TX[indTX++] = payload[0]; //8 pasa a 9
            TX[indTX++] = payload[1]; //9 pasa a 10
            testte+=3;
        break;
        case 0xC1:
            TX[indTX++] = cmd;//CMD
            TX[indTX++] = payload[0];
        break;
        case 0xF0: //Alive
            TX[indTX++] = cmd;
            testte+=1;
        break;
    }
    cks=0;

    for(int i=0; i<indTX; i++){
        cks ^= TX[i];
    }
    TX[indTX++] = cks; //10 pasa a 11
    testte++;

    if(QSerialPort1->isOpen()){
        QSerialPort1->write((char*)TX, indTX);
    }

}


void ComunArd::on_pushButton_clicked()
{
     EnviarComando(0x02,0xF0,payload);
}
