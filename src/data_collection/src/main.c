
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

//Add here necessary states
enum state { IDLE=1, RECEIVE=2};
enum state programState = IDLE;
float data[100][6];

//Prints the data in a nice and parseable form
//Each tick's dataset on its own line, each datapoint separated by a ,
void printData() {

    for (int set = 0; set < 100; set++) {
        for (int point = 0; point < 6; point++) {

            if (point == 5) { 
                printf("%f\n", data[set][point]); 
            }

            else {
                printf("%f,", data[set][point]);
            }
        }
    }
    //Stop data collection
    programState = IDLE;
}

//Press the button to start collecting data
static void button_interrupt(uint gpio, uint32_t eventMask) {
      toggle_led();
    if (programState == IDLE) {
        printf("\n****Starting data collection****\n");
        programState = RECEIVE;
    }
    else {
        programState = IDLE;
    }
}

static void IMU_task(void *arg) {
    (void) arg;
    float t = 0;
    int head = 0;
    int time = 0;
    while (1) {

        while (programState == RECEIVE) {

            if (ICM42670_read_sensor_data(&data[head][0], &data[head][1], &data[head][2], &data[head][3], 
                &data[head][4], &data[head][5], &t) == 0) {
                
                //print momentary data
                printf("Tick: %d | Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f | Temp: %2.2f°C\n", time, data[head][0], data[head][1], 
                    data[head][2], data[head][3], data[head][4], data[head][5], t);
            
                //if there is still left in databuffer
                if (head + 1 < 100) {
                    head++;
                }
                //print all data in easy to parse form
                else {
                    printData();
                }
                time++;
            } else {
                printf("Failed to read imu data\n");
            }
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        printf("Standby..\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main() {
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    while (!stdio_usb_connected()){
        sleep_ms(10);
    } 
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.
    
    init_i2c(DEFAULT_I2C_SDA_PIN, DEFAULT_I2C_SCL_PIN);

    //Initialize IMU
    if (init_ICM42670() == 0) {
        if (ICM42670_start_with_default_values() != 0) {
            printf("ICM initialization failed!\n");
        }
    }

    //Initialize buttons and led
    init_button1();
    init_led();    

    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, button_interrupt);

    TaskHandle_t myIMUtask = NULL;
    // Create the tasks with xTaskCreate
    
    BaseType_t result = xTaskCreate(IMU_task,
                "IMU",
                DEFAULT_STACK_SIZE,
                NULL,
                2,
                &myIMUtask);
    
    if (result != pdPASS) {
        printf("IMU task creation failed");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

