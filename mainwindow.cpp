#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_line->setPixmap(QPixmap(":/images/transverse-line.png"));

    //文件选择框中的 ‘+’ 号按钮
    m_selectFileButton = new QPushButton(ui->widget_selectFile);
    QIcon icon(":/images/select_file.png");
    m_selectFileButton->setFixedSize(100, 100);  //设置按钮控件尺寸
    m_selectFileButton->setStyleSheet("QPushButton{border:0px;}"); //样式表：设置QPushButton的边框为0像素，即没有边框
    m_selectFileButton->setIcon(icon);
    m_selectFileButton->setIconSize(QSize(100, 100)); //如宽度和高度都只有 16 个像素，那么即使调用setIconSize 方法，也看起来没有改变
    m_selectFileButton->show();
    QObject::connect(m_selectFileButton, &QPushButton::clicked, this, &MainWindow::on_select_upgrade_file);

    ui->pushButton_upload->setStyleSheet( //设置按钮按下效果
        "QPushButton {"
        "background-color: #34c388;"
        "border: none;"
        "color: white;"
        "font-size: 16px;"
        "padding: 10px;"
        "}"
        "QPushButton:pressed {"
        "background-color: #34c378;"
        "border: 2px solid white;"
        "color: white;"
        "}");

    m_fileForm = new fileForm(ui->widget_selectFile);
    QObject::connect(m_fileForm, &fileForm::fileCancel, this, &MainWindow::on_fileFormCancel);

    m_upgradeIconFlashTimer = new QTimer();
    m_upgradeIconFlashTimer->setInterval(500);
    //m_upgradeIconFlashTimer->start();
    QObject::connect(m_upgradeIconFlashTimer, &QTimer::timeout, this, &MainWindow::upgradeIconFlash);

    m_tboxUpgrader = new tboxUpgradeThread();
    QObject::connect(m_tboxUpgrader, &tboxUpgradeThread::tboxConnectStat,  this, &MainWindow::on_tbox_connect_status);
    QObject::connect(m_tboxUpgrader, &tboxUpgradeThread::fileUpgradStat ,  this, &MainWindow::on_file_upgrade_status);
    QObject::connect(m_tboxUpgrader, &tboxUpgradeThread::fileUploadPercent,this, &MainWindow::on_file_upload_percent);
    QObject::connect(m_tboxUpgrader, &tboxUpgradeThread::tboxIdentify,     this, &MainWindow::on_tbox_identify);
    QObject::connect(m_tboxUpgrader, &tboxUpgradeThread::tboxID,           this, &MainWindow::on_tboxID);
    m_tboxUpgrader->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_upload_clicked()
{
    //盒子未连接
    if(m_tBoxConnectStatus_e != TBOX_CONN_STAT_CONNECTED)
    {
        QMessageBox::information(this, "Warn", "Tbox Unconnect");
        return;
    }
    //未选择升级文件
    if(m_upgradeFile.isNull())
    {
        QMessageBox::information(this, "Warn", "No Upgrade File select");
        return;
    }
    //不是pro版本盒子
    if(TBOX_IDENTIFY_IS_PRO != m_tbox_pro_identify)
    {
        QMessageBox::information(this, "Warn", "Is Not Pro Version Tbox");
        return;
    }
    //升级未完成
    if(FILE_UPGRADE_STAT_UPGRADING == m_fileUpgradeStatus_e)
    {
        QMessageBox::information(this, "Warn", "In Upgrading");
        return;
    }
    //让用户确认盒子ID，避免升错设备
    if(QMessageBox::Ok != QMessageBox::information(this, "comfirm ID", m_tboxID, QMessageBox::Ok | QMessageBox::Cancel))
    {
        qDebug() << "cancel upgrade operation";
        return;
    }

    qDebug() << "upgrade start";
    m_tboxUpgrader->UpgradeStart(m_upgradeFile);
}

