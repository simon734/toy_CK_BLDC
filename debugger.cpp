#include "debugger.h"
#include <QDebug>

// 数据包有：型号  数据字节数（不包含校验和）  数据  校验和
constexpr quint32 DEBUG_RSP_BYTE_COUNT = 5;
constexpr quint8 DEBUG_CMD = 0x4B;

Debugger::Debugger() {
}

Debugger::~Debugger() {
    if (IsInDebugging()) {
        Stop();
    }
}

Debugger* Debugger::Instance() {
    static Debugger inst;
    return &inst;
}

void Debugger::SetDataChangedCallback(IDataChanged *cb) {
    data_changed_cb_ = cb;
}

void Debugger::SetShowStatusCallback(IShowStatus *cb) {
    show_status_cb_ = cb;
}

void Debugger::Start() {
    qCritical() << "Debugger::Open";
    if (!IsInDebugging()) {
        SetDebugMode();
        SerialComm::Instance()->SetDataReadCallback(this);
        timer_id_for_write_ = startTimer(std::chrono::milliseconds(DEBUG_WRIRE_DATA_INTERVAL).count());
        assert(timer_id_for_write_ != 0);
        timer_id_for_read_ = startTimer(std::chrono::milliseconds(MAX_READ_DATA_INTERVAL).count());
        assert(timer_id_for_read_ != 0);
    }
}

void Debugger::Stop() {
    qCritical() << "Debugger::Close";
    if (IsInDebugging()) {
        if (timer_id_for_write_ != 0) {
            killTimer(timer_id_for_write_);
            timer_id_for_write_ = 0;
        }
        if (timer_id_for_read_ != 0) {
            killTimer(timer_id_for_read_);
            timer_id_for_read_ = 0;
        }
    }

    SetClosedMode();
}

bool Debugger::Write() {
    qCritical() << "Debugger::Write, op_mode=" << static_cast<int>(op_mode_);
    assert(IsInDebugging());
    if (!IsInDebugging()) {
        return false;
    }

    QByteArray data;
    bool is_ok = Pack(data);
    if (!is_ok) {
        assert(false);
        qCritical() << "Debugger::Write, failed to pack data";
        return false;
    }

    is_ok = SerialComm::Instance()->writeData(data);
    qCritical() << "Debugger::Write, writeData " << is_ok;
    if (is_ok) {
        // SetWriteMode(std::move(data));
    }

    return is_ok;
}

void Debugger::Shutdown() {
    //
}

void Debugger::timerEvent(QTimerEvent *event) {
    if (!IsInDebugging()) {
        return;
    }

    const auto timer_id = event->timerId();
    if (timer_id != timer_id_for_write_ && timer_id != timer_id_for_read_) {
        return;
    }


    if (timer_id == timer_id_for_write_) {
        handleWriteTimer();
    } else if (timer_id == timer_id_for_read_) {
        handleReadTimer();
    }
}

void Debugger::handleWriteTimer() {
    Write();
}

void Debugger::handleReadTimer() {
    using namespace std::chrono;
    auto current_time = steady_clock::now();
    auto diff = duration_cast<milliseconds>(current_time - last_read_time_).count();

    bool has_error = false;
    if (data_read_.isEmpty()) {
        if (diff >= DEBUG_MAX_WAIT_FOR_RSP_TIME) {
            has_error = true;
        }
    } else if (diff >= DEBUG_WRIRE_DATA_INTERVAL) {
       has_error = true;
    }

    if (has_error) {
        // report error
        qCritical() << "Debugger::timerEvent, timeout. "
                    << "op_mode=" << static_cast<int>(op_mode_)
                    << ", time diff=" << diff;
        assert(false);
        SetClosedMode();
    }
}

