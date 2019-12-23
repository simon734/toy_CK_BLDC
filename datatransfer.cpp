#include "datatransfer.h"
#include <QDebug>

// 因为数据包有：型号  数据字节数（不包含校验和）  数据  校验和, 所以加3
constexpr unsigned int CK3864S_BYTE_COUNT = CK3864S_ITEM_COUNT + 3;
constexpr unsigned int CK3862S_BYTE_COUNT = CK3862S_ITEM_COUNT + 3;

constexpr quint8 CK3864S_CMD = 0xAA;
constexpr quint8 CK3862S_CMD = 0x55;

DataTransfer::DataTransfer() {

}

DataTransfer::~DataTransfer() {
    if (IsOpen()) {
        Close();
    }
}

DataTransfer* DataTransfer::Instance() {
    static DataTransfer inst;
    return &inst;
}

void DataTransfer::SetDataChangedCallback(IDataChanged *cb) {
    data_changed_cb_ = cb;
}

void DataTransfer::SetShowStatusCallback(IShowStatus *cb) {
    show_status_cb_ = cb;
}

void DataTransfer::Open() {
    qCritical() << "DataTransfer::Open";
    if (!IsOpen()) {
        SetReadyMode();
        SerialComm::Instance()->SetDataReadCallback(this);
        timer_id_ = startTimer(std::chrono::milliseconds(MAX_READ_DATA_INTERVAL).count());
        assert(timer_id_ != 0);
    }
}

void DataTransfer::Close() {
    qCritical() << "DataTransfer::Close";
    if (IsOpen()) {
        if (timer_id_ != 0) {
            killTimer(timer_id_);
            timer_id_ = 0;
        }
    }

    SetClosedMode();
}

bool DataTransfer::Write() {
    qCritical() << "DataTransfer::Write, op_mode=" << static_cast<int>(op_mode_);
    assert(op_mode_ == OP_MODE::READY);
    if (op_mode_ != OP_MODE::READY) {
        return false;
    }

    QByteArray data;
    bool is_ok = Pack(data);
    if (!is_ok) {
        assert(false);
        qCritical() << "DataTransfer::Write, failed to pack data";
        return false;
    }

    is_ok = SerialComm::Instance()->writeData(data);
    qCritical() << "DataTransfer::Write, writeData " << is_ok;
    if (is_ok) {
        SetWriteMode(std::move(data));
    }

    return is_ok;
}

// 发送读取命令为：38H  02H  00H  00H  02H
bool DataTransfer::Read() {
    qCritical() << "DataTransfer::Read, op_mode=" << static_cast<int>(op_mode_);
    assert(op_mode_ == OP_MODE::READY);
    if (op_mode_ != OP_MODE::READY) {
        return false;
    }

   QByteArray data;
   data.push_back(0x38);
   data.push_back(0x02);
   data.push_back(static_cast<char>(0));
   data.push_back(static_cast<char>(0));
   data.push_back(0x02);

   const auto is_ok = SerialComm::Instance()->writeData(data);
   assert(is_ok);
   qCritical() << "DataTransfer::Read, writeData " << is_ok;
   if (is_ok) {
       SetReadMode();
   }
   return is_ok;
}

bool DataTransfer::IsOpen() {
    return (op_mode_ != OP_MODE::CLOSED);
}

void DataTransfer::timerEvent(QTimerEvent *event) {
    if (event->timerId() != timer_id_) {
        return;
    }

    if (!IsInTransitionMode()) {
        return;
    }

    using namespace std::chrono;
    auto current_time = steady_clock::now();
    auto diff = duration_cast<milliseconds>(current_time - last_read_time_).count();
    if (diff >= MAX_READ_DATA_INTERVAL) {
        // report error
        qCritical() << "DataTransfer::timerEvent, timeout. "
                    << "op_mode=" << static_cast<int>(op_mode_)
                    << ", time diff=" << diff;
        // assert(false);
        SetReadyMode();
    }
}

