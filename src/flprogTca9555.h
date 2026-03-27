#pragma once
#include "flprogUtilites.h"

#define FLPROG_TCA9555_INPUT_PORT_REGISTER_0 0x00 //  read()
#define FLPROG_TCA9555_INPUT_PORT_REGISTER_1 0x01
#define FLPROG_TCA9555_OUTPUT_PORT_REGISTER_0 0x02 //  write()
#define FLPROG_TCA9555_OUTPUT_PORT_REGISTER_1 0x03
#define FLPROG_TCA9555_POLARITY_REGISTER_0 0x04 //  get/setPolarity()
#define FLPROG_TCA9555_POLARITY_REGISTER_1 0x05
#define FLPROG_TCA9555_CONFIGURATION_PORT_0 0x06 //  pinMode()
#define FLPROG_TCA9555_CONFIGURATION_PORT_1 0x07

class FLProgTca9555 : public AbstractI2CDevice
{
public:
  FLProgTca9555(uint8_t address, uint8_t bus);
  FLProgTca9555(uint8_t address, uint8_t bus, uint8_t expander, uint8_t channel);
  void pinMode(uint8_t pin, uint8_t mode);
  void write(uint8_t pin, uint8_t value);
  bool read(uint8_t pin);
  void setReqestPerion(uint32_t period);
  uint32_t getReqestPerion() { return _reqestPeriod; }

protected:
  virtual void init();
  void privatePinMode(uint8_t pin, uint8_t mode);
  bool canReqestInputs();
  virtual void workPool();

  uint8_t _modes[16] = {INPUT};
  bool _values[16] = {false};
  bool _isNeedSend = false;
  bool _hasInputs = false;
  uint32_t _reqestPeriod = 0;
  uint32_t _lastRequestTime = 0;
};