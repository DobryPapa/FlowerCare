//W przyszłości można wprowadzić "interfejs"

class Supply
{
private:
    byte pin;

public:
    Supply(byte pin)
    {
        this->pin = pin;
        Init();
    }

    void Init()
    {
        pinMode(pin, OUTPUT);
    }

    void On()
    {
        digitalWrite(pin, HIGH);
    }

    void Off()
    {
        digitalWrite(pin, LOW);
    }
};

class MoistureSensor
{
private:
    byte pin;
    byte enablePin;

public:
    MoistureSensor(byte pin)
    {
        this->pin = pin;
        Init();
    }
    MoistureSensor(byte pin, byte enablePin)
    {
        this->pin = pin;
        this->enablePin = enablePin;
        Init();
    }

    void Init()
    {
        pinMode(pin, INPUT);
        pinMode(enablePin, OUTPUT);
        Off();
    }

    void On()
    {
        digitalWrite(enablePin, HIGH);
    }

    void Off()
    {
        digitalWrite(enablePin, LOW);
    }

    float GetMoisture()
    {
        float moisture = 0;
        moisture = analogRead(0) * 100 / 1024;
        delay(50);
        moisture = analogRead(0) * 100 / 1024;
        Serial.print("Wilgotnosc: ");
        Serial.println(moisture);
        return moisture;
    }
};