#include "main.h"
#include <Ticker.h>
#include <ERa.hpp>

void ERa_Setup();
void timerEvent();
bool objectDetecter0();
bool objectDetecter1();
bool objectDetecter2();
void tick();
void servoHandler(uint8_t servo, uint8_t position);
void pickObjectFunction(uint8_t gate);
void tcsSetup();

Preferences preferences;
Servo servo1, servo2;
Ticker ticker(tick, 100, 0, MILLIS);
WiFiClient mbTcpClient;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_240MS, TCS34725_GAIN_1X);

bool sensFlag1, sensFlag2 = false;
bool gate1_learn, gate2_learn = false;
uint8_t gate1_task, gate2_task, gate3_task = 0;
bool gate1_trigger, gate2_trigger, gate3_trigger = false;
uint16_t counterAll, counter1, counter2, counter3 = 0;
ERaString current_Type = 0;
boolean motorStatus = false;
float r_tmp, g_tmp, b_tmp = 0;
float r1, g1, b1 = 0;
float r2, g2, b2 = 0;
uint8_t color_deviation = 13;

void setup()
{
  pinMode(PIN_LED, OUTPUT); // đặt mode cho pinout
  pinMode(PIN_SERVO1, OUTPUT);
  pinMode(PIN_SERVO2, OUTPUT);
  pinMode(PIN_SEN0, INPUT);
  pinMode(PIN_SEN1, INPUT);
  pinMode(PIN_SEN2, INPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_MOTOR, OUTPUT);

  Serial.begin(115200); // Khởi tạo Serial
  preferences.begin("eep-db", false);

  MOTOR_OFF();
  ticker.start();
  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);
  servoHandler(0, 0); // Set servo về vị trí mặc địnhđịnh

  r1 = preferences.getFloat("R1", 0);
  g1 = preferences.getFloat("G1", 0);
  b1 = preferences.getFloat("B1", 0);
  r2 = preferences.getFloat("R2", 0);
  g2 = preferences.getFloat("G2", 0);
  b2 = preferences.getFloat("B2", 0);
  counter1 = preferences.getUShort("S1", 0);
  counter2 = preferences.getUShort("S2", 0);
  counter3 = preferences.getUShort("S3", 0);
  counterAll = preferences.getUShort("SA", 0);
  color_deviation = preferences.getUChar("cd", 13);

  Serial.println("Loadinging WiFi info...");
  if ((preferences.getString("ssid", "") != "") && (preferences.getString("password", "") != ""))
  {
    Serial.println("Connecting WiFi info...");
    WiFi.begin(preferences.getString("ssid", ""), preferences.getString("password", ""));
    uint8_t timeCounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      tick();
      delay(500);
      Serial.print(".");
      if (++timeCounter > 40)
      {
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("\nWiFi connection failed");
    }
    else
    {
      Serial.println("\nWiFi connected successfully");
    }
  }
  else
  {
    Serial.println("No stored WiFi info!");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    wifiServerSetup();
    while (WiFi.status() != WL_CONNECTED)
    {
      ticker.update();
      wifiServerHandle();
      if ((millis() % 5000 == 0))
      {
        Serial.println("\n========================\n");
        Serial.print("Access Point: ");
        Serial.print(ssid_ap);
        if (password_ap != "")
        {
          Serial.print(" - ");
          Serial.println(password_ap);
        }
        else
        {
          Serial.println();
        }
        Serial.print("Client num: ");
        Serial.println(WiFi.softAPgetStationNum());
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("Port: ");
        Serial.println(port);
      }
    }
  }

  ERa_Setup();
  tcsSetup();
}

void loop()
{
  ticker.update();
  if (WiFi.status() == WL_CONNECTED)
  {
    ticker.pause();
    LED_ON();

    ERa.run();

    ///////////////////////////
    if ((gate1_learn == false) && (gate2_learn == false))
    {
      pickObjectFunction(0);
      pickObjectFunction(1);
      pickObjectFunction(2);
      pickObjectFunction(3);
      counterAll = counter1 + counter2 + counter3;
    }
    else if (gate1_learn == true)
    {
      uint16_t tmp;
      motorStatus = true;
      if (objectDetecter0() == true)
      {
        MOTOR_OFF();
        motorStatus = false;
        delay(500);
        tcs.getRGB(&r1, &g1, &b1);
        preferences.putFloat("R1", r1);
        preferences.putFloat("G1", g1);
        preferences.putFloat("B1", b1);
        gate1_learn = false;
      }
    }
    else if (gate2_learn == true)
    {
      uint16_t tmp;
      motorStatus = true;
      if (objectDetecter0() == true)
      {
        MOTOR_OFF();
        motorStatus = false;
        delay(500);
        tcs.getRGB(&r2, &g2, &b2);
        preferences.putFloat("R2", r2);
        preferences.putFloat("G2", g2);
        preferences.putFloat("B2", b2);
        gate2_learn = false;
      }
    }
    else
    {
      // Donothing
    }
  }
  else
  {
    Serial.println("Lost connection");
    if (ticker.state() != RUNNING)
    {
      ticker.resume();
    }
    if (ticker.interval() != 1000)
    {
      ticker.interval(1000);
    }
  }

  if (motorStatus == true)
  {
    MOTOR_ON();
  }
  else
  {
    MOTOR_OFF();
  }
}

