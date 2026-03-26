#include "flprogTca9555.h"

FLProgTca9555::FLProgTca9555(uint8_t address, uint8_t bus)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, RT_HW_I2C_SPEED);
}

FLProgTca9555::FLProgTca9555(uint8_t address, uint8_t bus, uint8_t expander, uint8_t channel)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, RT_HW_I2C_SPEED, expander, channel);
}

void FLProgTca9555::pool()
{
  if (_status == FLPROG_NOT_REDY_STATUS)
  {
    RT_HW_Base.i2cInitDevice(_device);
    if (!_device.link)
    {
      return;
    }
    if (_device.status == 1)
    {
      _status = FLPROG_WAIT_I2C_FIND_ADDRESS;
      return;
    }
    else
    {
      return;
    }
  }
  if (_status == FLPROG_WAIT_I2C_FIND_ADDRESS)
  {
    RT_HW_Base.i2cFindAdr(_device);
    if (!_device.link)
    {
      return;
    }
    if (_device.codeErr)
    {
      _status = FLPROG_WAIT_I2C_REFIND_ADDRES_PAUSE;
      _pauseStartTime = millis();
      return;
    }
    else
    {
      _status = FLPROG_WAIT_I2C_DEVICE_INIT;
      return;
    }
  }

  if (_status == FLPROG_WAIT_I2C_REFIND_ADDRES_PAUSE)
  {
    if (flprog::isTimer(_pauseStartTime, 1000))
    {
      _status = FLPROG_WAIT_I2C_FIND_ADDRESS;
    }
    else
    {
      return;
    }
  }
  if (_status == FLPROG_WAIT_I2C_DEVICE_INIT)
  {
    init();
    return;
  }
  updateData();
  if (_device.codeErr)
  {
    _status = FLPROG_NOT_REDY_STATUS;
    return;
  }
}

uint8_t FLProgTca9555::readRegister(uint8_t reg)
{
  RT_HW_Base.i2cWrite(_device, reg);
  RT_HW_Base.i2cRead(_device);
  return _device.bf8;
}

void FLProgTca9555::writeRegister(uint8_t reg, uint8_t value)
{
  uint8_t data[2];
  data[0] = reg;
  data[1] = value;
  RT_HW_Base.i2cWriteArr(_device, data, 2);
}

void FLProgTca9555::init()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    privatePinMode(i, _modes[i]);
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

bool FLProgTca9555::hasInputs()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    if (_modes[i] == INPUT)
    {
      return true;
    }
  }
  return false;
}

bool FLProgTca9555::hasOutputs()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    if (_modes[i] == OUTPUT)
    {
      return true;
    }
  }
  return false;
}

void FLProgTca9555::updateData()
{
  uint8_t inputReg0 = 0;
  uint8_t inputReg1 = 0;
  uint8_t outReg0 = 0;
  uint8_t outRreg1 = 0;
  if (hasInputs())
  {
    inputReg0 = readRegister(FLPROG_TCA9555_INPUT_PORT_REGISTER_0);
    inputReg1 = readRegister(FLPROG_TCA9555_INPUT_PORT_REGISTER_1);
  }
  if (hasOutputs())
  {
    outReg0 = readRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_0);
    outRreg1 = readRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_1);
  }
  uint8_t oldOutReg0 = outReg0;
  uint8_t oldOutRreg1 = outRreg1;
  for (uint8_t i = 0; i < 16; i++)
  {
    uint8_t inputRreg = inputReg0;
    uint8_t pin = i;
    if (i > 7)
    {
      inputRreg = inputReg1;
      pin -= 8;
    }
    uint8_t mask = 1 << pin;
    if (_modes[i] == INPUT)
    {
      _values[i] = (inputRreg & mask) != 0;
    }
    else
    {
      if (i > 7)
      {
        if (_values[i])
        {
          outRreg1 |= mask;
        }
        else
        {
          outRreg1 &= ~mask;
        }
      }
      else
      {
        if (_values[i])
        {
          outReg0 |= mask;
        }
        else
        {
          outReg0 &= ~mask;
        }
      }
    }
  }
  if (oldOutReg0 != outReg0)
  {
    writeRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_0, outReg0);
  }
  if (oldOutRreg1 != outRreg1)
  {
    writeRegister(FLPROG_TCA9555_OUTPUT_PORT_REGISTER_1, outRreg1);
  }
}