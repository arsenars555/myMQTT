


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h> 

Servo myservo;

#define Blue_LED 2

const char *ssid =  "ars";  // Имя вайфай точки доступа
const char *pass =  "55555555"; // Пароль от точки доступа

const char *mqtt_server = "192.168.0.14"; // Имя сервера MQTT
const int mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = "user1"; // Логин от сервер
const char *mqtt_pass = "ars555"; // Пароль от сервера

#define BUFFER_SIZE 100

bool LedState = false;
int tm=300;
int temp=0;



void callback(const MQTT::Publish& pub)     // Функция получения данных от сервера
{
    Serial.print(pub.topic());                // выводим в сериал порт название топика
    Serial.print(" => ");
    Serial.println(pub.payload_string());     // выводим в сериал порт значение полученных данных
    
    String payload = pub.payload_string();
    
    if(String(pub.topic()) == "test/Bled")    //  проверяем из нужного ли нам топика пришли данные 
    {
        int stledB = payload.toInt();         //  преобразуем полученные данные в тип integer
        digitalWrite(Blue_LED, stledB);        //  включаем или выключаем светодиод в зависимоти от полученных значений данных
    }
    
    if(String(pub.topic()) == "test/Servo")   // проверяем из нужного ли нам топика пришли данные 
    {
        int pos = payload.toInt();            //  преобразуем полученные данные в тип integer
        myservo.write(pos);                   //  поворачиваем сервопривод
    }
}



WiFiClient wclient;      
PubSubClient client(wclient, mqtt_server, mqtt_port);

void setup() {
    Serial.begin(115200);
    
    myservo.attach(D2);
    
    Serial.println();
    Serial.println();
    pinMode(Blue_LED, OUTPUT);
}

void loop() {
    // подключаемся к wi-fi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to ");
        Serial.print(ssid);
        Serial.println("...");
        WiFi.begin(ssid, pass);
        
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
            return;
        Serial.println("WiFi connected");
    }
    
    // подключаемся к MQTT серверу
    if (WiFi.status() == WL_CONNECTED) {
        if (!client.connected()) {
            Serial.println("Connecting to MQTT server");
            if (client.connect(MQTT::Connect("arduinoClient2")
                                 .set_auth(mqtt_user, mqtt_pass))) {
                Serial.println("Connected to MQTT server");
                client.set_callback(callback);
                client.subscribe("test/Bled");                  // подписывааемся по топик с данными для светодиода
                client.subscribe("test/Servo");                 // подписывааемся по топик с данными для Сервопривода
            } else {
                Serial.println("Could not connect to MQTT server");   
            }
        }
        
        if (client.connected()){
            client.loop();
            TempSend();
        }
    }
} // конец основного цикла


// Функция отправки показаний с термодатчика
void TempSend(){
    if (tm==0)
    {
        temp = analogRead(A0);
        client.publish("test/pot", String(temp)); // отправляем в топик для термодатчика значение температуры
        //Serial.println(temp);
        Serial.print("Pot = \t");Serial.println(temp);

        tm = 2000;  // пауза меду отправками значений температуры  коло 3 секунд
    }
    tm--; 
    delay(10);  
}
