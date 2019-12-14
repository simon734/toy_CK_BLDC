#include "deviceitem.h"
#include <sstream>
#include <iomanip>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include "mainwindow.h"
#include "basic_def.h"


static const char* FILE_CK3864S_BIN = "CK3864S.bin";
static const char* FILE_CK3862S_BIN = "CK3862S.bin";
static const char* FILE_CK3864S_JSON = "CK3864S.json";
static const char* FILE_CK3862S_JSON = "CK3862S.json";

static const char* J_DEVICENAME = "device_name";
static const char* J_VERSION = "version";
static const char* J_META = "meta";
static const char* J_ITEMS = "items";

static const char* J_NAME       = "name";
static const char* J_ITEM_TYPE  = "item_type";
static const char* J_DESC       = "desc";
static const char* J_MIN_VALUE  = "min_value";
static const char* J_MAX_VALUE  = "max_value";
static const char* J_VALUE      = "value";

static const unsigned int VERSION = 1;  // 递增

DeviceItem::DeviceItem() {
}

void DeviceItem::write(QJsonObject &json) const {
   json[J_NAME]      = name_;
   json[J_DESC]      = desc_;
   json[J_MIN_VALUE] = std::to_string(min_value_).c_str();
   json[J_MAX_VALUE] = std::to_string(max_value_).c_str();
   json[J_VALUE]     = std::to_string(current_value_).c_str();
}

bool DeviceItem::read(const QJsonObject &json) {
    bool is_ok = true;

    if (json.contains(J_NAME) && json[J_NAME].isString()) {
        name_ = json["name"].toString();
    } else {
        is_ok = false;
    }

    if (is_ok && json.contains(J_DESC) && json[J_DESC].isString()) {
        desc_ = json[J_DESC].toString();
    } else {
        is_ok = false;
    }

    if (is_ok && json.contains(J_MIN_VALUE) && json[J_MIN_VALUE].isString()) {
        auto valueStr = json[J_MIN_VALUE].toString();
        setMinValue(valueStr);
    } else {
        is_ok = false;
    }


    if (is_ok && json.contains(J_MAX_VALUE) && json[J_MAX_VALUE].isString()) {
        auto valueStr = json[J_MAX_VALUE].toString();
        setMaxValue(valueStr);
    } else {
        is_ok = false;
    }


    if (is_ok && json.contains(J_VALUE) && json[J_VALUE].isString()) {
        auto valueStr = json[J_VALUE].toString();
        setValue(valueStr);
    } else {
        is_ok = false;
    }

    assert(is_ok);
    return is_ok;
}

void DeviceItem::makeItem(const char* n, ValueType min, ValueType max, ValueType value, const QString& d) {
   name_ = n;
   min_value_ = min;
   max_value_ = max;
   current_value_ = value;
   desc_ = d;
}

void DeviceItem::setMinValue(const QString& strValue) {
    min_value_ = static_cast<ValueType>(strValue.toShort());
}

void DeviceItem::setMaxValue(const QString& strValue) {
    max_value_ = static_cast<ValueType>(strValue.toShort());
}

void DeviceItem::setValue(const QString& strValue) {
    current_value_ = static_cast<ValueType>(strValue.toShort());
}

void DeviceItem::setValue(ValueType value) {
    current_value_ = value;
}

QString DeviceItem::getValueString() const {
    return std::to_string(current_value_).c_str();
}

void DeviceItem::clear() {
    name_.clear();
    min_value_ = 0;
    max_value_ = 1;
    current_value_ = 0;
    desc_.clear();
}

QString DeviceItem::getName() const {
    return name_;
}

QString DeviceItem::getDesc() const {
    return desc_;
}

QString DeviceItem::getMinString() const {
    return QString::number(min_value_);
}

QString DeviceItem::getMaxString() const {
    return std::to_string(max_value_).c_str();
}

ValueType DeviceItem::getMin() const {
    return min_value_;
}

ValueType DeviceItem::getMax() const {
    return max_value_;
}

ValueType DeviceItem::getValue() const {
    return current_value_;
}

////////////////////////////////////////////////////////
DeviceManager::DeviceManager() {
}

DeviceManager &DeviceManager::Instance() {
    static DeviceManager inst;
    return inst;
}

