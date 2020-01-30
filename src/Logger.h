//
// Created by claire on 1/29/20.
//
#include "FileDesc.h"

#ifndef AFIT_CSCE689_HW2_S_LOGGER_H
#define AFIT_CSCE689_HW2_S_LOGGER_H


class Logger {

public:
    Logger();
    ~Logger();
    void log_alert(std::string msg, SocketFD log_fd);
    void log_alert(std::string msg);

private:
    FileFD _log_file = FileFD("server.log");

};


#endif //AFIT_CSCE689_HW2_S_LOGGER_H
