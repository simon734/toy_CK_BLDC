#include "itemwidget.h"
#include "mainwindow.h"
#include <string>

constexpr unsigned int VSP_MIN = 0;
constexpr unsigned int VSP_MAX = 222;
constexpr unsigned int POLE_MIN = 1;
constexpr unsigned int POLE_MAX = 250;
constexpr unsigned int MAX_ITEM_COUNT = 13;
constexpr const char* ICON_BUTTON_ON = ":/images/button_on.png";
constexpr const char* ICON_BUTTON_OFF = ":/images/button_off.png";
constexpr const char* ICON_SWITCH_ON = ":/images/switch_on.png";
constexpr const char* ICON_SWITCH_OFF = ":/images/switch_off.png";

void DeviceBindData(MainWindow* ui) {
    assert(ui != nullptr);
    if (ui == nullptr) {
        return;
    }

   const auto& items = DeviceManager::Instance().getItems();
   ui->removeItemsConnection();

   ItemWidgetHelper item_widget(ui);
   unsigned int index = 1;
   for (const auto& item: items) {
       item_widget.Reset(index++);
       item_widget.SetVisible(true);
       item_widget.SetValue(item);
   }

   while (index <= MAX_ITEM_COUNT) {
       item_widget.Reset(index++);
       item_widget.SetVisible(false);
   }
}

void DeviceEnableState(MainWindow* ui, bool enable) {
    assert(ui != nullptr);
    if (ui == nullptr) {
        return;
    }

   const auto& items = DeviceManager::Instance().getItems();
   ui->removeItemsConnection();

   ItemWidgetHelper item_widget(ui);
   for (unsigned int index = 1; index <= items.size(); index++) {
       item_widget.Reset(index);
       item_widget.Enable(enable);
   }
}

DbgWidgetMgr::DbgWidgetMgr() {
}

void DbgWidgetMgr::findAllItems() {
    sb_pole = ui->findChild<QSpinBox*>("sb_pole");
    gb_pi = ui->findChild<QGroupBox*>("gb_pi");
    sb_p = ui->findChild<QSpinBox*>("sb_p");
    sb_i = ui->findChild<QSpinBox*>("sb_i");
    sb_pi_time = ui->findChild<QSpinBox*>("sb_pi_time");

    pb_debug_switch = ui->findChild<QPushButton*>("pb_power");
    pb_fr = ui->findChild<QPushButton*>("pb_fr");
    pb_bk = ui->findChild<QPushButton*>("pb_bk");
}

bool DbgWidgetMgr::init(MainWindow *parent) {
    if (has_inited) {
        return true;
    }

    ui = parent;
    assert(ui != nullptr);
    if (ui == nullptr) {
        return false;
    }

    findAllItems();
    initVSP();
    initPole();

    setDebugIcon();
    setFrIcon();
    setBkIcon();

    has_inited = true;
    return true;
}

void DbgWidgetMgr::uninit() {
    ui = nullptr;
    has_inited = false;

    pb_debug_switch = nullptr;
    pb_fr = nullptr;
    pb_bk = nullptr;
    is_in_debugging = false;
    is_fr_on = false;
    is_bk_on = false;
}

bool DbgWidgetMgr::IsInDebugging() {
    return is_in_debugging;
}

void DbgWidgetMgr::setEnableDbgState(bool enable) {
    if (pb_debug_switch) {
        pb_debug_switch->setEnabled(enable);
    }
}

void DbgWidgetMgr::setEnableState(bool enable, bool is_global) {
    if (sb_pole == nullptr || gb_pi == nullptr || sb_p == nullptr ||
    sb_i == nullptr || sb_pi_time == nullptr ||
    pb_debug_switch == nullptr || pb_fr == nullptr || pb_bk == nullptr) {
        return;
    }

    if (is_global) {
        pb_debug_switch->setEnabled(enable);
    }
    
    sb_pole->setEnabled(enable);
    gb_pi->setEnabled(enable);
    sb_p->setEnabled(enable);
    sb_i->setEnabled(enable);
    sb_pi_time->setEnabled(enable);
    pb_fr->setEnabled(enable);
    pb_bk->setEnabled(enable);

    if (enable) {
        onPIFlagChanged();
    }
}

void DbgWidgetMgr::flickDebug() {
   is_in_debugging = !is_in_debugging;
    setDebugIcon();
    setEnableState(is_in_debugging, false);
}

void DbgWidgetMgr::flickFr() {
   is_fr_on = !is_fr_on;
   setFrIcon();
}

void DbgWidgetMgr::flickBk() {
   is_bk_on = !is_bk_on;
   setBkIcon();
}

void DbgWidgetMgr::onPIFlagChanged() {
    gb_pi->setChecked(is_pi_checked_);
    sb_p->setEnabled(is_pi_checked_);
    sb_i->setEnabled(is_pi_checked_);
    sb_pi_time->setEnabled(is_pi_checked_);
}

void DbgWidgetMgr::flickPI(bool checked) {
    if (sb_p == nullptr || sb_i == nullptr || sb_pi_time == nullptr) {
        assert(false);
        return;
    }

    is_pi_checked_ = checked;
    onPIFlagChanged();
}

