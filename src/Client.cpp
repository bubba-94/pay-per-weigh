#include "Client.hpp"

const std::string LF = "Client.cpp";

Client::Client(const std::string &path, int port)
    : client(path, port),
      logger("logs", "client", "logs.txt", true, false, true) {
  if (client.is_valid()) {
    logger.log(moody::Loggr::Level::INFO, "CLIENT", "Host initialized", {LF});
    state = true;
  } else
    logger.log(moody::Loggr::Level::FATAL, "CLIENT", "No host avaialable",
               {LF});
  if (state) {
    worker = std::thread(&Client::pollStatus, this);
  }
}

Client::~Client() {
  state = false;

  if (worker.joinable()) {
    worker.join();
  }
}

void Client::pollStatus() {

  while (state.load()) {
    checkStatusOfPaymentId();

    for (int i = 0; i < 30 && state.load(); ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

States::PaymentData &Client::getPayment() {
  std::lock_guard<std::mutex> lock(dataMtx);
  return payment;
}

void Client::postNewTestPaymentRequest() {
  // Incoming from request / response
  const std::string PATH = "/paymentRequest";
  json body;

  body["amount"] = 23;
  body["payeeAlias"] = "0767639956";
  body["currency"] = "SEK";
  body["callbackUrl"] = "http://localhost:8443/callback";
  body["message"] = "Testing payment with client";

  auto res = client.Post(PATH.c_str(), body.dump(4), "application/json");
  if (!res) {
    logger.log(moody::Loggr::Level::ERROR, "CLIENT", "No valid post request",
               {LF});
    return;
  } else {
    json response = json::parse(res->body);
    {
      std::lock_guard<std::mutex> lock(dataMtx);

      payment.id = response["id"].get<int>();
    }
  }
}

void Client::checkStatusOfPaymentId() {

  // Add timer to reset?
  const std::string VALID_STATUS = "VALID";
  int id = 0;
  {
    std::lock_guard<std::mutex> lock(dataMtx);

    id = payment.id;
  }

  if (id <= 0) {
    return;
  }

  const std::string PATH = "/paymentRequest/" + std::to_string(id);

  // Get the result and populate atmoic string.
  auto res = client.Get(PATH.c_str());
  if (!res) {
    logger.log(moody::Loggr::Level::ERROR, "CLIENT", "Bad request", {LF});
    return;
  }
  if (res->status == 200) {

    json response = json::parse(res->body);

    std::string status = response.at("status").get<std::string>();
    std::string errMessage =
        "PaymentID: " + std::to_string(id) + " | " + status;
    logger.log(moody::Loggr::Level::INFO, "CLIENT", errMessage, {LF});
    {

      std::lock_guard<std::mutex> lock(dataMtx);

      if (status == VALID_STATUS &&
          payment.status == States::PaymentStatus::NONE) {

        paymentSuccessful = true;
        payment.status = States::PaymentStatus::SENT;
      }
    }
  }
}