//pinout: 
//    Výstupy:
//
//      Front Digital : D2 (4)
//      Rear Digital  : D4 (6)
//      Right Blinker : D9 (15)
//      Left Blinker  : D10 (16)
//      Rear PWM      : D11 (17)
//      Front High    : D12 (18)
//      Front Low     : D13 (19)     
//      
//    Vstupy:
//      Lights SW     : A0 (23)
//      High BTN      : A1 (24)
//      Brake BTN     : A2 (25)
//      Left SW       : A3 (26)
//      Right SW      : A4 (27)


#include <Adafruit_NeoPixel.h>
#define REAR_NUM_LEDS 16
#define FRONT_NUM_LEDS 16
#define Blinker_speed 600
//config
int ba_speed = 40; // disabled // lower / faster


// Inputs
int lights_SW = A0;
int high_BTN = A1;
int brake_BTN = A2;
int left_SW = A3;
int right_SW = A4;

// Outputs
int FD_OPin = 2; //Front Digital
int RD_OPin = 4; //Rear Digital
int RIGHT_OPin = 9; //Right Blinker
int LEFT_OPin = 10; //Left Blinker
int REAR_OPin = 11; //Rear PWM 
int FH_OPin = 12; //Front High
int FL_OPin = 13; //Front Low

// Stavy
int light_st = 0;
int front_high_st = 0;
int brake_st = 0;
int right_st = 0;
int left_st = 0;
int forcedHLoff = 0;

