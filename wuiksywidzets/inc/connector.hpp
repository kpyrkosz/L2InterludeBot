#pragma once

#include <string>

class connector
{
  const std::string ip;
  const std::string port;
  int mySock;

  std::size_t recv_header();
  std::string recv_data(std::size_t expected_size);

public:
  connector(const std::string& ip, const std::string& port);
  ~connector();

  void send_data(char* data, unsigned len);
  std::string get_reply();

  connector(connector&& rhs) = delete;
  void init_connection();
  void close_connection();
};
