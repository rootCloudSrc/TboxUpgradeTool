#include "tboxupgradethread.h"
#include <QDebug>
#include <QSettings>

int tboxUpgradeThread::uploadFile(LIBSSH2_SESSION *session)
{
    // 打开 SFTP 会话
    LIBSSH2_SFTP *sftp = libssh2_sftp_init(session);
    if (!sftp)
    {
        qDebug() << "sftp init failed";
        return -1;
    }
    qDebug() << "sftp init success";
    QFile file(m_upgradeFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered))
    {
        qDebug() << "can not open file:" << m_upgradeFileName;
        libssh2_sftp_shutdown(sftp);
        return -1;
    }
    qDebug() << "open file : "<< m_upgradeFileName<< "success";
    //const char *remote_file = "/data/tunnel/tmp/ir_ota.tar.gz"; //上传到服务器文件路径
    LIBSSH2_SFTP_HANDLE *handle = libssh2_sftp_open(sftp, SSH_REMOTE_FILE, LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC, 0666);
    if (!handle)
    {
        qDebug() << "sftp handle open failed";
        file.close();
        libssh2_sftp_shutdown(sftp);
        return -1;
    }
    qDebug() << "ssh handle open success";
    qint64 fileSize = file.size();
    //char buffer[1024*1024]; //局部变量过大会导致栈溢出，（现象：一调用函数就崩溃）
    int   bufferSize = 1024*100;
    char *buffer = new char[bufferSize];
    qint64 readCount = 0;
    // 二进制方式读取
    qDebug() << "file begin upload(file size:"<< fileSize <<")";
    //emit fileUpgradStat(FILE_UPGRADE_STAT_UPGRADING);//发送正在升级信号
    while (readCount < fileSize && !file.atEnd())
    {
        qint64 bytesRead = file.read(buffer, bufferSize);
        if (-1 == bytesRead)
        {
            qDebug() << "read file faild";
            break;
        }
        readCount += bytesRead;
        qint64 bytesSend = 0;
        while(1)
        {
            int bytesSendOnce = libssh2_sftp_write(handle, buffer + bytesSend, bytesRead);
            if (bytesSendOnce < 0)
            {
                qDebug() << "libssh2_sftp_write file faild";
                break;
            }

            bytesSend += bytesSendOnce; //已发送字节数
            bytesRead -= bytesSendOnce; //剩余多少未发完
            //qDebug() << "bytesSend:" << bytesSend << "remain:" << bytesRead;

            if(0 == bytesRead)          //全发完了
            {
                break;
            }
        }
        if(bytesRead) //没发完了就异常退出了
        {
            qDebug() << "do not send all data";
            break;
        }

        //qDebug() << "already send:"<< readCount <<" Bytes, percent:" << readCount*100/ fileSize <<"%";
        emit fileUploadPercent(readCount*100/ fileSize);
    }

    if(readCount != fileSize) //文件没发送完全
    {
        qDebug() << "upload file faild, readCount:" <<readCount
                 << "fileSize:" << fileSize;
    }

    delete[] buffer;
    file.close();
    libssh2_sftp_close(handle);
    libssh2_sftp_shutdown(sftp);
    return (readCount - fileSize); //发完文件readCount == fileSize，否则readCount < fileSize
}


int tboxUpgradeThread::isTboxProVersion(LIBSSH2_SESSION *session)
{
    LIBSSH2_SFTP *sftp = libssh2_sftp_init(session);
    if (!sftp)
    {
        qDebug() << "isTboxProVersion sftp init failed";
        return -1;
    }
    // 检查目录是否存在
    qDebug() << "isTboxProVersion() detect file path:" << TBOX_PRO_DIR_FLAG;
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int ret = libssh2_sftp_stat(sftp, TBOX_PRO_DIR_FLAG, &attrs);
    if (ret == 0)
    {
        // 目录存在
        qDebug() << "isTboxProVersion dir exist";
    }
    else
    {
        // 目录不存在
        qDebug() << "is not TboxPro Version";
    }

    libssh2_sftp_shutdown(sftp);
    return ret;
}

int tboxUpgradeThread::getTboxID(LIBSSH2_SESSION *session)
{
    // 执行SFTP初始化
    LIBSSH2_SFTP *sftp = libssh2_sftp_init(session);
    if (!sftp)
    {
        qDebug() << "getTboxID sftp init failed";
        return -1;
    }
    // 打开文件进行读取
    LIBSSH2_SFTP_HANDLE *handle;
    handle = libssh2_sftp_open(sftp, TBOX_CONFIG_FILE, LIBSSH2_FXF_READ, 0);

    // 读取文件内容
    char buffer[512];
    int bytes_read;
    bytes_read = libssh2_sftp_read(handle, buffer, sizeof(buffer));
    if(bytes_read > 0)
    {
        QString content = QString(QLatin1String(buffer));
        // 处理读取的内容
        qDebug() << "read ini content:" << content;

        // 将buffer内容写入临时文件
        QFile file("temp.ini");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            file.write(buffer, sizeof(buffer));
            file.close();

            // 使用QSettings解析临时文件
            QSettings settings("temp.ini", QSettings::IniFormat);

            // 读取配置项的值
            m_getTboxId = settings.value("dev/id").toString();

            // 删除临时文件
            file.remove("temp.ini");
        }

        qDebug() << "tbox ID:" << m_getTboxId;
    }
    else
    {
        qDebug() << "getTboxID failed";
    }

    // 关闭文件
    libssh2_sftp_close(handle);

    // 关闭SFTP会话
    libssh2_sftp_shutdown(sftp);
    return 0;
}

