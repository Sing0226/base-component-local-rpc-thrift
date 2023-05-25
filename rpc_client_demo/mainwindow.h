#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ipc_error.h"


class TestIpcClientNotify;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onConnected();
    void onDisconnected();
    void onConnectErr();
    void onAuthor(const QString& author);
    void onGetAuthorInfo(const QString& product, int status);
    void onLogin(LoginErrCode err);
    

private slots:
    void on_pushBtn_license_clicked();

    void on_pushBtn_license__clicked();

    void on_pushBtn_login__clicked();

private:
    Ui::MainWindow *ui;
    TestIpcClientNotify* m_pIpcClient;
    QString m_ipcMsg;
};
#endif // MAINWINDOW_H
