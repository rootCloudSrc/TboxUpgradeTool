#include "fileform.h"
#include "ui_fileform.h"
#include <QDebug>

fileForm::fileForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fileForm)
{
    ui->setupUi(this);
    ui->label_icon->setPixmap(QPixmap(":/images/file_info_icon.png"));
    ui->label_icon->setScaledContents(true);//让图片填满控件

    ui->pushButton_cancel->setIcon(QPixmap(":/images/file_info_cancel.png"));
    //this->setStyleSheet("border: 1px solid white;");//设置了窗体的边框为1像素宽的白色实线
}

fileForm::~fileForm()
{
    delete ui;
}

int fileForm::setFileInfo(QString &fileName, QString &fileSize)
{
    QFont fontFileName("Arial", 13, QFont::Bold);
    ui->label_fileName->setFont(fontFileName);             //设置字体
    //ui->label_fileName->setAlignment(Qt::AlignLeft);        //设置对齐方式
    ui->label_fileName->setStyleSheet("color: #666666;");//设置字体颜色
    ui->label_fileName->setWordWrap(true); //自动换行
    ui->label_fileName->setText(fileName);

    QFont fontFileSize("Arial", 8, QFont::Bold);
    ui->label_fileSize->setFont(fontFileSize);             //设置字体
    ui->label_fileSize->setAlignment(Qt::AlignTop);        //设置对齐方式
    ui->label_fileSize->setStyleSheet("color: #BFBFBF;");//设置字体颜色
    ui->label_fileSize->setText(fileSize);
    return 0;
}

void fileForm::clearFileInfo()
{
    ui->label_fileName->clear();
    ui->label_fileSize->clear();
}

void fileForm::on_pushButton_cancel_clicked()
{
    qDebug() << "fileForm::on_pushButton_cancel_clicked";
    emit fileCancel();
}
