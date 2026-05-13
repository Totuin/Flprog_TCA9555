#include "flprogTca9555.h"

FLProgTca9555::FLProgTca9555(uint8_t address, uint8_t bus)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, RT_HW_I2C_SPEED);
}

FLProgTca9555::FLProgTca9555(uint8_t address, uint8_t bus, uint8_t expander, uint8_t channel)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, 800000, expander, channel);
}

void FLProgTca9555::setReqestPerion(uint32_t period)
{
  if (_reqestPeriod == period)
  {
    return;
  }
  _lastRequestTime = millis();
  _reqestPeriod = period;
}

void FLProgTca9555::init()
{
  _isNeedSend = false;
  _hasInputs = false;
  for (uint8_t i = 0; i < 16; i++)
  {
    privatePinMode(i, _modes[i]);
    if (_modes[i] == OUTPUT)
    {
      _isNeedSend = true;
    }
    if (_modes[i] == INPUT)
    {
      _hasInputs = true;
    }
  }
  _status = FLPROG_READY_STATUS;
}

void FLProgTca9555::pinMode(uint8_t pin, uint8_t mode)
{
  if (pin > 15)
  {
    return;
  }
  if ((mode != INPUT) && (mode != OUTPUT))
  {
    return;
  }
  _modes[pin] = mode;
  if (_status == FLPROG_READY_STATUS)
  {
    _status = FLPROG_WAIT_I2C_DEVICE_INIT;
  }
}

void FLProgTca9555::privatePinMode(uint8_t pin, uint8_t mode)
{
  uint8_t configReg = FLPROG_TCA9555_CONFIGURATION_PORT_0;
  if (pin > 7)
  {
    configReg = FLPROG_TCA9555_CONFIGURATION_PORT_1;
    pin -= 8;
  }
  uint8_t val = readRegister(configReg);
  uint8_t prevVal = val;
  uint8_t mask = 1 << pin;
  if (mode == INPUT)
    val |= mask;
  else
    val &= ~mask;
  if (val != prevVal)
  {
    writeRegister(configReg, val);
  }
}

void FLProgTca9555::write(uint8_t pin, uint8_t value)
{
  if (pin > 15)
  {
    return;
  }
  if (_values[pin] == value)
  {
    return;
  }
  _isNeedSend = true;
  _values[pin] = value;
}

bool FLProgTca9555::read(uint8_t pin)
{
  if (pin > 15)
  {
    return false;
  }
  return _values[pin];
}

bool FLProgTca9555::canReqestInputs()
{
  if (!_hasInputs)
  {
    return false;
  }
  if (_reqestPeriod == 0)
  {
    return true;
  }
  if (flprog::isTimer(_lastRequestTime, _reqestPeriod))
  {
    _lastRequestTime = millis();
    return true;
  }
  return false;
}

void FLProgTca9555::workPool()
{
  if (canReqestInputs())
  {
    uint8_t inRreg = readRegister(FLPROG_TCA9555_INPUT_PORT_REGISTER_0);
    uint8_t inRreg1 = readRegister(FLPROG_TCA9555_INPUT_PORT_REGISTER_1);
    for (uint8_t i = 0; i < 16; i++)
    {
      uint8_t inPin = i;
      if (inPin > 7)
      {
        inPin -= 8;
      }
      if (_modes[i] == INPUT)
      {
        uint8_t mask = 1 << inPin;
        if (i > 7)
        {
          _values[i] = (inRreg1 & mask) != 0;
        }
        else
        {
          _values[i] = (inRreg & mask) != 0;
        }
      }
    }
  }
  if (_isNeedSend)
  {
    _isNeedSend = false;
    uint8_t reg0 = readRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_0);
    uint8_t reg1 = readRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_1);
    for (uint8_t i = 0; i < 16; i++)
    {
      if (_modes[i] == OUTPUT)
      {
        uint8_t pin = i;
        if (pin > 7)
        {
          pin -= 8;
        }
        uint8_t mask = 1 << pin;

        if (_values[i])
        {
          if (i > 7)
          {
            reg1 |= mask;
          }
          else
          {
            reg0 |= mask;
          }
        }
        else
        {
          if (i > 7)
          {
            reg1 &= ~mask;
          }
          else
          {
            reg0 &= ~mask;
          }
        }
      }
    }
    writeRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_0, reg0);
    writeRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_1, reg1);
  }
}