void tboxUpgradeThread::run()
{
    link_status_e linkStatus = LINK_STATUS_SSH_INIT;
    QTcpSocket socket;
    int socketFd;
    int ret = -1;

    //初始化libssh2库
    libssh2_init(0);
    //创建一个会话
    LIBSSH2_SESSION *session = nullptr;

    while(1)
    {
        //qDebug() << "tboxUpgradeThread statu:" << linkStatus;
        switch (linkStatus)
        {
        case LINK_STATUS_SSH_INIT:
            qDebug() << "LINK_STATUS_SSH_INIT";
            emit tboxConnectStat(TBOX_CONN_STAT_CONNECTING);
            socket.connectToHost(SERVER_HOST, SERVER_PORT);
            if(socket.waitForConnected())
            {
                qintptr socketDescriptor = socket.socketDescriptor();
                socketFd = static_cast<int>(socketDescriptor);
                linkStatus = LINK_STATUS_SSH_HANDSHAKE;
                qDebug() << "connectToHost successed";

                session = libssh2_session_init();
            }
            else
            {
                qDebug() << "connectToHost faild";
                sleep(5);
            }
            //session = libssh2_session_init();
            break;

        case LINK_STATUS_SSH_HANDSHAKE:
            qDebug() << "LINK_STATUS_SSH_HANDSHAKE";
            ret = libssh2_session_handshake(session, socketFd);
            if (!ret)
            {
                //连接成功
                qDebug() << "libssh2_session_handshake successed";
                linkStatus = LINK_STATUS_SSH_AUTH;
            }
            else
            {
                qDebug() << "libssh2_session_handshake faild";
                sleep(5);
            }
            break;

        case LINK_STATUS_SSH_AUTH:
            qDebug() << "LINK_STATUS_SSH_AUTH";
            ret = libssh2_userauth_password(session, SSH_USER_NAME, SSH_PASSWD);//这个接口二次使用卡住，可能需要刷新session -- 待测试
            if (!ret)
            {
                //认证成功
                qDebug() << "libssh2_userauth_password successed";
                linkStatus = LINK_STATUS_SSH_RECOGNIZE_TBOX_PRO;
                emit tboxConnectStat(TBOX_CONN_STAT_CONNECTED);
            }
            else
            {
                qDebug() << "libssh2_userauth_password faild";
                sleep(5);
            }
            break;

        case LINK_STATUS_SSH_RECOGNIZE_TBOX_PRO:
            qDebug() << "LINK_STATUS_SSH_RECOGNIZE_TBOX_PRO";
            ret = isTboxProVersion(session);
            if(ret)
            {
                emit tboxIdentify(TBOX_IDENTIFY_NOT_PRO);
                m_waitCustomerComfirm = true;
                linkStatus = LINK_STATUS_SSH_WAITING_COMFORM;
            }
            else
            {
                getTboxID(session);
                linkStatus = LINK_STATUS_SSH_IDEL;
                emit tboxIdentify(TBOX_IDENTIFY_IS_PRO);
            }

            break;

        case LINK_STATUS_SSH_UPLOAD_FILE:
            qDebug() << "LINK_STATUS_SSH_UPLOAD_FILE";
            emit fileUpgradStat(FILE_UPGRADE_STAT_UPGRADING);//发送正在升级信号
            ret = uploadFile(session);
            if(ret)
            {
                emit fileUpgradStat(FILE_UPGRADE_STAT_UPGRADFAILD);
                linkStatus = LINK_STATUS_SSH_RESET; //文件上传异常，复位链接
            }
            else
            {
                emit fileUpgradStat(FILE_UPGRADE_STAT_UPGRADONE);
                linkStatus = LINK_STATUS_SSH_IDEL;
            }
            m_upgradeFileName.clear();

            break;

        case LINK_STATUS_SSH_IDEL:
            //qDebug() << "LINK_STATUS_SSH_IDEL";
            if(m_upgradeStart)
            {
                linkStatus = LINK_STATUS_SSH_UPLOAD_FILE;
                m_upgradeStart = false;
            }
            msleep(50);
            break;

        case LINK_STATUS_SSH_RESET:
            qDebug() << "LINK_STATUS_SSH_RESET";
            socket.close();
            libssh2_session_disconnect(session, "reset");
            libssh2_session_free(session);
            //libssh2_exit();
            linkStatus = LINK_STATUS_SSH_INIT;
            break;

        case LINK_STATUS_SSH_WAITING_COMFORM: //等待用户确认
            if(!m_waitCustomerComfirm)
            {
                linkStatus = LINK_STATUS_SSH_RESET;
            }
            else
            {
                sleep(1);
            }
            break;
        default:
            break;
        }
        msleep(10);
    }

    // 清理libssh2库
    //libssh2_exit();
}


