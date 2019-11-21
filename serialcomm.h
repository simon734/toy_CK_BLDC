#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <QObject>
#include <QSerialPort>
#include "settingsdialog.h"
#include "basic_def.h"

class SerialComm: public QObject {
public:
    static SerialComm* Instance();
    bool is_ready();
    bool writeData(const QByteArray &data);
    void readData();
    void SetDataReadCallback(IDataRead* cb);
    void SetShowStatusCallback(IShowStatus* cb);
    bool openSerialPort();
    void closeSerialPort(int err);

public slots:
    void showSetting();

private:
    SerialComm();
    ~SerialComm();

private:
    void ShowStatus(const QString& s);
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serial_ = nullptr;
    SettingsDialog *m_settings_ = nullptr;
    IDataRead *data_read_= nullptr;
    IShowStatus *show_status_ = nullptr;
};

#endif // SERIALCOMM_H
