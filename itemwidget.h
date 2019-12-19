#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include "deviceitem.h"
#include <QtWidgets>
#include <QLineEdit>
#include <QSlider>

class MainWindow;
class ItemWidgetHelper {
public:
    ItemWidgetHelper(MainWindow* mw);
    void Reset(unsigned int index);
    void SetValue(const DeviceItem& item);
    void SetVisible(bool is_visible);
    void Enable(bool enable);

private:
    MainWindow* ui = nullptr;
    QLabel* name = nullptr;
    QLabel* min = nullptr;
    QLabel* max = nullptr;
    QLabel* desc = nullptr;
    QSlider* slider = nullptr;
    QLineEdit* edit = nullptr;
};
void DeviceBindData(MainWindow* ui);
void DeviceEnableState(MainWindow* ui, bool enable);

class DbgWidgetMgr {
public:
    DbgWidgetMgr();
    bool init(MainWindow* parent);
    void uninit();

void EnableDebugBtn();
    void flickDebug();
    void flickFr();
    void flickBk();
    void flickPI(bool checked);

    void setEnableDbgState(bool enable);
    void setEnableState(bool enable, bool is_global = true);
    bool IsInDebugging();

private:
    void findAllItems();
    void initVSP();
    void initPole();
    void onPIFlagChanged();
    void setDebugIcon();
    void setFrIcon();
    void setBkIcon();

private:
    MainWindow* ui = nullptr;
    bool has_inited = false;
    bool is_pi_checked_ = true;

    QSpinBox* sb_pole = nullptr;
    QGroupBox* gb_pi = nullptr;
    QSpinBox* sb_p = nullptr;
    QSpinBox* sb_i = nullptr;
    QSpinBox* sb_pi_time = nullptr;

    QPushButton* pb_debug_switch = nullptr;
    QPushButton* pb_fr = nullptr;
    QPushButton* pb_bk = nullptr;
    bool is_in_debugging = false;
    bool is_fr_on = false;
    bool is_bk_on = false;
};

#endif // ITEMWIDGET_H
