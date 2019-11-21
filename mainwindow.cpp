#include "mainwindow.h"
#include <sstream>
#include <iomanip>
#include "ui_mainwindow.h"
#include "datatransfer.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_status(new QLabel) {
            ui->setupUi(this);
    // setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(width(), height());
    ui->statusbar->addWidget(m_status);
    setStatus(QStringLiteral("未连接"));
    DataTransfer::Instance()->SetShowStatusCallback(this);
    SerialComm::Instance()->SetShowStatusCallback(this);
    widgetMgr.init(this);

    createAction();
    connectDbgItems();
    deviceTypeChanged(CK3864S);
    SetDisconnectMode();
}

MainWindow::~MainWindow() {
    DataTransfer::Instance()->SetShowStatusCallback(nullptr);
    SerialComm::Instance()->SetShowStatusCallback(nullptr);

    delete m_status;
    m_status = nullptr;

    delete ui;
    ui = nullptr;
}

void MainWindow::removeItemsConnection() {
    for (auto c: item_connections) {
        QObject::disconnect(c);
    }
    item_connections.clear();
}

void MainWindow::deviceTypeChanged(const QString &type) {
   if (type == CK3862S) {
       DeviceManager::Instance().load_CK3862S_Default();
   }  else if (type == CK3864S) {
       DeviceManager::Instance().load_CK3864S_Default();
   } else {
       assert(false);
       return;
   }

   DeviceBindData(this);
}

bool MainWindow::isConnected() {
    return (op_mode_ != OP_MODE::DISCONNECT);
}

bool MainWindow::isInNormalMode() {
    return (op_mode_ == OP_MODE::NORMAL);
}

bool MainWindow::isInDebugMode() {
    return (op_mode_ == OP_MODE::DEBUG);
}

void MainWindow::SetDisconnectMode() {
    qCritical() << "MainWindow::SetDisconnectMode";
    op_mode_ = OP_MODE::DISCONNECT;

    auto dt = DataTransfer::Instance();
    dt->SetDataChangedCallback(nullptr);
    dt->Close();
}

void MainWindow::SetNormalMode() {
    qCritical() << "MainWindow::SetNormalMode";
    op_mode_ = OP_MODE::NORMAL;

    auto dt = DataTransfer::Instance();
    dt->SetDataChangedCallback(this);
    dt->Open();
}

void MainWindow::SetDebugMode() {
    qCritical() << "MainWindow::SetDebugMode";
    op_mode_ = OP_MODE::DEBUG;

    auto dt = DataTransfer::Instance();
    dt->SetDataChangedCallback(nullptr);
    dt->Close();
}

void MainWindow::load() {
    qCritical() << "MainWindow::load, op_mode=" << static_cast<int>(op_mode_);
    if (!isInNormalMode()) {
        assert(false);
        return;
    }

    auto& devMgr = DeviceManager::Instance();
    auto is_ok = devMgr.load_from_file(DeviceManager::Json);
    if (is_ok) {
        DeviceBindData(this);
    }
}

bool MainWindow::save() {
    qCritical() << "MainWindow::save, op_mode=" << static_cast<int>(op_mode_);
    if (!isInNormalMode()) {
        assert(false);
        return false;
    }

    auto& devMgr = DeviceManager::Instance();
    devMgr.refreshItemData(this);
    devMgr.save_to_file(DeviceManager::Json);
    return true;
}

void MainWindow::write() {
    qCritical() << "MainWindow::write, op_mode=" << static_cast<int>(op_mode_);
    if (!isInNormalMode()) {
        assert(false);
        return;
    }

    DataTransfer::Instance()->Write();
}

void MainWindow::read() {
    qCritical() << "MainWindow::read, op_mode=" << static_cast<int>(op_mode_);
    if (!isInNormalMode()) {
        assert(false);
        return;
    }

    DataTransfer::Instance()->Read();
}

void MainWindow::help() {
    qCritical() << "MainWindow::help";
}

void MainWindow::onPowerBtnClicked() {
    qCritical() << "MainWindow::onPowerBtnClicked, op_mode=" << static_cast<int>(op_mode_);
    if (!isConnected()) {
        assert(false);
        return;
    }

   widgetMgr.flickPower();
}

void MainWindow::onFrBtnClicked() {
    qCritical() << "MainWindow::onFrBtnClicked, op_mode=" << static_cast<int>(op_mode_);
    if (!isInDebugMode()) {
        assert(false);
        return;
    }

   widgetMgr.flickFr();
}

void MainWindow::onBkBtnClicked() {
    qCritical() << "MainWindow::onBkBtnClicked, op_mode=" << static_cast<int>(op_mode_);
    if (!isInDebugMode()) {
        assert(false);
        return;
    }

    widgetMgr.flickBk();
}

void MainWindow::onPIChanged(bool checked) {
    qCritical() << "MainWindow::onPIChange, op_mode=" << static_cast<int>(op_mode_)
                << ", checked=" << checked;
    if (!isInDebugMode()) {
        assert(false);
        return;
    }

    widgetMgr.flickPI(checked);
}

