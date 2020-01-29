//
// Created by claire on 1/29/20.
//
#include <chrono>
#include <ctime>
#include "Logger.h"
Logger::Logger(){ // LogMgr &server_log):_server_log(server_log) {

}


Logger::~Logger() {

}

void Logger::log_alert(std::string msg) {
    std::string str;
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    str.append(std::ctime(&end_time));
    str.append(": ");
    str.append(msg);
    str.append("\n");
    _log_file.openFile(FileFD::appendfd);
    _log_file.writeFD(str);
    _log_file.closeFD();
}