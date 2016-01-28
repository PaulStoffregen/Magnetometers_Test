// Simple Magnetometers Test...
// Read 4 different sensors and see if they give similar results

#include <Wire.h>
#include <SPI.h>
#include <ILI9341_t3.h>
#include <font_Arial.h>
#include "FXOS8700.h"
#include "LSM9DS0.h"
#include "MPU9250.h"
#include "BNO055.h"

#define TFT_DC  9
#define TFT_CS 10
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);


const char *fmt(float num)
{
  static char buf[16];
  sprintf(buf, "%7.2f", num);
  return buf;
}


void setup(void)
{
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) ;
  Serial.println("Magnetic Test");

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);

  Serial.println("Begin BNO055");
  if (!initBNO055()) {
    Serial.println("Ooops, no BNO055 detected");
    while(1);
  }

  Serial.println("Begin LSM9DS0");
  if (!initLSM9DS0()) {
    Serial.println("Ooops, no LSM9DS0 detected");
    while (1);
  }

  Serial.println("Begin AK8963 (MPU9250's compass)");
  if (!initAK8963()) {
    Serial.println("Ooops, no MPU9250/AK8963 detected");
    while (1);
  }

  Serial.println("Begin FXOS8700");
  if (!initFXOS8700()) {
    Serial.println("Ooops, no FXOS8700 detected");
    while (1);
  }

  delay(1000);
}


void loop(void)
{
  float mag[3];

  readFXOS8700mag(mag);
  Serial.print("FXOS8700: ");
  Serial.print(fmt(mag[0]));
  Serial.print(fmt(mag[1]));
  Serial.print(fmt(mag[2]));

  Serial.print("     ");
  readBNO055mag(mag);
  Serial.print("BNO055: ");
  Serial.print(fmt(mag[0]));
  Serial.print(fmt(mag[1]));
  Serial.print(fmt(mag[2]));

  Serial.print("     ");

  readAK8963mag(mag);
  Serial.print("MPU9250: ");
  Serial.print(fmt(mag[0]));
  Serial.print(fmt(mag[1]));
  Serial.print(fmt(mag[2]));

  Serial.print("     ");

  readLSM9DS0mag(mag);
  Serial.print("LSM9DS0: ");
  Serial.print(fmt(mag[0]));
  Serial.print(fmt(mag[1]));
  Serial.print(fmt(mag[2]));

  Serial.println("");
  delay(100);
}


/*

                Def     Alt
                Addr    Addr

LSM9DS0   A+M   0x1D    0x1E
          Gryo  0x6B    0x6A

BNO055    9DOF  0x29    0x28

MPU9250   A+G   0x68    0x69
AK8963     M    0x0C

FXOS8700  A+M   0x1E
FXAS21002  G    0x20
MPL3115A2 Alt   0x60


*/
