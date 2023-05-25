#include "testipcclientnotify.h"
#include <QDebug>
#include "mainwindow.h"

TestIpcClientNotify::TestIpcClientNotify(MainWindow* pMainWin)
    :m_pMainWin(pMainWin)
{

}

TestIpcClientNotify::~TestIpcClientNotify()
{
    qDebug() << "~TestIpcClientNotify";
}

void TestIpcClientNotify::notifyConnected()
{
    qDebug() << "notifyConnected";
    QMetaObject::invokeMethod(m_pMainWin, "onConnected", Qt::AutoConnection);
}

void TestIpcClientNotify::notifyDisconnected()
{
    qDebug() << "notifyDisconnected";
    QMetaObject::invokeMethod(m_pMainWin, "onDisconnected", Qt::AutoConnection);
}

void TestIpcClientNotify::notifyConnectErr()
{
    qDebug() << "notifyDisconnected";
    QMetaObject::invokeMethod(m_pMainWin, "onConnectErr", Qt::AutoConnection);
}

void TestIpcClientNotify::notifyAuthorInfo(const std::string& autorDes)
{
    qDebug() << "notifyAuthorInfo";
    QMetaObject::invokeMethod(m_pMainWin, "onAuthor", Qt::AutoConnection,
                              Q_ARG(QString, QString::fromUtf8(autorDes.c_str())));
}

void TestIpcClientNotify::notifyGetAuthorInfo(const rs_client_ipc::AuthorInfo& authorInfo)
{
    qDebug() << "notifyGetAuthorInfo";
    QMetaObject::invokeMethod(m_pMainWin, "onGetAuthorInfo", Qt::AutoConnection,
                              Q_ARG(QString, QString::fromUtf8(authorInfo.authorInfo.c_str())),
                              Q_ARG(int, authorInfo.authorStatus));
}


void TestIpcClientNotify::notifyUserLoginStatus(LoginErrCode err)
{
    qDebug() << "notifyUserLoginStatus = " << err;
    QMetaObject::invokeMethod(m_pMainWin, "onLogin", Qt::AutoConnection,
        Q_ARG(LoginErrCode, err));
}