// This function is called every time the Virtual Pin 0 state changes
ERA_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.getInt();
  if (value == 1)
  {
    motorStatus = true;
    // Serial.println("ERa: Motor On");
  }
  else
  {
    motorStatus = false;
    // Serial.println("ERa: Motor Off");
    gate1_task = 0;
    gate1_trigger = false;
    gate2_task = 0;
    gate2_trigger = false;
    gate1_learn = false;
    gate2_learn = false;
  }
}
ERA_CONNECTED()
{
  ERa.virtualWrite(V4, color_deviation);
}

ERA_WRITE(V1)
{
  int value1 = param.getInt();
  if (value1 == 1 && gate1_learn == false)
  {
    gate1_learn = true;
    // Serial.println("ERa: Learn 1");
  }
  else
  {
    gate1_learn = false;
  }
}

ERA_WRITE(V2)
{
  int value2 = param.getInt();
  if (value2 == 1 && gate2_learn == false)
  {
    gate2_learn = true;
    // Serial.println("ERa: Learn 2");
  }
  else
  {
    gate2_learn = false;
  }
}

ERA_WRITE(V3)
{
  int value3 = param.getInt();
  if (value3 == 1)
  {
    counter1 = 0;
    counter2 = 0;
    counter3 = 0;
    counterAll = 0;
  }
  ERa.virtualWrite(V3, 0);
}

ERA_WRITE(V4)
{
  // Set incoming value from pin V2 to a variable
  int value4 = param.getInt();
  color_deviation = value4;
  preferences.putUChar("cd", color_deviation);
}

ERA_WRITE(V9)
{
  int value9 = param.getInt();
  if (value9 == 1)
  {
    tcs.getRGB(&r_tmp, &g_tmp, &b_tmp);
    ERa.virtualWrite(V9, 0);
  }
}

void timerEvent()
{
  ERa.virtualWrite(V0, motorStatus);
  ERa.virtualWrite(V1, gate1_learn);
  ERa.virtualWrite(V2, gate2_learn);
  ERa.virtualWrite(V10, ERaString(ERaString((uint8_t)r1) + "-" + ERaString((uint8_t)g1) + "-" + ERaString((uint8_t)b1)));
  ERa.virtualWrite(V11, ERaString(ERaString((uint8_t)r2) + "-" + ERaString((uint8_t)g2) + "-" + ERaString((uint8_t)b2)));
  ERa.virtualWrite(V12, ERaString(ERaString((uint8_t)r_tmp) + "-" + ERaString((uint8_t)g_tmp) + "-" + ERaString((uint8_t)b_tmp)));
  ERa.virtualWrite(V13, current_Type);
  ERa.virtualWrite(V15, counterAll);
  ERa.virtualWrite(V16, counter1);
  ERa.virtualWrite(V17, counter2);
  ERa.virtualWrite(V18, counter3);
  ERa.virtualWrite(V21, (millis() / 1000));

  preferences.putUShort("S1", counter1);
  preferences.putUShort("S2", counter2);
  preferences.putUShort("S3", counter3);
  preferences.putUShort("SA", counterAll);
}

void ERa_Setup()
{
  ERa.setModbusClient(mbTcpClient);
  ERa.setScanWiFi(true);
  ERa.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Setup a function to be called every second
  ERa.addInterval(1000L, timerEvent);
}

bool objectDetecter0()
{
  if (digitalRead(PIN_SEN0) == 0)
  {
    delay(15);
    if (digitalRead(PIN_SEN0) == 0)
    {
      return true;
    }
  }

  return false;
}

bool objectDetecter1()
{
  static int dt11, dt12;
  static bool retVal1;

  if (sensFlag1)
  {
    if ((digitalRead(PIN_SEN1) == 1) && ((millis() - dt11) > 700))
    {
      sensFlag1 = false;
      retVal1 = false;
    }
    else if (digitalRead(PIN_SEN1) == 0)
    {
      dt11 = millis();
    }
  }
  else
  {
    if ((digitalRead(PIN_SEN1) == 0) && ((millis() - dt12) > 20))
    {
      sensFlag1 = true;
      retVal1 = true;
    }
    else if (digitalRead(PIN_SEN1) == 1)
    {
      dt12 = millis();
    }
  }
  return retVal1;
}

