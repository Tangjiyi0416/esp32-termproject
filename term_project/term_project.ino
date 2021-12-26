#include<Wire.h>
#include"SSD1306Wire.h"
#include "pitches.h"
///*
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

// 1. Define the WiFi credentials
#define WIFI_SSID "tttddd"
#define WIFI_PASSWORD "idkhowto"

//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

// 2. Define the API Key
#define API_KEY "AIzaSyBheQiegwSHSxObaMvDSRc7nMpG2rQKQfY"

// 3. Define the RTDB URL
#define DATABASE_URL "esp32-term-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

// 4. Define the user Email and password that alreadey registerd or added in your project
#define USER_EMAIL "111@mail.com"
#define USER_PASSWORD "123456"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long checkResetPrevMillis = 0;

//*/


SSD1306Wire display(0x3c, 21, 22);
int freq = 2000; 
int channel = 0; 
int resolution = 8;


unsigned long currentTime = 0;
int totalWaterDrank = 0;
//notification
unsigned long notificationPreviousTime = 0;
int goal = 0;

unsigned long notificationDisplayPreviousTime = 0;
bool playNotification = false;
void StartNotificationDisplay() {
  notificationDisplayPreviousTime = currentTime;
  StartMusic();
  StartText(String(goal));
  playNotification = true;
}
void StopNotificationDisplay() {
  notificationDisplayPreviousTime = currentTime;
  StopMusic();
  StartText("Total: " + String(totalWaterDrank));
  playNotification = false;
}

//Music
int melody[] = {
  0, NOTE_G3, NOTE_G3, NOTE_G3, NOTE_DS3, 0, NOTE_F3, NOTE_F3,
  NOTE_F3, NOTE_D3
};
int noteDurations[] = {
  8, 8, 8, 8, 2, 8, 8, 8, 8, 2
};

unsigned long tonePreviousTime = 0;
int pauseBetweenNotes = 0;
int noteDuration = 0;
int thisNote = 0;
boolean nextTone = true;
bool playMusic = false;

void StartMusic() {
  tonePreviousTime = currentTime;
  pauseBetweenNotes = 0;
  noteDuration = 0;
  thisNote = 0;
  nextTone = true;
  playMusic = true;
  Serial.println("Music started");
}
void StopMusic() {
  playMusic = false;
  Serial.println("Music stopped");
}

//text
unsigned long textStartedTime = 0;
bool playText = false;
String displayStr = "";
String displayStr2 = "";
void StartText(String str) {
  textStartedTime = currentTime;
  playText = true;
  displayStr = str + " ml";
  Serial.print("Text ");
  Serial.print(str);
  Serial.println(" started");
}
void StopText() {
  playText = false;
  Serial.print("Text");
  Serial.println(" stopped");
}

//water level
int waterLevel = 0;
int lastWaterReadings[5] = {0};
int currentWaterReading = 0;
unsigned long waterReadingPreviousTime = 0;