void Debugger::onRead(QByteArray &&data) {
    qCritical() << "Debugger::onRead, op_mode=" << static_cast<int>(op_mode_);
    assert(IsInDebugging());
    if (!IsInDebugging()) {
        return;
    }

    last_read_time_ = std::chrono::steady_clock::now();
    data_read_.push_back(data);
    if (CheckPackage(data_read_)) {
        const auto data_size = data_read_.size();
        bool is_ok = false;
        if (data_size == DEBUG_RSP_BYTE_COUNT) {
            is_ok = DeviceManager::Instance().updateCK3864S(data_read_);
        }

        qCritical() << "Debugger::onRead, data read correctly, "
                    << "data size=" << data_size
                    << ", update result=" << is_ok;

        if (is_ok && data_changed_cb_) {
            data_changed_cb_->onDataChange();
        }
        SetClosedMode();
    }
}

void Debugger::onClose(int err) {
    qCritical() << "Debugger::onClose, err code=" << err
                << ", op_mode=" << static_cast<int>(op_mode_);
    if (IsInDebugging()) {
        // report error
        qCritical() << "onClose";
    }

    if (err != 0 && data_changed_cb_) {
        data_changed_cb_->onError(err);
    }

    SetClosedMode();
}

// 型号  数据字节数（不包含校验和）  数据  校验和
// 每个数据的长度为一个字节
// 校验和计算方式：字节数 + 各个数据 等和的最低字节
bool Debugger::Pack(QByteArray& data) {
    const auto& devMgr = DeviceManager::Instance();
    const auto& items = devMgr.getItems();
    const auto item_count = items.size();
    qCritical() << "Debugger::Pack, item count=" << item_count;
    assert(item_count == CK3864S_ITEM_COUNT ||
           item_count == CK3862S_ITEM_COUNT);
    if (item_count != CK3864S_ITEM_COUNT &&
        item_count != CK3862S_ITEM_COUNT) {
        return false;
    }

    data.clear();
    char cmd = DEBUG_CMD;
    data.push_back(cmd);
    data.push_back(static_cast<char>(items.size()));
    for (const auto& item: items) {
        const auto value = static_cast<char>(item.getValue());
        data.push_back(value);
    }
    const auto check = GetCheckSum(data, 1, -1);
    data.push_back(static_cast<char>(check));

    return true;
}

bool Debugger::CheckPackage(const QByteArray &data) {
    const auto byte_count = static_cast<unsigned int>(data.size());
    qCritical() << "Debugger::CheckPackage, data len=" << byte_count;
    if (byte_count == 0) {
        return false;
    }

    const auto cmd = static_cast<quint8>(data[0]);
    qCritical() << "Debugger::CheckPackage, cmd=" << cmd;
    if (cmd == DEBUG_CMD) {
       if (byte_count < DEBUG_RSP_BYTE_COUNT) {
           return false;
       }
    } else {
        assert(false);
        return false;
    }

    const auto item_count = data[1];
    const auto check_calc = GetCheckSum(data, 1, item_count);
    const auto check_in = data[byte_count - 1];
    const bool is_ok =  (check_in == check_calc);
    qCritical() << "Debugger::CheckPackage, check result=" << is_ok
                << ", check_in=" << check_in
                << ", check_calc=" << check_calc;

    return is_ok;
}

quint8 Debugger::GetCheckSum(const QByteArray &data, int pos, int len) {
    if (pos < 0 || pos >= data.size()) {
        return 0;
    }

    unsigned int sum = 0;
    auto items = data.mid(pos, len);
    for (auto& item: items) {
        sum += static_cast<unsigned int>(item);
    }
    const quint8 check = static_cast<quint8>(sum & 0x000000FF);
    qCritical() << "Debugger::GetCheckSum, check=" << check;
    return check;
}

void Debugger::SetClosedMode() {
    qCritical() << "Debugger::SetClosedMode";
    if (IsInDebugging()) {
        Shutdown();
    }
    data_writen_.clear();
    data_read_.clear();
    pkg_status = PKG_STATUS::COMPLETED;
    op_mode_ = OP_MODE::CLOSED;
}

void Debugger::SetDebugMode() {
    qCritical() << "Debugger::SetDebugMode";
    data_writen_.clear();
    data_read_.clear();
    pkg_status = PKG_STATUS::COMPLETED;
    op_mode_ = OP_MODE::DEBUG;
}

bool Debugger::IsInDebugging() {
    return (op_mode_ == OP_MODE::DEBUG);
}
