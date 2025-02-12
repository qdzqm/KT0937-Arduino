#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

int j=0, i=0, k=0;
bool FM_AM = 0; //0 is FM, 1 is AM
void setup() {
  Serial.begin(57600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  Wire.begin();
}

void loop() {
  k = analogRead(A0);
  if (k>800)
    setFM();
  else
    setAM();
  i = analogRead(A1);
  if(i > j+1 || i < j-1){
    uint32_t freq;
    float freqfm;
    GetFrequency(&freq);
    display.clearDisplay();
    display.setTextSize(2); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 12);
    freqfm = (float)freq/1000;
    display.println(freqfm);
    j=i;
    display.display();      // Show initial text
    Serial.print(millis()/1000);
    Serial.print("  freq: ");
    Serial.println(freq);
  }
  delay(1);
}
void setFM(){
  WriteRegister(0x04, 0x80);//Clock initialization completed.
  WriteRegister(0x51, 0x02);//Dial controlled channel increase/decrease
  WriteRegister(0x9b, 0xcd);//设置87.5MHz~108MHz之间分为205个通道，间隔0.1MHz
  WriteRegister(0x9a, 0x00);//设置87.5MHz~108MHz之间分为205个通道，间隔0.1MHz
  WriteRegister(0x88, 0x88);//可以改变band，FM模式，最高频率是108MHz
  WriteRegister(0x89, 0x70);//最高频率是108MHz
  WriteRegister(0x99, 0xd6);//最低频率是87.5MHz
  WriteRegister(0x98, 0x06);//最低频率是87.5MHz
  WriteRegister(0x71, 0x04);//Enable Channel ADC, Channel ADC Start
  WriteRegister(0x22, 0xd5);//Enable Tune Interrupt, INT is edge triggered, FM_SMUTE_START_SNR=default
  WriteRegister(0x1F, 0xd3);//INT interrupt is active high, FM_SMUTE_START_RSSI=default, FM_SMUTE_SLOPE_RSSI=default
}
void setAM(){
  //delay(10);WriteRegister(0x71, 0x80);//Disable ADC, all others set to default
  delay(10);WriteRegister(0x04, 0x80);//Clock initialization completed.
  delay(10);WriteRegister(0x51, 0x02);//Dial controlled channel increase/decrease
  delay(10);WriteRegister(0x88, 0xc8);//Change band, AM mode, High edge frequency of FM band set to 108MHz
  delay(10);WriteRegister(0x99, 0x0a);//MW最低频率522KHz
  delay(10);WriteRegister(0x98, 0x02);//MW最低频率522KHz
  delay(10);WriteRegister(0x8c, 0x06);//MW最高频率1620KHz
  delay(10);WriteRegister(0x8d, 0x54);//MW最高频率1620KHz
  delay(10);WriteRegister(0x9b, 0x4a);//通道间隔1620-522=1098
  delay(10);WriteRegister(0x9a, 0x04);//通道间隔1620-522=1098
  delay(10);WriteRegister(0x18, 0x59);//FM Band Space 100KHz, MW Band Space 9KHz
  //delay(10);WriteRegister(0xa0, 0x17);//Channel guard range in dial mode (default)
  delay(10);WriteRegister(0x71, 0x04);//Enable Channel ADC, Channel ADC Start
  delay(10);WriteRegister(0x22, 0xd5);//Enable Tune Interrupt, INT is edge triggered, FM_SMUTE_START_SNR=default
  delay(10);WriteRegister(0x1F, 0xd3);//INT interrupt is active high, FM_SMUTE_START_RSSI=default, FM_SMUTE_SLOPE_RSSI=default
}
void WriteRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x35);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void ReadRegister(uint8_t reg, uint8_t *value) {
  Wire.beginTransmission(0x35);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(0x35, 1);
  *value = Wire.read();
}

void GetFrequency(uint32_t *frequency) {
  uint8_t high_byte, low_byte;
  ReadRegister(0xe4, &high_byte);
  ReadRegister(0xe5, &low_byte);
  uint32_t channel = (high_byte << 8) | low_byte;
  *frequency = channel * 50;
}
