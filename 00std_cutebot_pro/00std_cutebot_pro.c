#include "nrf52833.h"

/*
sources:
    https://github.com/elecfreaks/pxt-Cutebot-Pro/blob/master/main.ts
I2C:
    same a Cutebot
*/

/*
motor control
[
    0x99,
    command,   // 0x01; set, 0x09: stop
    motor,     // 0x01: left, 0x02: right, 0x03 both
    direction, // 0x01: forward, 0x00: backward
    speed,     // between 0 and 100
    0x00,
    0x88,
]
*/
#define MOTOR_SPEED 50 // [0...100]
uint8_t I2CBUF_MOTOR_LEFT_FWD[]   = {0x99,0x01,0x01,0x01,MOTOR_SPEED,0x00,0x88};
uint8_t I2CBUF_MOTOR_LEFT_BACK[]  = {0x99,0x01,0x01,0x00,MOTOR_SPEED,0x00,0x88};

uint8_t I2CBUF_MOTOR_RIGHT_FWD[]  = {0x99,0x01,0x02,0x01,MOTOR_SPEED,0x00,0x88};
uint8_t I2CBUF_MOTOR_RIGHT_BACK[] = {0x99,0x01,0x02,0x00,MOTOR_SPEED,0x00,0x88};

uint8_t I2CBUF_MOTORS_STOP[]      = {0x99,0x09,0x03,0x00,0x00,0x00,0x88};

/*
LED control
[
    0x99,
    0x0f,
    led,       // 0x02: left, 0x01: right, 0x03 both
    r,
    g,
    b,
    0x88,
]
*/
#define LED_INTENSITY 0xff // [0x00...0xff]
uint8_t I2CBUF_LED_LEFT_WHITE[]   = {0x99,0x0f,0x02,LED_INTENSITY,LED_INTENSITY,LED_INTENSITY,0x88};
uint8_t I2CBUF_LED_LEFT_RED[]     = {0x99,0x0f,0x02,LED_INTENSITY,         0x00,         0x00,0x88};
uint8_t I2CBUF_LED_LEFT_GREEN[]   = {0x99,0x0f,0x02,         0x00,LED_INTENSITY,         0x00,0x88};
uint8_t I2CBUF_LED_LEFT_BLUE[]    = {0x99,0x0f,0x02,         0x00,         0x00,LED_INTENSITY,0x88};
uint8_t I2CBUF_LED_LEFT_OFF[]     = {0x99,0x0f,0x02,         0x00,         0x00,         0x00,0x88};
uint8_t I2CBUF_LED_RIGHT_WHITE[]  = {0x99,0x0f,0x01,LED_INTENSITY,LED_INTENSITY,LED_INTENSITY,0x88};
uint8_t I2CBUF_LED_RIGHT_RED[]    = {0x99,0x0f,0x01,LED_INTENSITY,         0x00,         0x00,0x88};
uint8_t I2CBUF_LED_RIGHT_GREEN[]  = {0x99,0x0f,0x01,         0x00,LED_INTENSITY,         0x00,0x88};
uint8_t I2CBUF_LED_RIGHT_BLUE[]   = {0x99,0x0f,0x01,         0x00,         0x00,LED_INTENSITY,0x88};
uint8_t I2CBUF_LED_RIGHT_OFF[]    = {0x99,0x0f,0x01,         0x00,         0x00,         0x00,0x88};


void wait(void) {
    volatile uint32_t a;
    for (a=0; a<4000000;a++);
}

void i2c_init(void) {
   //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:   0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT: 1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:  0=Disabled
    // .... .... .... .... .... .DDD .... .... D: DRIVE: 6=S0D1
    // .... .... .... ..EE .... .... .... .... E: SENSE: 0=Disabled
    // xxxx xxxx xxxx xx00 xxxx x110 xxxx 0010 
    //    0    0    0    0    0    6    0    2 0x00000602
    NRF_P0->PIN_CNF[26]           = 0x00000602; // SCL (P0.26)
    NRF_P1->PIN_CNF[0]            = 0x00000602; // SDA (P1.00)

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    26 (P0.26)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.26)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx01 1010 
    //    0    0    0    0    0    0    1    a 0x0000001a
    NRF_TWI0->PSEL.SCL            = 0x0000001a;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    00 (P1.00)
    // .... .... .... .... .... .... ..B. .... B: PORT:    1 (P1.00)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx10 0000 
    //    0    0    0    0    0    0    2    0 0x00000020
    NRF_TWI0->PSEL.SDA            = 0x00000020;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // AAAA AAAA AAAA AAAA AAAA AAAA AAAA AAAA A: FREQUENCY: 0x01980000==K100==100 kbps
    NRF_TWI0->FREQUENCY           = 0x01980000;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .AAA AAAA A: ADDRESS: 16
    // xxxx xxxx xxxx xxxx xxxx xxxx x001 0000 
    //    0    0    0    0    0    0    1    0 0x00000010
    NRF_TWI0->ADDRESS             = 0x10;
}

