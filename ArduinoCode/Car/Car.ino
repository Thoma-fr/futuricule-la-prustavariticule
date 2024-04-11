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