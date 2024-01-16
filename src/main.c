#include <stdio.h>
#include <FreeRTOS.h> 
#include <task.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <hardware/gpio.h>
#include <hardware/watchdog.h>
#include <hardware/timer.h>
#include "dht11_driver.h"

#define DHT11_PIN 15
#define DHT11_DELAY_MS 1000
#define DHT11_WATCHDOG_ADDITIONAL_DELAY_MS 10 


TaskHandle_t print_sensor_data_task = NULL;

void request_sensor_data(void *paramster);
void print_sensor_data(void *paramster);
bool init_all();


void request_sensor_data(void *paramster)
{
    const uint delay = DHT11_DELAY_MS / portTICK_PERIOD_MS;

    while (true) 
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_HIGH);
        dht11_request_data(DHT11_PIN, print_sensor_data_task);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, GPIO_LOW);

        watchdog_update();
        vTaskDelay(delay);
    }
}

void print_sensor_data(void *paramster)
{
    while (true)
    {
        vTaskSuspend(print_sensor_data_task);

        if(dht11_state.sensor_data.is_valid)
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
    }
}


bool init_all()
{
    if(!stdio_init_all())
    {
        printf("stdio init failed\n");
        return false;
    }
    
    printf("FreeRTOS SMP started\n");

    if (cyw43_arch_init() != PICO_OK) 
    {
        printf("CYW43 arch init failed\n");
        return false;
    }

    gpio_init(DHT11_PIN);

    return true;
}

int main() 
{
    if(watchdog_caused_reboot())
    {
        printf("Timeout reached");
    }

    if(!init_all()) 
    {
        return -1;
    }

    xTaskCreate(
        request_sensor_data,
        "request_sensor_data",
        configMINIMAL_STACK_SIZE,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        print_sensor_data, 
        "print sensor data", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        2, 
        &print_sensor_data_task
    );

    watchdog_enable(DHT11_DELAY_MS + DHT11_WATCHDOG_ADDITIONAL_DELAY_MS, true);
    vTaskStartScheduler();   
 
    return 0;
}