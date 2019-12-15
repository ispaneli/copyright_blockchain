#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "checkin_map.h"
#include "client-server.h"
#include "functions.h"
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_sighIn_SighUp_clicked();

    void on_pushButton_CheckIn_Cancel_clicked();

    void on_pushButton_CheckIn_SighUp_clicked();

    void on_pushButton_sighIn_SighIn_clicked();

    void on_pushButton_main_SighOut_clicked();

    void on_pushButton_add_path_clicked();

    void on_pushButton_add_go_clicked();

    void on_pushButton_test_path_clicked();

    void on_pushButton_test_go_clicked();

    void on_tabWidget_tabBarClicked(int index);



    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    Buffer BUF;
    AVTOR::CheckInMap BASE;
    //std::string LOGIN_OF_THIS_USER;
    AVTOR::Blockchain BC;
    std::string PATH_TO_FILE_NOW;
    bool flag_first_log = true;
};

#endif // MAINWINDOW_H
