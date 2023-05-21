#ifndef FILEFORM_H
#define FILEFORM_H

#include <QWidget>

namespace Ui {
class fileForm;
}

class fileForm : public QWidget
{
    Q_OBJECT

public:
    explicit fileForm(QWidget *parent = nullptr);
    ~fileForm();

    int setFileInfo(QString &fileName, QString &fileSize);
    void clearFileInfo();

private slots:
    void on_pushButton_cancel_clicked();

signals:
    void fileCancel();

private:
    Ui::fileForm *ui;
};

#endif // FILEFORM_H
