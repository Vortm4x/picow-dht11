#include <stdio.h>
#include <math.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <pico/cyw43_arch.h>

#define GPIO_HIGH true
#define GPIO_LOW false

typedef struct dht11_data
{
    uint8_t integral_RH;
    uint8_t decimal_RH;
    uint8_t integral_T;
    uint8_t decimal_T;
    uint8_t checksum;
} 
dht11_data, *pdht11_data;

void dht11_request(const uint dht_pin);
void dht11_response(const uint dht_pin);
uint8_t dht11_receive_value(const uint dht_pin);
bool dht11_is_valid_data(const pdht11_data pdata);
bool dht11_get_data(const uint dht_pin, pdht11_data pdata);

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

bool dht11_is_data_valid(const pdht11_data pdata)
{
    return (
        pdata->integral_RH + 
        pdata->decimal_RH + 
        pdata->integral_T + 
        pdata->decimal_T
    ) == pdata->checksum;
}

bool dht11_get_data(const uint dht_pin, pdht11_data pdata)
{
    dht11_request(dht_pin);
    dht11_response(dht_pin);

    pdata->integral_RH = dht11_receive_value(dht_pin);
    pdata->decimal_RH = dht11_receive_value(dht_pin);
    pdata->integral_T = dht11_receive_value(dht_pin);
    pdata->decimal_T = dht11_receive_value(dht_pin);

    pdata->checksum = dht11_receive_value(dht_pin);

    return dht11_is_data_valid(pdata);
}


int main() 
{   
    const uint DHT_PIN = 15;

    stdio_init_all();
    
    if (cyw43_arch_init()) 
    {
        printf("Arch init failed");
        return -1;
    }

    gpio_init(DHT_PIN);
    dht11_data sensor_data;
    
    while (1) 
    {
        memset(&sensor_data, 0, sizeof(dht11_data));

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_HIGH);

        if(dht11_get_data(DHT_PIN, &sensor_data))
        {
            printf("HUMIDITY: %d.%d\tTEMP: %d.%d\n",
                sensor_data.integral_RH,
                sensor_data.decimal_RH,
                sensor_data.integral_T,
                sensor_data.decimal_T
            );
        }
        else
        {
            printf("INVALID DATA!\n");
        }
        
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_LOW);

        sleep_ms(2000);
    }
}