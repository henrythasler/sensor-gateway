#include <unity.h>
#include <circularbuffer.h>

#ifndef ARDUINO
#include <stdio.h>
#include <chrono>
#endif

typedef std::vector<float> float_vec_t;

void test_function_timeseries_initial(void)
{
  Circularbuffer series;
  TEST_ASSERT_EQUAL_FLOAT(0, series.mean());
  TEST_ASSERT_EQUAL_UINT32(0, series.size());
}

void test_function_timeseries_single(void)
{
  Circularbuffer series(8);
  series.push(2.);
  TEST_ASSERT_EQUAL_FLOAT(2., series.mean());
  TEST_ASSERT_EQUAL_UINT32(1, series.size());
}

void test_function_timeseries_simple(void)
{
  Circularbuffer series(8);
  series.push(0.);
  series.push(1.);
  TEST_ASSERT_EQUAL_FLOAT(.5, series.mean());
  TEST_ASSERT_EQUAL_UINT32(2, series.size());
}

void test_function_timeseries_limiter(void)
{
  Circularbuffer series(4);
  series.push(0);
  series.push(1);
  series.push(2);
  series.push(3);
  series.push(4);
  series.push(5);

  TEST_ASSERT_EQUAL_FLOAT_MESSAGE(3.5, series.mean(), "series.mean()");
  TEST_ASSERT_EQUAL_UINT32(4, series.size());

  TEST_ASSERT_EQUAL_FLOAT(2., series.data.at(0));
  TEST_ASSERT_EQUAL_FLOAT(5., series.data.at(3));
}

void test_function_timeseries_huge(void)
{
  int maxEntries = 20000;
#ifdef ARDUINO
  maxEntries = 5000;
#endif

  Circularbuffer series(maxEntries);
  for (int i = 0; i < 50000; i++)
  {
    series.push(i % 100);
  }
  TEST_ASSERT_EQUAL_FLOAT(49.5, series.mean());
  TEST_ASSERT_EQUAL_UINT32(maxEntries, series.size());

#ifndef ARDUINO
  char message[64];
  int size = sizeof(series.data) + sizeof(float) * series.data.capacity();
  snprintf(message, 64, "size=%u", size);
  TEST_MESSAGE(message);
#endif
}


void process(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_function_timeseries_initial);
  RUN_TEST(test_function_timeseries_single);
  RUN_TEST(test_function_timeseries_simple);
  RUN_TEST(test_function_timeseries_limiter);
  RUN_TEST(test_function_timeseries_huge);
  UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
#define LED_BUILTIN (13) // LED is connected to IO13

void setup()
{
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.printf("Heap: %u KiB free\n", ESP.getFreeHeap() / 1024);
  process();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}

#else

int main(int argc, char **argv)
{
  auto start = std::chrono::high_resolution_clock::now();
  process();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  printf("Elapsed time: %lums\n", duration.count() / 1000);
  return 0;
}
#endif