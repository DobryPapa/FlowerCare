#include <Wire.h>
#include <SparkFun_Si7021_Breakout_Library.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <class.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define supplyPeriphetialsPin 13
#define supplyBatteryMonitorPin 14
#define moistureSensorPin 0
#define batteryStatusPin 2

Adafruit_BME280 bme;

//WiFi connection info
WiFiClient client;
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "********";

//Domoticz server info
const char *host = "192.168.*.*"; //Domoticz host IP
int port = ****;                  //Domoticz port

Supply supplyPeriphetials(supplyPeriphetialsPin);
Supply supplyBatteryMonitor(supplyBatteryMonitorPin);
MoistureSensor soilMoistureSensor(moistureSensorPin, supplyPeriphetialsPin);

void setup()
{
    Serial.begin(115200); //Begin serial port connection

    Serial.println("Initializing IO.");
    pinMode(batteryStatusPin, INPUT);
    supplyPeriphetials.Init();
    supplyBatteryMonitor.Init();
    soilMoistureSensor.Init();
    Serial.println("IO initialization done.");

    BMEConnectionCheck();
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
    delay(500);

    SendBatteryStatus();
    SendSoilMoistureStatus();
    SendBMEStatus();

    delay(200);
    supplyPeriphetials.Off();
    supplyBatteryMonitor.Off();

    Serial.println("Turning on deep sleep mode for 60 seconds.");
    ESP.deepSleep(3600e6);
}

void BMEConnectionCheck()
{
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }
    else
    {
        Serial.println("BME initialization done.");
    }
}

void ConnectToWifi()
{
    WiFi.begin(ssid, password);

    Serial.println("Connecting to wireless network.../n");

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
    Serial.println("Connecting to Domoticz server...");
    if (!client.connect(host, port))
    {
        Serial.println("Connection failed");
        delay(1000);
        return false;
    }
    else
    {
        Serial.println("Connection successful");
        return true;
    }
}

int BatteryStatus()
{
    int batteryStatus = 0;
    batteryStatus = digitalRead(2);
    Serial.print("Battery status: ");
    Serial.println(batteryStatus);
    return batteryStatus;
}

void SendBatteryStatus()
{
    String serialMessage = "Error";
    String domoticzMessage = "Error";
    int batteryStatus = 3;

    if (BatteryStatus())
    {
        batteryStatus = 1;
        serialMessage = "Battery status: OK /nBattery status updated.";
        domoticzMessage = "OK";
    }
    else if (BatteryStatus() == 0)
    {
        batteryStatus = 4;
        serialMessage = "Battery status: LOW BATTERY /nBattery status updated.";
        domoticzMessage = "Naladuj_baterie";
    }
    else
    {
        serialMessage = "Error";
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
        Serial.println("Could not connect to host.");
    }
}

void SendSoilMoistureStatus()
{
    String serialMessage = "Error";
    MoistureSensor soilMoistureSensor(moistureSensorPin);
    serialMessage = "Soil moisture status updated.";

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
        Serial.println("Could not connect to host.");
    }
}

void SendBMEStatus()
{
    if (client.connect(host, port))
    {
        client.print("GET /json.htm?type=command&param=udevice&idx=5&nvalue=0&svalue=");
        client.print(bme.readTemperature());
        client.print(";");
        client.print(bme.readHumidity());
        client.print(";0;");
        client.print(bme.readPressure() / 100.0F);
        client.print(";0");
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.print(host);
        client.print(":");
        client.println(port);
        client.println("User-Agent: Arduino-ethernet");
        client.println("Connection: close");
        client.println();
        Serial.println("Ambient temperature, humidity and pressure updated.");
        client.stop();
    }
    else
    {
        Serial.println("Could not connect to host.");
    }
}
