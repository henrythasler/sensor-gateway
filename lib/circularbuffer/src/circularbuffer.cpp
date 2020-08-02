#include <circularbuffer.h>

Circularbuffer::Circularbuffer(uint32_t maxLength)
{
  maxHistoryLength = maxLength;
}

void Circularbuffer::push(float value)
{
  if (data.size() >= maxHistoryLength)
  {
    data.erase(data.begin());
  }
  data.push_back(value);
}

float Circularbuffer::mean()
{
  float mean = 0;

  if (data.size())
  {
    for (float &p : data)
    {
      mean += p;
    }
    mean = mean / data.size();
  }
  return mean;
}

uint32_t Circularbuffer::size()
{
  return data.size();
}

uint32_t Circularbuffer::capacity()
{
  return data.capacity();
}