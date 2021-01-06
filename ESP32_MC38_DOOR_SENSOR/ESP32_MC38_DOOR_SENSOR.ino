#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "wifi name";
const char* password = "wifi password";
const char* mqttServer = "mqtt server ip address";
const int mqttPort = 1883;
const char* mqttUser = "ESP32-12";
const char* mqttPassword = "esp12";
char message0;
char message1;
char message2;
char message3;
char message4;
WiFiClient espClient;
PubSubClient client(espClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("arduinoClient")) {
    // Once connected, publish an announcement...
    client.publish("outTopic","hello world");
    // ... and resubscribe
    client.subscribe("inTopic");
  }
  return client.connected();
}

void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

// For Instructable Sensor Contest
int prevState = -1;
int currState = -1;
long lastChangeTime = 0;
int val=0;

void checkPin() {
  // Invert state, since button is "Active LOW"
  int state = !digitalRead(27);
  // Debounce mechanism
  long t = millis();
  if (state != prevState) {
    lastChangeTime = t;
  }
  if (t - lastChangeTime > 50) {
    if (state != currState) {
      currState = state;
      Serial.println(state); //0 is open, 1 is closed.
      char b[2];
      String str = String(state);
      Serial.println(str);
      str.toCharArray(b,2);
      Serial.println(b[0]);     
      char* charStatePtr = &b[0];
      Serial.println(charStatePtr);
      client.publish("esp/esp12", charStatePtr);
    }
  }
  prevState = state;
}

void setup() {
  // Debug console
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client12", mqttUser, mqttPassword )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish("text/all", "hello from the esp32-12"); //Topic name
  client.subscribe("esp/esp12");
  pinMode(27, INPUT_PULLUP); //Door Sensor is connected to D7
  pinMode(2, OUTPUT);
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  message0 = ((char)payload[0]);
  message1 = ((char)payload[1]);
  message2 = ((char)payload[2]);
  message3 = ((char)payload[3]);
  message4 = ((char)payload[4]);
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    //client.loop();
    checkPin();
    val = digitalRead(27);
    if (val==HIGH){//door open
     //Serial.println("val = high");
     //digitalWrite(2, LOW);
    }else{//door closed
      //Serial.println("val = low");
      //digitalWrite(2, HIGH);
    }
  }
}