void DbgWidgetMgr::setDebugIcon() {
    if (pb_debug_switch == nullptr) {
        return;
    }

    QIcon icon((is_in_debugging ? ICON_BUTTON_OFF : ICON_BUTTON_ON));
    pb_debug_switch->setIcon(icon);
    pb_debug_switch->setIconSize(QSize(100, 100));
}

void DbgWidgetMgr::setFrIcon() {
    if (pb_fr == nullptr) {
        return;
    }

    QIcon icon((is_fr_on ? ICON_SWITCH_OFF : ICON_SWITCH_ON));
    pb_fr->setIcon(icon);
    pb_fr->setIconSize(QSize(87, 32));
}

void DbgWidgetMgr::setBkIcon() {
    if (pb_bk == nullptr) {
        return;
    }

    QIcon icon((is_bk_on ? ICON_SWITCH_OFF : ICON_SWITCH_ON));
    pb_bk->setIcon(icon);
    pb_bk->setIconSize(QSize(87, 32));
}

void DbgWidgetMgr::initVSP() {
    QLineEdit* edit = ui->findChild<QLineEdit*>("le_vsp");
    QSlider* slider = ui->findChild<QSlider*>("sd_vsp");
    assert(slider != nullptr);
    assert(edit != nullptr);
    if (slider == nullptr || edit == nullptr) {
        return;
    }

    slider->setRange(VSP_MIN, VSP_MAX);
    edit->setValidator(new QIntValidator(VSP_MIN, VSP_MAX));
    ui->connectItemChange(slider, edit, false);
    edit->setText("0");
}

void DbgWidgetMgr::initPole() {
    assert(sb_pole != nullptr && gb_pi != nullptr && sb_p != nullptr &&
            sb_i != nullptr && sb_pi_time != nullptr);
    if (sb_pole == nullptr || gb_pi == nullptr || sb_p == nullptr ||
            sb_i == nullptr || sb_pi_time == nullptr) {
        return;
    }

    sb_pole->setMinimum(POLE_MIN);
    sb_pole->setMaximum(POLE_MAX);
    sb_pole->setSingleStep(1);
    sb_pole->setValue(POLE_MIN);

/*
    QHBoxLayout* hbox = new QHBoxLayout(gb_pi);
    hbox->addLayout(ui->findChild<QHBoxLayout*>("hl_p"));
    hbox->addLayout(ui->findChild<QHBoxLayout*>("hl_i"));
    hbox->addLayout(ui->findChild<QHBoxLayout*>("hl_pi_time"));
    hbox->addStretch(1);
    gb_pi->setLayout(hbox);
    */

    flickPI(is_pi_checked_);
}

void ItemWidgetHelper::SetValue(const DeviceItem& item) {
    if (name != nullptr) {
        name->setText(item.getName());
    }

    if (min != nullptr && max != nullptr) {
        min->setText(item.getMinString());
        max->setText(item.getMaxString());
    }

    if (desc != nullptr) {
        desc->setText(item.getDesc());
    }

    if (slider != nullptr && edit != nullptr) {
        slider->setRange(item.getMin(), item.getMax());
        edit->setValidator(new QIntValidator(item.getMin(), item.getMax()));
    }
    if (ui != nullptr) {
        ui->connectItemChange(slider, edit);
    }
    if (edit != nullptr) {
        edit->setText(item.getValueString());
    }
}

void ItemWidgetHelper::SetVisible(bool is_visible) {
    if (name != nullptr) {
        name->setVisible(is_visible);
    }

    if (min != nullptr && max != nullptr) {
        min->setVisible(is_visible);
        max->setVisible(is_visible);
    }

    if (desc != nullptr) {
        desc->setVisible(is_visible);
    }

    if (slider != nullptr && edit != nullptr) {
        slider->setVisible(is_visible);
        edit->setVisible(is_visible);
    }
}

void ItemWidgetHelper::Enable(bool enable) {
    if (slider != nullptr && edit != nullptr) {
        slider->setEnabled(enable);
        edit->setEnabled(enable);
    }
}

void ItemWidgetHelper::Reset(unsigned int index) {
    name = nullptr;
    min = nullptr;
    max = nullptr;
    desc = nullptr;
    slider = nullptr;
    edit = nullptr;

    if (ui == nullptr) {
        return;
    }

    const std::string prefix = "item_" + std::to_string(index++) + "_";
    name = ui->findChild<QLabel*>(std::string(prefix+"name").c_str());

    min = ui->findChild<QLabel*>(std::string(prefix+"min").c_str());
    max = ui->findChild<QLabel*>(std::string(prefix+"max").c_str());

    desc = ui->findChild<QLabel*>(std::string(prefix+"desc").c_str());

    slider = ui->findChild<QSlider*>(std::string(prefix+"range").c_str());
    edit = ui->findChild<QLineEdit*>(std::string(prefix+"value").c_str());
}

ItemWidgetHelper::ItemWidgetHelper(MainWindow *mw) {
    ui = mw;
}
