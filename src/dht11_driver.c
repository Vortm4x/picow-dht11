#include "dht11_driver.h"

dht11_state_t dht11_state;

void dht11_append_bit(uint8_t* num, const bool bit_value)
{
    *num <<= 1;

    if(bit_value)
    {
        *num |= 0x01;
    }
}

void dht11_irq_callback(uint gpio, uint32_t event_mask)
{
    if(gpio_get_dir(gpio) != GPIO_IN) return;

    uint64_t current_time = time_us_64();

    if(dht11_state.low_response_received && dht11_state.high_response_received)
    {
        if(gpio_get(gpio) == GPIO_LOW)
        {
            if(dht11_state.received_bits < 40)
            {
                uint64_t signal_time = current_time - dht11_state.last_signal_time;
                bool bit_value = (signal_time > 53);

                switch (dht11_state.received_bits >> 3)
                {
                    case 0:
                        dht11_append_bit(&dht11_state.sensor_data.integral_RH, bit_value);
                        break;
                    case 1:
                        dht11_append_bit(&dht11_state.sensor_data.decimal_RH, bit_value);
                        break;
                    case 2:
                        dht11_append_bit(&dht11_state.sensor_data.integral_T, bit_value);
                        break;
                    case 3:
                        dht11_append_bit(&dht11_state.sensor_data.decimal_T, bit_value);
                        break;
                    case 4:
                        dht11_append_bit(&dht11_state.sensor_data.checksum, bit_value);
                        break;
                    default:
                        break;
                }

                ++dht11_state.received_bits;
            }
        }
        else if(dht11_state.received_bits == 40)
        {
            dht11_state.data_received = true;

            gpio_set_irq_enabled(gpio, event_mask, false);
        }
    }
    else
    {
        if(gpio_get(gpio) == GPIO_LOW)
        {
            dht11_state.low_response_received = true;
        }
        else if(dht11_state.low_response_received)
        {
            dht11_state.high_response_received = true;
        }
    }

    dht11_state.last_signal_time = current_time;
}

void dht11_request_data(const uint dht_pin)
{
    memset(&dht11_state, 0, sizeof(dht11_state_t));
    gpio_set_irq_enabled_with_callback(
        dht_pin, 
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        dht11_irq_callback
    );

    dht11_request(dht_pin);
    gpio_set_dir(dht_pin, GPIO_IN);
}

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