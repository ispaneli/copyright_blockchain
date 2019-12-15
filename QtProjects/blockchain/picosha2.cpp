// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#include "picosha2.h"

using namespace picosha2;

std::string picosha2::sha256(std::string str) {
    std::vector<unsigned char> hash(picosha2::k_digest_size);

    picosha2::hash256(str.begin(), str.end(), hash.begin(), hash.end());

    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}