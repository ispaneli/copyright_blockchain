// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "block.h"
#include <cmath>
#include <random>

extern std::mutex mining_mutex;
bool flag_mining_on = false;
extern bool status_chain;
AVTOR::Block Finished_block;

using namespace AVTOR;

Block::Block() {
    for (size_t i = 0; i < 64; ++i) {
        this->Prev_Block_Hash += "0";
    }
}

Block::Block(std::string hash_of_prev_block) {
    this->Prev_Block_Hash = hash_of_prev_block;
}

Block& Block::operator=(const Block& other) {
    if (this != &other) {
        this->Number = other.Number;
        this->rights = other.rights;
        this->Prev_Block_Hash = other.Prev_Block_Hash;
        this->Root_Merkle_Tree = other.Root_Merkle_Tree;
        this->Nonce = other.Nonce;
        this->Block_Status = other.Block_Status;
    }
    return *this;
}

void Block::add_copyright(Copyright& cop) {
    this->rights.push_back(cop);
    if (rights.size() == 3) {
        this->find_Merkle_root();

        std::thread thr_1([&]() {
            Block tmp = *this;
            Finished_block = tmp.start_mining_process();
        });
        thr_1.detach();
    }
}

void Block::add_copyright_from_storage(Copyright& cop) {
    this->rights.push_back(cop);
}

void Block::add_number(size_t num) {
    this->Number = num;
}

void Block::add_PBH(std::string& hash_of_prev_block) {
    this->Prev_Block_Hash = hash_of_prev_block;
}

void Block::add_RMT(std::string& root_of_merkle_tree) {
    this->Root_Merkle_Tree = root_of_merkle_tree;
}

void Block::add_nonce(unsigned& nonce) {
    this->Nonce = nonce;
}

void Block::close_this_block() {
    this->Block_Status = false;
}

size_t& Block::get_number() {
    return this->Number;
}

std::string& Block::get_PBH() {
    return this->Prev_Block_Hash;
}

std::string& Block::get_RMT() {
    return this->Root_Merkle_Tree;
}

unsigned& Block::get_nonce() {
    return this->Nonce;
}

bool& Block::get_block_status() {
    return this->Block_Status;
}

std::string Block::get_hash_of_block() const {
    return picosha2::sha256(this->Prev_Block_Hash +
           this->Root_Merkle_Tree +
           std::to_string(this->Nonce));
}

std::string Block::find_Merkle_root() {
    // Создаем первый уровень хэшей.
    std::vector<std::string> hashs;
    for (size_t i = 0; i < rights.size(); ++i) {
        hashs.push_back(rights[i].get_hash_of_copyright());
    }

    /* Калибруем вектор хэшей, чтобы
     * его длина была равна 2^n. */
    size_t value = pow(2, 0), // value = 1
            degree = 0;
    while (value < hashs.size()) {
        ++degree;
        value = pow(2, degree);
    }
    size_t index = value - hashs.size();
    for (size_t i = 0; i < index; ++i) {
        hashs.push_back(hashs[hashs.size() - 1]);
    }

    // Ищем корень дерева хэшей.
    std::vector<std::string> tmp;
    for (size_t i = 0; i <= float(log2(hashs.size())); ++i) {
        for (size_t j = 0; j < hashs.size(); j += 2) {
            tmp.push_back(picosha2::sha256(hashs[j] + hashs[j + 1]));
        }
        hashs = tmp;
        tmp.clear();
    }

    this->Root_Merkle_Tree = hashs[0];
    return hashs[0];
}

Block& Block::start_mining_process() {
    status_chain = false;
    flag_mining_on = true;

    std::random_device rd;
    std::mt19937 mersenne(rd());

    std::cout << "----------info: Майнинг блока №" << this->get_number() << " начался." << std::endl;

    std::string hash_of_this_block = this->get_hash_of_block();

    while (hash_of_this_block[63] != '0' || hash_of_this_block[62] != '0' || hash_of_this_block[61] != '0' ||
           hash_of_this_block[60] != '0' || hash_of_this_block[59] != '0') {
        if (status_chain) {
            flag_mining_on = false;
            this->Nonce = 0;
            return *this;
        }

        this->Nonce = mersenne();

        hash_of_this_block = this->get_hash_of_block();
    }

    std::cout << "----------info: Майнинг блока №" << this->get_number() << " закончился с параметром nonce = " << this->Nonce << "." << std::endl;

    this->close_this_block();
    flag_mining_on = false;

    return *this;
}

bool Block::is_true_nonce() {
    std::string hash_of_this_block = this->get_hash_of_block();

    return (hash_of_this_block[63] == '0' && hash_of_this_block[62] == '0' && hash_of_this_block[61] == '0' &&
            hash_of_this_block[60] == '0' && hash_of_this_block[59] == '0');
}

std::string Block::to_string() {
    std::string result = "@generblock@" + std::to_string(this->Number) + "@" + this->get_PBH() +
                         this->get_RMT() + std::to_string(this->get_nonce());

    for(size_t i = 0; i < 3; ++i) {
        result += "#"+ this->rights[i].get_login() + "/" + this->rights[i].get_FH();
        result += this->rights[i].get_WH() + this->rights[i].get_date() + "!";
    }

    result += "$";

    return result;
}

void Block::from_string(std::string str) {
    str = str.substr(12);

    std::string tmp;
    size_t i = 0;

    for (; str[i] != '@'; ++i) {
        tmp += str[i];
    }

    this->Number = std::atoi(tmp.c_str());
    str = str.substr(i + 1);

    tmp = str.substr(0, 64);
    str = str.substr(64);
    this->add_PBH(tmp);

    tmp = str.substr(0, 64);
    str = str.substr(64);
    this->add_RMT(tmp);

    i = 0;
    tmp.clear();

    for (; str[i] != '#'; ++i) {
        tmp += str[i];
    }

    size_t nonce = std::atoi(tmp.c_str());
    unsigned un_nonce = nonce;
    this->add_nonce(un_nonce);

    for (size_t k = 0; k < 3; ++k) {
        tmp.clear();
        AVTOR::Copyright cop;

        ++i;
        for (; str[i] != '/'; ++i) {
            tmp += str[i];
        }
        ++i;

        cop.add_login(tmp);

        tmp = str.substr(i, 64);
        cop.add_FH(tmp);
        i += 64;

        tmp = str.substr(i, 64);
        cop.add_WH(tmp);
        i += 64;

        tmp.clear();
        for (; str[i] != '!'; ++i) {
            tmp += str[i];
        }
        ++i;
        cop.add_date(tmp);

        this->rights.push_back(cop);
    }

    this->close_this_block();
}
