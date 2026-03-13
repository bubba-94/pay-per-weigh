#ifdef RPI
#ifndef STATES_HPP
#define STATES_HPP

#include <string>

/**
 * @class States
 *
 * @brief States used by application
 *
 * @details
 * Package of values sent to Application so that UI can be updated accordingly
 */
namespace States {

enum class PaymentStatus { NONE, PENDING, SUCCESS, FAILED, SENT, RECEIVED };

struct DeviceValue {
  int weight = 0;
  std::string timepoint;
};

struct PinState {
  bool shutdownRequested = false;
  bool keyEnabled = false;
};

struct PaymentData {
  int id = 0;
  PaymentStatus status = PaymentStatus::NONE;
};

/**
 * @brief Collection of states for updating UI
 */
struct AppInput {
  PinState pins;
  PaymentData payments;
  DeviceValue values;
};

}; // namespace States
#endif
#endif