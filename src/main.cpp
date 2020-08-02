#include <Arduino.h>

#define PIN_ADC (A0)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#include <secrets.h>

// uptime calculation
#include <uptime.h>
Uptime uptime;

// Statistics Helper-Class
#include <circularbuffer.h>
Circularbuffer rawSeries(15U);

// Flow control, basic task scheduler
#define SCHEDULER_MAIN_LOOP_MS (10) // ms
uint32_t counterBase = 0;
uint32_t counter300s = 0;
uint32_t counter1h = 0;
uint32_t initStage = 0;

uint32_t uptimeSeconds = 0;

char byteBuffer[100];
char textBuffer[100];

void ICACHE_RAM_ATTR onTimerISR()
{
  uptimeSeconds++;
  timer1_write(312500U); // 1s
}

void setup()
{
  // LED output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  initStage++;

  // Setup serial connection for debugging
  Serial.begin(115200U);
  delay(500);
  Serial.println();
  Serial.println("[  INIT  ] Begin");
  initStage++;

  //Initialize uptime calculation
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(312500U); // 1s
  initStage++;

  //connect to your local wi-fi network
  Serial.printf("[  INIT  ] Connecting to Wifi '%s'", secrets.wifiSsid);
  WiFi.begin(secrets.wifiSsid, secrets.wifiPassword);

  //check wi-fi is connected to wi-fi network
  int retries = 5;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    retries--;
    if (retries <= 0)
    {
      ESP.restart();
    }
  }
  Serial.print(" connected!");
  Serial.print(" (IP=");
  Serial.print(WiFi.localIP());
  Serial.println(")");
  initStage++;

  // Clock setup
  Serial.println("[  INIT  ] Clock synchronization");
  configTime(0, 0, secrets.ntpServer);
  delay(200); // wait for ntp-sync

  // set timezone and DST
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0", 1); //"Europe/Berlin"  from: http://www.famschmid.net/timezones.html
  tzset();                                     // Assign the local timezone from setenv

  tm *tm = uptime.getTime();
  Serial.printf("[  INIT  ] Current time is: %02d.%02d.%04d %02d:%02d:%02d\n", tm->tm_mday, tm->tm_mon, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
  initStage++;


  Serial.print("[  INIT  ] Connecting to MQTT-Server... ");
  mqttClient.setServer(secrets.mqttServer, 1883);
  Serial.println("ok");
  initStage++;

  Serial.printf("[  INIT  ] Completed at stage %u\n\n", initStage);
}

void reconnect()
{
  Serial.print("[  MQTT  ] Attempting MQTT connection... ");
  if (mqttClient.connect(WiFi.hostname().c_str()))
  {
    Serial.println("connected");
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
  }
}

void loop()
{
  // 100ms Tasks
  if (!(counterBase % (100L / SCHEDULER_MAIN_LOOP_MS)))
  {
    digitalWrite(LED_BUILTIN, HIGH); // regularly turn on LED
    mqttClient.loop();
  }

  // 500ms Tasks
  if (!(counterBase % (500L / SCHEDULER_MAIN_LOOP_MS)))
  {
  }

  // 2s Tasks
  if (!(counterBase % (2000L / SCHEDULER_MAIN_LOOP_MS)))
  {
    // indicate alive
    digitalWrite(LED_BUILTIN, LOW);
    rawSeries.push(analogRead(PIN_ADC));
  }

  // 30s Tasks
  if (!(counterBase % (30000L / SCHEDULER_MAIN_LOOP_MS)))
  {
    Serial.printf("[ STATUS ] Free: %u KiB (%u KiB)  Uptime: %us\n",
                  ESP.getFreeHeap() / 1024,
                  ESP.getMaxFreeBlockSize() / 1024,
                  uptimeSeconds);

    if (!mqttClient.connected())
    {
      reconnect();
    }

    int len = 0;
    float mean = rawSeries.mean();
    float temp = 0.112 * mean + 212.832 - 273.15;

    len = snprintf(textBuffer, sizeof(textBuffer), "{\"raw\": %.1f, \"value\": %.1f, \"timestamp\": %u, \"unit\": \"\u00b0C\"}", mean, temp, uptime.getSeconds());
    mqttClient.publish("home/out/temp", textBuffer, len);

    len = snprintf(textBuffer, sizeof(textBuffer), "%.1f", temp);
    mqttClient.publish("home/out/temp/value", textBuffer, len);
  }

  // 300s Tasks
  if (!(counterBase % (300000L / SCHEDULER_MAIN_LOOP_MS)))
  {
    counter300s++;
  }

  // 1h Tasks
  if (!(counterBase % (3600000L / SCHEDULER_MAIN_LOOP_MS)))
  {
    counter1h++;
  }

  delay(SCHEDULER_MAIN_LOOP_MS);
  counterBase++;
}