#ifndef BASIC_DEF_H
#define BASIC_DEF_H

#include <QString>
#include <QDebug>

constexpr quint32 MAX_READ_DATA_INTERVAL = 50;  // 50 milliseconds
constexpr quint32 DEBUG_WRIRE_DATA_INTERVAL = 200;  // 0.2 second
constexpr quint32 DEBUG_MAX_WAIT_FOR_RSP_TIME = 1000;  // 1 second

struct IShowStatus {
    virtual void setStatus(const QString& s) = 0;
};

struct IDataChanged {
    virtual void onDataChange() = 0;
    virtual void onError(int err) = 0;
};

struct IDataRead {
    virtual void onRead(QByteArray&& data) = 0;
    virtual void onClose(int err) = 0;
};

#endif // BASIC_DEF_H
