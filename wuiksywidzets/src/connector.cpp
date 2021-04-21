#include <connector.hpp>
#define WIN32_LEAN_AND_MEAN
#include <aes_boy.hpp>
#include <stdexcept>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

connector::connector(const std::string& ip, const std::string& port)
  : ip(ip)
  , port(port)
  , mySock(INVALID_SOCKET)
{
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult)
    throw std::runtime_error("WSAStartup failed");
}

void
connector::init_connection()
{
  addrinfo *result = NULL, hints;
  int iResult;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  iResult = getaddrinfo(ip.c_str(), port.c_str(), &hints, &result);
  if (iResult)
    throw std::runtime_error("getaddrinfo failed");
  for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next)
  {
    mySock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (mySock == INVALID_SOCKET)
      throw std::runtime_error("socket failed");

    iResult = connect(mySock, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
      closesocket(mySock);
      mySock = INVALID_SOCKET;
      continue;
    }
    break;
  }

  freeaddrinfo(result);
  if (mySock == INVALID_SOCKET)
    throw std::runtime_error("connection init failed - server down most probably");
}

void
connector::send_data(char* data, unsigned len)
{
  aes_boy::inplace_encrypt(data, len);
  send(mySock, (char*)&len, 4, 0);
  send(mySock, data, len, 0);
}

std::string
connector::get_reply()
{
  auto res = recv_data(recv_header());
  std::string copy = res;
  aes_boy::inplace_decrypt(copy.data(), copy.size());
  return copy;
}

std::size_t
connector::recv_header()
{
  std::size_t out;
  auto recvResult = recv(mySock, (char*)&out, 4, 0);
  if (recvResult == 4)
    return out;
  throw std::runtime_error("did not receive header data before connection has been closed");
}

std::string
connector::recv_data(std::size_t expected_size)
{
  std::string out;
  std::size_t current_index = 0;
  out.resize(expected_size);
  while (current_index < expected_size)
  {
    auto recv_result =
      recv(mySock, &out[current_index], min(1024, expected_size - current_index), 0);
    if (recv_result <= 0)
      throw std::runtime_error(
        std::string("did not receive data before connection has been closed. Info: ") +
        std::to_string(recv_result) + ' ' + std::to_string(GetLastError()) + ' ' +
        std::to_string(current_index) + ' ' + std::to_string(expected_size));
    current_index += recv_result;
  }
  return out;
}

void
connector::close_connection()
{
  closesocket(mySock);
}

connector::~connector()
{
  close_connection();
  WSACleanup();
}
