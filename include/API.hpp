#ifndef API_HPP
#define API_HPP

#include "external-libs/httplib.h"

#include <cstdint>
#include <iostream>
#include <string_view>
#include <thread>
#include <vector>

/***
 *
 * @class
 * SwishClient
 *
 * @brief
 * Wrapper around httplib to handle the QR and payment generating
 */
class SwishClient {
public:
private:
  httplib::Client client;
  httplib::Request req;
  httplib::Response res;
};

#endif