// Copyright 2019 Medvate <ilia.bezverzhenko@mail.ru>

#ifndef BLOCKCHAIN_FUNCTIONS_H
#define BLOCKCHAIN_FUNCTIONS_H

#include "checkin_map.h"
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <dirent.h>
#include <boost/filesystem.hpp>

void check_main_directory();
void MENU_START();
void MENU_START(AVTOR::CheckInMap& BASE, Buffer& BUF);
void MAIN_MENU(Buffer& BUF);
void MAIN_MENU(AVTOR::Blockchain& BC, Buffer& BUF);

#endif //BLOCKCHAIN_FUNCTIONS_H