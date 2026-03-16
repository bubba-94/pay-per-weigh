#include "Application.hpp"
#include "Client.hpp"

int main() {
  Application ppw("pay-per-weigh", "/dev/ttyACM0", "/dev/gpiochip4");

  Client client("localhost", 8443);

  client.postNewTestPaymentRequest();

#ifdef RPI
  States::AppInput input{};
#endif

  while (ppw.getStatus()) {
#ifdef RPI
    ppw.update(input, client.getPayment());
#endif
  }

  return 0;
}