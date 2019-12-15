// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "blockchain.h"
#include "functions.h"
#include <boost/filesystem.hpp>

extern size_t NUMBER_ALL_USERS;
extern std::string LOGIN_OF_THIS_USER;
extern std::mutex mining_mutex;
extern std::string PATH_TO_BLOCKCHAIN;
extern bool flag_mining_on;
size_t NUMBER_BLOCK;
size_t NUMBER_COP;
std::vector<AVTOR::Copyright> user_cops;
bool status_chain = true;
extern AVTOR::Block Finished_block;

using namespace AVTOR;

void Blockchain::add_empty_block() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (this->Block_Chain.empty()) {
        Block block;
        block.add_number(0);

        Block_Chain.push_back(block);
        return;
    }

    this->add_empty_block(this->Block_Chain[this->Block_Chain.size() - 1].get_hash_of_block());
}

void Blockchain::add_empty_block(std::string hash_of_prev_block) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (this->Block_Chain[this->Block_Chain.size() - 1].get_block_status() == false) {
        Block block(hash_of_prev_block);
        block.add_number(this->get_number_of_closed_blocks());

        Block_Chain.push_back(block);
    }
}

void Blockchain::add_copyright(std::string path_to_file, Buffer& BUF) {
    AVTOR::Copyright cop(path_to_file);

    this->add_copyright(cop, BUF);
}

void Blockchain::add_copyright(AVTOR::Copyright& cop, Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (cop.get_login().empty()) {
        return;
    }

    if (this->is_new_copyright(cop)) {
        BUF.mtx_to_send.lock();
        BUF.buf_to_send.emplace_back(cop.to_string());
        BUF.mtx_to_send.unlock();

        if (!status_chain) {
            return;
        }

        if (this->Block_Chain.empty()) {
            this->add_empty_block();
            this->Block_Chain[0].add_copyright(cop);

        } else if (this->Block_Chain[this->Block_Chain.size() - 1].get_block_status()) {

            this->Block_Chain[this->Block_Chain.size() - 1].add_copyright(cop);

        } else {
            this->add_empty_block();
            this->Block_Chain[this->Block_Chain.size() - 1].add_copyright(cop);
        }
    }
}

void Blockchain::add_block(Block& block) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    this->Block_Chain.push_back(block);
}

void Blockchain::add_number_of_closed_blocks(size_t num) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    this->number_of_closed_blocks = num;
}

size_t Blockchain::get_number_of_closed_blocks() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    return this->number_of_closed_blocks;
}

void Blockchain::from_file(Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx, std::defer_lock);

    std::ifstream fin(PATH_TO_BLOCKCHAIN);

    uni_lock.lock();

    if (fin.is_open()) {
        // Находим число блоков в цепи.
        std::string line;
        getline(fin, line);

        if (line.length() == 0) {
            uni_lock.unlock();
            this->checking_upd_blockchain(BUF);
            return;
        }

        unsigned tmp = std::atoi(line.substr(9, line.size() - 9).c_str());
        this->add_number_of_closed_blocks(tmp);

        if (this->get_number_of_closed_blocks() != 0) {
            // Собираем цепь по-блочно.
            for(size_t i = 0; i < this->get_number_of_closed_blocks(); ++i) {
                getline(fin, line);
                getline(fin, line);

                AVTOR::Block block;

                getline(fin, line);
                line = line.substr(7, 64);
                block.add_PBH(line);

                getline(fin, line);
                line = line.substr(7, 64);
                block.add_RMT(line);

                getline(fin, line);
                line = line.substr(7, line.size() - 7);
                tmp = std::atoi(line.c_str());
                block.add_nonce(tmp);

                AVTOR::Copyright cop;

                // Собираем блок по праворегистрациям.
                for (size_t i = 0; i < 3; ++i) {
                    getline(fin, line);

                    getline(fin, line);
                    line = line.substr(6, line.size() - 8);
                    cop.add_login(line);

                    getline(fin, line);
                    line = line.substr(6, 64);
                    cop.add_FH(line);

                    getline(fin, line);
                    line = line.substr(6, 64);
                    cop.add_WH(line);

                    getline(fin, line);
                    line = line.substr(6, line.size() - 8);
                    cop.add_date(line);

                    block.add_copyright_from_storage(cop);
                }

                block.add_number(i);
                block.close_this_block();

                this->add_block(block);

                getline(fin, line);
            }
        }
    }


    if (this->is_true_chain()) {
        std::cout << "----------info: На ПК была сохранена интинная цепь!" << std::endl;
    } else {
        std::cout << "----------info: На ПК была сохранена ЛОЖНАЯ цепь! Она была удалена." << std::endl;
        this->add_number_of_closed_blocks(0);
        this->Block_Chain.clear();
    }

    uni_lock.unlock();

    this->checking_upd_blockchain(BUF);
}

