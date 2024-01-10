#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <stdint.h>
#include <pico/stdlib.h>

#define GPIO_HIGH true
#define GPIO_LOW false

typedef struct dht11_data_t
{
    uint8_t integral_RH;
    uint8_t decimal_RH;
    uint8_t integral_T;
    uint8_t decimal_T;
    uint8_t checksum;
}
dht11_data_t;

void dht11_request(const uint dht_pin);
void dht11_response(const uint dht_pin);
uint8_t dht11_receive_value(const uint dht_pin);
bool dht11_is_valid_data(const dht11_data_t* pdata);
void dht11_get_data(const uint dht_pin, dht11_data_t* pdata);

#endif //DHT11_DRIVER_H