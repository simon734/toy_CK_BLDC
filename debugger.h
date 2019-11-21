#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <chrono>
#include <QTimerEvent>
#include "serialcomm.h"
#include "deviceitem.h"

class Debugger: public QObject, public IDataRead {
    Q_OBJECT

public:
    static Debugger* Instance();
    void SetDataChangedCallback(IDataChanged* cb);
    void SetShowStatusCallback(IShowStatus* cb);
    void Start();
    void Stop();
    bool IsInDebugging();

protected:
    void timerEvent(QTimerEvent *event) override;
    void handleWriteTimer();
    void handleReadTimer();

    // IDataRead interface
    virtual void onRead(QByteArray&& data) override;
    virtual void onClose(int err) override;

private:
    Debugger();
    ~Debugger();

private:
    bool Write();
    void Shutdown();
    bool Pack(QByteArray& data);
    bool Unpack();
    bool CheckPackage(const QByteArray& data);
    quint8 GetCheckSum(const QByteArray& data, int pos, int len);

    void SetClosedMode();
    void SetDebugMode();

private:
    int timer_id_for_write_ = 0;
    int timer_id_for_read_ = 0;
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
        DEBUG= 1
    };
    OP_MODE op_mode_ = OP_MODE::CLOSED;
};
#endif // DATATRANSFER_H