void MainWindow::createAction() {
    QToolBar *commToolBar = addToolBar(tr("Comm"));
    const QIcon settingIcon = QIcon::fromTheme("document-open", QIcon(":/images/settings.png"));
    QAction *settingAct = new QAction(settingIcon, tr("串口设置"), this);
    connect(settingAct, &QAction::triggered, SerialComm::Instance(), &SerialComm::showSetting);
    commToolBar->addAction(settingAct);

    const auto connIcon = QIcon(":/images/connect.png");
    QAction *connAct = new QAction(connIcon, tr("连接串口"), this);
    connect(connAct, &QAction::triggered, this, &MainWindow::openSerialPort);
    commToolBar->addAction(connAct);

    const auto disconnIcon = QIcon(":/images/disconnect.png");
    QAction *disconnAct = new QAction(disconnIcon, tr("断开连接"), this);
    connect(disconnAct, &QAction::triggered, this, &MainWindow::closeSerialPort);
    commToolBar->addAction(disconnAct);

    ////////////////////////////////////////////////
    QToolBar *fileToolBar = addToolBar(tr("File"));
    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("加载参数"), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::load);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("保存参数"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileToolBar->addAction(saveAct);

    ////////////////////////////////////////////////
    QToolBar *typeToolBar = addToolBar(tr("Type"));
    QComboBox *typeCombo = new QComboBox;
    typeCombo->setEditable(false);
    typeCombo->addItem(CK3864S);
    typeCombo->addItem(CK3862S);
    connect(typeCombo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::deviceTypeChanged);
    typeToolBar->addWidget(typeCombo);

    ////////////////////////////////////////////////
    QToolBar *operToolBar = addToolBar(tr("Operation"));
    const QIcon downIcon = QIcon::fromTheme("document-down", QIcon(":/images/down.png"));
    QAction *downAct = new QAction(downIcon, tr("写入设备"), this);
    connect(downAct, &QAction::triggered, this, &MainWindow::write);
    operToolBar->addAction(downAct);

    const QIcon upIcon = QIcon::fromTheme("document-up", QIcon(":/images/up.png"));
    QAction *upAct = new QAction(upIcon, tr("读取设备"), this);
    connect(upAct, &QAction::triggered, this, &MainWindow::read);
    operToolBar->addAction(upAct);

    ////////////////////////////////////////////////
    QToolBar *helpToolBar = addToolBar(tr("Help"));
    const QIcon helpIcon = QIcon::fromTheme("document-help", QIcon(":/images/help.png"));
    QAction *helpAct = new QAction(helpIcon, tr("帮助文档"), this);
    connect(helpAct, &QAction::triggered, this, &MainWindow::help);
    helpToolBar->addAction(helpAct);
}

void MainWindow::connectDbgItems() {
    auto pb_power = findChild<QPushButton*>("pb_power");
    auto pb_fr = findChild<QPushButton*>("pb_fr");
    auto pb_bk = findChild<QPushButton*>("pb_bk");
    auto cb_pi = findChild<QCheckBox*>("cb_pi");
    assert(pb_power != nullptr && pb_fr != nullptr && pb_bk != nullptr && cb_pi != nullptr);
    if (pb_power) {
        connect(pb_power, SIGNAL(released()), this, SLOT(onPowerBtnClicked()));
    }
    if (pb_fr) {
        connect(pb_fr, SIGNAL(released()), this, SLOT(onFrBtnClicked()));
    }
    if (pb_bk) {
        connect(pb_bk, SIGNAL(released()), SLOT(onBkBtnClicked()));
    }
    if (cb_pi) {
        connect(cb_pi, SIGNAL(clicked(bool)), this, SLOT(onPIChanged(bool)));
    }
}


void MainWindow::connectItemChange(QSlider* slider, QLineEdit* edit, bool need_record) {
    if (slider == nullptr || edit == nullptr) {
        assert(false);
        return;
    }

    QMetaObject::Connection conn;
    conn = connect(edit, &QLineEdit::textChanged,
                   [slider](const QString& val)->void{slider->setValue(val.toInt());});

    conn =  connect(slider, &QSlider::valueChanged,
                    [edit](const int& val)->void{edit->setText(std::to_string(val).c_str());});

    if (need_record && conn) {
        item_connections << conn;
    }
}

void MainWindow::setStatus(const QString &s) {
    if (m_status) {
        m_status->setText(s);
    }
}

void MainWindow::onDataChange() {
    DeviceBindData(this);
}

void MainWindow::onError(int err) {
    qCritical() << "MainWindow::onError, err=" << err;
    SetDisconnectMode();
}

bool MainWindow::openSerialPort() {
    const auto is_ok = SerialComm::Instance()->openSerialPort();
    qCritical() << "MainWindow::openSerialPort, result=" << is_ok;
    if (is_ok) {
        SetNormalMode();
    }
}

void MainWindow::closeSerialPort() {
    qCritical() << "MainWindow::closeSerialPort";
    SerialComm::Instance()->closeSerialPort(0);
    SetDisconnectMode();
}

