#include <WiFi.h> // ���������� ���������� ��� ������ � WiFi
#include <PubSubClient.h> // ���������� ���������� ��� ������ � MQTT
#include "DHT.h" // ���������� ���������� ��� ������ � �������� ����������� � ��������� DHT

// ������� SSID � ������ ����� WiFi-����
const char* ssid = "���_SSID";
const char* password = "���_������";

// IP-����� ������� MQTT
const char* mqtt_server = "192.168.1.103";

// ���������� ����� ����, � �������� ��������� ������ DHT
#define DHTPIN 14
// ���������� ��� ������� DHT (DHT11, DHT21 ��� DHT22)
#define DHTTYPE DHT22

// ������� ������ ������� DHT
DHT dht(DHTPIN, DHTTYPE);

// ��������� ������ WiFiClient ��� ����������� � WiFi
WiFiClient espClient;

// ������� ������ PubSubClient ��� ������ � MQTT
PubSubClient client(espClient);

// ���������� ����� ����, � �������� ��������� ���������
const char led = 12;

// ���������� ���� MQTT ��� ���������� ������ � ����������� � ���������� �����������
#define TEMP_TOPIC    "smarthome/room1/temp"
#define LED_TOPIC     "smarthome/room1/led" // 1=��������, 0=���������

// ���������� ���������� ��� �������� ������� ���������� ���������� ������ � �����������
long lastMsg = 0;

// ���������� ������ ��� �������� �������� ���������
char msg[20];

// ���������� ������� �������, ������� ����� ���������� ��� ��������� ��������� �� ����������� ����
void receivedCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("�������� ��������� �� ����: ");
    Serial.println(topic);

    Serial.print("�������� ��������: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // ��������� �������� �������� �������� � ��������� �������� � ����������� �� ����
    if ((char)payload[0] == '1') {
        digitalWrite(led, HIGH);  // �������� ���������
    }
    else {
        digitalWrite(led, LOW);   // ��������� ���������
    }
}

// ���������� ������� ����������� � ������� MQTT
void mqttconnect() {
    // ���� �� ���������� � ������� MQTT, ������� ������������
    while (!client.connected()) {
        Serial.print("����������� � MQTT: ");
        // ��������� ���������� ������������� �������
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        // ������� ������������ � ��������� ���������������
        if (client.connect(clientId.c_str())) {
            Serial.println("�������!");
            // ������������� �� ���� ���������� �����������
            client.subscribe(LED_TOPIC);
        }
        else {
            Serial.print("������, ��� ");
            Serial.print(client.state());
            Serial.println(". �������� ������� ����� 5 ������.");
            // ���� 5 ������, ������ ��� ����������� �����
            delay(5000);
        }
    }
}

void setup() {
    // �������������� ���������������� ����
    Serial.begin(115200);
    // �������������� WiFi
    Serial.println();
    Serial.print("����������� � WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    // ����, ���� �� ����������� � WiFi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("������� ���������� � WiFi!");
    Serial.println("IP-�����: ");
    Serial.println(WiFi.localIP());

    // ������������� ��������� � ����� ������
    pinMode(led, OUTPUT);

    // ������������ � ������� MQTT
    client.setServer(mqtt_server, 1883);
    // ��������� ������� ������� ��� ��������� �������� ���������
    client.setCallback(receivedCallback);
    // �������������� ������ DHT
    dht.begin();
}

void loop() {
    // ���� ������ �� ���������, �� ��������� ������� �����������
    if (!client.connected()) {
        mqttconnect();
    }

    // ������� loop ������� ������������ �������� ��������� �� ����������� ����
    client.loop();

    // �������� ����������� ������ 3 �������
    // ���������� ���������� now ��� ��������� �������� ������� � �������������
    // ���������� � ��������� �������� ��������� ����������� lastMsg,
    // ����� �������� ���������� ��������� ��� ������������� ������� delay()
    long now = millis();
    if (now - lastMsg > 3000) {
        lastMsg = now;

        // ��������� ����������� � ������� DHT11/DHT22 � ������������ � ������
        temperature = dht.readTemperature();

        // ���������, ��� �� ������ ���������� ���������
        if (!isnan(temperature)) {
            // ������������ �������� ����������� � ������
            snprintf(msg, 20, "%lf", temperature);
            // ��������� ��������� �� ���� �����������
            client.publish(TEMP_TOPIC, msg);
        }
    }
}
// ������� mqttconnect() ���������� ���� � ���� � ������������� ��� ������������ ���������� � �������� MQTT.
