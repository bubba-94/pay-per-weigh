#include "Application.hpp"
#include "Client.hpp"

int main() {
  Application ppw("pay-per-weigh", "/dev/ttyACM0", "/dev/gpiochip4");
  Client client("localhost", 8443);

  client.postNewTestPaymentRequest();

  States::AppInput input{};

  while (ppw.getStatus()) {
    ppw.update(input, client.getPayment());
  }

  return 0;
}