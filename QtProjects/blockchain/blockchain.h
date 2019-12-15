// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "block.h"

namespace AVTOR
{
    class Blockchain
    {
    public:
        Blockchain() = default;
        ~Blockchain() = default;

        // Добавляют новый блок в цепь.
        void add_empty_block();
        void add_empty_block(std::string hash_of_prev_block);
        // Добавляют новую праворегистрацию в цепь.
        void add_copyright(std::string path_to_file, Buffer& BUF);
        void add_copyright(AVTOR::Copyright& cop, Buffer& BUF);
        // Добавляет в цепь готовый блок из файла.
        void add_block(Block& block);
        void add_number_of_closed_blocks(size_t num);

        // Выдает число закрытых блоков в цепи.
        size_t get_number_of_closed_blocks();

        /* Считывает цепь из файла и запускает
         * её постоянное обновление. */
        void from_file(Buffer& BUF);
        // Сохраняет цепь в файл.
        void to_file();

        // Выводит параметры сети.
        void cout_chain_info();
        // Выводит все регистрации пользователя.
        void cout_user_copyrights();

        // Проверяет актуальность продукта.
        bool is_new_product(std::string path_to_file);
        /* Проверяет наличие данной
         * праворегистрации в цепи. */
        bool is_new_copyright(AVTOR::Copyright& cop);

        // Проверяет, все ли впорядке с цепью.
        bool is_true_chain();

        // Выдает строку с цепью для отправки в сеть.
        std::string to_string();
        // Обрабатывает полученную из сети строку с цепью.
        void from_string(std::string str);

        /* Сравнивается две цепи и выбирается лучшая.
         * Вовзращает ответ на вопрос: 'Цепь this - лучшая?' */
        bool is_won_the_consensus(AVTOR::Blockchain& test_chain, Buffer& BUF);

        // Постоянное обновление цепи данными из сети.
        void checking_upd_blockchain(Buffer& BUF);

        // Функции для работы со статусом цепи.
        bool get_status();
        void close_chain();
        void open_chain();

    protected:
        std::recursive_mutex mtx;

        std::vector<Block> Block_Chain;

        size_t number_of_closed_blocks = 0;

    };
};

#endif // BLOCKCHAIN_H*/
