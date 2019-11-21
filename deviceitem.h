#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <cstdint>
#include <string>
#include <vector>
#include <QJsonObject>


static const char* CK3864S = "CK3864S";
static const char* CK3862S = "CK3862S";

constexpr unsigned int CK3864S_ITEM_COUNT = 13;
constexpr unsigned int CK3862S_ITEM_COUNT = 8;


class DeviceItem;
using ItemVector = std::vector<DeviceItem>;
using ValueType = quint8;

class DeviceItem {
public:
    DeviceItem();
    void write(QJsonObject &json) const;
    bool read(const QJsonObject &json);

    void makeItem(const char* name_, ValueType min, ValueType max, ValueType value, const QString& desc_);
    void setValue(const QString& strValue);
    void setValue(ValueType value);
    void setMinValue(const QString& strValue);
    void setMaxValue(const QString& strValue);
    QString getValueString() const;
    void clear();

    QString getName() const;
    QString getDesc() const;
    QString getMinString() const;
    QString getMaxString() const;

    ValueType getMin() const;
    ValueType getMax() const;
    ValueType getValue() const;

private:
    QString name_;
    ValueType min_value_ = 0;
    ValueType max_value_ = 1;
    ValueType current_value_ = 0;

    QString desc_;
};

class MainWindow;
class DeviceManager {
public:
    enum SaveFormat {Json, Binary};
    enum class DeviceType {CK3864S, CK3862S};

public:
    static DeviceManager& Instance();
    void load_CK3864S_Default();
    void load_CK3862S_Default();
    const ItemVector& getItems() const;
    DeviceType getDeviceType();
    bool isCK3864S() const;
    bool isCK3862S() const;
    bool updateCK3864S(const QByteArray &data);
    bool updateCK3862S(const QByteArray &data);
    std::string getDeviceName() const;
    void refreshItemData(MainWindow* ui);
    bool load_from_file(SaveFormat save_format);
    bool save_to_file(SaveFormat save_format);

private:
    DeviceManager();
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

private:
    bool isInValidRange(const QByteArray& data) const;
    void updateValue(const QByteArray& data);
    bool read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    bool readMeta(const QJsonObject &json);
    void writeMeta(QJsonObject &json) const;
    QString defaultFileName(SaveFormat save_format) const;
    void addItem(const char* name, ValueType min, ValueType max, ValueType value, const QString& desc = QString());

private:
    ItemVector items_;
    DeviceType device_type_ = DeviceType::CK3864S;
};

#endif // DEVICEITEM_H
