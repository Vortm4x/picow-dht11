#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <pico/cyw43_arch.h>

#include "dht11_driver.h"

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
    dht11_data_t sensor_data;
    
    while (true) 
    {
        memset(&sensor_data, 0, sizeof(dht11_data_t));

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_HIGH);
        dht11_get_data(DHT_PIN, &sensor_data);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_LOW);

        if(dht11_is_valid_data(&sensor_data))
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
        
        sleep_ms(1000);
    }
}