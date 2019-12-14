#ifndef BASIC_DEF_H
#define BASIC_DEF_H

#include <QString>
#include <QDebug>

constexpr quint32 MAX_READ_DATA_INTERVAL = 50;  // 50 milliseconds
constexpr quint32 DEBUG_WRIRE_DATA_INTERVAL = 200;  // 0.2 second
constexpr quint32 DEBUG_MAX_WAIT_FOR_RSP_TIME = 1000;  // 1 second

constexpr wchar_t* A_LIMIT          = L"限流";
constexpr wchar_t* A_OVERLOAD       = L"过流";
constexpr wchar_t* SPD              = L"(MS) 缓启动";
constexpr wchar_t* LOCKED_ROTOR_TIME= L"(100 MS) 堵转保护时间";
constexpr wchar_t* REBOOT_COUNT     = L"堵转重启次数";
constexpr wchar_t* REBOOT_INTERVAL  = L"(100 MS) 重启等待时间";
constexpr wchar_t* STALL            = L"(MS) 软换相深度";
constexpr wchar_t* STARTP           = L"(1/220) 启动力度";
constexpr wchar_t* STARTT           = L"(MS) 启动周期";
constexpr wchar_t* EVOL_COUNT       = L"连续同步次数";
constexpr wchar_t* ZC_LIMIT         = L"(50 US) ZC滤波深度";
constexpr wchar_t* START_LIMIT      = L"(250 MS) 启动时间限定";
constexpr wchar_t* START_STEP       = L"强制同步次数";

constexpr wchar_t* DISCONNECTED     = L"未连接";
constexpr wchar_t* COM_SETTING      = L"串口设置";
constexpr wchar_t* COM_CONNECT      = L"连接串口";
constexpr wchar_t* COM_DISCONNECT   = L"断开连接";
constexpr wchar_t* LOAD_DATA        = L"加载参数";
constexpr wchar_t* SAVE_DATA        = L"保存参数";

constexpr wchar_t* WRITE_TO_DEVICE  = L"写入设备";
constexpr wchar_t* READ_FROM_DEVICE = L"读取设备";
constexpr wchar_t* HELP             = L"帮助文档";

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
