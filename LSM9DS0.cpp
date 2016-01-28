#include "LSM9DS0.h"
#include <Wire.h>

#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW

#define WHO_AM_I_XM                     0x0F
#define OUT_X_L_M                       0x08
#define CTRL_REG5_XM            0x24
#define CTRL_REG6_XM            0x25
#define CTRL_REG7_XM            0x26

static void I2CwriteByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire.beginTransmission(address);
	Wire.write(subAddress);
	Wire.write(data);
	Wire.endTransmission();
}

static uint8_t I2CreadByte(uint8_t address, uint8_t subAddress)
{
	Wire.beginTransmission(address);
	Wire.write(subAddress);
	Wire.endTransmission(false);
	Wire.requestFrom(address, (uint8_t)1);
	return Wire.read();
}

static void I2CreadBytes(uint8_t address, uint8_t subAddress, uint8_t *dest, int count)
{
        Wire.beginTransmission(address);
        Wire.write(subAddress | 0x80);
        Wire.endTransmission(false);
        Wire.requestFrom(address, count);
	int i = 0;
	while (Wire.available() && i < count) {
		dest[i++] = Wire.read(); 
	}
}

static void xmWriteByte(uint8_t subAddress, uint8_t data)
{
	I2CwriteByte(LSM9DS0_XM, subAddress, data);
}

static uint8_t xmReadByte(uint8_t subAddress)
{
	return I2CreadByte(LSM9DS0_XM, subAddress);
}

static void xmReadBytes(uint8_t subAddress, uint8_t * dest, uint8_t count)
{
	I2CreadBytes(LSM9DS0_XM, subAddress, dest, count);
}



bool initLSM9DS0()
{
	Wire.begin();

	if (xmReadByte(WHO_AM_I_XM) != 0x49) return false;

	//xmWriteByte(CTRL_REG5_XM, 0x08); // low res, 12.5 Hz rate
	xmWriteByte(CTRL_REG5_XM, 0x68); // high res, 12.5 Hz rate
	xmWriteByte(CTRL_REG6_XM, 0x00); // Mag scale to +/- 2GS
	xmWriteByte(CTRL_REG7_XM, 0x00); // Continuous conversion mode
	return true;
}

void readLSM9DS0mag(float *data)
{
	uint8_t temp[6];

#if 1
	xmReadBytes(OUT_X_L_M, temp, 6);
#else
	temp[0] = xmReadByte(OUT_X_L_M + 0);
	temp[1] = xmReadByte(OUT_X_L_M + 1);
	temp[2] = xmReadByte(OUT_X_L_M + 2);
	temp[3] = xmReadByte(OUT_X_L_M + 3);
	temp[4] = xmReadByte(OUT_X_L_M + 4);
	temp[5] = xmReadByte(OUT_X_L_M + 5);
#endif
	float factor = (2.0 / 32768.0 * 100.0); // output in uT units
	//factor *= -1.0; // why negative?
	int16_t x = (int16_t)((temp[1] << 8) | temp[0]);
	int16_t y = (int16_t)((temp[3] << 8) | temp[2]);
	int16_t z = (int16_t)((temp[5] << 8) | temp[4]);
	//Serial.printf(" %02X%02X %02X%02X %02X%02X ",
		//temp[1], temp[0], temp[3], temp[2], temp[5], temp[4]);
	data[0] = (float)x * factor;
	data[1] = (float)y * factor;
	data[2] = (float)z * factor;
}

