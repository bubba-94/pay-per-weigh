#ifndef API_HPP
#define API_HPP

#define CPPHTTPLIB_OPENSSL_SUPPORT // SSL Server support for httplib.h
#include "external-libs/httplib.h"

#include <cstdint>
#include <iostream>
#include <string_view>
#include <thread>
#include <vector>

/*
curl -v -X POST https://mpc.getswish.net/qrg-swish/api/v1/prefilled \
-H "Content-Type: application/json" \
--output assets/img/qr.png \
--data '{
    "format": "png",
    "payee": {
        "value": "0767639956",
        "editable": false
    },
    "amount": {
        "value": 1,
        "editable": false
    },
    "message": {
        "value": "Pay-per-weigh",
        "editable": false
    },
    "size": 300,
    "border" : 2,
    "transparent" : true
}'
*/

/***
 *img
 * SwishClient
 *
 * @brief
 * Wrapper around httplib to handle the QR and payment generating
 */
class SwishClient {
public:
  SwishClient(const std::string &client, const std::string &base_url, int port);
  int startPaymentProcess();

  bool checkSucessfulPayment();

private:
  bool paymentSuccessful = false;

  httplib::SSLClient client; // Interact with endpoints
  httplib::SSLServer server; // Serve endpoints

  httplib::Request req;
  httplib::Params params; // Possibility of making requests with params.

  httplib::Response res;
};

#endif