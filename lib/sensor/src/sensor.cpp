#include <sensor.h>

Sensor::Sensor(void)
{
}

float Sensor::linearRawToCelsius(float raw)
{
    return 0.1046 * raw + 218.72 - 273.15;
};

float Sensor::modelRawToCelsius(float raw)
{
    float B = 3528.01, R_N = 1000, T_N = 298.15;
    float a = 2644974.581, b = -2481.087;

    float RT = a / raw + b;
    return B * T_N / (B + log(RT / R_N) * T_N) - 273.15;
};
