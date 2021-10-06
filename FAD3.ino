/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/
#include <iostream>
//using namespace std;
#include <BlynkSimpleEsp32.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <stdio.h>
#include<string>



// define the GPIO connected with Relays and switches

#define RelayPin1 15  //D15
#define RelayPin2 2  //D2
#define Buzzer 5     //D5
#define led 4       //D4
#define AUTH "cnfuKwrmrR9slQmpnF5e4uFXQ9TaWafm"                 // You should get Auth Token in the Blynk App.  

#define SwitchPin1 10  //SD3
#define SwitchPin2 0   //D3 


#define VPIN_BUTTON_1    V1
#define VPIN_BUTTON_2    V2

int toggleState_1 = 1; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 1; //Define integer to remember the toggle state for relay 1



const char *SSID = "iPhone";
const char *PWD = "pppppppppp";

// MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 

const char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;

void connectToWiFi()
{
  Serial.print("connecting to ");
  
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, LOW);
    Serial.println("Wifi not connected/n");
    delay(500);
  }
  
  Serial.print("Connected.");
  digitalWrite(led, HIGH);
 
  

}

BlynkTimer timer;

void relayOnOff(int relay){

  switch(relay){
    case 1: 
      if(toggleState_1 == 1){
        digitalWrite(RelayPin1, LOW); // turn on relay 1
        toggleState_1 = 0;
        Serial.println("Device1 ON");
        }
      else{
        digitalWrite(RelayPin1, HIGH); // turn off relay 1
        toggleState_1 = 1;
        Serial.println("Device1 OFF");
        }
        delay(100);
        break;
    case 2: 
      if(toggleState_2 == 1){
        digitalWrite(RelayPin2, LOW); // turn on relay 2
        toggleState_1 = 0;
        Serial.println("Device2 ON");
        }
      else{
        digitalWrite(RelayPin2, HIGH); // turn off relay 1
        toggleState_2 = 1;
        Serial.println("Device2 OFF");
        }
        delay(100);
        break;
        default : break;      
    }
  
}

void with_internet(){
  //Manual Switch Control
  if (digitalRead(SwitchPin1) == LOW){
    delay(200);
    relayOnOff(1); 
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);   // Update Button Widget  
  }
}
void without_internet(){
  //Manual Switch Control
  if (digitalRead(SwitchPin1) == LOW){
    delay(200);
    relayOnOff(1);      
  }
}

BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(VPIN_BUTTON_1);
  }

// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, toggleState_1);
}

void checkBlynkStatus() { // called every 3 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    //wifiFlag = 1;
    //digitalWrite(wifiLed, HIGH); //Turn off WiFi LED
  }
  if (isconnected == true) {
    //wifiFlag = 0;
    //digitalWrite(wifiLed, LOW); //Turn on WiFi LED
  }
}

bool heartBeatPrint(){
  if ( WiFi.status()== WL_CONNECTED){
    Serial.println("Still connected to Wifi network"); 
    return true;
    delay(1000);
  }
  else{
    Serial.println("Connection to network failed. Attempting to reconnect to network");        
    connectToWiFi();
    return false;
  }
}



//MQTT callback functionc

void callback(char* topic, byte* payload, unsigned int length) {
  String converter = "";
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    converter += (char)payload[i];
  }
  varTopic(topic,converter); 
}
void varTopic(String topic, String converter){
  Serial.println(topic + converter);
  if (topic == "/futurehomes/light/par"){
    Serial.println("Switching parlour light ON");
    if (converter == "on"){
      digitalWrite(RelayPin1, LOW); // turn on relay 1
      toggleState_1 = 0;
      Serial.println("Device1 ON");
      }
    else if (converter == "off"){
      
      Serial.println("Switching parlour light OFF");
      digitalWrite(RelayPin1, HIGH); // turn off relay 1
      toggleState_1 = 1;
      Serial.println("Device1 OFF");
      }
  }
  else if (topic ==  "/futurehomes/light/kit"){
    Serial.println("Switching KITCHEN light ON");
    if (converter == "on"){
      digitalWrite(RelayPin2, LOW); // turn on relay 1
      toggleState_2 = 0;
      Serial.println("Device1 ON");
      }
    else if (converter == "off"){
      
      Serial.println("Switching parlour light OFF");
      digitalWrite(RelayPin2, HIGH); // turn off relay 1
      toggleState_2 = 1;
      Serial.println("Device1 OFF");
      }
  }
   
  else{
     Serial.println("NO TOPIC MATCHED");
  }
  }

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}


void setup() {
  Serial.begin(9600);
  
  connectToWiFi();
  setupMQTT();
  //set relay output pin
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(led, OUTPUT);

  //Add a pullup resistor to relaypin to prevent floating circuit
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  //Switch relay OFF on start
  digitalWrite(RelayPin1, toggleState_1);
  digitalWrite(RelayPin2, toggleState_1);
  //digitalWrite(alarm, LOW);           // Initially buzzer off
  Blynk.begin(AUTH, SSID, PWD);
    
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  if (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("mqtt reConnected.");
         digitalWrite(Buzzer, HIGH);
         delay(1000);
         digitalWrite(Buzzer, LOW);                     
         
        // subscribe to topic
         mqttClient.subscribe("/futurehomes/light/kit");
         mqttClient.subscribe("/futurehomes/light/par");
        
      }
      
  }
}


void loop() {
  if (heartBeatPrint() && !mqttClient.connected()){
    reconnect();
  }

  mqttClient.loop();
  Blynk.run();

}
