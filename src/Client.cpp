#include "Client.hpp"

Client::Client(const std::string &path, int port) : client(path, port) {
  if (client.is_valid()) {
    std::cout << "[CLIENT] Initialized\n";
    state = true;
  } else
    std::cout << "[CLIENT] No valid host" << "\n";

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

bool Client::getPaymentStatus() {
  std::lock_guard<std::mutex> lock(dataMtx);
  return paymentSuccessful;
}

void Client::postNewPaymentRequest() {
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
    std::cout << "[CLIENT] No valid post request\n";
    return;
  } else {
    json response = json::parse(res->body);
    {
      std::lock_guard<std::mutex> lock(dataMtx);
      paymentId = response["id"].get<int>();
    }
  }
}

void Client::checkStatusOfPaymentId() {
  if (statusSent) {
    return;
  }
  // Copy paymentId into local variable???
  const std::string VALID_STATUS = "VALID";
  int id = 0;
  {
    std::lock_guard<std::mutex> lock(dataMtx);
    id = paymentId;
  }

  if (id <= 0) {
    return;
  }

  const std::string PATH = "/paymentRequest/" + std::to_string(id);

  // Get the result and populate atmoic string.
  auto res = client.Get(PATH.c_str());
  if (!res) {
    std::cout << "[CLIENT] No valid get request\n";
    return;
  }
  if (res->status == 200) {

    json response = json::parse(res->body);

    std::string status = response.at("status").get<std::string>();
    std::cout << "[CLIENT] Payment: " << id << " |  " << status << "\n";
    {
      std::lock_guard<std::mutex> lock(dataMtx);
      if (status == VALID_STATUS && !paymentSuccessful) {
        paymentSuccessful = true;
        statusSent = true;
      }
    }
  }
}