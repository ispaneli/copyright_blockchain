// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "checkin_map.h"

size_t NUMBER_ALL_USERS;
extern std::string PATH_TO_CHECKIN;

using namespace AVTOR;

bool CheckInMap::is_true_login_and_password(std::string& login, std::string& password) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    for (auto& i : base) {
        if (i.first == login) {
            if (i.second == picosha2::sha256(login + password)) {
                return true;
            }
            return false;
        }
    }

    return false;
}

bool CheckInMap::is_new_login(std::string& login) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    for (auto& i : base) {
        if (i.first == login) {
            return false;
        }
    }

    return true;
}

void CheckInMap::checking_upd_database(Buffer& BUF) {
    BUF.mtx_to_send.lock();
    BUF.buf_to_send.emplace_back("@give_Dbase@");
    BUF.mtx_to_send.unlock();

    while (true) {
        BUF.mtx_from_receive.lock();
        if (BUF.buf_from_receive.size() != 0) {
            std::string str = BUF.buf_from_receive[0];

            if (str.substr(0, 12) == "@regis_base@") {
                this->from_string_with_database(str, BUF);
                BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());
            }
            else if (str.substr(0, 12) == "@new_regist@") {
                this->from_string_with_one_registration(str, BUF);
                BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());
            }
            else if (str.substr(0, 12) == "@give_Dbase@") {
                BUF.mtx_to_send.lock();
                BUF.buf_to_send.emplace_back(this->to_string_with_database());
                BUF.mtx_to_send.unlock();
                BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());
            }
        }
        BUF.mtx_from_receive.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void CheckInMap::from_file(Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx, std::defer_lock);

    std::ifstream fin(PATH_TO_CHECKIN);

    std::string line;
    getline(fin, line);

    if (line.length() == 0) {
        NUMBER_ALL_USERS = base.size();
        this->checking_upd_database(BUF);
        return;
    }

    NUMBER_ALL_USERS = std::atoi(line.substr(7, line.size() - 7).c_str());

    uni_lock.lock();

    for (size_t i = 0; i < NUMBER_ALL_USERS; ++i) {
        getline(fin, line);

        std::string login, pas_hash;

        getline(fin, login);
        login = login.substr(7, login.size() - 9);

        getline(fin, pas_hash);
        pas_hash = pas_hash.substr(7, 64);

        if (this->is_new_login(login)) {
            std::pair<std::string, std::string> slot(login, pas_hash);
            base.push_back(slot);
        }
    }

    fin.close();
    uni_lock.unlock();

    this->checking_upd_database(BUF);
}

void CheckInMap::to_file() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    std::ofstream fout(PATH_TO_CHECKIN);

    if (fout.is_open()) {
        fout << "size = " << base.size() << std::endl;

        for (auto& i : base) {
            fout << "#" << std::endl;
            fout << "log = \"" << i.first << "\";" << std::endl;
            fout << "pas = \"" << i.second << "\"." << std::endl;
        }
    }

    uni_lock.unlock();
}

void CheckInMap::add_registration(std::string& login, std::string& password, Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (!this->is_new_login(login)) {
        std::cout << "Такой логин уже используется. Попробуйте другой!" << std::endl;
        return;
    }

    std::cout << "Регистрация прошла успешно!" << std::endl;

    std::string pas_hash = picosha2::sha256(login + password);
    std::pair<std::string, std::string> tmp(login, pas_hash);
    base.push_back(tmp);

    ++NUMBER_ALL_USERS;

    BUF.mtx_to_send.lock();
    BUF.buf_to_send.emplace_back(to_string_one_registration(login, pas_hash));
    BUF.mtx_to_send.unlock();

    this->to_file();

    uni_lock.unlock();
}

void CheckInMap::add_registration_from_network(std::string& login, std::string& pas_hash, Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (!this->is_new_login(login)) {
        return;
    }

    std::pair<std::string, std::string> tmp(login, pas_hash);
    base.push_back(tmp);

    ++NUMBER_ALL_USERS;

    this->to_file();

    uni_lock.unlock();
}

void CheckInMap::from_string_with_one_registration(std::string& str, Buffer& BUF) {
    str = str.substr(12);

    size_t i = 0;
    std::string login;

    for (i = 0; str[i] != '/'; ++i) {
        login += str[i];
    }

    std::string pas_hash = str.substr(i + 1, 64);

    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    this->add_registration_from_network(login, pas_hash, BUF);
}

std::string CheckInMap::to_string_with_database() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    std::string result = "@regis_base@" + std::to_string(this->base.size()) + "@";

    for (auto& i : this->base) {
        result += i.first + "/" + i.second;
    }

    return result;
}

void CheckInMap::from_string_with_database(std::string& str, Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    str = str.substr(12);
    size_t i = 0, j = 0;
    std::string login, tmp_size;

    for (i = 0; str[i] != '@'; ++i) {
        tmp_size += str[i];
    }
    ++i;
    size_t size = std::atoi(tmp_size.c_str());

    for (j = 0; j < size; ++j) {
        for (i; str[i] != '/'; ++i) {
            login += str[i];
        }
        ++i;
        std::string pas_hash = str.substr(i, 64);
        i += 64;
        this->add_registration_from_network(login, pas_hash, BUF);
        login.clear();
    }
}

std::string to_string_one_registration(std::string& login, std::string& pas_hash) {
    return "@new_regist@" + login + "/" + pas_hash;
}



