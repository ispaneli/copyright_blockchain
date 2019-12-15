// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#ifndef BLOCKCHAIN_CHECKIN_MAP_H
#define BLOCKCHAIN_CHECKIN_MAP_H

#include "blockchain.h"
#include <map>
#include <chrono>
#include <boost/filesystem.hpp>

namespace AVTOR {

    class CheckInMap {
    public:
        CheckInMap() = default;
        ~CheckInMap() = default;

        // Проверяет введенные логин и пароль при авторизации.
        bool is_true_login_and_password(std::string& login, std::string& password);
        // Проверяет, занят ли данный логин.
        bool is_new_login(std::string& login);

        // Постоянное обновление БД данными из сети.
        void checking_upd_database(Buffer& BUF);
        /* Считывает БД из файла и запускает
         * её постоянное обновление. */
        void from_file(Buffer& BUF);
        // Сохраняет БД в файл.
        void to_file();

        // Добавляет регистрацию с этого ПК в БД.
        void add_registration(std::string& login, std::string& password, Buffer& BUF);
        // Добавляет регистрацию из сети в БД.
        void add_registration_from_network(std::string& login, std::string& pas_hash, Buffer& BUF);

        // Обрабатывает полученную из сети строку с одной регистрацией.
        void from_string_with_one_registration(std::string& str, Buffer& BUF);

        // Выдает строку с БД для отправки в сеть.
        std::string to_string_with_database();
        // Обрабатывает полученную из сети строку с БД.
        void from_string_with_database(std::string& str, Buffer& BUF);

    protected:
        std::vector<std::pair<std::string, std::string>> base;
        std::recursive_mutex mtx;
    };

};

// Выдает строку с одной регистрацией для отправки в сеть.
std::string to_string_one_registration(std::string& login, std::string& pas_hash);

#endif //BLOCKCHAIN_CHECKIN_MAP_H