bool objectDetecter2()
{
  static int dt21, dt22;
  static bool retVal2;
  if (sensFlag2)
  {
    if ((digitalRead(PIN_SEN2) == 1) && ((millis() - dt21) > 700))
    {
      sensFlag2 = false;
      retVal2 = false;
    }
    else if (digitalRead(PIN_SEN2) == 0)
    {
      dt21 = millis();
    }
  }
  else
  {
    if ((digitalRead(PIN_SEN2) == 0) && ((millis() - dt22) > 20))
    {
      sensFlag2 = true;
      retVal2 = true;
    }
    else if (digitalRead(PIN_SEN2) == 1)
    {
      dt22 = millis();
    }
  }
  return retVal2;
}

void pickObjectFunction(uint8_t gate)
{
  switch (gate)
  {
  case 0:
  {
    if (objectDetecter0() == true)
    {
      if (gate1_task == 0 && gate2_task == 0 && gate3_task == 0 && motorStatus == true)
      {
        MOTOR_OFF();
        motorStatus = false;
        delay(500);
        tcs.getRGB(&r_tmp, &g_tmp, &b_tmp);
        if ((r_tmp >= (r1 - color_deviation)) && (r_tmp <= (r1 + color_deviation)) && (g_tmp >= (g1 - color_deviation)) && (g_tmp <= (g1 + color_deviation)) && (b_tmp >= (b1 - color_deviation)) && (b_tmp <= (b1 + color_deviation)))
        {
          gate1_task = 1;
          // Serial.println("Type1");
          current_Type = "Type1";
        }
        else if ((r_tmp >= (r2 - color_deviation)) && (r_tmp <= (r2 + color_deviation)) && (g_tmp >= (g2 - color_deviation)) && (g_tmp <= (g2 + color_deviation)) && (b_tmp >= (b2 - color_deviation)) && (b_tmp <= (b2 + color_deviation)))
        {
          gate2_task = 1;
          // Serial.println("Type2");
          current_Type = "Type2";          
        }
        else
        {
          gate3_task = 1;
          // Serial.println("Type3");
          current_Type = "Type3";
        }
        MOTOR_ON();
        motorStatus = true;
      }
    }
    break;
  }
  case 1:
  {
    if (objectDetecter1() == true)
    {
      if (gate1_task == 1 && gate1_trigger == false)
      {
        servoHandler(1, 50);
        gate1_trigger = true;
      }
    }
    else
    {
      if (gate1_task == 1 && gate1_trigger == true)
      {
        servoHandler(1, 0);
        gate1_task = 0;
        gate1_trigger = false;
        counter1++;
      }
    }
    break;
  }
  case 2:
  {
    if (objectDetecter2() == true)
    {
      if (gate2_task == 1 && gate2_trigger == false)
      {
        servoHandler(2, 50);
        gate2_trigger = true;
      }
    }
    else
    {
      if (gate2_task == 1 && gate2_trigger == true)
      {
        servoHandler(2, 0);
        gate2_task = 0;
        gate2_trigger = false;
        counter2++;
      }
    }
    break;
  }

  case 3:
  {
    if (objectDetecter2() == true)
    {
      if (gate3_task == 1 && gate3_trigger == false)
      {
        gate3_trigger = true;
      }
    }
    else
    {
      if (gate3_task == 1 && gate3_trigger == true)
      {
        gate3_task = 0;
        gate3_trigger = false;
        counter3++;
      }
    }
    break;
  }

  default:
    break;
  }
}

void tick() // Hàm chuyển trạng thái LED
{
  // toggle state
  int state = digitalRead(PIN_LED); // get the current state of GPIO1 pin
  digitalWrite(PIN_LED, !state);    // set pin to the opposite state
}

void servoHandler(uint8_t servo, uint8_t position)
{
  if (servo == 1)
  {
    servo1.write(position);
  }
  else if (servo == 2)
  {
    servo2.write(position);
  }
  else if (servo == 0)
  {
    servo1.write(position);
    servo2.write(position);
  }
  else
  {
    // Do nothing
  }
}

void tcsSetup()
{
  if (tcs.begin())
  {
    Serial.println("TCS34725 sensor found.");
  }
  else
  {
    Serial.println("No TCS34725 found ... check your connections then restart");
    while (1)
      ;
  }
}
