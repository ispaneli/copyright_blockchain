// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "copyright.h"

using namespace AVTOR;

Copyright::Copyright(std::string path_to_file) {
    if (this->file_registration(path_to_file)) {
        this->add_login(LOGIN_OF_THIS_USER);
        this->fill_in_current_date();
    }
}

void Copyright::add_login(std::string& name) {
    this->login = name;
}

void Copyright::add_FH(std::string& file_hash) {
    this->File_Hash = file_hash;
}

void Copyright::add_WH(std::string& weight_hash) {
    this->Weight_Hash = weight_hash;
}

void Copyright::add_date(std::string& old_date) {
    this->Date = old_date;
}

bool Copyright::file_registration(std::string& path_to_file) {
    std::ifstream file(path_to_file);

    if (!file) {
        std::cout << path_to_file << std::endl;
        std::cout << "Invalid file path!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t weight = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string file_str;
    while (!file.eof()) {
        char buff[8];
        memset(buff, 0x00, sizeof(buff));
        file.read(buff, 7);
        file_str += buff;
    }

    file.close();

    this->File_Hash = picosha2::sha256(file_str);
    this->Weight_Hash = picosha2::sha256(std::to_string(weight));
    return true;
}

void Copyright::fill_in_current_date() {
    time_t now = time(0);
    char* dt = ctime(&now);
    int index = 0;

    std::string tmp;
    while (dt[index] != '\n') {
        tmp += dt[index];
        ++index;
    }

    this->Date = tmp;
}

std::string& Copyright::get_login() {
    return this->login;
}

std::string& Copyright::get_FH() {
    return this->File_Hash;
}

std::string& Copyright::get_WH() {
    return this->Weight_Hash;
}

std::string& Copyright::get_date() {
    return this->Date;
}

std::string Copyright::get_hash_of_copyright() const {
    return picosha2::sha256(login + File_Hash + Weight_Hash + Date);
}

std::string Copyright::to_string() {
    return "@copy_right@" + this->get_login() + "/" + this->get_FH() + this->get_WH() + this->get_date() + "/";
}

void Copyright::from_string(std::string str) {
    str = str.substr(12);
    size_t i = 0;
    std::string tmp = "";
    for (i; str[i] != '/'; ++i) {
        tmp += str[i];
    }

    ++i;
    this->add_login(tmp);

    tmp = str.substr(i, 64);
    i += 64;
    this->add_FH(tmp);

    tmp = str.substr(i, 64);
    i += 64;
    this->add_WH(tmp);

    tmp.clear();
    for (i; str[i] != '/'; ++i) {
        tmp += str[i];
    }
    this->add_date(tmp);
}
