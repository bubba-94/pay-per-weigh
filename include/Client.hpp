#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "States.hpp"

#include <chrono>
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
  void postNewTestPaymentRequest();
#ifdef RPI
  const States::PaymentData &getPayment();
#endif

private:
  void pollStatus();
  void checkStatusOfPaymentId();
  // Evaluate with response from getStatus()
  std::thread worker;
  std::atomic<bool> state;
  std::atomic<bool> statusSent = false;
  std::atomic<bool> paymentSuccessful = false;

  // Timer
  bool timer = false;
  std::chrono::steady_clock::time_point tp;

  std::mutex dataMtx;
  httplib::Client client;
#ifdef RPI
  // Should have a vector of payments
  States::PaymentData payment;
#endif
};

#endif