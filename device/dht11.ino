#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* ssid = "ERDE";
const char* password = "hakade17";
const char* mqtt_server = "34.121.186.105";

WiFiClient espClient;
PubSubClient client(espClient);

//publish
String msg, msgtopic;
char msg1[50];
char msg2[50];

#define DHTPIN 5     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 5000;  

//Print data yang diterima serta topiknya
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  msgtopic = String((char*)topic);
  msg = "";
  for (int i = 0; i < length; i++) { // Concat payload char to string (msg)
    msg += (char)payload[i];
  }
  Serial.println(msg);
}

//Fungsi mencoba ulang koneksi ketika gagal
void reconnect() {
  //Diulang sampai berhasil terkoneksi
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    //Menguji koneksi
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("topic/hum", "0"); //Send first confirmation message if connected
      client.subscribe("topic/hum");
      client.subscribe("topic/tem");

    } else {
      //Keluaran untuk indikator gagal terkoneksi
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      //Menunggu 5 detik sebelum mencoba lagi
      delay(5000);
    }
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  randomSeed(micros());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
 
void loop(){  
  //Apabila koneksi gagal, coba lagi
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }

    snprintf (msg1, 50, "%f", h);
    snprintf (msg2, 50, "%f", t);

    //publish message
    client.publish("topic/hum", msg1);
    client.publish("topic/tem", msg2);
  }
}
