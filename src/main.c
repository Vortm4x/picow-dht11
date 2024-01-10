#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <hardware/gpio.h>
#include <hardware/watchdog.h>

#include "dht11_driver.h"

int main() 
{
    if(watchdog_caused_reboot())
    {
        printf("Timeout reached");
    }

    watchdog_enable(1100, true);

    const uint DHT_PIN = 15;

    stdio_init_all();
    
    if (cyw43_arch_init()) 
    {
        printf("Arch init failed");
        return -1;
    }

    gpio_init(DHT_PIN);
    
    while (true) 
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_HIGH);

        dht11_request_data(DHT_PIN);
        while(!dht11_state.data_received);

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_LOW);


        if(dht11_is_valid_data(&dht11_state.sensor_data))
        {
            printf("HUMIDITY: %d.%d\tTEMP: %d.%d\n",
                dht11_state.sensor_data.integral_RH,
                dht11_state.sensor_data.decimal_RH,
                dht11_state.sensor_data.integral_T,
                dht11_state.sensor_data.decimal_T
            );
        }
        else
        {
            printf("INVALID DATA!\n");
        }

        watchdog_update();
        
        sleep_ms(1000);
    }
}