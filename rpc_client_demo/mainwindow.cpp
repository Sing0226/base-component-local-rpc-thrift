#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include "testipcclientnotify.h"
#include "include/ipc_client_interface.h"
#include <QtCore/QTextCodec>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pIpcClient = new TestIpcClientNotify(this);
    initIpcClientInterfaceInstance(m_pIpcClient);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pIpcClient;
    m_pIpcClient = nullptr;
    releaseIpcClientInterfaceInstance();
}

void MainWindow::onConnected()
{
    m_ipcMsg.append("ipc start, ipc client connected to server  successful \n");
    ui->outputTextEdit->setText(m_ipcMsg);
}

void MainWindow::onDisconnected()
{
    m_ipcMsg.append("ipc stop, ipc client disconnected from server \n");
    ui->outputTextEdit->setText(m_ipcMsg);
}

void MainWindow::onConnectErr()
{
    m_ipcMsg.append("ipc stop, ipc client connected to server error \n");
    ui->outputTextEdit->setText(m_ipcMsg);
}

void MainWindow::onAuthor(const QString& author)
{
    if (author.isEmpty())
    {
        m_ipcMsg.append(QString("product cupid unactive ! \n").arg(author));
    }
    else
    {
        m_ipcMsg.append(QString("product cupid (id: %1) active ! \n").arg(author));
    }
     ui->outputTextEdit->setText(m_ipcMsg);
}

void MainWindow::onGetAuthorInfo(const QString& autor, int status)
{

     QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
     QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");
     QString strUnicode = utf8Codec->toUnicode(autor.toUtf8());   
     QByteArray ByteGb2312 = gb2312Codec->fromUnicode(strUnicode);    
     std::string gb2312Str = ByteGb2312.data();
     if (status == 0)
     {
         m_ipcMsg.append(QString("product unauthorized cupid info! \n"));
     }
     else
     {
         m_ipcMsg.append(QString("product authorized (lic: %1) info! \n").arg(autor));
     }
     ui->outputTextEdit->setText(m_ipcMsg);
}

void MainWindow::onLogin(LoginErrCode err)
{
    m_ipcMsg.append(QString("get login status: %1\n").arg(err));
    ui->outputTextEdit->setText(m_ipcMsg);
}


void MainWindow::on_pushBtn_license_clicked()
{
    IRsIpcClientInterface* interface = getIpcClientInterfaceInstance();
    if (interface)
    {
        interface->getAuthorInfo();
    }
}

void MainWindow::on_pushBtn_license__clicked()
{
    IRsIpcClientInterface* interface = getIpcClientInterfaceInstance();
    if (interface)
    {
        interface->getAuthorInfo();
    }
}

void MainWindow::on_pushBtn_login__clicked()
{
    IRsIpcClientInterface* interface = getIpcClientInterfaceInstance();
    if (interface)
    {
        std::string userName = ui->lineEdit_userName->text().toLocal8Bit().data();
        std::string userPwd = ui->lineEdit_userPwd->text().toLocal8Bit().data();
        interface->userLogin(userName, userPwd);
        m_ipcMsg.append(QString::fromLocal8Bit("µÇÂ¼ÕËºÅ£º user name: %1, user password id:%2\n").
            arg(QString::fromLocal8Bit(userName.c_str())).
            arg(QString::fromLocal8Bit(userPwd.c_str())));
        ui->outputTextEdit->setText(m_ipcMsg);
    }
}


