#include <RadioLib.h> //https://github.com/jgromes/RadioLib
#include "pin.h"

const long frequency = 868E6; // LoRa Frequency
SPIClass spi;
SX1262 radio = nullptr;

void setup()
{
  pinMode(NRSET, OUTPUT);
  digitalWrite(NRSET, 1);
  Serial.begin(115200);
  Serial.println("Hello Radio!");
  Serial.print(F("[SX1278] Initializing ... "));
  spi.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, csPin);
  radio = new Module(csPin, DIO1, NRSET, BUSY, spi);
  int state = radio.begin(868.0);
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}

void loop()
{
  Serial.print(F("[SX1262] Transmitting packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long
  // NOTE: transmit() is a blocking method!
  //       See example SX126x_Transmit_Interrupt for details
  //       on non-blocking transmission method.
  int state = radio.transmit("Hello World!");

  // you can also transmit byte array up to 256 bytes long
  /*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
    int state = radio.transmit(byteArr, 8);
  */

  if (state == ERR_NONE) {
    // the packet was successfully transmitted
    Serial.println(F("success!"));

    // print measured data rate
    Serial.print(F("[SX1262] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));

  } else if (state == ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));

  } else if (state == ERR_TX_TIMEOUT) {
    // timeout occured while transmitting packet
    Serial.println(F("timeout!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);

  }

  // wait for a second before transmitting again
  delay(1000);
}
