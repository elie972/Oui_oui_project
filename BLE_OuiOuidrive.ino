
//====================================================================================
//                                  Libraries
//====================================================================================
//Server BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// OUIOUI screen
// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

#ifdef ESP32
  #include "SPIFFS.h" // ESP32 only
#endif

// JPEG decoder library
#include <JPEGDecoder.h>

#include <TFT_eSPI.h>      // Hardware-specific library


//ROS
#include <ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Float64.h>

// Radar Recul
#include <Wire.h>
#include <VL53L0X.h>



//====================================================================================
//                                 Initialisation variables
//====================================================================================


TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

int count = -560;


//BLE
String valor;


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//ROS

ros::NodeHandle  nh;

std_msgs::String str_msg;
std_msgs::Float64  dist_msg;

ros::Publisher chatter("chatter", &str_msg);
ros::Publisher distance("distance", &dist_msg);

char command[13] = "";

//Radar Recul
VL53L0X sensor;
int led = 13; // select the pin for the LED
int count_radar = 0;  // compteur de temporisation

int BackwardPin = A0;    // select the input pin for the potentiometer     
int sensorValue = 0;  // variable to store the value coming from the sensor

int mesured_range = 0; // Distance mesurée


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      
      if (value.length() > 0) {
        int i = 0;
        char valor[value.length() +1] = "";
        for ( i = 0; i < value.length(); i++){
          // Serial.print(value[i]); // Presenta value.
          valor[i] = value[i];
        }
        valor[i+1] = '\0';

        str_msg.data = valor;
        chatter.publish( &str_msg );
        
        //Serial.println(valor);
        
/*
      if (valor == "ForwardStart") {
        Serial.println("En avant OUI OUI");
        //chatter.publish( &str_msg );
        //char reponse[12] = { valor.toCharArray() };
        //str_msg.data = reponse;
        //chatter.publish( &str_msg );
        // Commande ROS Avancer - commande en vitesse en avant
      }

      if (valor == "BackwardStart") {
        Serial.println("En arriere OUI OUI");
        // Commande ROS Reculer - commande en vitesse en arrière
        //Commande radar de recule
      }

      if ((valor == "ForwardStop") || (valor == "BackwardStop")) {
        Serial.println("Du calme");
        // Commande ROS arrêter - commande en vitesse nulle
      }

      if (valor == "RightStart") {
        Serial.println("A droite");
        // Commande ROS a droite - commande en position angle négatif
      }
      if (valor == "LeftStart") {
        Serial.println("A gauche");
        // Commande ROS a droite - commande en position angle positif
      }
      if ((valor == "LeftStop") || (valor == "RightStop")) {
        Serial.println("Tout droit");
        // Commande ROS tout droit - commande en position angle nul
      }*/

      }
    }
};

void setup() {
  //Serial.begin(115200);

  //ROS
  nh.initNode();
  nh.advertise(chatter);
  nh.advertise(distance);
  
  // OuiOui
  delay(10);
  //Serial.println("NodeMCU decoder test!");

  tft.begin();
  tft.setRotation(1);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_WHITE);

  if (!SPIFFS.begin()) {
    //Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  //Serial.println("\r\nInitialisation OuiOui done.");
  listFiles(); // Lists the files so you can see what is in the SPIFFS


  //BLE
  BLEDevice::init("OUI OUI");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Initialisation BLE.");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // Radar recul
  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    //Serial.println("Failed to detect and initialize sensor!");
    //while (1) {}
  }
  
  sensor.startContinuous();
  
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  
}

void loop() {

  

  //OuiOui
  // Note the / before the SPIFFS file name must be present, this means the file is in
  // the root directory of the SPIFFS, e.g. "/tiger.jpg" for a file called "tiger.jpg"

  tft.setRotation(1);  // portrait
  //tft.fillScreen(random(0xFFFFFF));

  drawJpeg("/ouioui2.jpg", count , 0);     // 240 x 320 image
  //drawJpeg("/Baboon40.jpg", 0, 0); // 320 x 480 image

  count += 2;
  delay(0.1);

  if (count > 100){
    count = -560; 
  }

  // Radar Recul

  mesured_range = sensor.readRangeContinuousMillimeters();
  
  if( mesured_range> 100 && mesured_range< 900){
    dist_msg.data = mesured_range;
    distance.publish(&dist_msg);
    
    if( count_radar < mesured_range/100){
      //Serial.println("[debug] in count_radar");
      //Serial.print("count_radar = ");
      //Serial.println(count_radar);
      count_radar++;
    }
    else {
      digitalWrite(led, !digitalRead(led));
      //Serial.println("[debug] in else");
      count_radar = 0;
    }
  }
  else {
    count_radar = 0;
    digitalWrite(led, LOW);
  }

  //ROS

  nh.spinOnce();

  
  
  //BLE
  // put your main code here, to run repeatedly:
  //delay(2000);
  //Serial.println("valor en el loop = ");
  //Serial.println(valor); // Presenta valor.
};
