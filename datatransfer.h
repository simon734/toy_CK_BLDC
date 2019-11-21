#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <chrono>
#include <QTimerEvent>
#include "serialcomm.h"
#include "deviceitem.h"

class DataTransfer: public QObject, public IDataRead {
    Q_OBJECT

public:
    static DataTransfer* Instance();
    void SetDataChangedCallback(IDataChanged* cb);
    void SetShowStatusCallback(IShowStatus* cb);
    void Open();
    void Close();
    bool Write();
    bool Read();

    bool IsOpen();

protected:
    void timerEvent(QTimerEvent *event) override;

    // IDataRead interface
    virtual void onRead(QByteArray&& data) override;
    virtual void onClose(int err) override;

private:
    DataTransfer();
    ~DataTransfer();

private:
    bool Pack(QByteArray& data);
    bool Unpack();
    bool CheckPackage(const QByteArray& data);
    quint8 GetCheckSum(const QByteArray& data, int pos, int len);

    void SetClosedMode();
    void SetReadyMode();
    void SetReadMode();
    void SetWriteMode(QByteArray&& data);
    bool IsInTransitionMode();

private:
    int timer_id_ = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_read_time_;

    QByteArray data_writen_;
    QByteArray data_read_;
    IDataChanged *data_changed_cb_ = nullptr;
    IShowStatus *show_status_cb_ = nullptr;

    enum class PKG_STATUS {
        ONGOING = 1,
        COMPLETED = 2
    };
    PKG_STATUS pkg_status = PKG_STATUS::COMPLETED;

    enum class OP_MODE {
        CLOSED = 0,
        READY = 1,
        READ = 2,
        WRITE = 3
    };
    OP_MODE op_mode_ = OP_MODE::CLOSED;
};
#endif // DATATRANSFER_H
