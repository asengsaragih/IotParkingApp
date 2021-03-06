// define QRCodeReader library
#include <Arduino.h>
#include <ESP32QRCodeReader.h>
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

struct QRCodeData qrCodeData;

// define Firebase library
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h" //Provide the token generation process info.
#include "addons/RTDBHelper.h"  //Provide the RTDB payload printing info and other helper functions.

#define FIREBASE_HOST "https://smart-parking-adaab-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "ql7sXO5Lto6MnCAVdKI5IfAoZFGRxVA2vHYIkEIR"
#define WIFI_SSID "TITOO"
#define WIFI_PASSWORD "titoo666"

bool isConnected = false;
FirebaseData firebaseData; //Define Firebase Data object
FirebaseConfig config;

//defined buzzer and other component
#define BUZZER 12

//fungsi untuk menghidupkkan buzzer
void turnBuzzer(int looping, int delayTime)
{
  for (size_t i = 0; i < looping; i++)
  {
    digitalWrite(BUZZER, HIGH);
    delay(delayTime);
    digitalWrite(BUZZER, LOW);
    delay(delayTime);
  }
}

void updateStatusOrderAndSlotFirebase(String orderKey)
{
  //function for updating status in qrcode and slot

  // Serial.println("Membuka gerbang otomatis");
  turnBuzzer(2, 100);

  //open gate with serial communication
  Serial.write("Open");

  //update status in root order
  //status 2 on order pending
  Firebase.setInt(firebaseData, "/Order/" + orderKey + "/status", 2);
}

void checkQrcode(String qrcode)
{
  if (Firebase.ready())
  {
    if (Firebase.getString(firebaseData, "/Qrcode/" + qrcode))
    {
      //update status order
      updateStatusOrderAndSlotFirebase(firebaseData.stringData());
    }
    else
    {
      // Serial.println("Qr tidak terdaftar di database");
      turnBuzzer(1, 2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);

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

  /* Assign the api key (required) */
  config.api_key = FIREBASE_AUTH;

  /* Assign the RTDB URL (required) */
  config.database_url = FIREBASE_HOST;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.reconnectWiFi(true);

  //initialize qrcode
  Serial.println();

  reader.setup();
  Serial.println("Setup QRCode Reader");
  reader.begin();
  Serial.println("QR Parking App");

  //initialize component
  pinMode(BUZZER, OUTPUT);

  turnBuzzer(3, 50);

  delay(1000);
}

void loop()
{
  //validate reader receiver
  if (reader.receiveQrCode(&qrCodeData, 100))
  {
    //get valid qrcode value
    if (qrCodeData.valid)
    {
      //check firebase
      String qrcodeResult = (const char *)qrCodeData.payload;
      checkQrcode(qrcodeResult);
    }
  }
}