void Blockchain::to_file() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    std::ofstream fout(PATH_TO_BLOCKCHAIN);

    if (fout.is_open()) {
        fout << "length = " << this->get_number_of_closed_blocks() << std::endl;

        for(size_t i = 0; i < this->get_number_of_closed_blocks(); ++i) {
            for(size_t j = 0; j < 72; ++j) {
                fout << "_";
            }
            fout << std::endl << "{" << std::endl;

            fout << "PBH = \"" << this->Block_Chain[i].get_PBH() << "\"" << std::endl;
            fout << "RMT = \"" << this->Block_Chain[i].get_RMT() << "\"" << std::endl;
            fout << "nnc = \"" << this->Block_Chain[i].get_nonce() << "\"" << std::endl;

            for(size_t j = 0; j < 3; ++j) {
                fout << "#" << std::endl;

                fout << "lg = \"" << this->Block_Chain[i].rights[j].get_login() << "\";" << std::endl;
                fout << "FH = \"" << this->Block_Chain[i].rights[j].get_FH() << "\";" << std::endl;
                fout << "WH = \"" << this->Block_Chain[i].rights[j].get_WH() << "\";" << std::endl;
                fout << "dt = \"" << this->Block_Chain[i].rights[j].get_date() << "\"." << std::endl;
            }

            fout << "}" << std::endl;
        }
    }
}

void Blockchain::cout_chain_info() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx, std::defer_lock);

    std::cout << "============== BLOCKCHAIN INFORMATION ==============" << std::endl;
    size_t number_blocks = this->Block_Chain.size(), number_copyright;

    uni_lock.lock();
    if (this->Block_Chain.size() == 0) {
        number_copyright = 0;

    } else if (this->Block_Chain[this->Block_Chain.size() - 1].get_block_status()) {
        --number_blocks;
        number_copyright = number_blocks * 3 + this->Block_Chain[this->Block_Chain.size() - 1].rights.size();

    } else if (!this->Block_Chain[this->Block_Chain.size() - 1].get_block_status()) {
        number_copyright = number_blocks * 3;
    }
    NUMBER_BLOCK = number_blocks;
    NUMBER_COP = number_copyright;

    std::cout << "--- your login:________________________ " << LOGIN_OF_THIS_USER << std::endl;
    std::cout << "--- number blocks in chain:____________ " << number_blocks << std::endl;
    std::cout << "--- number copyright in chain:_________ " << number_copyright << std::endl;
    std::cout << "--- number all users in project:_______ " << NUMBER_ALL_USERS << std::endl;

    uni_lock.unlock();
}


void Blockchain::cout_user_copyrights() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx, std::defer_lock);

    std::cout << "=================================== YOUR REGISTERED PRODUCTS ===================================" << std::endl;
    size_t number = 1;

    uni_lock.lock();
    user_cops.clear();

    for (size_t i = 0; i < this->Block_Chain.size(); ++i) {

        for (size_t j = 0; j < this->Block_Chain[i].rights.size(); ++j) {

            if (Block_Chain[i].rights[j].get_login() == LOGIN_OF_THIS_USER) {
                std::cout << "Number " << number << "." << std::endl;
                ++number;

                std::cout << "--- date this registration: " << Block_Chain[i].rights[j].get_date() << "." << std::endl;
                std::cout << "--- hashs of this product:  1) " << Block_Chain[i].rights[j].get_FH() << " ;" << std::endl;
                std::cout << "---                         2) " << Block_Chain[i].rights[j].get_WH() << " ." << std::endl;

                user_cops.push_back(Block_Chain[i].rights[j]);
            }
        }
    }

    uni_lock.unlock();
}

