#include <Arduino.h>

#define PIN_ADC (A0)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#include <secrets.h>

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

    int len = 0;
    int raw = analogRead(PIN_ADC);

    float currentTemperatureCelsius = 0;


    float U2 = float(raw) / (1023.-5.);

    float U4 = U2 / 100. * (330 + 220) + U2;
    // Serial.printf("raw=%u, U2=%.2f U4=%.2f",raw, U2, U4);

    len = snprintf(textBuffer, sizeof(textBuffer), "{\"raw\": %u, \"U2\": %.2f, \"U4\": %.2f}", raw, U2, U4);
    mqttClient.publish("home/test", textBuffer, len);
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