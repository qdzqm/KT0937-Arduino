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
bool FM_AM = HIGH; //HIGH is FM, LOW is MW
bool currentMode = HIGH; //HIGH is FM, LOW is MW
uint32_t freq=0,freqAM=0;
float freqFM=0;
void setup() {
  Serial.begin(57600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  Wire.begin();
  pinMode(10,INPUT);
  FM_AM = digitalRead(10); //D10作为波段开关
  setFM();
  delay(10);
  UpdateDisplay();
}
void loop() {
  FM_AM = digitalRead(10);

  if (FM_AM == HIGH && currentMode == LOW){
    setFM();
    currentMode = HIGH;
    display.setTextSize(1);
  }
  if (FM_AM == LOW && currentMode == HIGH){
    setAM();
    currentMode = LOW;
    display.setTextSize(1);
  }
  i = analogRead(A1); //A1连接到CH，旋转电位器会引起CH电压变化，进而导致频率变化
  if(i > j+1 || i < j-1){

    GetFrequency(&freq);
    freqFM = (float)freq/1000;
    freqAM = freq/50;
    UpdateDisplay();      
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
  WriteRegister(0x18, 0x59);//FM Band Space 100KHz, MW Band Space 9KHz
  WriteRegister(0x71, 0x04);//Enable Channel ADC, Channel ADC Start
  WriteRegister(0x22, 0xd5);//Enable Tune Interrupt, INT is edge triggered, FM_SMUTE_START_SNR=default
  WriteRegister(0x1F, 0xd3);//INT interrupt is active high, FM_SMUTE_START_RSSI=default, FM_SMUTE_SLOPE_RSSI=default
}
void setAM(){
  WriteRegister(0x04, 0x80);//Clock initialization completed.
  WriteRegister(0x51, 0x02);//Dial controlled channel increase/decrease
  WriteRegister(0x9b, 0x7a);//通道间隔(1620-522)/9=122
  WriteRegister(0x9a, 0x00);//通道间隔(1620-522)/9=122
  WriteRegister(0x88, 0xc8);//Change band, AM mode, High edge frequency of FM band set to 108MHz
  WriteRegister(0x99, 0x0a);//MW最低频率522KHz
  WriteRegister(0x98, 0x02);//MW最低频率522KHz
  WriteRegister(0x8c, 0x06);//MW最高频率1620KHz
  WriteRegister(0x8d, 0x54);//MW最高频率1620KHz
  WriteRegister(0x18, 0x59);//FM Band Space 100KHz, MW Band Space 9KHz
  WriteRegister(0x71, 0x04);//Enable Channel ADC, Channel ADC Start
  WriteRegister(0x22, 0xd5);//Enable Tune Interrupt, INT is edge triggered, FM_SMUTE_START_SNR=default
  WriteRegister(0x1F, 0xd3);//INT interrupt is active high, FM_SMUTE_START_RSSI=default, FM_SMUTE_SLOPE_RSSI=default
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

void UpdateDisplay(){
  display.clearDisplay();
  if(FM_AM == HIGH){
    display.setCursor(20, 12);
    display.setTextSize(2);
    display.print(freqFM);
    display.setTextSize(1);
    display.print(" MHz");
    display.setCursor(110, 22);
    display.print("FM");
  }
  else{
    display.setCursor(20, 12);
    display.setTextSize(2);
    display.print(freqAM);
    display.setTextSize(1);
    display.print(" KHz");
    display.setCursor(110, 22);
    display.print("MW");
  }
}