bool Blockchain::is_new_product(std::string path_to_file) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx, std::defer_lock);

    std::ifstream file(path_to_file);

    /* Пробегаем по файлу туда и обратно,
     * чтобы определить его вес. */
    file.seekg(0, std::ios::end);
    size_t WeightFile = file.tellg();
    file.seekg(0, std::ios::beg);

    // Конвертируем файл в строку.
    std::string str;
    while (!file.eof()) {
        char buff[8];
        memset(buff, 0x00, sizeof(buff));
        file.read(buff, 7);
        str += buff;
    }

    file.close();

    // Хэшируем все это дело.
    std::string weight_hash = picosha2::sha256(std::to_string(WeightFile)),
                file_hash = picosha2::sha256(str);

    uni_lock.lock();

    // Бегаем по цепи в поисках регистрации данного файла.
    for (size_t i = 0; i < this->Block_Chain.size(); ++i) {

        for (size_t j = 0; j < this->Block_Chain[i].rights.size(); ++j) {

            if (Block_Chain[i].rights[j].get_FH() == file_hash &&
                Block_Chain[i].rights[j].get_WH() == weight_hash) {

                std::cout << "This product was registered by " << Block_Chain[i].rights[j].get_login()
                          << " on " << Block_Chain[i].rights[j].get_date() << "." << std::endl;

                return false;
            }
        }
    }

    uni_lock.unlock();

    std::cout << "This product was not registered." << std::endl;
    return true;
}

bool Blockchain::is_new_copyright(AVTOR::Copyright& cop) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    for (size_t i = 0; i < this->Block_Chain.size(); ++i) {

        for (size_t j = 0; j < this->Block_Chain[i].rights.size(); ++j) {

            if (cop.get_login() == this->Block_Chain[i].rights[j].get_login() &&
                cop.get_FH() == this->Block_Chain[i].rights[j].get_FH() &&
                cop.get_WH() == this->Block_Chain[i].rights[j].get_WH()) {

                return false;
            }
        }
    }

    return true;
}

bool Blockchain::is_true_chain() {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    for(size_t i = 0; i < this->get_number_of_closed_blocks(); ++i) {

        if (this->Block_Chain[i].find_Merkle_root() != this->Block_Chain[i].get_RMT()) {

            return false;
        }
        if (!this->Block_Chain[i].is_true_nonce()) {
            return false;
        }
        if (i > 0) {
            if (this->Block_Chain[i - 1].get_hash_of_block() != this->Block_Chain[i].get_PBH()) {
                return false;
            }
        }
    }
    return true;
}

std::string Blockchain::to_string() {
    std::string result = "@blockchain@size=" + std::to_string(this->get_number_of_closed_blocks()) + ";";

    for (size_t i = 0; i < this->get_number_of_closed_blocks(); ++i) {
        result += this->Block_Chain[i].to_string();
    }

    return result;
}

void Blockchain::from_string(std::string str) {
    str = str.substr(17);
    std::string tmp;

    size_t i = 0;
    for (; str[i] != ';'; ++i) {
        tmp += str[i];
    }

    this->add_number_of_closed_blocks(std::atoi(tmp.c_str()));

    str = str.substr(i + 1);

    for (size_t k = 0; k < this->get_number_of_closed_blocks(); ++k) {

        tmp.clear();
        i = 0;

        for (; str[i] != '$'; ++i) {
            tmp += str[i];
        }

        AVTOR::Block block;
        block.from_string(tmp);

        this->Block_Chain.push_back(block);

        str = str.substr(i + 1);
    }

}

bool Blockchain::is_won_the_consensus(AVTOR::Blockchain& test_chain, Buffer& BUF) {
    std::unique_lock<std::recursive_mutex> uni_lock(this->mtx);

    if (!this->is_true_chain()) {
        // Тут должно быть еще сохранение в буфер новых регистраций!
        this->Block_Chain = test_chain.Block_Chain;
        this->add_number_of_closed_blocks(test_chain.get_number_of_closed_blocks());

        return false;

    } else if (!test_chain.is_true_chain()) {
        BUF.mtx_to_send.lock();
        BUF.buf_to_send.emplace_back(this->to_string());
        BUF.mtx_to_send.unlock();

        return true;
    }

    if (this->get_number_of_closed_blocks() > test_chain.get_number_of_closed_blocks()) {
        BUF.mtx_to_send.lock();
        BUF.buf_to_send.emplace_back(this->to_string());
        BUF.mtx_to_send.unlock();

        return true;

    } else if (this->get_number_of_closed_blocks() == test_chain.get_number_of_closed_blocks()) {
        return true;

    } else {
        // Тут должно быть еще сохранение в буфер новых регистраций!
        this->Block_Chain = test_chain.Block_Chain;
        this->add_number_of_closed_blocks(test_chain.get_number_of_closed_blocks());

        return false;
    }
}




