#include "BNO055.h"
#include <Wire.h>

#define BNO055_ADDRESS			0x28
#define OPERATION_MODE_AMG		0X07
#define BNO055_OPR_MODE_ADDR		0X3D
#define BNO055_CHIP_ID_ADDR		0x00
#define BNO055_SYS_TRIGGER_ADDR		0X3F
#define BNO055_PWR_MODE_ADDR		0X3E
#define BNO055_PAGE_ID_ADDR		0X07
#define POWER_MODE_NORMAL		0X00
#define BNO055_ID        (0xA0)
#define OPERATION_MODE_CONFIG		0X00
#define BNO055_MAG_DATA_X_LSB_ADDR	0X0E



static uint8_t read8(int reg)
{
  Wire.beginTransmission(BNO055_ADDRESS);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(BNO055_ADDRESS, 1);
  return Wire.read();
}

static void readLen(int reg, uint8_t * buffer, uint8_t len)
{
  Wire.beginTransmission(BNO055_ADDRESS);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(BNO055_ADDRESS, len);
  for (uint8_t i = 0; i < len; i++) {
    buffer[i] = Wire.read();
  }
}

static void write8(int reg, byte value)
{
  Wire.beginTransmission(BNO055_ADDRESS);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

static void setMode(int mode)
{
  write8(BNO055_OPR_MODE_ADDR, mode);
  delay(30);
}


bool initBNO055()
{
  Wire.begin();
  uint8_t id = read8(BNO055_CHIP_ID_ADDR);
  if (id != BNO055_ID) return false;

  setMode(OPERATION_MODE_CONFIG);
  write8(BNO055_SYS_TRIGGER_ADDR, 0x20);
  while (read8(BNO055_CHIP_ID_ADDR) != BNO055_ID) {
    delay(10);
  }
  delay(50);

  write8(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
  delay(10);
  write8(BNO055_PAGE_ID_ADDR, 0);
  write8(BNO055_SYS_TRIGGER_ADDR, 0x0);
  delay(10);
  setMode(OPERATION_MODE_AMG);
  delay(20);
  return true;
}

void readBNO055mag(float *data)
{
  uint8_t buffer[6];

  readLen(BNO055_MAG_DATA_X_LSB_ADDR, buffer, 6);
  data[0] = (float)((int16_t)(buffer[0] | (buffer[1] << 8))) / 16.0f;
  data[1] = (float)((int16_t)(buffer[2] | (buffer[3] << 8))) / 16.0f;
  data[2] = (float)((int16_t)(buffer[4] | (buffer[5] << 8))) / 16.0f;
}
