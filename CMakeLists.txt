cmake_minimum_required(VERSION 3.15)
project(AFIT_CSCE689_HW2_S)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(Client
    include/Client.h
    include/exceptions.h
    include/strfuncts.h
    include/TCPClient.h
    include/FileDesc.h
    src/FileDesc.cpp
    src/Client.cpp
    src/client_main.cpp
        src/strfuncts.cpp
    src/TCPClient.cpp
   )

add_executable(Server include/Server.h
        include/TCPConn.h
        include/TCPServer.h
        include/strfuncts.h
        include/PasswdMgr.h
        include/FileDesc.h
        src/Logger.h
        src/Logger.cpp
        src/FileDesc.cpp
        src/PasswdMgr.cpp
        src/strfuncts.cpp
        src/Server.cpp
        src/server_main.cpp
        src/strfuncts.cpp
        src/TCPConn.cpp
        src/TCPServer.cpp
        )

add_executable(addUser
        src/adduser_main.cpp
        src/FileDesc.cpp
        src/PasswdMgr.cpp
        src/strfuncts.cpp
        include/strfuncts.h
        include/FileDesc.h
        include/PasswdMgr.h
)

target_include_directories(Client PRIVATE src include)
target_include_directories(Server PRIVATE src include)
target_include_directories(addUser PRIVATE src include)
target_link_libraries(Server argon2)
target_link_libraries(addUser argon2)
