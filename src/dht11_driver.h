#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <stdint.h>
#include <string.h>
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

typedef struct dht11_state_t
{
    dht11_data_t sensor_data;
    uint received_bits;
    uint64_t last_signal_time;
    bool low_response_received;
    bool high_response_received;
    bool data_received;
}
dht11_state_t;
extern dht11_state_t dht11_state;

void dht11_append_bit(uint8_t* num, const bool bit_value);
void dht11_irq_callback(uint gpio, uint32_t event_mask);
void dht11_request_data(const uint dht_pin);

void dht11_request(const uint dht_pin);
void dht11_response(const uint dht_pin);
uint8_t dht11_receive_value(const uint dht_pin);
bool dht11_is_valid_data(const dht11_data_t* pdata);
void dht11_get_data(const uint dht_pin, dht11_data_t* pdata);

#endif //DHT11_DRIVER_H