#include "dht11_driver.h"

dht11_state_t dht11_state;

bool dht11_is_valid_data(const dht11_data_t* pdata)
{
    return (
        pdata->integral_RH + 
        pdata->decimal_RH + 
        pdata->integral_T + 
        pdata->decimal_T
    ) == pdata->checksum;
}

void dht11_set_bit(const uint bit_value)
{
    uint value_no = dht11_state.received_bits >> 3;

    switch (value_no)
    {
        case 0:
            dht11_state.sensor_data.integral_RH <<= 1;
            dht11_state.sensor_data.integral_RH |= bit_value;
            break;
        case 1:
            dht11_state.sensor_data.decimal_RH <<= 1;
            dht11_state.sensor_data.decimal_RH |= bit_value;
            break;
        case 2:
            dht11_state.sensor_data.integral_T <<= 1;
            dht11_state.sensor_data.integral_T |= bit_value;
            break;
        case 3:
            dht11_state.sensor_data.decimal_T <<= 1;
            dht11_state.sensor_data.decimal_T |= bit_value;
            break;
        case 4:
            dht11_state.sensor_data.checksum <<= 1;
            dht11_state.sensor_data.checksum |= bit_value;
            break;
        default:
            break;
    }
}

int64_t dht11_low_request_alarm_callback(alarm_id_t id, void *user_data)
{
    dht11_state.fsm = DHT11_FSM_LOW_REQUEST_SENT;
    dht11_sfm_routine();
    
    return 0;
}

int64_t dht11_high_request_alarm_callback(alarm_id_t id, void *user_data)
{
    gpio_set_dir(dht11_state.dht_pin, GPIO_IN);

    dht11_state.fsm = DHT11_FSM_HIGH_REQUEST_SENT;

    return 0;
}

void dht11_irq_callback(uint gpio, uint32_t event_mask)
{
    if(gpio_get_dir(gpio) == GPIO_IN)
    {
        dht11_sfm_routine(gpio);
    }    
}

void dht11_sfm_send_low_request()
{
    gpio_set_dir(dht11_state.dht_pin, GPIO_OUT);
    gpio_put(dht11_state.dht_pin, GPIO_LOW);

    add_alarm_in_ms(DHT11_LOW_REQUEST_MS, dht11_low_request_alarm_callback, NULL, true);
}

void dht11_sfm_send_high_request()
{
    gpio_put(dht11_state.dht_pin, GPIO_HIGH);
    add_alarm_in_us(DHT11_HIGH_REQUEST_US, dht11_high_request_alarm_callback, NULL, true);
}

void dht11_sfm_receive_low_response()
{
    if(gpio_get(dht11_state.dht_pin) == GPIO_HIGH)
    {
        dht11_state.fsm = DHT11_FSM_LOW_RESPONSE_RECEIVED;
    }
}

void dht11_sfm_receive_high_response()
{
    if(gpio_get(dht11_state.dht_pin) == GPIO_LOW)
    {
        dht11_state.fsm = DHT11_FSM_HIGH_RESPONSE_RECEIVED;
    }
}

void dht11_sfm_receive_bit_value()
{
    uint64_t current_time = time_us_64();

    if(gpio_get(dht11_state.dht_pin) == GPIO_LOW)
    {
        uint64_t signal_time = current_time - dht11_state.last_signal_time;
        uint bit_value = (signal_time > DHT11_ZERO_BIT_LIMIT_US) ? 0b1 : 0b0;

        dht11_set_bit(bit_value);
        ++dht11_state.received_bits;

        if(dht11_state.received_bits == DHT11_DATA_BITS_COUNT)
        {
            dht11_state.fsm = DHT11_FSM_DATA_RECEIVED;
        }
    }

    dht11_state.last_signal_time = current_time;
}

void dht11_sfm_check_data()
{
    if(gpio_get(dht11_state.dht_pin) == GPIO_HIGH)
    {
        dht11_state.sensor_data.is_valid = dht11_is_valid_data(&dht11_state.sensor_data);
        dht11_state.fsm = DHT11_FSM_DATA_CHECKED;

        gpio_set_irq_enabled(
            dht11_state.dht_pin, 
            GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
            false
        );
    }
}

void dht11_sfm_routine()
{
    switch (dht11_state.fsm)
    {
    case DHT11_FSM_START:
        dht11_sfm_send_low_request();
        break;
    case DHT11_FSM_LOW_REQUEST_SENT:
        dht11_sfm_send_high_request();
        break;
    case DHT11_FSM_HIGH_REQUEST_SENT:
        dht11_sfm_receive_low_response();
        break;
    case DHT11_FSM_LOW_RESPONSE_RECEIVED:
        dht11_sfm_receive_high_response();
        break;
    case DHT11_FSM_HIGH_RESPONSE_RECEIVED:
        dht11_sfm_receive_bit_value();
        break;
    case DHT11_FSM_DATA_RECEIVED:
        dht11_sfm_check_data();
        break;
    case DHT11_FSM_DATA_CHECKED:
        break;
    default:
        break;
    }
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

    dht11_state.dht_pin = dht_pin;
    dht11_state.fsm = DHT11_FSM_START;

    dht11_sfm_routine();  
}