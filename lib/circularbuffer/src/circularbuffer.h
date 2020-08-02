#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdint.h>
#include <vector>

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