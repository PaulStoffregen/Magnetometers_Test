#include "FXOS8700.h"
#include <Wire.h>

#define FXOS8700_I2C_ADDR          0x1E // SA1 = Gnd, SA0 = Gnd
//#define FXOS8700_I2C_ADDR         0x1D // SA1 = Gnd, SA0 = Vcc
//#define FXOS8700_I2C_ADDR         0x1C // SA1 = Vcc, SA0 = Gnd
//#define FXOS8700_I2C_ADDR         0x1F // SA1 = Vcc, SA0 = Vcc

#define FXOS8700_WHO_AM_I          0x0D // Device ID
#define FXOS8700_CTRL_REG1         0x2A // System ODR, accelerometer OSR, operating mode
#define FXOS8700_M_DR_STATUS       0x32 // Magnetic data ready
#define FXOS8700_M_OUT_X_MSB       0x33 // MSB of 16-bit magnetic data for X-axis
#define FXOS8700_M_OUT_X_LSB       0x34 // LSB of 16-bit magnetic data for X-axis
#define FXOS8700_M_OUT_Y_MSB       0x35 // MSB of 16-bit magnetic data for Y-axis
#define FXOS8700_M_OUT_Y_LSB       0x36 // LSB of 16-bit magnetic data for Y-axis
#define FXOS8700_M_OUT_Z_MSB       0x37 // MSB of 16-bit magnetic data for Z-axis
#define FXOS8700_M_OUT_Z_LSB       0x38 // LSB of 16-bit magnetic data for Z-axis

#define FXOS8700_M_CTRL_REG1       0x5B // Control for magnetic sensor functions
#define FXOS8700_M_CTRL_REG2       0x5C // Control for magnetic sensor functions
#define FXOS8700_M_CTRL_REG3       0x5D // Control for magnetic sensor functions


// Wire.h read and write protocols
static void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire.beginTransmission(address);
	Wire.write(subAddress);
	Wire.write(data);
	Wire.endTransmission();
}

static uint8_t readByte(uint8_t address, uint8_t subAddress)
{
	Wire.beginTransmission(address);
	Wire.write(subAddress);
	Wire.endTransmission(false);
	Wire.requestFrom(address, (uint8_t)1);
	return Wire.read();
}

static void readBytes(uint8_t address, uint8_t subAddress, uint8_t *dest, int count)
{  
	Wire.beginTransmission(address);   // Initialize the Tx buffer
	Wire.write(subAddress);            // Put slave register address in Tx buffer
	Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
	int i = 0;
	Wire.requestFrom((int)address, count);  // Read bytes from slave register address 
	while (Wire.available() && i < count) {
  		dest[i++] = Wire.read();
	}
}


void readFXOS8700mag(float *data)
{
	unsigned char temp[7];
#if 1
        readBytes(FXOS8700_I2C_ADDR, FXOS8700_M_DR_STATUS, temp, 7);
#else
        temp[0] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_DR_STATUS);
        temp[1] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_X_MSB);
        temp[2] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_X_LSB);
        temp[3] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_Y_MSB);
        temp[4] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_Y_LSB);
        temp[5] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_Z_MSB);
        temp[6] = readByte(FXOS8700_I2C_ADDR, FXOS8700_M_OUT_Z_LSB);
#endif
	float factor = 1 / 10.0;
	int16_t x = (int16_t)((temp[1] << 8) | temp[2]);
	int16_t y = (int16_t)((temp[3] << 8) | temp[4]);
	int16_t z = (int16_t)((temp[5] << 8) | temp[6]);
	//Serial.printf(" %02X%02X %02X%02X %02X%02X ",
		//temp[1], temp[2], temp[3], temp[4], temp[5], temp[6]);
	data[0] = (float)x * factor;
	data[1] = (float)y * factor;
	data[2] = (float)z * factor;
}

bool initFXOS8700()
{
	Wire.begin();

	int id = readByte(FXOS8700_I2C_ADDR, FXOS8700_WHO_AM_I);
	Serial.printf("id = %02X\n", id);
	if (id != 0xC7) return false;

	// place into standby mode
	writeByte(FXOS8700_I2C_ADDR, FXOS8700_CTRL_REG1, 0);
	// configure magnetometer
	writeByte(FXOS8700_I2C_ADDR, FXOS8700_M_CTRL_REG1, 0x1F);
	writeByte(FXOS8700_I2C_ADDR, FXOS8700_M_CTRL_REG2, 0x20);
	// configure accelerometer
	writeByte(FXOS8700_I2C_ADDR, FXOS8700_CTRL_REG1, 0x0D);
	Serial.println("FXOS8700CConfigured");

	return true;
}