void MainWindow::on_tbox_connect_status(int stat)
{
    m_tBoxConnectStatus_e = (tbox_connect_stat_e)stat;
    switch (m_tBoxConnectStatus_e)
    {
    case TBOX_CONN_STAT_CONNECTING:
        m_qlabelStatusMessage->setText(m_statuBartboxConnecting);
        m_tboxConnectIconFlashTimer->start();
        ui->label_tboxConMsg->setText(m_tboxConnecting);
        break;

    case TBOX_CONN_STAT_CONNECTED:
        m_qlabelStatusMessage->setText(m_statuBartboxConnected);
        m_tboxConnectIconFlashTimer->stop();
        ui->label_wifiIcon->setPixmap(QPixmap(":/images/bx-wifi-G.png"));
        ui->label_tboxConMsg->setText(m_tboxConnected);
        break;

    case TBOX_CONN_STAT_DISCONNECT:
        m_qlabelStatusMessage->setText(m_statuBartboxUnconnect);
        break;
    }
}

void MainWindow::on_tbox_identify(int identify)
{
    m_tbox_pro_identify = (tbox_pro_identify_e)identify;
    if(TBOX_IDENTIFY_IS_PRO != m_tbox_pro_identify) //识别到不是pro版本，弹出通知用户并阻塞线程
    {
        QMessageBox::information(this, "Warn", "Is not pro Tbox,Please connect correct wifi AP");
        m_tboxUpgrader->customerConfirmed();
    }
}

void MainWindow::on_tboxID(QString ID)
{
    m_tboxID = ID;
    qDebug() << "MainWindow get tbox ID:" << ID;
}

void MainWindow::on_file_upgrade_status(int stat)
{
    m_fileUpgradeStatus_e = (file_upgrade_stat_e)stat;
    switch (m_fileUpgradeStatus_e)
    {
    case FILE_UPGRADE_STAT_IDEL:
        break;

    case FILE_UPGRADE_STAT_UPGRADING:
        m_qlabelStatusMessage->setText(m_statuBartboxUpgrading);
        if(!m_upgradeIconFlashTimer->isActive())
        {
            m_upgradeIconFlashTimer->start(); //开启upgrade图标闪烁定时器
        }
        break;

    case FILE_UPGRADE_STAT_UPGRADONE:
        upgradeIconStopFlash();//upgrade图标长亮
        ui->label_sendMsg->setText(m_upgrade);
        m_qlabelStatusMessage->setText(m_statuBartboxUpgradeDone);
        QMessageBox::information(this, "Info", "Update Success");
        m_tboxID.clear();
        break;

    case FILE_UPGRADE_STAT_UPGRADFAILD:
        upgradeIconStopFlash();//upgrade图标长亮
        ui->label_sendMsg->setText(m_upgrade);
        m_qlabelStatusMessage->setText(m_statuBartboxUpgradeFailed);
        QMessageBox::information(this, "Info", "Update Failed");
        break;
    }
}

void MainWindow::on_select_upgrade_file()
{
    m_upgradeFile = QFileDialog::getOpenFileName(this, "select file");
    if(m_upgradeFile.isNull())
    {
        qDebug() << "user cancel dialog";
    }
    qDebug() << "get filename:" << m_upgradeFile;

    if(0 == fileFormShow(m_upgradeFile))
    {
         m_selectFileButton->hide();
    }
    else //文件异常
    {
        m_upgradeFile.clear();
    }
}

void MainWindow::on_fileFormCancel()
{
    qDebug() << "MainWindow::on_fileFormCancel";
    m_upgradeFile.clear();
    m_fileForm->hide();
    m_fileForm->clearFileInfo();
    m_selectFileButton->show();
}

//状态栏展示文件上传百分比
void MainWindow::on_file_upload_percent(int percent)
{
    QString uploadMsg = QString("%1\%").arg(percent);
    //qDebug() << uploadMsg;
    ui->label_sendMsg->setText(uploadMsg);
}