Adafruit_NeoPixel strip_rear = Adafruit_NeoPixel(REAR_NUM_LEDS, RD_OPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_front = Adafruit_NeoPixel(FRONT_NUM_LEDS, FD_OPin, NEO_GRB + NEO_KHZ800);
unsigned long PM_01 = 0; // Left LED Timmer
unsigned long PM_02 = 0; // Right LED Timmer
unsigned long PM_03 = 0; // SW to high timer
unsigned long PM_04 = 0; // force HL Off
unsigned long PM_05 = 0; // HL BTN HOLD
unsigned long PM_10 = 0; // FR
unsigned long PM_11 = 0; // FL

int RLI = 0; // rear light intensity
int FLI_r = 0; // RED front light intensity
int FLI_g = 0; // GREEN front light intensity
int FLI_b = 0; // BLUE front light intensity
int blinker_delay = 500; // 
int right_LED_on = 0;
int left_LED_on = 0;
int fr_i = 0;
int fl_i = 0;
int front_blinker_pixels = (FRONT_NUM_LEDS/3)-1; // (FRONT_NUM_LEDS/2); -> 1/2
int rear_blinker_pixels = (REAR_NUM_LEDS/3)-1; // (FRONT_NUM_LEDS/2); -> 1/2; (REAR_NUM_LEDS/3)-1; -> 1/3
int HL_BTNST = 0;int last_HL_BTNST = 0;int HL_pressed = 0;int HL_pulled = 0;

void setup() { 
  // begin  
  Serial.begin(9600);
  strip_rear.begin();
  strip_rear.show();
  strip_front.begin();
  strip_front.show();
  // inputs
  pinMode(lights_SW, INPUT);
  pinMode(high_BTN, INPUT);
  pinMode(brake_BTN, INPUT);
  pinMode(left_SW, INPUT);
  pinMode(right_SW, INPUT);
  // outputs  
  pinMode(FD_OPin, OUTPUT);
  pinMode(RD_OPin, OUTPUT);
  pinMode(RIGHT_OPin, OUTPUT);
  pinMode(LEFT_OPin, OUTPUT);
  pinMode(REAR_OPin, OUTPUT);
  pinMode(FH_OPin, OUTPUT);
  pinMode(FL_OPin, OUTPUT); 
}

void loop() {
  frontLights();
  setLightIntensity();
  handleBlinkers();
  showStrip();
}

void frontLights(){
  unsigned long CM = millis();
  HL_BTNST = digitalRead(high_BTN);
  HL_pressed = 0;
  HL_pulled = 0;
  if (HL_BTNST != last_HL_BTNST) {
    if (HL_BTNST == HIGH) {
      HL_pressed = 1;
      last_HL_BTNST = 1;
      Serial.println("HL_pressed");
    }
    else{
      HL_pressed = 0;
      HL_pulled = 1;
      last_HL_BTNST = 0;
      Serial.println("HL_pulled");
    }
  }
  ///////
  if(digitalRead(lights_SW)){
    light_st = 1;
    digitalWrite(FL_OPin, 1);
  }
  else{
    light_st = 0;
    digitalWrite(FL_OPin, 0);
  }
  if(light_st && HL_BTNST){
    if(CM - PM_05 > 2000){PM_05 = CM;
      if(!front_high_st){        
        Serial.println("HL_Switched_ON");
        front_high_st = 1;
        digitalWrite(FH_OPin, 1);
      }
      else{
        Serial.println("HL_Switched_Off");
        front_high_st = 0;
        forceHLoff();
        digitalWrite(FH_OPin, 0);
      }
    }
  }
 else{
    PM_05 = CM;
  }
  if(!light_st && !forcedHLoff){
    if(HL_BTNST){
      Serial.println("HL_ON");
      front_high_st = 1;
      digitalWrite(FH_OPin, 1);
    }
    else{
      Serial.println("HL_Off");
      front_high_st = 0;
      digitalWrite(FH_OPin, 0);
    }
  }
  if(HL_BTNST && !forcedHLoff){
    digitalWrite(FH_OPin, 1);
  }
  else{
    if(!front_high_st){
      digitalWrite(FH_OPin, 0);
    }
  }
}
  
void forceHLoff(){
  unsigned long CM = millis();
  if(CM - PM_04 > 2000){PM_04 = CM;
    forcedHLoff = 1;
  }
  else{
    forcedHLoff = 0;
  }
}
  
void handleBlinkers(){
  unsigned long CM = millis();
  if(digitalRead(left_SW) || digitalRead(right_SW)){
    if(digitalRead(left_SW) && digitalRead(right_SW)){
      if(CM - PM_03 > blinker_delay){PM_03 = CM;
        if(!left_st){
          left_st = 1;
          right_st = 1;
        }
        else{
          left_st = 0;
          right_st = 0;
        }
      }
    }
    else{
      if(CM - PM_01 > blinker_delay){PM_01 = CM;
        if(digitalRead(left_SW)){
          if(!left_st){
            left_st = 1;
          }
          else{
            left_st = 0;
          }
        }
        else{
          left_st = 0;
          PM_01 = blinker_delay+1+CM;
        }
      }
      if(CM - PM_02 > blinker_delay){PM_02 = CM;
        if(digitalRead(right_SW)){
          if(!right_st){
            right_st = 1;
          }
          else{
            right_st = 0;
          }
        }
        else{
          right_st = 0;
          PM_02 = blinker_delay+1+CM;
        }
      }
    }
  }
  blinkers();
}


void blinkers(){
  /// LEFT
  if(digitalRead(left_SW)){
    if(left_st){
      digitalWrite(LEFT_OPin, 1);
      if(ba_speed){
      //front orange dynamic
        unsigned long CM = millis();
        if(CM - PM_10 > ba_speed){PM_10 = CM;
          if(fl_i == 0)fl_i = front_blinker_pixels;
          if(fl_i > 0){
            setFrontPixel(fl_i-1, 0xFF, 0x44, 0x00);
            setRearPixel(fl_i-1, 0xFF, 0x44, 0x00);
            fl_i--;
          }
        }
      }
      else{
        //front orange static
        for (int i = 0; i < front_blinker_pixels; i++){
          setFrontPixel(i, 0xFF, 0x44, 0x00);
          setRearPixel(i, 0xFF, 0x44, 0x00);
        }
      }  
    }
    else{
      fl_i = 0;
      digitalWrite(LEFT_OPin, 0);
      //front default
      for (int i = 0; i < front_blinker_pixels; i++){
        setFrontPixel(i, 0, 0, 0);    
      }
      //rear default
      for (int i = 0; i < rear_blinker_pixels; i++){
        setRearPixel(i, 0, 0, 0);
      }
    }
  }
  else{
    digitalWrite(LEFT_OPin, 0);
    for (int i = 0; i < front_blinker_pixels; i++){
      setFrontPixel(i, FLI_r, FLI_g, FLI_b);    
    }
    for (int i = 0; i < rear_blinker_pixels; i++){
      setRearPixel(i, RLI, 0, 0);
    }
  }
  /// RIGHT
  if(digitalRead(right_SW)){
    if(right_st){
      digitalWrite(RIGHT_OPin, 1);
      //front orange   
      if(ba_speed){
        //front orange dynamic
        unsigned long CM = millis();
        if(CM - PM_11 > ba_speed){PM_11 = CM;
          if(fr_i == 0)fr_i = (FRONT_NUM_LEDS-front_blinker_pixels);
          if(fr_i < FRONT_NUM_LEDS){
            setFrontPixel(fr_i, 0xFF, 0x44, 0x00);
            setRearPixel(fr_i, 0xFF, 0x44, 0x00);
            fr_i++;
          }
        }
      }
      else{
        //rear orange 
        for (int i = (REAR_NUM_LEDS-rear_blinker_pixels); i < REAR_NUM_LEDS; i++){
          setRearPixel(i, 0xFF, 0x44, 0x00);
          setFrontPixel(i, 0xFF, 0x44, 0x00);
        }
      }
    }
    else{
      fr_i =0;
      digitalWrite(RIGHT_OPin, 0);
      //front default 
      for (int i = (FRONT_NUM_LEDS-front_blinker_pixels); i < FRONT_NUM_LEDS; i++){
        setFrontPixel(i, 0, 0, 0);    
      }
      //rear default 
      for (int i = (REAR_NUM_LEDS-rear_blinker_pixels); i < REAR_NUM_LEDS; i++){
        setRearPixel(i, 0, 0, 0);  
      }
    }
  }
  else{
    digitalWrite(RIGHT_OPin, 0);
    for (int i = (FRONT_NUM_LEDS-front_blinker_pixels); i < FRONT_NUM_LEDS; i++){
      setFrontPixel(i, FLI_r, FLI_g, FLI_b);    
    }
    for (int i = (REAR_NUM_LEDS-rear_blinker_pixels); i < REAR_NUM_LEDS; i++){
      setRearPixel(i, RLI, 0, 0);
    }
  }
}

void setLightIntensity(){  
  if(digitalRead(brake_BTN)){
    RLI = 255;
  }
  else{
     if(digitalRead(lights_SW)){
       RLI = 128;
     }
     else{
       RLI = 0;
     }
  }
  if(digitalRead(lights_SW)){
    // WHITE
     FLI_r = 255;
     FLI_g = 100;
     FLI_b = 50;
  }
  else{
    // BLACK
     FLI_r = 0;
     FLI_g = 0;
     FLI_b = 0;
  }
  for (int i = front_blinker_pixels; i < (FRONT_NUM_LEDS-front_blinker_pixels); i++){
    setFrontPixel(i, FLI_r, FLI_g, FLI_b);  
  }
  for (int i = rear_blinker_pixels; i < (REAR_NUM_LEDS-rear_blinker_pixels); i++){
    setRearPixel(i, RLI, 0, 0);  
  }
  digitalWrite(REAR_OPin, RLI); // Riadi brzdove svetlo na zaklade nastavenia farby
}
void setFrontPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip_front.setPixelColor(Pixel, strip_front.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void setRearPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip_rear.setPixelColor(Pixel, strip_rear.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip_rear.show();
   strip_front.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}
