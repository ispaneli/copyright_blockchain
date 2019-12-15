// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#ifndef BLOCK_H
#define BLOCK_H

#include "copyright.h"
#include <vector>
#include <mutex>

namespace AVTOR
{
    class Block
    {
        friend class Blockchain;

    public:
        Block();
        ~Block() = default;
        Block& operator=(const Block& other);
        // Конструктор для всех блоков, кроме первого.
        Block(std::string hash_of_prev_block);

        // Добавление праворегистрации в блок.
        void add_copyright(Copyright& cop);
        /* То же добавление, только отправления блока
         * на майнинг после достижения длины. равной 3-ем. */
        void add_copyright_from_storage(Copyright& cop);

        // Функции задают параметры этого блока.
        void add_number(size_t num);
        void add_PBH(std::string& hash_of_prev_block);
        void add_RMT(std::string& root_of_merkle_tree);
        void add_nonce(unsigned& nonce);
        /* Закрывает блок, т.е. изменяет его статус
         * после успешного завершения майнинга. */
        void close_this_block();

        // Функции выдают параметры этого блока.
        size_t& get_number();
        std::string& get_PBH();
        std::string& get_RMT();
        unsigned& get_nonce();
        bool& get_block_status();

        /* Выдает хэш блока, который требуется для создания
         * следующего блока и проверки целостности цепи. */
        std::string get_hash_of_block() const;

        /* Поиск корня дерева Меркле, запись его в блок
         * и выдача из функции для проверки пришедших
         * и считанных блоков на истинность. */
        std::string find_Merkle_root();

        /* Начинает майнинг. Он прекращается, если
         * приходит уже готовый блок. По успешному
         * завершению майнинга блок закрывается. */
        Block& start_mining_process();

        /* Проверяет, верный ли параметр
         * Nonce "вымайнин". */
        bool is_true_nonce();

        /* Выдает строку с данными этого
         * блока для отправки в сеть. */
        std::string to_string();
        /* Заполняет этот блок данными из
         * строки, пришедшей из сети. */
        void from_string(std::string str);


    protected:

        size_t Number;
        std::vector<Copyright> rights;

        std::string Prev_Block_Hash;
        std::string Root_Merkle_Tree;

        unsigned Nonce = 0;

        // false <-> блок закрыт.
        bool Block_Status = true;
    };
}; // namespace ad_patres

#endif // BLOCK_H