void DeviceManager::load_CK3864S_Default() {
    device_type_ = DeviceType::CK3864S;
    items_.clear();

    addItem("A-Limit:",          1, 250      , 12, A_LIMIT          );
    addItem("A-OverLoad:",       1, 250      , 20, A_OVERLOAD       );
    addItem("Spd:",              1, 100      , 10, SPD              );
    addItem("Locked Rotor-Time:",1, 100      , 5,  LOCKED_ROTOR_TIME);
    addItem("Reboot-Count:",     1, 255      , 5,  REBOOT_COUNT     );
    addItem("Reboot-Interval:",  1, 250      , 30, REBOOT_INTERVAL  );
    addItem("Stall:",            1, 250      , 25, STALL            );
    addItem("StartP:",           1, 120      , 15, STARTP           );
    addItem("StartT:",           5, 100      , 25, STARTT           );
    addItem("Evol-Count:",       1, 30       , 5,  EVOL_COUNT       );
    addItem("ZC-Limit:",         1, 200      , 10, ZC_LIMIT         );
    addItem("Start-Limit:",      1, 250      , 4,  START_LIMIT      );
    addItem("Start-Step:",       1, 250      , 2,  START_STEP       );
}

void DeviceManager::load_CK3862S_Default() {
    device_type_ = DeviceType::CK3862S;
    items_.clear();

    addItem("A-Limit:",          1, 250      , 12, A_LIMIT          );
    addItem("A-OverLoad:",       1, 250      , 20, A_OVERLOAD       );
    addItem("Spd:",              1, 100      , 10, SPD              );
    addItem("Locked Rotor-Time:",1, 100      , 5 , LOCKED_ROTOR_TIME);
    addItem("Reboot-Count:",     1, 255      , 5 , REBOOT_COUNT     );
    addItem("Reboot-Interval:",  1, 250      , 30, REBOOT_INTERVAL  );
    addItem("Stall:",            1, 250      , 25, STALL            );
    addItem("StartP:",           1, 120      , 5 , STARTP           );
}

const ItemVector &DeviceManager::getItems() const {
    return items_;
}

DeviceManager::DeviceType DeviceManager::getDeviceType() {
    return device_type_;
}

bool DeviceManager::isCK3864S() const {
    return (device_type_ == DeviceType::CK3864S);
}

bool DeviceManager::isCK3862S() const {
    return (device_type_ == DeviceType::CK3862S);
}

bool DeviceManager::isInValidRange(const QByteArray &data) const {
    if (data.size() != static_cast<int>(items_.size())) {
        return false;
    }

    bool all_match = true;
    quint32 index = 0;
    for (auto& item: items_) {
        auto value = data[index++];
        if (value < item.getMin() || value > item.getMax()) {
            all_match = false;
            break;
        }
    }

    return all_match;
}

void DeviceManager::updateValue(const QByteArray &data) {
    if (data.size() != static_cast<int>(items_.size())) {
        return;
    }

    quint32 index = 0;
    for (auto& item: items_) {
        auto value = static_cast<ValueType>(data[index++]);
        item.setValue(value);
    }
}

bool DeviceManager::updateCK3864S(const QByteArray &data) {
    if (device_type_ != DeviceType::CK3864S) {
        return false;
    }

    if (data.size() != CK3864S_ITEM_COUNT ||
            data.size() != static_cast<int>(items_.size())) {
       return false;
    }

    if (!isInValidRange(data)) {
       return false;
    }

    updateValue(data);
    return true;
}

bool DeviceManager::updateCK3862S(const QByteArray &data) {
    if (device_type_ != DeviceType::CK3862S) {
        return false;
    }

    if (data.size() != CK3862S_ITEM_COUNT ||
            data.size() != static_cast<int>(items_.size())) {
       return false;
    }

    if (!isInValidRange(data)) {
       return false;
    }

    updateValue(data);
    return true;
}

std::string DeviceManager::getDeviceName() const {
    if (device_type_ == DeviceType::CK3864S) {
        return CK3864S;
    }
    return CK3862S;
}

