#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <sys/time.h>
#include <algorithm>

using namespace std;

class Circularbuffer
{
public:
  uint32_t maxHistoryLength;

  vector<float> data;

  Circularbuffer(uint32_t maxLength = 32);

  void push(float value);
  uint32_t size();
  uint32_t capacity();
  float mean();
};
#endif