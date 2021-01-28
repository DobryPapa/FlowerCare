#include <Wire.h>
#include <SparkFun_Si7021_Breakout_Library.h> //https://github.com/sparkfun/Si7021_Breakout/blob/master/Libraries/Arduino/Si7021/src/SparkFun_Si7021_Breakout_Library.h
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <class.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define supplyPeriphetialsPin 13
#define supplyBatteryMonitorPin 14
#define moistureSensorPin 0

Adafruit_BME280 bme;

//Dane połączenia WIFI
WiFiClient client;
const char *ssid = "TP-LINK_80C0F1";
const char *password = "87729542";

// domoticz
const char *host = "192.168.0.104"; //Domoticz host
int port = 8080;                    //Domoticz port

Supply supplyPeriphetials(supplyPeriphetialsPin);
Supply supplyBatteryMonitor(supplyBatteryMonitorPin);
MoistureSensor soilMoistureSensor(moistureSensorPin, supplyPeriphetialsPin);

void setup()
{
    pinMode(2, INPUT);
    supplyPeriphetials.Init();
    supplyBatteryMonitor.Init();
    soilMoistureSensor.Init();
    Serial.begin(115200); //Otwarcie podgladu

    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }

    ConnectToWifi();
}

void loop()
{

    if (!HostConnectionCheck())
    {
        return;
    }

    supplyPeriphetials.On();
    supplyBatteryMonitor.On();
    digitalWrite(14, HIGH);
    delay(500);

    SendBatteryStatus();
    SendSoilMoistureStatus();
    SendBMEStatus();

    delay(200);
    supplyPeriphetials.Off();
    supplyBatteryMonitor.Off();

    Serial.println("Wchodze w tryb deep sleep");
    ESP.deepSleep(3600e6);
}

//Łączenie WIFI
void ConnectToWifi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("/n connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

bool HostConnectionCheck()
{
    if (!client.connect(host, port))
    {
        Serial.println("connection failed"); //Jeśli nie połączy z hostem poinformuj
        delay(5000);
        return false;
    }
    else
    {
        return true;
    }
}

int BatteryStatus()
{
    int batteryStatus = 0;
    batteryStatus = digitalRead(2);
    Serial.print("Stan baterii: ");
    Serial.println(batteryStatus);
    return batteryStatus;
}

// Domoticz format /json.htm?type=command&param=udevice&idx=IDX&nvalue=0&svalue=TEMP;HUM;HUM_STAT
// /json.htm?type=command&param=udevice&idx=IDX&nvalue=LEVEL&svalue=TEXT
void SendBatteryStatus()
{
    String serialMessage = "Blad";
    String domoticzMessage = "Blad";
    int batteryStatus = 3;

    if (BatteryStatus())
    {
        batteryStatus = 1;
        serialMessage = "Wyslano do domoticza informacje o dobrym stanie baterii";
        domoticzMessage = "OK";
    }
    else if (BatteryStatus() == 0)
    {
        batteryStatus = 4;
        serialMessage = "Wyslano do domoticza informacje o niskim stanie naładowania baterii";
        domoticzMessage = "Naladuj_baterie";
    }
    else
    {
        serialMessage = "Błąd";
    }

    if (client.connect(host, port))
    {
        client.print("GET /json.htm?type=command&param=udevice&idx=4&nvalue=");
        client.print(batteryStatus);
        client.print("&svalue=");
        client.print(domoticzMessage);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.print(host);
        client.print(":");
        client.println(port);
        client.println("User-Agent: Arduino-ethernet");
        client.println("Connection: close");
        client.println();
        Serial.println(serialMessage);
        client.stop();
    }
    else
    {
        Serial.println("Nie można połączyć z serwerem.");
    }
}

void SendSoilMoistureStatus()
{
    String serialMessage = "Blad";
    MoistureSensor soilMoistureSensor;
    serialMessage = "Wysłano do domoticza pomiar wilgotności gleby.";

    if (client.connect(host, port))
    {
        client.print("GET /json.htm?type=command&param=udevice&idx=1&nvalue=");
        client.print(soilMoistureSensor.GetMoisture());
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.print(host);
        client.print(":");
        client.println(port);
        client.println("User-Agent: Arduino-ethernet");
        client.println("Connection: close");
        client.println();
        Serial.println(serialMessage);
        client.stop();
    }
    else
    {
        Serial.println("Nie można połączyć z serwerem.");
    }
}

void SendBMEStatus()
{
    if (client.connect(host, port))
    {
        client.print("GET /json.htm?type=command&param=udevice&idx=3&nvalue=0&svalue=");
        client.print(bme.readHumidity());
        client.print(";");
        client.print(bme.readTemperature());
        client.print(";2");
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.print(host);
        client.print(":");
        client.println(port);
        client.println("User-Agent: Arduino-ethernet");
        client.println("Connection: close");
        client.println();
        Serial.println("Wyslano do domoticza temperature i wilgotnosc powietrza");
        client.stop();
    }
    else
    {
        Serial.println("Nie można połączyć z serwerem.");
    }
}