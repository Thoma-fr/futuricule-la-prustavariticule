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
