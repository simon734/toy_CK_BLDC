import qbs.FileInfo

QtApplication {
    Depends { name: "Qt.widgets"}
    Depends { name: "Qt.serialport"}

    // The following define makes your compiler emit warnings if you use
    // any Qt feature that has been marked deprecated (the exact warnings
    // depend on your compiler). Please consult the documentation of the
    // deprecated API in order to know how to port your code away from it.
    // You can also make your code fail to compile if it uses deprecated APIs.
    // In order to do so, uncomment the second entry in the list.
    // You can also select to disable deprecated APIs only up to a certain version of Qt.
    cpp.defines: [
        "QT_DEPRECATED_WARNINGS",
        /* "QT_DISABLE_DEPRECATED_BEFORE=0x060000" */ // disables all the APIs deprecated before Qt 6.0.0
    ]

    files: [
        "basic_def.h",
        "datatransfer.cpp",
        "datatransfer.h",
        "deviceitem.cpp",
        "deviceitem.h",
        "itemwidget.cpp",
        "itemwidget.h",
        "logutils.cpp",
        "logutils.h",
        "main.cpp",
        "mainwindow.cpp",
        "mainwindow.h",
        "mainwindow.ui",
        "settingsdialog.ui",
        "settingsdialog.h",
        "settingsdialog.cpp",
        "mainwindow.qrc",
        "serialcomm.cpp",
        "serialcomm.h",
        "debugger.h",
        "debugger.cpp"
    ]

    install: true
    installDir: qbs.targetOS.contains("qnx") ? FileInfo.joinPaths("/tmp", name, "bin") : base
}
