#include <WiFi.h> // подключаем библиотеку для работы с WiFi
#include <PubSubClient.h> // подключаем библиотеку для работы с MQTT
#include "DHT.h" // подключаем библиотеку для работы с датчиком температуры и влажности DHT

// введите SSID и пароль вашей WiFi-сети
const char* ssid = "ваш_SSID";
const char* password = "ваш_пароль";

// IP-адрес сервера MQTT
const char* mqtt_server = "192.168.1.103";

// определяем номер пина, к которому подключен датчик DHT
#define DHTPIN 14
// определяем тип датчика DHT (DHT11, DHT21 или DHT22)
#define DHTTYPE DHT22

// создаем объект датчика DHT
DHT dht(DHTPIN, DHTTYPE);

// объявляем объект WiFiClient для подключения к WiFi
WiFiClient espClient;

// создаем объект PubSubClient для работы с MQTT
PubSubClient client(espClient);

// определяем номер пина, к которому подключен светодиод
const char led = 12;

// определяем темы MQTT для публикации данных о температуре и управления светодиодом
#define TEMP_TOPIC    "smarthome/room1/temp"
#define LED_TOPIC     "smarthome/room1/led" // 1=включить, 0=выключить

// определяем переменную для хранения времени последнего обновления данных о температуре
long lastMsg = 0;

// определяем массив для хранения входящих сообщений
char msg[20];

// определяем функцию коллбэк, которая будет вызываться при получении сообщений на подписанную тему
void receivedCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Получено сообщение на тему: ");
    Serial.println(topic);

    Serial.print("Полезная нагрузка: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // проверяем значение полезной нагрузки и выполняем действия в зависимости от него
    if ((char)payload[0] == '1') {
        digitalWrite(led, HIGH);  // включаем светодиод
    }
    else {
        digitalWrite(led, LOW);   // выключаем светодиод
    }
}

// определяем функцию подключения к серверу MQTT
void mqttconnect() {
    // пока не подключены к серверу MQTT, пробуем подключаться
    while (!client.connected()) {
        Serial.print("Подключение к MQTT: ");
        // формируем уникальный идентификатор клиента
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        // пробуем подключиться с указанным идентификатором
        if (client.connect(clientId.c_str())) {
            Serial.println("успешно!");
            // подписываемся на тему управления светодиодом
            client.subscribe(LED_TOPIC);
        }
        else {
            Serial.print("ошибка, код ");
            Serial.print(client.state());
            Serial.println(". Повторим попытку через 5 секунд.");
            // ждем 5 секунд, прежде чем попробовать снова
            delay(5000);
        }
    }
}

void setup() {
    // инициализируем последовательный порт
    Serial.begin(115200);
    // инициализируем WiFi
    Serial.println();
    Serial.print("Подключение к WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    // ждем, пока не подключимся к WiFi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Успешно подключены к WiFi!");
    Serial.println("IP-адрес: ");
    Serial.println(WiFi.localIP());

    // устанавливаем светодиод в режим вывода
    pinMode(led, OUTPUT);

    // подключаемся к серверу MQTT
    client.setServer(mqtt_server, 1883);
    // назначаем функцию коллбэк для обработки входящих сообщений
    client.setCallback(receivedCallback);
    // инициализируем датчик DHT
    dht.begin();
}

void loop() {
    // если клиент не подключен, то повторить попытку подключения
    if (!client.connected()) {
        mqttconnect();
    }

    // функция loop клиента прослушивает входящие сообщения на подписанные темы
    client.loop();

    // измеряем температуру каждые 3 секунды
    // используем переменную now для измерения текущего времени в миллисекундах
    // сравниваем с последним временем измерения температуры lastMsg,
    // чтобы избежать блокировки программы при использовании функции delay()
    long now = millis();
    if (now - lastMsg > 3000) {
        lastMsg = now;

        // считываем температуру с датчика DHT11/DHT22 и конвертируем в строку
        temperature = dht.readTemperature();

        // проверяем, был ли считан корректный результат
        if (!isnan(temperature)) {
            // конвертируем значение температуры в строку
            snprintf(msg, 20, "%lf", temperature);
            // публикуем сообщение на тему температуры
            client.publish(TEMP_TOPIC, msg);
        }
    }
}
// функция mqttconnect() определена выше в коде и предназначена для установления соединения с сервером MQTT.