int presurePin = 34;
int tonePin = 17;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ///*
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);
  Serial.printf("Get totalWaterDrank... %s\n", Firebase.RTDB.getInt(&fbdo, "users/111/water") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());;
  totalWaterDrank = fbdo.to<int>();
  StartText("Total: " + String(totalWaterDrank));
  //*/
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(tonePin, channel);
  ledcWriteTone(channel, 0);
  pinMode(presurePin, INPUT);
  display.init();
  display.flipScreenVertically();
  display.setContrast(255); //數值介於0~255，調整對比
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);

}
void loop() {
  char in = Serial.read();

  if (in == '1') {
    StartMusic();
  }
  else if (in == '0') StopMusic();


  //get stable water level
  float avgWaterReading = lastWaterReadings[0];
  for (int i = 0; i < 4; i++) {
    avgWaterReading +=  lastWaterReadings[i] = lastWaterReadings[i + 1];
  }
  avgWaterReading /= 5.f;
  currentWaterReading = analogRead(presurePin);

  //Serial.print("avgWaterReading ");
  //Serial.print(avgWaterReading);
  //Serial.print(" currentWaterReading ");
  //Serial.println(currentWaterReading);

  currentTime = millis(); //記錄現在時間
  if (fabs(avgWaterReading - currentWaterReading) < 30.f) {
    //Serial.print("Input stable ");
    //Serial.println(currentWaterReading);

    //here we got the stable water level, now we can compare it to our last stable record
    if (waterReadingPreviousTime == 0)waterReadingPreviousTime = currentTime;

    displayStr2 = ("Reading: " + String((currentTime - waterReadingPreviousTime) / 7.f) + "%");

    if (currentTime - waterReadingPreviousTime > 700) {
      Serial.println("Input Water level stable");
      if (currentWaterReading == 0) {
        //bottle gone
        Serial.println("Bottle gone");
      }
      else if (currentWaterReading - waterLevel > 20) { //ensure its not of noise
        //water level increased
        Serial.print("Water Level Increased to: ");
        Serial.println(currentWaterReading);
        waterLevel = currentWaterReading;
      } else if (waterLevel - currentWaterReading > 20) { //ensure its not of noise
        //water level decreased
        Serial.print("Water Level Decreased to: ");
        Serial.println(currentWaterReading);
        goal -= waterLevel - currentWaterReading;
        StartText(String(goal));
        if (goal < 0)goal = 0;
        totalWaterDrank += waterLevel - currentWaterReading;
        waterLevel = currentWaterReading;
      }
      displayStr2 = "";
      waterReadingPreviousTime = 0;
    }
  } else {
    waterReadingPreviousTime = 0;
  }

  lastWaterReadings[4] = currentWaterReading;

  delay(10);

  //Check notification
  currentTime = millis(); //記錄現在時間
  if ( notificationPreviousTime == 0 )
    notificationPreviousTime = currentTime;

  if (currentTime >= notificationPreviousTime + 30000) {//30秒提醒
    goal += 50;
    StartNotificationDisplay();
    notificationPreviousTime = currentTime;
  }

  ///* reset
  if (Firebase.ready() && (millis() - checkResetPrevMillis > 5000 || checkResetPrevMillis == 0))
  {
    checkResetPrevMillis = millis();
    bool reset = false;
    Serial.printf("Get reset... %s\n", Firebase.RTDB.getBool(&fbdo, "users/111/reset") ? (reset = fbdo.to<bool>()) ? "true" : "false" : fbdo.errorReason().c_str());
    if (reset) {
      totalWaterDrank = 0;
      if(!playNotification)  StartText("Total: " + String(totalWaterDrank));
      Serial.printf("Set reset... %s\n", Firebase.RTDB.setBool(&fbdo, "users/111/reset", false) ? "ok" : fbdo.errorReason().c_str());
    }
  }
  //*/

  //Check if goal reached
  if (goal == 0 && playNotification) {
    StopNotificationDisplay();
    ///*
    if (Firebase.ready())
    {
      Serial.printf("Set water... %s\n", Firebase.RTDB.setInt(&fbdo, "users/111/water", totalWaterDrank) ? "ok" : fbdo.errorReason().c_str());
    }
    //*/
  }

  delay(10);

  currentTime = millis(); //記錄現在時間
  if ((currentTime - tonePreviousTime) > noteDuration || !playMusic) {
    ledcWriteTone(channel, 0);
  } //buzzer停止播放
  if ((currentTime - tonePreviousTime) > (pauseBetweenNotes) && playMusic) {
    noteDuration = 1300 / noteDurations[thisNote];
    ledcWriteTone(channel, melody[thisNote]);
    pauseBetweenNotes = noteDuration * 1.3;
    thisNote++;
    if (thisNote == sizeof(melody) / sizeof(melody[0])) thisNote = 0;
    tonePreviousTime = currentTime;
  } //buzzer播放下一個音

  if (playText) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 20, displayStr);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 44, displayStr2);
    display.display();
  }
  delay(10);
}
