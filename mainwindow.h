#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QPushButton>
#include <QTimer>
#include <QLabel>

#include "fileform.h"
#include "tboxupgradethread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_upload_clicked();
    void on_tbox_connect_status(int stat);
    void on_tbox_identify(int identify);
    void on_file_upgrade_status(int stat);
    void on_select_upgrade_file();
    void on_fileFormCancel();
    void on_file_upload_percent(int percent);

private:
    Ui::MainWindow *ui;
    // ui界面，盒子连接QLabel显示内容
    QString m_tboxConnecting = "Tbox Connecting...";
    QString m_tboxConnected  = "Tbox Connected";
    // ui界面，估计上传QLabel显示内容
    QString m_upgrade   = "Upgrade";
    QString m_upgrading = "Upgrading";
    QString m_upgraded  = "Upgraded";
    // 状态栏显示内容
    QLabel  * m_qlabelStatusMessage;
    QString m_statuBartboxUnconnect     = "Tbox Unconnect";
    QString m_statuBartboxConnecting    = "Tbox Connecting";
    QString m_statuBartboxConnected     = "Tbox Connected";
    QString m_statuBartboxUpgrading     = "Tbox Upgrading";
    QString m_statuBartboxUpgradeDone   = "Tbox Upgrade done";
    QString m_statuBartboxUpgradeFailed = "Tbox Upgrade Failed";
    tbox_connect_stat_e m_tBoxConnectStatus_e;//盒子连接状态
    file_upgrade_stat_e m_fileUpgradeStatus_e = FILE_UPGRADE_STAT_IDEL;//文件升级状态
    tbox_pro_identify_e m_tbox_pro_identify = TBOX_IDENTIFY_UNKNOW;//识别是否pro版本盒子


    QPushButton * m_selectFileButton; //选择文件的按钮
    fileForm    * m_fileForm; //选择文件后信息展示框
    QTimer      * m_tboxConnectIconFlashTimer;//tbox连接图标闪烁定时器
    QTimer      * m_upgradeIconFlashTimer;//upgrade图标闪烁定时器

    tboxUpgradeThread * m_tboxUpgrader; //盒子升级线程（ssh连接，文件上传）

    QString m_upgradeFile;

    /**>主界面设置方法windowSet... */
    void windowSetStart();//程序启动
//    void windowSetTboxUnconnect();//tbox未连接
//    void windowSetTboxConnecting();//tbox连接ing
//    void windowSetTboxConnected();//连接上tbox
//    void windowSetFileSelected();//选择了升级文件
//    void windowSetUpgrading();//升级ing
//    void windowSetUpgradone();//升级完成
//    void windowSetFileformClose(); //关掉了选择的文件

    /**>控件操作方法 */
    void tboxConnectIconFlash(); //tbox链接中(wifi图标)控件闪烁
    void upgradeIconFlash();     //升级图标控件闪烁
    void upgradeIconStopFlash();      //升级图标控件停止闪烁
    int fileFormShow(QString fileName);

protected:
    void showEvent(QShowEvent *event) override;
};
#endif // MAINWINDOW_H