void DataTransfer::onRead(QByteArray &&data) {
    qCritical() << "DataTransfer::onRead, op_mode=" << static_cast<int>(op_mode_);
    assert(IsInTransitionMode());
    if (!IsInTransitionMode()) {
        return;
    }

    last_read_time_ = std::chrono::steady_clock::now();
    data_read_.push_back(data);
    if (op_mode_ == OP_MODE::WRITE) {
        if (data_read_ == data_writen_) {
           // MessageBox
            qCritical() << "DataTransfer::onRead, data write completedly";
            SetReadyMode();
        }
    } else if (op_mode_ == OP_MODE::READ) {
        if (CheckPackage(data_read_)) {
            const auto data_size = data_read_.size();
            bool is_ok = false;
            if (data_size == CK3864S_BYTE_COUNT) {
                is_ok = DeviceManager::Instance().updateCK3864S(data_read_);
            } else if (data_size == CK3862S_BYTE_COUNT) {
                is_ok = DeviceManager::Instance().updateCK3862S(data_read_);
            }

            qCritical() << "DataTransfer::onRead, data read correctly, "
                        << "data size=" << data_size
                        << ", update result=" << is_ok;

            if (is_ok && data_changed_cb_) {
                data_changed_cb_->onDataChange();
            }
            SetReadyMode();
        } else {
            // qCritical() << "DataTransfer::onRead, data read incorrectly";
        }
    } else {
        // other unknown mode ?
        assert(false);
    }
}

void DataTransfer::onClose(int err) {
    qCritical() << "DataTransfer::onClose, err code=" << err
                << ", op_mode=" << static_cast<int>(op_mode_);
    if (IsInTransitionMode()) {
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
bool DataTransfer::Pack(QByteArray& data) {
    const auto& devMgr = DeviceManager::Instance();
    const auto& items = devMgr.getItems();
    const auto item_count = items.size();
    qCritical() << "DataTransfer::Pack, item count=" << item_count;
    assert(item_count == CK3864S_ITEM_COUNT ||
           item_count == CK3862S_ITEM_COUNT);
    if (item_count != CK3864S_ITEM_COUNT &&
        item_count != CK3862S_ITEM_COUNT) {
        return false;
    }

    data.clear();
    char cmd = 0;
    if (devMgr.isCK3864S()) {
        cmd = static_cast<char>(CK3864S_CMD);
    } else if (devMgr.isCK3862S()) {
        cmd = static_cast<char>(CK3862S_CMD);
    }
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

bool DataTransfer::CheckPackage(const QByteArray &data) {
    const auto byte_count = static_cast<unsigned int>(data.size());
    qCritical() << "DataTransfer::CheckPackage, data len=" << byte_count;
    if (byte_count == 0) {
        return false;
    }

    const auto cmd = static_cast<quint8>(data[0]);
    qCritical() << "DataTransfer::CheckPackage, cmd=" << cmd;
    if (cmd == CK3864S_CMD) {
       if (byte_count < CK3864S_BYTE_COUNT) {
           return false;
       }
    } else if (cmd == CK3862S_CMD) {
       if (byte_count < CK3862S_BYTE_COUNT) {
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
    qCritical() << "DataTransfer::CheckPackage, check result=" << is_ok
                << ", check_in=" << check_in
                << ", check_calc=" << check_calc;

    return is_ok;
}

quint8 DataTransfer::GetCheckSum(const QByteArray &data, int pos, int len) {
    if (pos < 0 || pos >= data.size()) {
        return 0;
    }

    unsigned int sum = 0;
    auto items = data.mid(pos, len);
    for (auto& item: items) {
        sum += static_cast<unsigned int>(item);
    }
    const quint8 check = static_cast<quint8>(sum & 0x000000FF);
    qCritical() << "DataTransfer::GetCheckSum, check=" << check;
    return check;
}

void DataTransfer::SetClosedMode() {
    qCritical() << "DataTransfer::SetClosedMode";
    data_writen_.clear();
    data_read_.clear();
    pkg_status = PKG_STATUS::COMPLETED;
    op_mode_ = OP_MODE::CLOSED;
}

void DataTransfer::SetReadyMode() {
    qCritical() << "DataTransfer::SetReadyMode";
    data_writen_.clear();
    data_read_.clear();
    pkg_status = PKG_STATUS::COMPLETED;
    op_mode_ = OP_MODE::READY;
}

void DataTransfer::SetReadMode() {
    qCritical() << "DataTransfer::SetReadMode";
    data_writen_.clear();
    data_read_.clear();
    pkg_status = PKG_STATUS::ONGOING;
    op_mode_ = OP_MODE::READ;
    last_read_time_ = std::chrono::steady_clock::now();
}

void DataTransfer::SetWriteMode(QByteArray &&data) {
    qCritical() << "DataTransfer::SetWriteMode";
    data_writen_ = std::move(data);
    data_read_.clear();
    pkg_status = PKG_STATUS::ONGOING;
    op_mode_ = OP_MODE::WRITE;
    last_read_time_ = std::chrono::steady_clock::now();
}

bool DataTransfer::IsInTransitionMode() {
    return (op_mode_ == OP_MODE::WRITE ||
            op_mode_ == OP_MODE::READ);
}
