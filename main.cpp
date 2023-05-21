#include "mainwindow.h"

#include <QApplication>
/***************************************************************
 * @brief 颜色备注
 * @param wifi闪烁图标黄颜色rgb：#f4ce69 ，长亮绿颜色：#34c388
 * @param 升级按钮颜色：#58c3c3
 * @param 按钮禁止后颜色：#cacaca
 * @return
 ***************************************************************/


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // 设置应用程序图标
    QApplication::setWindowIcon(QIcon(":/images/window-icon.png"));
    w.show();
    return a.exec();
}
