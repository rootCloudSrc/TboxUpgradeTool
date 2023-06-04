#ifndef TBOXUPGRADETHREAD_H
#define TBOXUPGRADETHREAD_H

#include <QObject>
#include <QThread>
//#include <QMutex>
#include <QtNetwork>
#include <QTcpSocket>
#include <QFile>

#include <include/libssh2.h>
#include <include/libssh2_sftp.h>


#if 1 //实际tbox
    #define SERVER_HOST "192.168.225.1"
    #define SERVER_PORT (22)

    #define SSH_USER_NAME  "root"
    #define SSH_PASSWD     "quectel123"

    #define SSH_REMOTE_FILE   "/usrdata/testir_ota.tar.gz"  //上传到服务器文件路径
    #define TBOX_PRO_DIR_FLAG "/usrdata/"                //存在这个目录的就是pro版本盒子
    #define TBOX_CONFIG_FILE  "/opt/conf.ini"           //盒子配置文件

#else //测试服务器

    #define SERVER_HOST "18.196.0.17"
    #define SERVER_PORT (22)

    #define SSH_USER_NAME  "tunnel"
    #define SSH_PASSWD     "Tunnel@irootech.com"

    #define SSH_REMOTE_FILE   "/data/tunnel/tmp/ir_ota.tar.gz"  //上传到服务器文件路径
    #define TBOX_PRO_DIR_FLAG "/data/tunnel/tmp"                //存在这个目录的就是pro版本盒子
    #define TBOX_CONFIG_FILE  "/data/tunnel/tmp/conf.ini"       //盒子配置文件

#endif

typedef enum
{
    LINK_STATUS_SSH_INIT               = 0,
    //LINK_STATUS_SOCKET_CONNECT         = 1,
    LINK_STATUS_SSH_HANDSHAKE          = 2,
    LINK_STATUS_SSH_AUTH		       = 3,
    LINK_STATUS_SSH_RECOGNIZE_TBOX_PRO = 4,
    LINK_STATUS_SSH_UPLOAD_FILE        = 5,
    LINK_STATUS_SSH_IDEL               = 6,
    LINK_STATUS_SSH_RESET              = 7,
    LINK_STATUS_SSH_WAITING_COMFORM    = 8
}link_status_e;

typedef enum
{
    TBOX_CONN_STAT_CONNECTING = 0,
    TBOX_CONN_STAT_CONNECTED  = 1,
    TBOX_CONN_STAT_DISCONNECT = 2
}tbox_connect_stat_e;

typedef enum
{
    FILE_UPGRADE_STAT_IDEL        = 0,
    FILE_UPGRADE_STAT_UPGRADING   = 1,
    FILE_UPGRADE_STAT_UPGRADONE   = 2,
    FILE_UPGRADE_STAT_UPGRADFAILD = 3
}file_upgrade_stat_e;

typedef enum
{
    TBOX_IDENTIFY_UNKNOW   = 0,
    TBOX_IDENTIFY_IS_PRO   = 1,
    TBOX_IDENTIFY_NOT_PRO  = 2
}tbox_pro_identify_e;


class tboxUpgradeThread : public QThread
{
    Q_OBJECT
public:
    explicit tboxUpgradeThread(){};
    void UpgradeStart(QString &fileName)
    {
        m_upgradeFileName = fileName;
        m_upgradeStart = true;
    };
    void customerConfirmed()
    {
        m_waitCustomerComfirm = false;
    };


signals:
    void tboxConnectStat(int stat);
    void tboxIdentify(int identify);
    void tboxID(QString ID);
    void fileUpgradStat(int stat);
    void fileUploadPercent(int percent);//数据上传百分比

private:
    bool    m_upgradeStart = false;
    bool    m_waitCustomerComfirm = false; //等待客户确认
    QString m_upgradeFileName;
    QString m_getTboxId;

    void run() override;
    int  uploadFile(LIBSSH2_SESSION *session);
    int  isTboxProVersion(LIBSSH2_SESSION *session);//判断是否pro版本盒子
    int  getTboxID(LIBSSH2_SESSION *session);

};

#endif // TBOXUPGRADETHREAD_H