void DeviceManager::refreshItemData(MainWindow* ui) {
    assert(ui != nullptr);
    if (ui == nullptr) {
        return;
    }

    unsigned int index = 1;
    for (auto& item: items_) {
        std::string prefix = "item_" + std::to_string(index) + "_";
        QLineEdit* edit = ui->findChild<QLineEdit*>(std::string(prefix+"value").c_str());
        assert(edit != nullptr);
        if (edit != nullptr) {
            item.setValue(edit->text());
        }
        index++;
    }
}

QString DeviceManager::defaultFileName(SaveFormat save_format) const {
    if (device_type_ == DeviceType::CK3864S) {
        return ((save_format == Json) ? FILE_CK3864S_JSON : FILE_CK3864S_BIN);
    } else {
        return ((save_format == Json) ? FILE_CK3862S_JSON : FILE_CK3862S_BIN);
    }
}

void DeviceManager::addItem(const char *name, ValueType min, ValueType max, ValueType value, const wchar_t* desc) {
   DeviceItem item;
   item.makeItem(name, min, max, value, (desc != nullptr ? QString::fromWCharArray(desc) : QString()));
   items_.push_back(item);
}

bool DeviceManager::load_from_file(SaveFormat save_format) {
    bool is_ok = false;
    QFile loadFile(defaultFileName(save_format));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return is_ok;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(save_format == Json
        ? QJsonDocument::fromJson(saveData)
        : QJsonDocument::fromBinaryData(saveData));

    is_ok = read(loadDoc.object());

    QTextStream(stdout) << "Loaded save result: " << is_ok << ", using "
                        << (save_format != Json ? "binary " : "") << "JSON...\n";

    assert(is_ok);
    return is_ok;
}

bool DeviceManager::save_to_file(SaveFormat save_format) {
    bool is_ok = false;
    QFile saveFile(defaultFileName(save_format));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return is_ok;
    }

    QJsonObject gameObject;
    write(gameObject);
    QJsonDocument saveDoc(gameObject);
    const auto wtByte = saveFile.write(save_format == Json
        ? saveDoc.toJson()
        : saveDoc.toBinaryData());

    is_ok = (wtByte != -1);
    assert(is_ok);
    return is_ok;
}

bool DeviceManager::readMeta(const QJsonObject &json) {
    bool has_name = false;
    if (json.contains(J_DEVICENAME) && json[J_DEVICENAME].isString()) {
        const auto deviceName = json[J_DEVICENAME].toString();
        if (deviceName == CK3864S) {
            device_type_ = DeviceType::CK3864S;
            has_name = true;
        } else if (deviceName == CK3862S) {
            device_type_ = DeviceType::CK3862S;
            has_name = true;
        }
    }

    bool is_ver_ok = false;
    if (json.contains(J_VERSION) && json[J_VERSION].isString()) {
        auto verStr = json[J_VERSION].toString();
        auto verNum = verStr.toInt();
        if (verNum == VERSION) {
            is_ver_ok = true;
        }
    }

    const auto is_ok = (has_name && is_ver_ok);
    assert(is_ok);
    return is_ok;
}

void DeviceManager::writeMeta(QJsonObject &json) const {
   json[J_DEVICENAME] = getDeviceName().c_str();
   json[J_VERSION] = std::to_string(VERSION).c_str();
}

bool DeviceManager::read(const QJsonObject &json) {
    bool is_ok = false;
    if (json.contains(J_META) && json[J_META].isObject())
        is_ok = readMeta(json[J_META].toObject());

    if (!is_ok) {
        return false;
    }

    if (json.contains(J_ITEMS) && json[J_ITEMS].isArray()) {
        QJsonArray itemArray = json[J_ITEMS].toArray();
        items_.clear();
        items_.reserve(static_cast<size_t>(itemArray.size()));
        for (int level= 0; level< itemArray.size(); ++level) {
            QJsonObject itemObject = itemArray[level].toObject();
            DeviceItem item;
            item.read(itemObject);
            items_.push_back(item);
        }
    }
    return true;
}

void DeviceManager::write(QJsonObject &json) const {
    QJsonObject meta;
    writeMeta(meta);
    json[J_META] = meta;

    QJsonArray itemArray;
    for (const auto &item : items_) {
        QJsonObject levelObject;
        item.write(levelObject);
        itemArray.append(levelObject);
    }
    json[J_ITEMS] = itemArray;
}
