#ifndef SENSOR_H
#define SENSOR_H

#include <math.h> 

class Sensor
{
public:
  Sensor();
  float linearRawToCelsius(float raw);
  float modelRawToCelsius(float raw);
};
#endif