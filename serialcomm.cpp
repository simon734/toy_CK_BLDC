#include "serialcomm.h"
#include <QMessageBox>
#include "settingsdialog.h"

SerialComm::SerialComm():
    m_serial_(new QSerialPort(this)),
    m_settings_(new SettingsDialog) {

    connect(m_serial_, &QSerialPort::errorOccurred, this, &SerialComm::handleError);

    connect(m_serial_, &QSerialPort::readyRead, this, &SerialComm::readData);
}

SerialComm::~SerialComm() {
    delete m_serial_;
    m_serial_ = nullptr;

    delete m_settings_;
    m_settings_ = nullptr;
}

SerialComm* SerialComm::Instance() {
    static SerialComm inst;
    return &inst;
}

bool SerialComm::is_ready() {
    if (m_serial_) {
        return m_serial_->isOpen();
    }
    return false;
}

bool SerialComm::writeData(const QByteArray &data) {
    const auto byte_written = m_serial_->write(data);
    qCritical() << "SerialComm::writeData, byte_written=" << byte_written;
    assert(byte_written == data.size());
    return (byte_written == data.size());
}

void SerialComm::readData() {
    QByteArray data = m_serial_->readAll();

    qCritical() << "SerialComm::readData, data_size=" << data.size();
    if (data_read_) {
        data_read_->onRead(std::move(data));
    }
}

void SerialComm::SetDataReadCallback(IDataRead *cb) {
    data_read_ = cb;
}

void SerialComm::SetShowStatusCallback(IShowStatus *cb) {
    show_status_ = cb;
}

void SerialComm::handleError(QSerialPort::SerialPortError error) {
    qCritical() << "SerialComm::handleError, error_code=" << error
                << ", error_msg=" << m_serial_->errorString();
    if (error == QSerialPort::ResourceError) {
        closeSerialPort(-2);
    }
}

void SerialComm::showSetting() {
    m_settings_->show();
}

bool SerialComm::openSerialPort() {
    const SettingsDialog::Settings p = m_settings_->settings();
    m_serial_->setPortName(p.name);
    m_serial_->setBaudRate(p.baudRate);
    m_serial_->setDataBits(p.dataBits);
    m_serial_->setParity(p.parity);
    m_serial_->setStopBits(p.stopBits);
    m_serial_->setFlowControl(p.flowControl);
    const auto is_ok = m_serial_->open(QIODevice::ReadWrite);

    if (is_ok) {
        const auto msg = QString("Connected to %1").arg(p.name);
        qCritical() << tr("SerialComm::openSerialPort, Connected to %1").arg(p.name);
        ShowStatus(QString::fromWCharArray(L"已连接 %1").arg(p.name));
    } else {
        qCritical() << "SerialComm::openSerialPort, Failed to open port:"
                    << p.name << ", error=" << m_serial_->errorString();
        ShowStatus(QString::fromWCharArray(L"无法连接%1").arg(p.name));
        data_read_->onClose(-1);
    }

    return is_ok;
}

void SerialComm::closeSerialPort(int err) {
    if (m_serial_->isOpen())
        m_serial_->close();
    ShowStatus(QString::fromWCharArray(DISCONNECTED));
    qCritical() << "SerialComm::closeSerialPort, err=" << err;

    if (data_read_) {
        data_read_->onClose(err);
    }
}

void SerialComm::ShowStatus(const QString &s) {
    if (show_status_) {
        show_status_->setStatus(s);
    }
}
