#include <QCoreApplication>
#include "ripcqcserver.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include "rslogger_declare.h"
#include "rslog.h"
#include "rslogging.h"



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    g_RsLog.setLogLevel(ERsLogLevel::RS_LOG_DEBUG);
    RIpcQcServer::getInstance().serve();
    // 中文测试的例子，将来啊
    RSLOG_DEBUG << "services start!";

    return a.exec();
}
