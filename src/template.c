
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <morse_utils.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

//IMU setting values
#define UP_TRESHOLD 1.134 //Treshold for upwards motion detection
#define SIDE_TRESHOLD 0.222 // Treshold for sideways motion detection
#define PULSE_LENGTH 650 //Length of morse "tick" in ms, one symbol = one tick

//Message length settings
#define MESSAGE_MAX_TRESHOLD 100 // Maximum sendMessage length
#define CODE_MAX_TRESHOLD 5 // Maximum length of a morse string, the standard morse code is max 5 long

//Add here necessary states
enum state {IDLE=1, RECEIVE=2, COMPOSE = 3, SEND = 4, DRAW = 5};
enum state programState = DRAW; //Start by drawing main menu

//Menu drawing states, set before changing program state to DRAW
enum drawstate {STANDBY=0, IDLEMENU = 1, RECEIVEMENU = 2, COMPOSEMESSAGE = 3, SENDMESSAGE = 4, MORSE = 5};
enum drawstate menuState = IDLEMENU; //Start by drawing main menu

//Receive message
char receiveMessage[MESSAGE_MAX_TRESHOLD]; //A buffer used to receive a Message received from the workstation
int receiveMessageLength; //Length of the message

//Send message
char sendMessage[MESSAGE_MAX_TRESHOLD]; //Message composed with IMU
int messageIndex = -1; // Used in IMU task to compose the sendMessage
char character; // Most recent character recognized by the IMU
char parsedMessage[MESSAGE_MAX_TRESHOLD][CODE_MAX_TRESHOLD]; // Buffer for the message parser to write the final message


/**
 * Interrupt function the buttons called, used to navigate the UI
 */
static void button_interrupt(uint gpio, uint32_t eventMask) {
    //Right button
    if (gpio == BUTTON1) {
        
        if (programState == IDLE) { //Main menu state
            menuState = COMPOSEMESSAGE; //Set the menu state
            programState = DRAW;
        }

        else if (programState == COMPOSE) {
            menuState = SENDMESSAGE;
            programState = DRAW;
        }
    }
    //Left button
    else {
        if (programState == IDLE) {
            menuState = RECEIVEMENU;
            programState = DRAW;
        }
    }
}

/**
 * Parses a given morse message into an array of individual words
 */
void parse_message(char message[], char buff[MESSAGE_MAX_TRESHOLD][CODE_MAX_TRESHOLD]) {
    char code[CODE_MAX_TRESHOLD];
    int letterCount = 0;
    int codeIndex = 0;
    for (int symbol = 0; symbol != NULL; i++) {
        if (message[symbol] == ' ') {

            for (i = 0; i < CODE_MAX_TRESHOLD; i++) {
                if (code[i] == NULL) {
                    code[i] = ' ';
                }
            }
            buff[letterCount][0] = code;
        }


    }
} 

/**
 * Works more as an UI task
 * To use, set a menuState and switch programState to DRAW
 */
static void display_task(void *arg) {
    (void) arg;

    while (1) {
        while (programState == DRAW) {

            //Prints the main menu
            while (menuState == IDLEMENU) {
                clear_display();
                write_text_xy(20, 0, "Morsenator 200");
                write_text_xy(0, 32, "Receive");
                write_text_xy(84, 32, "Compose");
                menuState = STANDBY; //Wait for a new order
                programState = IDLE;
            }

            //Prints the tooltips for composing messages
            while (menuState == COMPOSEMESSAGE) {
                clear_display();
                write_text_xy(0, 0, "Sides is -, Up is .");
                write_text_xy(0, 32, "Right button to send");
                menuState = STANDBY;
                programState = COMPOSE; //Start listening to the IMU
            }
            
            //Prints morse characters as they are received by the IMU
            while (menuState == MORSE) {
                clear_display();
                if (character == '.') {
                    draw_circle(64, 32, 3, true);
                }

                else if (character == '-') {
                    draw_line(61, 32, 67, 32);
                }
                menuState = STANDBY;
                programState = COMPOSE; //Back to IMU
            }

            //Shows while sendMessage is being processed and sent
            while (menuState == SENDMESSAGE) {
                clear_display();
                write_text_xy(0, 32, "Sending Message!");
                menuState = STANDBY;
                programState = SEND;
            }
        }
    }
}

/**
 * Listens to the IMU and converts movement into morse symbols.
 * Upwards motion is '.'
 * Sideways motion is '-'
 */
