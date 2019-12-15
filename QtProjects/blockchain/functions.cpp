// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "functions.h"

std::string PATH_TO_BLOCKCHAIN;
std::string PATH_TO_CHECKIN;
std::mutex mining_mutex;
std::string LOGIN_OF_THIS_USER;

void check_main_directory() {
    namespace fs = boost::filesystem;

    fs::path p = fs::current_path();
    if (p.has_parent_path()) {
        p = p.parent_path();
    }

    p += "/storage/";
    if (!fs::exists(p)) {
        create_directory(p);
    }

    p += "blockchain.txt";
    if (!fs::exists(p)) {
        fs::ofstream(p.string());
    }
    PATH_TO_BLOCKCHAIN = p.string();
    p = p.parent_path();

    p += "/checkin.txt";
    if (!fs::exists(p)) {
        fs::ofstream(p.string());
    }
    PATH_TO_CHECKIN = p.string();
}

void MENU_START() {
    check_main_directory();

    Buffer BUF;
    AVTOR::CheckInMap BASE;
    std::thread thr_1([&]() {
        BASE.from_file(BUF);
    });

    thr_1.detach();

    std::cout << "### START MENU ###" << std::endl;
    std::cout << "1. Войти." << std::endl;
    std::cout << "2. Регистрация." << std::endl;

    std::string com_num;
    std::cin >> com_num;

    if (com_num == "1") {
        std::string login, password;
        std::cout << "#login:    ";
        std::cin >> login;
        std::cout << "#password: ";
        std::cin >> password;

        if (BASE.is_true_login_and_password(login, password)) {
            std::cout << "Вы успешно вошли в систему!" << std::endl;
            LOGIN_OF_THIS_USER = login;
            MAIN_MENU(BUF);
        } else {
            std::cout << "Ошибка авторизации! Попробуйте снова." << std::endl;
            MENU_START(BASE, BUF);
        }

    } else if (com_num == "2") {
        std::string login, password;
        std::cout << "#login:    ";
        std::cin >> login;
        std::cout << "#password: ";
        std::cin >> password;

        std::thread thr_2([&]() {
            BASE.add_registration(login, password, BUF);
        });
        thr_2.detach();

        MENU_START(BASE, BUF);
    } else {
        MENU_START(BASE, BUF);
    }

}

void MENU_START(AVTOR::CheckInMap& BASE, Buffer& BUF) {
    std::cout << "### START MENU ###" << std::endl;
    std::cout << "1. Войти." << std::endl;
    std::cout << "2. Регистрация." << std::endl;

    std::string com_num;
    std::cin >> com_num;

    if (com_num == "1") {
        std::string login, password;
        std::cout << "#login:    ";
        std::cin >> login;
        std::cout << "#password: ";
        std::cin >> password;

        if (BASE.is_true_login_and_password(login, password)) {
            std::cout << "Вы успешно вошли в систему!" << std::endl;
            LOGIN_OF_THIS_USER = login;
            MAIN_MENU(BUF);
        } else {
            std::cout << "Ошибка авторизации! Попробуйте снова." << std::endl;
            MENU_START(BASE, BUF);
        }

    } else if (com_num == "2") {
        std::string login, password;
        std::cout << "#login:    ";
        std::cin >> login;
        std::cout << "#password: ";
        std::cin >> password;

        std::thread thr_3([&]() {
            BASE.add_registration(login, password, BUF);
        });
        thr_3.detach();

        MENU_START(BASE, BUF);
    } else {
        MENU_START(BASE, BUF);
    }
}

void MAIN_MENU(Buffer& BUF) {

    AVTOR::Blockchain BC;

    std::thread thr_1([&]() {
        BC.from_file(BUF);
    });
    thr_1.detach();

    std::cout << "##################### MAIN MENU #####################" << std::endl;
    std::cout << "1. Зарегистрировать новый продукт." << std::endl;
    std::cout << "2. Посмотреть список своих праворегистраций." << std::endl;
    std::cout << "3. Посмотреть информацию о нынешнем состоянии цепи." << std::endl;
    std::cout << "4. Проверить, был ли данный файл уже зарегистрирован." << std::endl;

    std::string com_num;
    std::cin >> com_num;

    if (com_num == "1") {
        std::string path;
        std::cout << "# path to file: " << std::endl;
        std::cin >> path;
        std::thread thr_4([&]() {
            BC.add_copyright(path, BUF);
        });
        thr_4.detach();
        MAIN_MENU(BC, BUF);
    } else if (com_num == "2") {
        std::thread thr_2([&]() {
            BC.cout_user_copyrights();
        });
        thr_2.detach();
        MAIN_MENU(BC, BUF);
    }
    else if (com_num == "3") {
        std::thread thr_5([&]() {
            BC.cout_chain_info();
        });
        MAIN_MENU(BC, BUF);
    }
    else if (com_num == "4") {
        std::string path;
        std::cout << "# path to file: " << std::endl;
        std::cin >> path;
        std::thread thr_3([&]() {
            BC.is_new_product(path);
        });
        thr_3.detach();
        MAIN_MENU(BC, BUF);
    } else {
        MAIN_MENU(BC, BUF);
    }
}

void MAIN_MENU(AVTOR::Blockchain& BC, Buffer& BUF) {

    std::cout << "##################### MAIN MENU #####################" << std::endl;
    std::cout << "1. Зарегистрировать новый продукт." << std::endl;
    std::cout << "2. Посмотреть список своих праворегистраций." << std::endl;
    std::cout << "3. Посмотреть информацию о нынешнем состоянии цепи." << std::endl;
    std::cout << "4. Проверить, был ли данный файл уже зарегистрирован." << std::endl;

    std::string com_num;
    std::cin >> com_num;

    if (com_num == "1") {
        std::string path;
        std::cout << "# path to file: " << std::endl;
        std::cin >> path;
        std::thread thr_4([&]() {
            BC.add_copyright(path, BUF);
        });
        thr_4.detach();
        MAIN_MENU(BC, BUF);
    } else if (com_num == "2") {
        std::thread thr_2([&]() {
            BC.cout_user_copyrights();
        });
        thr_2.detach();
        MAIN_MENU(BC, BUF);
    }
    else if (com_num == "3") {
        std::thread thr_5([&]() {
            BC.cout_chain_info();
        });
        MAIN_MENU(BC, BUF);
    }
    else if (com_num == "4") {
        std::string path;
        std::cout << "# path to file: " << std::endl;
        std::cin >> path;
        std::thread thr_3([&]() {
            BC.is_new_product(path);
        });
        thr_3.detach();
        MAIN_MENU(BC, BUF);
    } else {
        MAIN_MENU(BC, BUF);
    }
}
