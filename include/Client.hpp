#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "external-libs/httplib.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// SKIP SWISH. MOCK PAYMENT PROCESS
// Run a docker container hosting the endpoints
// https://developer.swish.nu/documentation/guides/qr-codes-for-your-terminal

class Client {
public:
  Client(const std::string &path, int port);
  ~Client();
  void pollStatus();
  bool getPaymentStatus();
  void postNewPaymentRequest();
  void checkStatusOfPaymentId();

private:
  // Evaluate with response from getStatus()
  std::thread worker;
  std::atomic<int> paymentId = 0;
  std::atomic<bool> state;
  std::atomic<bool> statusSent = false;
  std::atomic<bool> paymentSuccessful = false;
  std::mutex dataMtx;
  httplib::Client client;
};

#endif