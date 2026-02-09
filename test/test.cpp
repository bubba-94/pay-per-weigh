#include <iomanip>

// Serial/terminal communication
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <vector>

const char *port = "/dev/ttyACM0";

enum class Log : uint8_t {
  WRITE,
  READ,
  SERIAL,
  PORT,
  ERROR,
  INFO,
  VALID,
  INVALID,
  REQUEST,
  RESPONSE
};

int openPort();
void configureSerial(termios &settings, int baud);
std::string hexByte(uint8_t b);
void log(Log log, const std::string &msg);
const char *logStr(Log log);

int main() {

  int fd = openPort();

  std::vector<uint8_t> buffer = {0xB0, 0x00, 0x41, 0x03, 0x02, 0x00, 0x00};

  if (buffer.empty()) {
    buffer.push_back(0x80);
  } else {
    buffer.front() |= 0x80;
  }

  uint8_t reqId = buffer[0];
  uint8_t reqMa = buffer[1];
  uint8_t reqIogc = buffer[2];
  uint8_t reqCcode = buffer[3];
  uint8_t reqAct = buffer[4];

  uint16_t reqChksum = 0;

  for (size_t i = 0; i < buffer.size() - sizeof(uint16_t); i++) {
    reqChksum += buffer[i];
  }
  // Construct 16 bit value
  reqChksum = (((reqChksum << 1) & 0xFF00) | (reqChksum & 0x7F)) | 0xC000;

  buffer[5] = (reqChksum & 0xFF);
  buffer[6] = ((reqChksum >> 8) & 0xFF);

  log(Log::REQUEST, "FRAME");
  log(Log::WRITE, "Protocol ID:     " + hexByte(reqId));
  log(Log::WRITE, "Machine Address: " + hexByte(reqMa));
  log(Log::WRITE, "IO Group Code:   " + hexByte(reqIogc));
  log(Log::WRITE, "Command Code:    " + hexByte(reqCcode));
  log(Log::WRITE, "Action:          " + hexByte(reqAct));
  log(Log::WRITE, "Checksum 1:      " + std::to_string(buffer[5]));
  log(Log::WRITE, "Checksum 2:      " + std::to_string(buffer[6]));

  size_t written = 0;

  while (written < buffer.size()) {
    ssize_t bytes = write(fd, buffer.data(), buffer.size());
    if (bytes <= 0) {
      log(Log::SERIAL, "No bytes available to write");
      break;
    }
    written += bytes;
  }

  log(Log::SERIAL, "Total bytes written: " + std::to_string(written));

  // Allocate for new values
  buffer.resize(16);

  size_t readBytes = 0;
  size_t index = 0;

  while (index < buffer.size()) {
    ssize_t bytes = read(fd, &buffer[index], 1);
    if (bytes <= 0) {
      break;
    }
    if (buffer[7] & 0x01) {
      buffer[5] |= 0x80;
    }
    if (buffer[7] & 0x02) {
      buffer[6] |= 0x80;
    }

    readBytes += bytes;
    ++index;
  }

  uint16_t newChkSum = 0;
  // Print populated buffer
  log(Log::READ, "BYTE:             ADDRESS:");
  for (size_t i = 0; i < buffer.size() - 2; ++i) {
    newChkSum += buffer[i];
    log(Log::READ, "Byte:             " + hexByte(buffer[i]));
  }
  newChkSum = (((newChkSum << 1) & 0xFF00) | (newChkSum & 0x7F)) | 0xC000;

  log(Log::SERIAL, "Total bytes read: " + std::to_string(readBytes));

  uint8_t resId = buffer[0];
  uint8_t resMa = buffer[1];
  uint8_t resIogc = buffer[2];
  uint8_t resCcode = buffer[3];
  uint8_t resResult = buffer[4];
  uint16_t resWeight = (buffer[5] << 8) | buffer[6] | (buffer[7] & 0x80) << 1;

  buffer[8] = (newChkSum & 0xFF);
  buffer[9] = ((newChkSum >> 8) & 0xFF);

  float actualWeight = resWeight / 10.0f;

  log(Log::RESPONSE, "FRAME");
  log(Log::READ, "Protocol ID:      " + hexByte(resId));
  log(Log::READ, "Machine Address:  " + hexByte(resMa));
  log(Log::READ, "IO Group Code:    " + hexByte(resIogc));
  log(Log::READ, "Command Code:     " + hexByte(resCcode));
  log(Log::READ, "Action result:    " + hexByte(resResult));
  log(Log::READ, "Weight:           " + std::to_string(actualWeight));
  log(Log::READ, "Checksum 1:       " + std::to_string(buffer[8]));
  log(Log::READ, "Checksum 2:       " + std::to_string(buffer[9]));

  buffer.clear();

  return 0;
}

const char *logStr(Log log) {
  switch (log) {
  case Log::WRITE:
    return "WRITE";
  case Log::READ:
    return "READ";
  case Log::SERIAL:
    return "SERIAL";
  case Log::PORT:
    return "PORT";
  case Log::ERROR:
    return "ERROR";
  case Log::INFO:
    return "INFO";
  case Log::VALID:
    return "VALID";
  case Log::INVALID:
    return "INVALID";
  case Log::RESPONSE:
    return "RESPONSE";
  case Log::REQUEST:
    return "REQUEST";
  default:
    return "LOG";
  }
}

void log(Log log, const std::string &msg) {

  std::ostringstream ts;

  std::cout << "[" << logStr(log) << "] " << msg << "\n";
}

std::string hexByte(uint8_t b) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
      << static_cast<int>(b) << std::dec;
  return oss.str();
}

int openPort() {

  // Open port before configuration
  int fd = open(port, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    log(Log::PORT, "Port opening failed");
    return -1;
  }
  struct termios pts;

  if (tcgetattr(fd, &pts) != 0) {
    log(Log::PORT, "Existing settings not read");
    return -1;
  }
  // Terminal confiuration instance
  configureSerial(pts, 2400);

  if (tcsetattr(fd, TCSANOW, &pts) != 0) {
    log(Log::PORT, "Saving new setting not successful");
    return -1;
  }

  return fd;
}

void configureSerial(termios &settings, int baud) {
  // Control modes (how the data is packed)
  // Bit clearing (off)
  settings.c_cflag &= ~(PARENB | CSIZE | CRTSCTS);

  // Bit setting (on)
  settings.c_cflag |= (CS8 | CREAD | CLOCAL);

  // Local modes (how data is interpreted locally)
  // Clear local modes
  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

  // Input modes
  // Clear special handling of bytes
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  settings.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  // Output modes
  // Clear special handling of bytes
  settings.c_oflag &= ~(OPOST | ONLCR);

  settings.c_cc[VTIME] = 10;
  settings.c_cc[VMIN] = 0;

  cfsetispeed(&settings, baud);
  cfsetospeed(&settings, baud);
}