void MainWindow::windowSetStart()
{
    m_tboxConnectIconFlashTimer = new QTimer();
    m_tboxConnectIconFlashTimer->setInterval(300);
    m_tboxConnectIconFlashTimer->start();
    // 定时器溢出时，闪烁托盘图标
    QObject::connect(m_tboxConnectIconFlashTimer, &QTimer::timeout, this, &MainWindow::tboxConnectIconFlash);

    ui->label_tboxConMsg->setText(m_tboxConnecting);
    ui->label_sendIcon->setPixmap(QPixmap(":/images/sending.png"));
    ui->label_sendMsg->setText(m_upgrade);
    //状态栏
    m_qlabelStatusMessage = new QLabel(m_statuBartboxUnconnect, this);
    ui->statusbar->addWidget(m_qlabelStatusMessage);

    ui->label_sendIcon->setPixmap(QPixmap(":/images/sending.png"));
    m_fileForm->hide();
}

void MainWindow::tboxConnectIconFlash()
{
    static int iconIndex = 0;
    QPixmap *icon;
    iconIndex %= 4;
    switch (iconIndex++)
    {
    case 0:
        icon = new QPixmap(":/images/bx-wifi-0-Y.png");
        break;
    case 1:
        icon = new QPixmap(":/images/bx-wifi-1-Y.png");
        break;
    case 2:
        icon = new QPixmap(":/images/bx-wifi-2-Y.png");
        break;
    case 3:
        icon = new QPixmap(":/images/bx-wifi-Y.png");
        break;
    }
    ui->label_wifiIcon->setPixmap(*icon);
    delete icon;
    icon = nullptr;
}

void MainWindow::upgradeIconFlash()
{
    static int iconIndex = 1;
    QPixmap *icon;
    iconIndex %= 2;
    switch (iconIndex++)
    {
    case 0:
        icon = new QPixmap(":/images/sending.png");
        break;
    case 1:
        icon = new QPixmap(":/images/null.png");
        break;
    }
    ui->label_sendIcon->setPixmap(*icon);
    delete icon;
    icon = nullptr;
}

void MainWindow::upgradeIconStopFlash()
{
    if(m_upgradeIconFlashTimer->isActive())
    {
        m_upgradeIconFlashTimer->stop();

        ui->label_sendIcon->setPixmap(QPixmap(":/images/sending.png"));
    }
}

int MainWindow::fileFormShow(QString fileName)
{
    QFile file(fileName);
    if(!file.exists())
    {
        fileName.prepend("can not find file:");
        QMessageBox::information(this, "Error", fileName);
        return -1;
    }
    QString units;
    float size = file.size(); //字节
    if(size > 1024)
    {
        size /= 1024;
        units = " KB";
    }
    if(size > 1024)
    {
        size /= 1024;
        units = " MB";
    }
    QString fileNamePure = QFileInfo(fileName).fileName();//提取文件名，不要路径
    QString fileSize = QString::number(size, 'f', 2);;
    fileSize.append(units);

    m_fileForm->move(2, 2); //显示起始坐标
    m_fileForm->resize(ui->widget_selectFile->width()-4, ui->widget_selectFile->height()/3);//显示尺寸
    m_fileForm->setFileInfo(fileNamePure, fileSize);
    m_fileForm->show();
    return 0;
}
//因为构造函数在执行是未完成界面布局，获取到的窗口尺寸是布局前的，所以在showEvent事件处理
void MainWindow::showEvent(QShowEvent *event)
{
    m_selectFileButton->move(ui->widget_selectFile->width()/2 - m_selectFileButton->width()/2,
                             ui->widget_selectFile->height()*0.5 - m_selectFileButton->height()/2 - 30);//移动按钮到居中偏上30像素位置
    m_fileForm->setStyleSheet("border: 1px solid white;");//设置了窗体的边框为1像素宽的白色实线
    windowSetStart();
}
