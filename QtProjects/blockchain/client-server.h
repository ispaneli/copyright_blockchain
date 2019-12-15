// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#ifndef BLOCKCHAIN_CLIENT_SERVER1_H
#define BLOCKCHAIN_CLIENT_SERVER1_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

class Buffer {
public:
    Buffer();
    ~Buffer() = default;

    void the_server();
    void the_client();

    std::recursive_mutex mtx_to_send;
    std::recursive_mutex mtx_from_receive;

    std::vector<std::string> buf_to_send;
    std::vector<std::string> buf_from_receive;
};

#endif //BLOCKCHAIN_CLIENT_SERVER1_H