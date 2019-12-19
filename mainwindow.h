#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QLabel>
#include "deviceitem.h"
#include "serialcomm.h"
#include "itemwidget.h"
#include "basic_def.h"

QT_BEGIN_NAMESPACE
class QAction;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public IDataChanged, public IShowStatus {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void removeItemsConnection();
    void connectItemChange(QSlider* slider, QLineEdit* edit, bool need_record=true);

    // IDataChanged interface
    virtual void onDataChange() override;
    virtual void onError(int err) override;

    // IShowStatus interface
    virtual void setStatus(const QString& s) override;

private slots:
    bool openSerialPort();
    void closeSerialPort();
    void load();
    bool save();
    void write();
    void read();
    void help();
    void onDbgBtnClicked();
    void onFrBtnClicked();
    void onBkBtnClicked();
    void onPIChanged(bool checked);

private:
    void createAction();
    void connectDbgItems();
    void deviceTypeChanged(const QString& type);

    bool isConnected();
    bool isInNormalMode();
    bool isInDebugMode();
    void setDisconnectMode();
    void setNormalMode();
    void setDebugMode();

private:
    Ui::MainWindow *ui = nullptr;

    DbgWidgetMgr widgetMgr;
    QLabel *m_status = nullptr;
    QList<QMetaObject::Connection> item_connections;

    enum class OP_MODE {
        DISCONNECT = 0,
        NORMAL = 1,
        DEBUG = 2
    };
    OP_MODE op_mode_ = OP_MODE::DISCONNECT;
};

#endif // MAINWINDOW_H
