#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h> 
#include <task.h>
#include <pico/stdlib.h>
#include <hardware/timer.h>

#define GPIO_HIGH true
#define GPIO_LOW false
#define DHT11_LOW_REQUEST_MS 20
#define DHT11_HIGH_REQUEST_US 20
#define DHT11_ZERO_BIT_LIMIT_US 53
#define DHT11_DATA_BITS_COUNT 40


typedef struct dht11_data_t
{
    uint8_t integral_RH;
    uint8_t decimal_RH;
    uint8_t integral_T;
    uint8_t decimal_T;
    uint8_t checksum;
    bool is_valid;
}
dht11_data_t;

typedef enum dht11_fsm_state_t
{
    DHT11_FSM_IDLE,
    DHT11_FSM_STARTED,
    DHT11_FSM_LOW_REQUEST_SENT,
    DHT11_FSM_HIGH_REQUEST_SENT,
    DHT11_FSM_LOW_RESPONSE_RECEIVED,
    DHT11_FSM_HIGH_RESPONSE_RECEIVED,
    DHT11_FSM_DATA_RECEIVED,
    DHT11_FSM_DATA_CHECKED
}
dht11_fsm_state_t;


typedef enum dht11_fsm_trigger_t
{
    DHT11_FSM_TRIGGER_START,
    DHT11_FSM_TRIGGER_LOW_REQUEST_START,
    DHT11_FSM_TRIGGER_LOW_REQUEST_END,
    DHT11_FSM_TRIGGER_HIGH_REQUEST_START,
    DHT11_FSM_TRIGGER_HIGH_REQUEST_END,
    DHT11_FSM_TRIGGER_EDGE_FALL,
    DHT11_FSM_TRIGGER_EDGE_RISE,
    DHT11_FSM_TRIGGER_STOP,
}
dht11_fsm_trigger_t;

typedef struct dht11_state_t
{
    dht11_data_t sensor_data;
    uint received_bits;
    uint64_t last_signal_time;
    dht11_fsm_state_t fsm;
    uint dht_pin;
    TaskHandle_t callback_task;
}
dht11_state_t;
extern dht11_state_t dht11_state;

bool dht11_is_valid_data(const dht11_data_t* pdata);
void dht11_set_bit(const uint bit_value);

int64_t dht11_low_request_alarm_callback(alarm_id_t id, void *user_data);
int64_t dht11_high_request_alarm_callback(alarm_id_t id, void *user_data);
void dht11_irq_callback(uint gpio, uint32_t event_mask);

void dht11_sfm_start(dht11_fsm_trigger_t trigger);
void dht11_sfm_send_low_request(dht11_fsm_trigger_t trigger);
void dht11_sfm_send_high_request(dht11_fsm_trigger_t trigger);
void dht11_sfm_receive_low_response(dht11_fsm_trigger_t trigger);
void dht11_sfm_receive_high_response(dht11_fsm_trigger_t trigger);
void dht11_sfm_receive_bit_value(dht11_fsm_trigger_t trigger);
void dht11_sfm_check_data(dht11_fsm_trigger_t trigger);
void dht11_sfm_stop(dht11_fsm_trigger_t trigger);

void dht11_sfm_routine(dht11_fsm_trigger_t trigger);
void dht11_request_data(const uint dht_pin, TaskHandle_t callback_task);


#endif //DHT11_DRIVER_H