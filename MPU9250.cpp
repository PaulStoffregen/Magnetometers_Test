#include "MPU9250.h"
#include <Wire.h>

#define MPU9250_ADDRESS 0x68  // Device address when ADO = 0
//#define MPU9250_ADDRESS 0x69  // Device address when ADO = 1

#define AK8963_ADDRESS   0x0C
#define WHO_AM_I_AK8963  0x00 // should return 0x48

#define AK8963_ST1       0x02  // data ready status bit 0
#define AK8963_XOUT_L    0x03  // data
#define AK8963_ST2       0x09  // status2
#define AK8963_CNTL      0x0A
#define AK8963_ASAX      0x10
#define AK8963_ASAY      0x11
#define AK8963_ASAZ      0x12


#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define CONFIG           0x1A
#define SMPLRT_DIV       0x19
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define WHO_AM_I_MPU9250 0x75 // Should return 0x71



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
	uint8_t i = 0;
	Wire.requestFrom(address, count);  // Read bytes from slave register address 
	while (Wire.available() && i < count) {
  		dest[i++] = Wire.read();
	}
}


static float xscale, yscale, zscale;

void readAK8963mag(float *data)
{
	unsigned char temp[7];
#if 1
        readBytes(AK8963_ADDRESS, AK8963_XOUT_L, temp, 7);
#else
        temp[0] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 0);
        temp[1] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 1);
        temp[2] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 2);
        temp[3] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 3);
        temp[4] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 4);
        temp[5] = readByte(AK8963_ADDRESS, AK8963_XOUT_L + 5);
        temp[6] = readByte(AK8963_ADDRESS, AK8963_ST2);
#endif
	//float factor = 1 / 100.0;
	float factor = 4912.0 / 32760.0;
	int16_t x = (int16_t)((temp[1] << 8) | temp[0]);
	int16_t y = (int16_t)((temp[3] << 8) | temp[2]);
	int16_t z = (int16_t)((temp[5] << 8) | temp[4]);
	//Serial.printf(" %02X%02X %02X%02X %02X%02X ",
		//temp[1], temp[0], temp[3], temp[2], temp[5], temp[4]);
	data[0] = (float)x * factor * xscale;
	data[1] = (float)y * factor * yscale;
	data[2] = (float)z * factor * zscale;
}

bool initAK8963()
{
	Wire.begin();

	writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00); // Clear sleep bit (6), enable all
	delay(50);
	writeByte(MPU9250_ADDRESS, INT_PIN_CFG, 0x22);
	writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x01);
	delay(10);

	int id = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	Serial.printf("id = %02X\n", id);
	if (id != 0x71) return false;

	id = readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
	Serial.printf("id = %02X\n", id);
	if (id != 0x48) return false;

	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
 
	int x = readByte(AK8963_ADDRESS, AK8963_ASAX);
	int y = readByte(AK8963_ADDRESS, AK8963_ASAY);
	int z = readByte(AK8963_ADDRESS, AK8963_ASAZ);
	Serial.printf("sensitivity: %d, %d, %d\n", x, y, z);

	xscale = (float)(x - 128) * 0.5 / 128.0 + 1.0;
	yscale = (float)(y - 128) * 0.5 / 128.0 + 1.0;
	zscale = (float)(z - 128) * 0.5 / 128.0 + 1.0;

	Serial.printf("sensitivity: %.2f, %.2f, %.2f\n", xscale, yscale, zscale);

	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  

	// Continuous measurement mode 1
	// 16-bit output
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x12);
	return true;
}

