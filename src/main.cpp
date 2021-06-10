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
#define WIFI_SSID "Suncodeid"
#define WIFI_PASSWORD "Ganteng00!@#"

bool isConnected = false;

FirebaseData firebaseData; //Define Firebase Data object

FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;

void checkQrcode(String qrcode)
{
  if (Firebase.ready())
  {
    if (Firebase.getInt(firebaseData, "/Qrcode/" + qrcode))
    {
      Serial.println("Membuka gerbang otomatis");
    } else {
      Serial.println("Qr tidak terdaftar di database");
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

  delay(1000);
}

void loop()
{
  if (reader.receiveQrCode(&qrCodeData, 100))
  {
    Serial.println("Found QRCode");
    if (qrCodeData.valid)
    {
      Serial.print("Payload: ");
      Serial.println((const char *)qrCodeData.payload);

      String qrcodeResult = (const char *)qrCodeData.payload;

      checkQrcode(qrcodeResult);
    }
    else
    {
      Serial.print("Invalid: ");
      Serial.println((const char *)qrCodeData.payload);
    }
  }

  delay(500);
}