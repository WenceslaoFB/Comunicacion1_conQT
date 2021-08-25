#ifndef COMUNARD_H
#define COMUNARD_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class ComunArd;
}

class ComunArd : public QMainWindow
{
    Q_OBJECT

public:
    explicit ComunArd(QWidget *parent = nullptr);
    ~ComunArd();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ComunArd *ui;

    QSerialPort *QSerialPort1;

        void OnQSerialPort1Rx();

        void Decodificar(int ind, quint8 *buf);

        void RecibirComando(uint8_t ind);

        void EnviarComando(uint8_t length, uint8_t cmd, uint8_t payload[]);

        uint8_t TX[256], payload[8],RX[256],indiceRX=0;
        QString mensaje="--> 0x";
};

#endif // COMUNARD_H
