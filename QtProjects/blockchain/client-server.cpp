// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "client-server.h"
#include <thread>

Buffer::Buffer() {
    std::thread thr_1([&](){
        this->the_client();
    });

    std::thread thr_2([&](){
        this->the_server();
    });

    thr_1.detach();
    thr_2.detach();
}

void Buffer::the_server() {
    namespace ip = boost::asio::ip;

    boost::asio::io_service io_service;
    ip::udp::socket socket(io_service,ip::udp::endpoint(ip::udp::v4(), 0));
    socket.set_option(boost::asio::socket_base::broadcast(true));
    ip::udp::endpoint broadcast_endpoint(ip::address_v4::broadcast(), 8888);

    std::string message;

    while (true) {
        this->mtx_to_send.lock();

        if (this->buf_to_send.size() != 0) {
            this->mtx_to_send.lock();
            message = std::to_string(this->buf_to_send[0].length()) + this->buf_to_send[0];
            this->mtx_to_send.unlock();

            socket.send_to(boost::asio::buffer(&message[0], message.length()), broadcast_endpoint);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            socket.send_to(boost::asio::buffer(&message[0], message.length()), broadcast_endpoint);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            socket.send_to(boost::asio::buffer(&message[0], message.length()), broadcast_endpoint);

            /// TEST BLOCK
            if (message.length() > 20) {
                std::cout << "----------info: Сообщение \"" << message.substr(0, 20) << "...\" было отправлено!" << std::endl;
            } else {
                std::cout << "----------info: Сообщение \"" << message << "\" было отправлено!" << std::endl;
            }
            /// FOR BLOCK

            message.clear();
            this->mtx_to_send.lock();
            this->buf_to_send.erase(this->buf_to_send.begin());
            this->mtx_to_send.unlock();
        }

        this->mtx_to_send.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Buffer::the_client() {
    namespace ip = boost::asio::ip;

    boost::asio::io_service io_service;
    ip::udp::socket socket(io_service,ip::udp::endpoint(ip::udp::v4(), 8888 ));
    ip::udp::endpoint sender_endpoint;

    std::string message(" ", 35499);

    /// TEST BLOCK
    size_t flag_test = 1;
    /// TEST BLOCK

    while (true) {
        std::size_t bytes_transferred = socket.receive_from(boost::asio::buffer(&message[0], message.size()), sender_endpoint);
        /// TEST BLOCK
        std::cout << "----------info: " << flag_test << ") Got " << bytes_transferred << " bytes. " << std::endl;
        ++flag_test;
        /// TEST BLOCK

        std::string tmp;
        size_t i = 0;
        for(i; message[i] != '@'; ++i) {
            tmp += message[i];
        }

        std::string result = message.substr(i, std::atoi(tmp.c_str()));

        /// TEST BLOCK
        if (result.length() > 20) {
            std::cout << "----------info: Сообщение \"" << result.substr(0, 20) << "...\" было получено!" << std::endl;
        } else {
            std::cout << "----------info: Сообщение \"" << result << "\" было получено!" << std::endl;
        }
        /// FOR BLOCK

        this->mtx_from_receive.lock();
        this->buf_from_receive.emplace_back(result);
        this->mtx_from_receive.unlock();
    }
}
