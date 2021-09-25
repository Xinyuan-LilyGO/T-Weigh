#include "HX711.h" //https://github.com/bogde/HX711
#include "pin.h"

#define GapValue 2208 //The calibration value

HX711 scale;

const char channel[4] = {0x0F, 0X0B, 0X09, 0X0D};
static long Weight_Maopi;

void SelectChannel(uint8_t ch); //Select the channel 0-3
void CalibrationFunction(void);

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello Weight!");
  SelectChannel(0); //Select the channel 0
  CalibrationFunction();
}

void loop()
{
  float Weight;
  if (scale.is_ready())
  {
    long reading = scale.read();
    reading = reading - Weight_Maopi;
    Weight = (long)((float)reading / GapValue);
    Serial.printf("Weight : %0.3fkg\r\n", float(Weight / 1000), 3);
  }
}

void CalibrationFunction(void)
{
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  while (!scale.is_ready())
    ;

  Weight_Maopi = scale.read();
}

void SelectChannel(uint8_t ch)
{
  pinMode(CDA, OUTPUT);
  pinMode(CDB, OUTPUT);
  pinMode(CDC, OUTPUT);
  pinMode(CDD, OUTPUT);

  digitalWrite(CDA, channel[ch] & 0x08);
  digitalWrite(CDB, channel[ch] & 0x04);
  digitalWrite(CDC, channel[ch] & 0x02);
  digitalWrite(CDD, channel[ch] & 0x01);
}