static void IMU_task(void *arg) {
    (void) arg;
    float ax, ay, az, gx, gy, gz, t = 0; // Storages for IMU values 
    while (1) {

        while (programState == COMPOSE) {
            toggle_led(); //Begin the pulse 
            if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
                
                if (messageIndex < MESSAGE_MAX_TRESHOLD) {

                    if (az > UP_TRESHOLD) { // Device jerked up
                    character = '.';
                    sendMessage[messageIndex++] = character; // Save symbol to sendMessage buffer
                    menuState = MORSE; //Draw the symbol on the screen
                    programState = DRAW;
                    }

                    else if (ax > SIDE_TRESHOLD || ay > SIDE_TRESHOLD) { // Device jerked sideways 
                        character = '-';
                        sendMessage[messageIndex++] = character; // Save symbol to sendMessage buffer
                        menuState = MORSE; //Draw the symbol on the screen
                        programState = DRAW;
                    }

                    else { // No input = End of letter
                        character = ' ';
                        menuState = MORSE; //Clear the screen to indicate a space
                        programState = DRAW;
                    }
                }
            } else {
                printf("Failed to read imu data\n"); //Something went wrong
            }
            toggle_led(); //Pulse ends
            vTaskDelay(pdMS_TO_TICKS(PULSE_LENGTH)); 
        }
    }
}

/**
 * Listens to USB serial for a message in morse
 */
static void receive_task(void *arg) {
    (void) arg;
    int word = 0;
    int index = 0;
    char line[MESSAGE_MAX_TRESHOLD];

    while (1) {
        while(programState == RECEIVE) {
            int c = getchar_timeout_us(0);
            if (c != PICO_ERROR_TIMEOUT){// I have received a character
                if (c == '\r') continue; // ignore CR, wait for LF if (ch == '\n') { line[len] = '\0';
                if (c == '\n'){
                    // terminate and process the collected line
                    line[index] = '\0';
                    receiveMessageLength = index;
                    index = 0;
                    for (int i = 0; i < index; i++) {
                        receiveMessage[i] = line[i];
                    }
                    
                    programState = DRAW;
                }
                else if(index < MESSAGE_MAX_TRESHOLD - 1){
                    line[index++] = (char)c;
                }
                else { //Overflow: print and restart the buffer with the new character. 
                    line[index] = '\0';
                    receiveMessageLength = index;
                    index = 0;
                    for (int i = 0; i < index; i++) {
                        receiveMessage[i] = line[i];
                    }
                    line[index++] = (char)c; 
                    programState = DRAW;
                
                
                }
            }
            else {
                vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new sendMessage
            }
        }
    }
}

/**
 * Prints composed sendMessage via USB using regular alphabet 
 */
static void send_task(void * arg) {
    (void) arg;

    while (1) {

        while (programState == SEND) {
            parse_message(sendMessage, parsedMessage);
            /*
            for (int i = 0; sendMessage[i] != NULL; i++) {
                printf("%s", sendMessage[i]);
            }
            */
            menuState = IDLEMENU;
            programState = DRAW;
        }
        
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

    //Initialize other peripherals
    init_display();
    init_button1();
    init_button2();
    init_led();    
    init_buzzer();

    //Set interrupt for buttons
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, button_interrupt);
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, button_interrupt);

    TaskHandle_t myDisplaytask, myIMUtask, myReceiveTask, mySendTask = NULL;
    // Create the tasks with xTaskCreate

    BaseType_t result = xTaskCreate(display_task,       // (en) Task function
                "Display",              // (en) Name of the task 
                DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,               // (en) Arguments of the task 
                2,                  // (en) Priority of this task
                &myDisplaytask);    // (en) A handle to control the execution of this task

        if(result != pdPASS) {
            printf("Display Task creation failed\n");
            return 0;
        }
    
    result = xTaskCreate(IMU_task,
                "IMU",
                DEFAULT_STACK_SIZE,
                NULL,
                2,
                &myIMUtask);
    
        if (result != pdPASS) {
            printf("IMU task creation failed");
            return 0;
        }

    result = xTaskCreate(receive_task,
            "Receive",
            DEFAULT_STACK_SIZE,
            NULL,
            2,
            &myReceiveTask);

        if (result != pdPASS) {
            printf("Receive task creation failed");
            return 0;
        }

    result = xTaskCreate(send_task,
        "Send",
        DEFAULT_STACK_SIZE,
        NULL,
        2,
        &mySendTask);
    
        if (result != pdPASS) {
            printf("Receive task creation failed");
            return 0;
        }
    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