void Blockchain::checking_upd_blockchain(Buffer& BUF) {
    BUF.mtx_to_send.lock();
    BUF.buf_to_send.emplace_back("@give_bl-ch@");
    BUF.mtx_to_send.unlock();

    while (true) {
        BUF.mtx_from_receive.lock();

        if (flag_mining_on) {
            if (Finished_block.get_nonce() != 0) {
                status_chain = true;
                this->mtx.lock();

                this->Block_Chain.erase(this->Block_Chain.end() - 1);

                this->Block_Chain.push_back(Finished_block);
                this->add_number_of_closed_blocks(this->get_number_of_closed_blocks() + 1);

                this->mtx.unlock();

                BUF.mtx_to_send.lock();
                BUF.buf_to_send.emplace_back(Finished_block.to_string());
                BUF.mtx_to_send.unlock();

                this->to_file();
            }

            if (BUF.buf_from_receive.size() != 0) {
                for (size_t i = 0; i < BUF.buf_from_receive.size(); ++i) {
                    if (BUF.buf_from_receive[i].substr(0, 12) == "@generblock@") {
                        AVTOR::Block block;
                        block.from_string(BUF.buf_from_receive[i]);

                        if ((this->get_number_of_closed_blocks() == block.get_number()) &&
                            (block.get_PBH() == this->Block_Chain[this->number_of_closed_blocks - 1].get_hash_of_block())) {

                            status_chain = true;
                            this->mtx.lock();

                            this->Block_Chain.erase(this->Block_Chain.end() - 1);

                            this->Block_Chain.push_back(block);
                            this->add_number_of_closed_blocks(this->get_number_of_closed_blocks() + 1);

                            this->mtx.unlock();
                            this->to_file();
                        }
                        BUF.buf_from_receive.erase(BUF.buf_from_receive.begin() + i);
                    }
                }
            }
        } else {
            if (!status_chain) {
                if (Finished_block.get_nonce() != 0) {
                    status_chain = true;
                    this->mtx.lock();

                    this->Block_Chain.erase(this->Block_Chain.end() - 1);

                    this->Block_Chain.push_back(Finished_block);
                    this->add_number_of_closed_blocks(this->get_number_of_closed_blocks() + 1);

                    this->mtx.unlock();

                    BUF.mtx_to_send.lock();
                    BUF.buf_to_send.emplace_back(Finished_block.to_string());
                    BUF.mtx_to_send.unlock();
                    this->to_file();
                }
            }
            if (BUF.buf_from_receive.size() != 0) {
                std::string str = BUF.buf_from_receive[0];

                if (str.substr(0, 12) == "@copy_right@") {
                    AVTOR::Copyright cop;
                    cop.from_string(str);

                    this->add_copyright(cop, BUF);
                    BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());

                } else if (str.substr(0, 12) == "@generblock@") {
                    AVTOR::Block block;
                    block.from_string(str);

                    if (this->get_number_of_closed_blocks() < block.get_number()) {
                        BUF.mtx_to_send.lock();
                        BUF.buf_to_send.emplace_back("@give_bl-ch@");
                        BUF.mtx_to_send.unlock();

                    } else if (this->get_number_of_closed_blocks() > block.get_number()) {
                        BUF.mtx_to_send.lock();
                        BUF.buf_to_send.emplace_back(this->to_string());
                        BUF.mtx_to_send.unlock();

                    } else if (this->get_number_of_closed_blocks() == block.get_number() ) {

                        if (block.get_PBH() == this->Block_Chain[this->number_of_closed_blocks - 1].get_hash_of_block()) {
                            status_chain = true;
                            this->mtx.lock();

                            this->Block_Chain.erase(this->Block_Chain.end() - 1);

                            this->Block_Chain.push_back(block);
                            this->add_number_of_closed_blocks(this->get_number_of_closed_blocks() + 1);

                            this->mtx.unlock();
                        }
                    }
                    BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());

                } else if (str.substr(0, 12) == "@blockchain@") {
                    AVTOR::Blockchain new_chain;
                    new_chain.from_string(str);

                    this->is_won_the_consensus(new_chain, BUF);

                    BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());

                } else if (str.substr(0, 12) == "@give_bl-ch@") {
                    BUF.buf_from_receive.erase(BUF.buf_from_receive.begin());

                    BUF.mtx_to_send.lock();
                    BUF.buf_to_send.emplace_back(this->to_string());
                    BUF.mtx_to_send.unlock();
                }
            }
        }
        BUF.mtx_from_receive.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool Blockchain::get_status() {
    return status_chain;
}

void Blockchain::close_chain() {
    status_chain = false;
}

void Blockchain::open_chain() {
    status_chain = true;
}