void i2c_send(uint8_t* buf, uint8_t buflen) {
    uint8_t i;

    i=0;
    NRF_TWI0->TXD                 = buf[i];
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;
    i++;
    while(i<buflen) {
        while(NRF_TWI0->EVENTS_TXDSENT==0);
        NRF_TWI0->EVENTS_TXDSENT  = 0;
        NRF_TWI0->TXD             = buf[i];
        i++;
    }
    while(NRF_TWI0->EVENTS_TXDSENT==0);
    NRF_TWI0->TASKS_STOP     = 1;
}

int main(void) {
    
    i2c_init();

    // motor left
    i2c_send(I2CBUF_MOTOR_LEFT_FWD,    sizeof(I2CBUF_MOTOR_LEFT_FWD));
    i2c_send(I2CBUF_MOTOR_LEFT_BACK,   sizeof(I2CBUF_MOTOR_LEFT_BACK));
    i2c_send(I2CBUF_MOTORS_STOP,       sizeof(I2CBUF_MOTORS_STOP));
    // motor right
    i2c_send(I2CBUF_MOTOR_RIGHT_FWD,   sizeof(I2CBUF_MOTOR_RIGHT_FWD));
    i2c_send(I2CBUF_MOTOR_RIGHT_BACK,  sizeof(I2CBUF_MOTOR_RIGHT_BACK));
    i2c_send(I2CBUF_MOTORS_STOP,       sizeof(I2CBUF_MOTORS_STOP));
    // led left
    i2c_send(I2CBUF_LED_LEFT_WHITE,    sizeof(I2CBUF_LED_LEFT_WHITE));
    i2c_send(I2CBUF_LED_LEFT_RED,      sizeof(I2CBUF_LED_LEFT_RED));
    i2c_send(I2CBUF_LED_LEFT_GREEN,    sizeof(I2CBUF_LED_LEFT_GREEN));
    i2c_send(I2CBUF_LED_LEFT_BLUE,     sizeof(I2CBUF_LED_LEFT_BLUE));
    i2c_send(I2CBUF_LED_LEFT_OFF,      sizeof(I2CBUF_LED_LEFT_OFF));
    // led right
    i2c_send(I2CBUF_LED_RIGHT_WHITE,   sizeof(I2CBUF_LED_RIGHT_WHITE));
    i2c_send(I2CBUF_LED_RIGHT_RED,     sizeof(I2CBUF_LED_RIGHT_RED));
    i2c_send(I2CBUF_LED_RIGHT_GREEN,   sizeof(I2CBUF_LED_RIGHT_GREEN));
    i2c_send(I2CBUF_LED_RIGHT_BLUE,    sizeof(I2CBUF_LED_RIGHT_BLUE));
    i2c_send(I2CBUF_LED_RIGHT_OFF,     sizeof(I2CBUF_LED_RIGHT_OFF));


    // Really fun piece of code
    i2c_send(I2CBUF_MOTOR_LEFT_FWD,    sizeof(I2CBUF_MOTOR_LEFT_FWD));
    i2c_send(I2CBUF_MOTOR_RIGHT_BACK,  sizeof(I2CBUF_MOTOR_RIGHT_BACK));
    while(1){
        i2c_send(I2CBUF_LED_LEFT_RED,      sizeof(I2CBUF_LED_LEFT_RED));
        i2c_send(I2CBUF_LED_RIGHT_RED,   sizeof(I2CBUF_LED_RIGHT_RED));
        wait();
        i2c_send(I2CBUF_LED_LEFT_GREEN,    sizeof(I2CBUF_LED_LEFT_GREEN));
        i2c_send(I2CBUF_LED_RIGHT_GREEN,     sizeof(I2CBUF_LED_RIGHT_GREEN));
        wait();
        i2c_send(I2CBUF_LED_LEFT_BLUE,      sizeof(I2CBUF_LED_LEFT_BLUE));
        i2c_send(I2CBUF_LED_RIGHT_BLUE,   sizeof(I2CBUF_LED_RIGHT_BLUE));
        wait();
    };
}
