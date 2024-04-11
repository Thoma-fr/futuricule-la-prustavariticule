# futuricule-la-prustavariticule
## A remote controllable car from the futur 

# 3D Printing parts
For the 3D Parts you will need to print 3 parts for the visual, you can fill the holes with hot glue for a great effect withs the LEDS
2 Parts for the chassis of the car and a ball bearing to put in the round hole in the middle for the rotaion
and of course four wheels, you can use lego parts for the rubber

# Electronik parts
* x2 [ESP8266](https://www.amazon.fr/ESP8266-ESP-12F-D%C3%A9veloppement-NodeMCU-Compatible/dp/B093G72SHN/ref=sr_1_9?dib=eyJ2IjoiMSJ9.YQX2ZC-7ArE__8t9lEkImfeOFQlYW9eR_XyTZMPGCydiMPJwFo3SH0q-W_OFCQVaprBaSgDgKUFxiD5nIVrr0IW19PVJFc_z57plBFBfBoV3HlBrnDa3Q6lioYCwLwiRWXQX80fjCXf8CUyLkz-3upz11ng5gtnv0pGEZXcWmNKQMx-EA08kAbi9U8pXfaJ0V6Ex86LzvyrJIOS2v5PLTjFltLwQ4vC14ntNHFdAaefURVhJp7JbfYM552nLaF_tlNEuJ1JF3BeTkcBYuGTSM_XfV1aNdN-g9t7jZFVkHNA.qCq0N1dXJ9OmZ2acd5-Qqt2cb141rRNyPm0DflF4T6M&dib_tag=se&keywords=ESP8266&qid=1712824245&sr=8-9)(the link is already 2 pieces)
+ x1 [joystick](https://www.amazon.fr/AZDelivery-Module-joystick-Arduino-gratuit/dp/B07V3HQSVY?th=1)
+ x1 [electrical motor](https://fr.aliexpress.com/item/1005005016612156.html?spm=a2g0o.productlist.main.5.73d012016IDG91&algo_pvid=0ec19e4d-8114-4067-838b-e0b8d7f2a974&algo_exp_id=0ec19e4d-8114-4067-838b-e0b8d7f2a974-2&pdp_npi=4%40dis%21EUR%210.72%210.53%21%21%210.76%210.56%21%402103847817128248219031199ecfb0%2112000031353632363%21sea%21FR%210%21AB&curPageLogUid=yBVCkdcyI8l4&utparam-url=scene%3Asearch%7Cquery_from%3A)
+ x1 [Servo motor](https://boutique.semageek.com/fr/104-micro-servo-tower-pro-sg90-3007447379574.html)

# Other parts
+ x4 LEGO 4297209 PNEU Ø56 X 26

# Arduino Code
Controller
```
#include <ESP8266WiFi.h>
#include <espnow.h>

int JOYSTICK_BUTTON_PIN = D5;
int X_PIN = D6;
int Y_PIN = D7;
int ANALOG_PIN = A0;

int xValue;
int yValue;

typedef struct dataStruct {
  int y;
  int x;
  bool isButtonPressed;
} dataStruct;

dataStruct myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 30;  // send readings timer

uint8_t broadcastAddress[] = {0x08, 0xF9, 0xE0, 0x73, 0xB6, 0x3D};

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.print("Delivery fail: ");
    Serial.println(sendStatus);
  }
}

void setup() {
  Serial.begin(74880);
  pinMode(X_PIN, OUTPUT);
  pinMode(Y_PIN, OUTPUT);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(X_PIN, LOW);
  digitalWrite(Y_PIN, LOW);

  xValue = 0;
  yValue = 0;

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) { 
    readXValue();
    readYValue();
    myData.isButtonPressed = digitalRead(JOYSTICK_BUTTON_PIN);

    Serial.print("xValue : ");
    Serial.print(xValue);

    myData.y = yValue;
    myData.x = xValue;
    
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }
}

void readXValue(){
  digitalWrite(X_PIN, HIGH);
  digitalWrite(Y_PIN, LOW);
  xValue = analogRead(ANALOG_PIN);
}

void readYValue(){
  digitalWrite(X_PIN, LOW);
  digitalWrite(Y_PIN, HIGH);
  yValue = analogRead(ANALOG_PIN);
}

```

Recever
```
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h>
#include <NeoPixelBus.h>

int STEERING_PIN = D3;
int FRONT_MOTOR_A_PIN = D5;
int BACK_MOTOR_A_PIN = D6;
int FRONT_MOTOR_B_PIN = D7;
int BACK_MOTOR_B_PIN = D8;

int steeringValue;
int digitalXValue;
int accelerationValue;
int ledDelayIndex;
int delayTime;
Servo steering;

const uint16_t PixelCount = 28;
const uint8_t PixelPin = 2;
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);
int rightIndex = 0;
int leftIndex = 28;
int centerIndex = 13;
int rightAsyncIndex = 7;
int leftAsyncIndex = 19;

typedef struct dataStruct {
  int y;
  int x;
  bool isButtonPressed;
} dataStruct;

dataStruct myData;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

void setup() {
  Serial.begin(74880);
  Serial.println("led");

  digitalXValue = 0;
  steeringValue = 90;
  accelerationValue = 0;
  ledDelayIndex = 0;
  delayTime = 3;
  
  steering.attach(STEERING_PIN);
  steering.write(steeringValue);

  pinMode(BACK_MOTOR_A_PIN, OUTPUT);
  pinMode(BACK_MOTOR_B_PIN, OUTPUT);
  pinMode(FRONT_MOTOR_A_PIN, OUTPUT);
  pinMode(FRONT_MOTOR_B_PIN, OUTPUT);

  digitalWrite(BACK_MOTOR_A_PIN, LOW);
  digitalWrite(BACK_MOTOR_B_PIN, LOW);
  digitalWrite(FRONT_MOTOR_A_PIN, LOW);
  digitalWrite(FRONT_MOTOR_B_PIN, LOW);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  strip.Begin();
  strip.Show();
}

void loop() { 
  delay(30);
  steer();
  accelere();

  if(myData.isButtonPressed){
    if(ledDelayIndex>delayTime){
      led();
      ledDelayIndex = 0;
    }

    ledDelayIndex++;
  }
  else{
    for(int i = 0; i< PixelCount; i++){
      strip.SetPixelColor(i, RgbColor(255, 0, 0));
    }
    strip.Show();
  }
}

void steer(){
  steeringValue = myData.y * 180 / 1024;
  //Serial.print("steering : ");
  //Serial.println(steeringValue);
  steering.write(steeringValue);
}

void accelere(){
  digitalXValue  = constrain(myData.x/4, 0, 255);

  Serial.print("myData x : ");
  Serial.print(myData.x);
  Serial.print(" digitalXValue : ");
  Serial.print(digitalXValue);

  delayTime = 4;

  if(digitalXValue > 140 && digitalXValue < 256){
    accelerationValue = constrain((digitalXValue-128)*2, 0, 255);

    if(accelerationValue > 240){
      delayTime = 0;
    }
    else if(accelerationValue > 190){
      delayTime = 1;
    }
    else if(accelerationValue > 130){
      delayTime = 2;
    }
    else if(accelerationValue > 60){
      delayTime = 3;
    }

    digitalWrite(BACK_MOTOR_A_PIN, LOW);
    analogWrite(FRONT_MOTOR_A_PIN, accelerationValue);

    digitalWrite(BACK_MOTOR_B_PIN, LOW);;
    analogWrite(FRONT_MOTOR_B_PIN, accelerationValue);
  }
  else if(digitalXValue > 0 && digitalXValue < 116){
    accelerationValue = constrain(abs((digitalXValue)*2 - 255), 0, 255);

    analogWrite(BACK_MOTOR_A_PIN, accelerationValue);
    digitalWrite(FRONT_MOTOR_A_PIN, LOW);
    
    analogWrite(BACK_MOTOR_B_PIN, accelerationValue);
    digitalWrite(FRONT_MOTOR_B_PIN, LOW);
  }
  else{
    accelerationValue = 0;

    digitalWrite(FRONT_MOTOR_A_PIN, LOW);
    digitalWrite(BACK_MOTOR_A_PIN, LOW);

    digitalWrite(FRONT_MOTOR_B_PIN, LOW);
    digitalWrite(BACK_MOTOR_B_PIN, LOW);
  }

  Serial.print(" accelerationValue : ");
  Serial.println(accelerationValue);
}

void led(){ 
  RgbColor color;

  for(int i = 0; i< PixelCount; i++){
    if(rightIndex != i && leftIndex != i && rightIndex-1 != i && leftIndex+1 != i && 
      rightAsyncIndex != i && leftAsyncIndex != i && rightAsyncIndex-1 != i && leftAsyncIndex+1 != i){
      strip.SetPixelColor(i, RgbColor(0, 0, 0));
      }
    else{
      if(rightIndex == i || leftIndex == i){
        color = RgbColor(0,191,255); 
      }
      else if(rightAsyncIndex == i || leftAsyncIndex == i){
        color = RgbColor(220, 20, 60);
      }

      if(rightIndex-1 == i || leftIndex+1 == i){
        color = RgbColor::LinearBlend(
        RgbColor(0,191,255),
        RgbColor(0, 0, 0),
        0.8f);
      }
      else if (rightAsyncIndex-1 == i || leftAsyncIndex+1 == i){
        color = RgbColor::LinearBlend(
        RgbColor(220, 20, 60),
        RgbColor(0, 0, 0),
        0.8f);
      }

      strip.SetPixelColor(i, color);
    }
  }

  rightIndex--;
  if(rightIndex<0){
    rightIndex = centerIndex+1;
  }
  leftIndex++;
  if(leftIndex>=PixelCount){
    leftIndex = centerIndex;
  }
  
  rightAsyncIndex--;
  if(rightAsyncIndex<0){
    rightAsyncIndex = centerIndex+1;
  }
  leftAsyncIndex++;
  if(leftAsyncIndex>=PixelCount){
    leftAsyncIndex = centerIndex;
  }

  strip.Show();
}
```
