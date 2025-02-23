#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

int j=0, i=0, k=0;
bool FM_AM = HIGH; //HIGH is FM, LOW is MW
bool currentMode = HIGH; //HIGH is FM, LOW is MW
bool update = HIGH;

uint32_t freq=0,freqAM=0;
uint8_t fm_rssi=0,mw_rssi=0,fm_snr=0,mw_snr=0,sw_rssi=0,sw_snr=0;
float freqFM=0;
void setup() {
  Serial.begin(57600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(3);
  Wire.begin();
  pinMode(10,INPUT);pinMode(3,INPUT);
  FM_AM = digitalRead(10); //D10作为波段开关
  ChipInit();
  SetFM();
  UpdateDisplay();
}
void loop() {
  FM_AM = digitalRead(10);
  if (FM_AM == HIGH && currentMode == LOW){
    SetFM();
    currentMode = HIGH;
    display.setTextSize(1);
  }
  if (FM_AM == LOW && currentMode == HIGH){
    SetAM();
    currentMode = LOW;
    display.setTextSize(1);
  }
  update=digitalRead(3);
  if(update){
    GetFrequency(&freq);
    freqFM = (float)freq/20;
    freqAM = freq;
    GetFMrssi(&fm_rssi);
    GetFMsnr(&fm_snr);
    GetMWrssi(&mw_rssi);
    GetMWsnr(&mw_snr);
    UpdateDisplay();
    display.display();      
  }
  delay(1);
}
//Initialize KT0937-D8

bool ChipInit(){
  WakeUp();
  SetDepop();
  SetClock();
  SetRadio();
}
void SetStandby(){
  WriteRegister(0x10, 0x40);
  WriteRegister(0x76, 0xa4);
  WriteRegister(0x0e, 0x20);
}
void WakeUp(){
  WriteRegister(0x0e, 0x00);
  delay(2);
  WriteRegister(0x76, 0xa6);
}
void SetDepop(){ 
  WriteRegister(0x4E, 0x32);
}
void SetClock(){ //Use 32.768kHz Crystal
  WriteRegister(0x04, 0x00);
  WriteRegister(0x05, 0x01);
  WriteRegister(0x06, 0x02);
  WriteRegister(0x07, 0x9c);
  WriteRegister(0x08, 0x08);
  WriteRegister(0x09, 0x00);
  WriteRegister(0x0a, 0x00);
  WriteRegister(0x0d, 0xc3);
  WriteRegister(0x04, 0x80);
}
void SetRadio(){
  WriteRegister(0x62, 0x41);
  WriteRegister(0x2f, 0x25);
  WriteRegister(0x2a, 0xc0);
  WriteRegister(0x69, 0x8a);
  WriteRegister(0x0f, 0x1f);
}
//Set FM Band.
void SetFM(){
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
  WriteRegister(0x2b, 0x20);//Force mono, 75us De-emphasis, Blend enable
}
void SetAM(){
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
  *frequency = (high_byte << 8) | low_byte;
}

void GetFMrssi(uint8_t *rssi){
  uint8_t fm_rssi;
  ReadRegister(0xe6, &fm_rssi);
  *rssi = fm_rssi -3;
}

void GetFMsnr(uint8_t *snr){
  ReadRegister(0xe2, snr);
}

void GetMWrssi(uint8_t *rssi){
  uint8_t mw_rssi;
  ReadRegister(0xea, &mw_rssi);
  *rssi = mw_rssi -3;
}

void GetMWsnr(uint8_t *snr){
  ReadRegister(0xec, snr);
}

bool CheckStereo(){
  uint8_t i=0;
  ReadRegister(0xde, &i);
  i = i & 0x01;
  return i;
}
void UpdateDisplay(){
  display.clearDisplay();
  if(FM_AM == HIGH){
    display.setCursor(0, 60);
    display.setTextSize(2);
    String str = String(freqFM);
    str.remove(str.length()-1);
    display.print(str);
    display.setCursor(45, 80);
    display.setTextSize(1);
    display.print("MHz   ");
    display.setCursor(0, 44);
    display.print("FM");
    uint16_t fm_signal=fm_rssi * fm_snr;//the fm_signal will very big between the strong and weak channels.
    display.setCursor(0, 0);
    display.fillRect(0, 0, (fm_signal/27), 6, WHITE);
    if(CheckStereo()){
      display.setCursor(26,20);
      display.print("STEREO");
    }
    else{
      display.setCursor(38,20);
      display.print("MONO");
    }
  }
  else{
    display.setCursor(0, 60);
    display.setTextSize(2);
    String str = String(freqAM);
    display.print(str);
    display.setCursor(45, 80);
    display.setTextSize(1);
    display.print("KHz   ");
    display.setCursor(0, 44);
    display.print("MW");
    uint16_t mw_signal=mw_rssi * mw_snr;
    display.setCursor(0, 0);
    display.fillRect(0, 0, (mw_signal/47), 6, WHITE);
  }
}


