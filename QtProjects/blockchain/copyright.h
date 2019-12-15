// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

//#define _CRT_SECURE_NO_WARNINGS

#ifndef COPYRIGHT_H
#define COPYRIGHT_H

#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>
#include "client-server.h"
#include "picosha2.h"

extern std::string LOGIN_OF_THIS_USER;

namespace AVTOR
{
    class Copyright
    {
        friend class Blockchain;

    public:
        Copyright() = default;
        ~Copyright() = default;
        // Конструктор при регистрации файла.
        Copyright(std::string path_to_file);

        // Функции задают параметры этой праворегистрации.
        void add_login(std::string& name);
        void add_FH(std::string& file_hash);
        void add_WH(std::string& weight_hash);
        void add_date(std::string& old_date);

        /* Заполняет поля хэша файла и его веса
         * по пути файла. */
        bool file_registration(std::string& path_to_file);
        /* Заполняет поле данного времени
         * (которое сейчас). */
        void fill_in_current_date();

        // Функции выдают параметры этой праворегистрации.
        std::string& get_login();
        std::string& get_FH();
        std::string& get_WH();
        std::string& get_date();

        /* Функция выдает хэш всей праворегистрации
         * для крафта хэша всего блока. */
        std::string get_hash_of_copyright() const;

        /* Выдает строку с данными этой
         * прарегистрацией для отправки в сеть. */
        std::string to_string();
        /* Заполняет эту праворегистрацию данными
         * из строки, пришедшей из сети. */
        void from_string(std::string str);

    private:
        std::string login;
        std::string File_Hash;
        std::string Weight_Hash;
        std::string Date;
    };
};


#endif // COPYRIGHT_H