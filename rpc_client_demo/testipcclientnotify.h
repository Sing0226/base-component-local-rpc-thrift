#ifndef TESTIPCCLIENTNOTIFY_H
#define TESTIPCCLIENTNOTIFY_H
#include "include/ipc_client_interface.h"

class MainWindow;

class TestIpcClientNotify : public rs_client_ipc::IRsIpcClientNotify
{
public:
    TestIpcClientNotify(MainWindow* pMainWin = nullptr);
    virtual ~TestIpcClientNotify();

public:
    virtual void notifyConnected();
    virtual void notifyDisconnected();
    virtual void notifyConnectErr();

    // 广播通知

    virtual void notifyAuthorInfo(const std::string& autorDes) override;
    virtual void notifyGetAuthorInfo(const rs_client_ipc::AuthorInfo& authorInfo) override;
    virtual void notifyUserLoginStatus(LoginErrCode err) override;

private:
    MainWindow* m_pMainWin;
};

#endif // TESTIPCCLIENTNOTIFY_H
