#include "dht11_driver.h"


void dht11_request(const uint dht_pin)
{
    gpio_set_dir(dht_pin, GPIO_OUT);

    gpio_put(dht_pin, GPIO_LOW);
    sleep_ms(20);

    gpio_put(dht_pin, GPIO_HIGH);
    sleep_us(20);
}

void dht11_response(const uint dht_pin)
{
    gpio_set_dir(dht_pin, GPIO_IN);
    
    while (gpio_get(dht_pin) == GPIO_HIGH);
    while (gpio_get(dht_pin) == GPIO_LOW);
    while (gpio_get(dht_pin) == GPIO_HIGH);
}

uint8_t dht11_receive_value(const uint dht_pin)
{
    uint8_t value = 0;

    for(int i = 0; i < 8; ++i)
    {
        while (gpio_get(dht_pin) == GPIO_LOW);
        sleep_us(30);
        
        value <<= 1;
        if(gpio_get(dht_pin) == GPIO_HIGH)
        {
            value |= 0x01;
        }

        while (gpio_get(dht_pin) == GPIO_HIGH);
    }

    return value;
}

bool dht11_is_valid_data(const dht11_data_t* pdata)
{
    return (
        pdata->integral_RH + 
        pdata->decimal_RH + 
        pdata->integral_T + 
        pdata->decimal_T
    ) == pdata->checksum;
}

void dht11_get_data(const uint dht_pin, dht11_data_t* pdata)
{
    dht11_request(dht_pin);
    dht11_response(dht_pin);

    pdata->integral_RH = dht11_receive_value(dht_pin);
    pdata->decimal_RH = dht11_receive_value(dht_pin);
    pdata->integral_T = dht11_receive_value(dht_pin);
    pdata->decimal_T = dht11_receive_value(dht_pin);

    pdata->checksum = dht11_receive_value(dht_pin);
}