#ifndef REXON_H
#define REXON_H

// Icnludes
#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <unordered_map>

// Cross-platform things
#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define sock SOCKET
#define closeconnection closesocket
#define readconnection recv
#elif __linux__
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define sock int
#define closeconnection close
#define readconnection read
#endif

// Declarations
namespace REXON {
  class Response {
    private:
      std::string status;
      std::string content_type;
      std::string body;
    public:
      Response(std::string status, std::string content_type, std::string body) {
        this->status = status;
        this->content_type = content_type;
        this->body = body;
      }
      void set_status(std::string status) {
        this->status = status;
      }
      void set_content_type(std::string content_type) {
        this->content_type = content_type;
      }
      void set_body(std::string body) {
        this->body = body;
      }
      std::string get_status() {
        return this->status;
      }
      std::string get_content_type() {
        return this->content_type;
      }
      std::string get_body() {
        return this->body;
      }
  };
  class api {
    private:
      int port;
      sock connection;
      bool running;
      bool showlog;
      struct sockaddr_in address;
      int addrlen;
      std::unordered_map<std::string, std::function<Response(std::string)>> routes;
    #ifdef __WIN32__
      WSADATA wsaData;
    #endif
      void handle_client(sock client_socket);
      Response route(std::string path, std::string body, std::string method);
    public:
      api();
      api(int port);
      ~api();
      void start();
      void stop();
      void GET(std::string path, std::function<Response(std::string)> callback) {
        this->routes["GET " + path] = callback;
      }
      void POST(std::string path, std::function<Response(std::string)> callback) {
        this->routes["POST " + path] = callback;
      }
      void NOTFOUND(std::function<Response(std::string)> callback) {
        this->routes["NOTFOUND"] = callback;
      }
      void show_log(bool show) {
        this->showlog = show;
      }
    };
  std::string get_attribute(std::string str, std::string key) {
    size_t start = str.find(key + "=");
    if (start == std::string::npos) return "";
    start += key.length() + 1;
    size_t end = str.find("&", start);
    return str.substr(start, end - start);
  }
};

// Implementation
REXON::api::api(int port_) {
  #ifdef __WIN32__
    WSAStartup(MAKEWORD(2, 2), &this->wsaData);
  #endif
  this->port = port_;
  this->connection = socket(AF_INET, SOCK_STREAM, 0);
  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;
  this->address.sin_port = htons(port_);
  this->addrlen = sizeof(this->address);
}
REXON::api::api() {
  #ifdef __WIN32__
    WSAStartup(MAKEWORD(2, 2), &this->wsaData);
  #endif
  this->port = 8080;
  this->connection = socket(AF_INET, SOCK_STREAM, 0);
  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;
  this->address.sin_port = htons(8080);
  this->addrlen = sizeof(this->address);
}
REXON::api::~api() {
  closeconnection(this->connection);
  #ifdef __WIN32__
    WSACleanup();
  #endif
}
void REXON::api::start() {
  this->running = true;
  this->address.sin_port = htons(this->port);
  bind(this->connection, (sockaddr *)&this->address, sizeof(this->address));
  listen(this->connection, 10);
  struct sockaddr_in client_address;
  socklen_t client_addrlen = sizeof(client_address);
  char *client_ip;
  while (this->running) {
    sock new_client = accept(this->connection, (sockaddr *)&client_address, (socklen_t *)&client_addrlen);
    client_ip = inet_ntoa(client_address.sin_addr);
    if (this->showlog) printf("Client connected from %s:%d\n", client_ip, client_address.sin_port);
    std::thread(&REXON::api::handle_client, this, new_client).detach();
  }
}
void REXON::api::stop() {
  this->running = false;
}
void REXON::api::handle_client(sock client_socket) {
  char buffer[4096] = {0};
  int bytes_read = readconnection(client_socket, buffer, sizeof(buffer));
  if (bytes_read <= 0) {
    if (this->showlog) printf("Client disconnected\n");
    closeconnection(client_socket);
    return;
  }
  std::istringstream request_stream(buffer);
  std::string request_line;
  std::getline(request_stream, request_line);
  std::string method, path;
  std::istringstream request_line_stream(request_line);
  request_line_stream >> method >> path;
  int content_length = 0;
  std::string header;
  while (std::getline(request_stream, header) && header != "\r") {
    if (header.find("Content-Length:") == 0) {
      content_length = std::stoi(header.substr(15));
    }
  }
  std::string body;
  if (method == "POST" && content_length > 0) {
    body.resize(content_length);
    request_stream.readconnection(&body[0], content_length);
  }
  Response res = this->route(path, body, method);
  std::string response =
    "HTTP/1.1 " + res.get_status() + "\r\n"
    "Content-Type: " + res.get_content_type() + "\r\n"
    "Content-Length: " + std::to_string(res.get_body().size()) + "\r\n"
    "\r\n" + res.get_body();
  send(client_socket, response.c_str(), response.size(), 0);
  if (this->showlog) printf("Response sent\n");
  closeconnection(client_socket);
}
REXON::Response REXON::api::route(std::string path, std::string body, std::string method) {
  Response res("404 Not Found", "text/plain", "Not Found");
  if (this->routes.find(std::string(method + " " + path)) != this->routes.end()) {
    res = this->routes[std::string(method + " " + path)](body);
  } else if (this->routes.find(std::string("NOTFOUND")) != this->routes.end()) {
    res = this->routes[std::string("NOTFOUND")]("");
  }
  return res;
}


#undef sock
#undef closeconnection
#undef readconnection
#endif