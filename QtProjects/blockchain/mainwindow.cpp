#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPixmap>

extern std::string LOGIN_OF_THIS_USER;
extern size_t NUMBER_BLOCK;
extern size_t NUMBER_COP;
extern size_t NUMBER_ALL_USERS;
extern std::vector<AVTOR::Copyright> user_cops;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPixmap pix(":/resources/img/grey_ver.png");
    int width = ui->label_sighIn_img->width();
    int height = ui->label_sighIn_img->height();

    ui->label_sighIn_img->setPixmap(pix.scaled(width, height, Qt::KeepAspectRatio));
    ui->label_sighUp_img->setPixmap(pix.scaled(width, height, Qt::KeepAspectRatio));

    width = ui->label_main_img->width();
    height = ui->label_main_img->height();

    ui->label_main_img->setPixmap(pix.scaled(width, height, Qt::KeepAspectRatio));

    check_main_directory();

    std::thread thr_1([&]() {
        BASE.from_file(this->BUF);
    });

    thr_1.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_sighIn_SighUp_clicked()
{
    ui->lineEdit_CheckIn_login->clear();
    ui->lineEdit_CheckIn_password_1->clear();
    ui->lineEdit_CheckIn_password_2->clear();

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_CheckIn_Cancel_clicked()
{
    ui->lineEdit_sighIn_login->clear();
    ui->lineEdit_sighIn_password->clear();

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_CheckIn_SighUp_clicked()
{
    ui->lineEdit_sighIn_login->clear();
    ui->lineEdit_sighIn_password->clear();

    std::string login = ui->lineEdit_CheckIn_login->text().toStdString(),
                pass_1 = ui->lineEdit_CheckIn_password_1->text().toStdString(),
                pass_2 = ui->lineEdit_CheckIn_password_2->text().toStdString();

    ui->statusBar->setStyleSheet("color: red;");

    if (pass_1 != pass_2) {
        ui->statusBar->showMessage("Passwords doesn't match!", 1200);
        return;
    } else if (login.size() < 5) {
        ui->statusBar->showMessage("Minimal login length required 5 characters!", 1200);
        return;
    } else if (pass_1.size() < 6) {
        ui->statusBar->showMessage("Minimal password length required 5 characters!", 1200);
        return;
    } else if(!this->BASE.is_new_login(login)) {
        ui->statusBar->showMessage("Login is already used!. Try another.", 1200);
        return;
    }

    std::thread thr_1([&]() {
        this->BASE.add_registration(login, pass_1, this->BUF);
    });
    thr_1.detach();

    ui->statusBar->setStyleSheet("color: green;");

    ui->statusBar->showMessage("Register was successful!", 2300);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_sighIn_SighIn_clicked()
{
    std::string login = ui->lineEdit_sighIn_login->text().toStdString(),
                password = ui->lineEdit_sighIn_password->text().toStdString();

    if (this->BASE.is_true_login_and_password(login, password)) {
        if (this->flag_first_log) {
            std::thread thr_1([&]() {
                this->BC.from_file(this->BUF);
            });
            thr_1.detach();
            flag_first_log = false;
        }

        LOGIN_OF_THIS_USER = login;

        ui->statusBar->setStyleSheet("color: green;");

        ui->statusBar->showMessage("You are logged in!", 2300);

        ui->label_main_login->setText(QString::fromStdString(LOGIN_OF_THIS_USER));

        ui->stackedWidget->setCurrentIndex(2);
    } else {
        ui->statusBar->setStyleSheet("color: red;");

        ui->statusBar->showMessage("Authorization error! Try again.", 2300);
    }
}

void MainWindow::on_pushButton_main_SighOut_clicked()
{
    ui->lineEdit_sighIn_login->clear();
    ui->lineEdit_sighIn_password->clear();

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_add_path_clicked()
{
    QString path_to_file = QFileDialog::getOpenFileName(0, "Directory Dialog", "");
    ui->lineEdit_add_path->setText(path_to_file);
}

void MainWindow::on_pushButton_add_go_clicked()
{
    std::string path_to_file = ui->lineEdit_add_path->text().toStdString();

    this->PATH_TO_FILE_NOW = path_to_file;

    std::thread thr_1([&]() {
        this->BC.add_copyright(PATH_TO_FILE_NOW, BUF);
    });
    thr_1.detach();
}

void MainWindow::on_pushButton_test_path_clicked()
{
    QString path_to_file = QFileDialog::getOpenFileName(0, "Directory Dialog", "");
    ui->lineEdit_test_path->setText(path_to_file);
}

void MainWindow::on_pushButton_test_go_clicked()
{
    std::string path_to_file = ui->lineEdit_test_path->text().toStdString();
    if (this->BC.is_new_product(path_to_file)) {
        ui->statusBar->setStyleSheet("color: green;");
        ui->statusBar->showMessage("This product WAS NOT registered.", 2300);
    } else {
        ui->statusBar->setStyleSheet("color: red;");
        ui->statusBar->showMessage("This product WAS registered.", 2300);
    }

}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if (index == 1) {
        ui->listWidget->clear();

        this->BC.cout_user_copyrights();

        for(size_t i = 0; i < user_cops.size(); ++i) {
            std::string tmp = std::to_string(i + 1) + ")          ";
            tmp = tmp.substr(0, 6);
            QString str = QString::fromStdString(tmp + user_cops[i].get_date());
            this->ui->listWidget->addItem(str);
        }
    } else if (index == 3) {
        this->BC.cout_chain_info();
        this->ui->label_info_numberBlocks_important->setText(QString::fromStdString(std::to_string(NUMBER_BLOCK)));
        this->ui->label_info_numberCopyrights_important->setText(QString::fromStdString(std::to_string(NUMBER_COP)));
        this->ui->label_info_numberUsers_important->setText(QString::fromStdString(std::to_string(NUMBER_ALL_USERS)));
    }
}



void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    size_t ind = ui->listWidget->currentRow();
    ui->label_main_dateAndTime_important->setText(QString::fromStdString(user_cops[ind].get_date()));
    std::string tmp = user_cops[ind].get_FH();
    ui->label_main_hash1a_important->setText(QString::fromStdString("\"" + tmp.substr(0, 32) + "-"));
    ui->label_main_hash1b_important->setText(QString::fromStdString(tmp.substr(31, 32) + "\""));
    tmp = user_cops[ind].get_WH();
    ui->label_main_hash2a_important->setText(QString::fromStdString("\"" + tmp.substr(0, 32) + "-"));
    ui->label_main_hash2b_important->setText(QString::fromStdString(tmp.substr(31, 32) + "\""));
}
