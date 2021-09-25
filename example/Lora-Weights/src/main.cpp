#include <Arduino.h>
#include "HX711.h"
#include <pin.h>
#include <SPI.h>
#include <RadioLib.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "OneButton.h"
#include "WiFi.h"

#define GapValue 2208
#define WIFI_SSID "xinyuan-2"
#define WIFI_PASS "Xydz202104"

const long frequency = 868E6; // LoRa Frequency
static long Weight_Maopi[4];
bool isWIFItestDone = false;
SPIClass spi;
SX1262 radio = nullptr;
HX711 scale;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/IIC_SCL, /* data=*/IIC_SDA, /* reset=*/U8X8_PIN_NONE);
OneButton button(BTN0, true);

void testClient(const char *host, uint16_t port);
void BTN_task(void *pvParameter);
void WIFItest_task(void *pvParameter);

void SelectChannel(uint8_t channel)
{
  pinMode(CDA, OUTPUT);
  pinMode(CDB, OUTPUT);
  pinMode(CDC, OUTPUT);
  pinMode(CDD, OUTPUT);
  switch (channel)
  {
  case 0:
    digitalWrite(CDA, 1);
    digitalWrite(CDB, 1);
    digitalWrite(CDC, 1);
    digitalWrite(CDD, 1);
    break;
  case 1:
    digitalWrite(CDA, 1);
    digitalWrite(CDB, 0);
    digitalWrite(CDC, 1);
    digitalWrite(CDD, 1);
    break;
  case 2:
    digitalWrite(CDA, 1);
    digitalWrite(CDB, 0);
    digitalWrite(CDC, 0);
    digitalWrite(CDD, 1);
    break;
  case 3:
    digitalWrite(CDA, 1);
    digitalWrite(CDB, 1);
    digitalWrite(CDC, 0);
    digitalWrite(CDD, 1);
    break;
  default:
    break;
  }
}

void CalibrationFunction(void)
{
  uint8_t count = 0;
  while (count < 4)
  {
    do
    {
      SelectChannel(count);
      scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
      Weight_Maopi[count] = scale.read();
    } while (scale.is_ready());
    delay(200);
    count++;
  }
}

void setup()
{
  pinMode(NRSET, OUTPUT);
  digitalWrite(NRSET, 1);
  delay(1000);
  Serial.begin(115200);
  Serial.println("LORA Weight");
  //SPISettings spiSettings;
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

  CalibrationFunction();
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  button.attachClick(CalibrationFunction);

  xTaskCreate(BTN_task, "Task1", 10000, NULL, 2, NULL);
  xTaskCreate(WIFItest_task, "Task2", 10000, NULL, 1, NULL);
}

void loop()
{
  float Weight[4];
  float TotalWeight;
  static uint8_t count;
  char str[25];

  TotalWeight = 0;
  count = 0;
  while (count < 4)
  {
    SelectChannel(count);
    if (scale.is_ready())
    {
      long reading = scale.read();
      reading = reading - Weight_Maopi[count];
      Weight[count] = (long)((float)reading / GapValue);
      if (isWIFItestDone)
        Serial.printf("[%d] Weight : %0.3fkg\r\n", count, float(Weight[count] / 1000), 3); //串口显示重量
      TotalWeight += Weight[count];
      count++;
    }
  }

  u8g2.clearBuffer();
  sprintf(str, "[1] Weight : %0.3f kg", float(Weight[0] / 1000));
  u8g2.drawStr(0, 10, str);
  sprintf(str, "[2] Weight : %0.3f kg", float(Weight[1] / 1000));
  u8g2.drawStr(0, 22, str);
  sprintf(str, "[3] Weight : %0.3f kg", float(Weight[2] / 1000));
  u8g2.drawStr(0, 34, str);
  sprintf(str, "[4] Weight : %0.3f kg", float(Weight[3] / 1000));
  u8g2.drawStr(0, 46, str);
  sprintf(str, "Total Weight : %0.3f kg", float(TotalWeight / 1000));
  u8g2.drawStr(0, 58, str);
  u8g2.sendBuffer();

  if (isWIFItestDone)
    Serial.println(str);
  radio.transmit(str);

  //delay(100);
}

void BTN_task(void *pvParameter)
{
  while (1)
  {
    button.tick();
    vTaskDelay(5);
  }
  vTaskDelete(NULL);
}

void WIFItest_task(void *pvParameter)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      vTaskDelay(10);
    }
  }
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(500);
  }

  testClient("baidu.com", 80);
  isWIFItestDone = true;
  vTaskDelete(NULL);
}

void testClient(const char *host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}