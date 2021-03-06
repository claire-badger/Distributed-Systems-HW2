#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include "TCPServer.h"
#include <algorithm>>

TCPServer::TCPServer(){ // :_server_log("server.log", 0) {

    //init the whitelist of ip addresses
    std::fstream inFile;
    std::string x;
    inFile.open("whitelist.txt");

    if (!inFile) {
        std::cerr << "Unable to open file whitelist.txt";
        exit(1);   // call system to stop
    }

    while (inFile >> x) {
        ip_addresses.push_back(x);
    }
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

   struct sockaddr_in servaddr;

   // _server_log.writeLog("Server started.");

   // Set the socket to nonblocking
   _sockfd.setNonBlocking();

   // Load the socket information to prep for binding
   _sockfd.bindFD(ip_addr, port);
 
}

bool TCPServer::CheckIP(){
    std::string ip;
    _sockfd.getIPAddrStr(ip);
    return std::find(ip_addresses.begin(), ip_addresses.end(), ip) != ip_addresses.end();
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {

   bool online = true;
   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;
   int num_read = 0;

   // Start the server socket listening
   _log.log_alert("Server Started");
   _sockfd.listenFD(5);

   while (online) {
      struct sockaddr_in cliaddr;
      socklen_t len = sizeof(cliaddr);

      if (CheckIP()) {
          if (_sockfd.hasData()) {
              TCPConn *new_conn = new TCPConn();
              if (!new_conn->accept(_sockfd)) {
                  std::string strbuf;
                  _sockfd.getIPAddrStr(strbuf);
                  _log.log_alert("Data received on socket but failed to accept. at IP address: " + strbuf);
                  continue;
              }
              std::cout << "***Got a connection***\n";

              _connlist.push_back(std::unique_ptr<TCPConn>(new_conn));

              // Get their IP Address string to use in logging
              std::string ipaddr_str;
              new_conn->getIPAddrStr(ipaddr_str);
              _log.log_alert("New successful connection at " + ipaddr_str);


              new_conn->sendText("Welcome to the CSCE 689 Server!\n");

              // Change this later
              new_conn->startAuthentication();
          }

          // Loop through our connections, handling them
          std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connlist.begin();
          while (tptr != _connlist.end()) {
              // If the user lost connection
              if (!(*tptr)->isConnected()) {
                  // Log it

                  // Remove them from the connect list
                  tptr = _connlist.erase(tptr);
                  std::cout << "Connection disconnected.\n";
                  continue;
              }

              // Process any user inputs
              (*tptr)->handleConnection();

              // Increment our iterator
              tptr++;
          }
      }else{
          _log.log_alert("Attempted connection by IP not on whitelist: ", _sockfd);
      }

      // So we're not chewing up CPU cycles unnecessarily
      nanosleep(&sleeptime, NULL);
   }


   
}


/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {

   _sockfd.closeFD();
}


