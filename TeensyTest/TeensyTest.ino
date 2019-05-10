//
// TeensyTest
//
//
// Developed with [embedXcode](http://embedXcode.weebly.com)
//
// Author         William Lies
//                 William Lies
//
// Date            10/7/17 10:49 PM
// Version        <#version#>
//
// Copyright    © William Lies, 2017
// Licence        <#licence#>
//
// See         ReadMe.txt for references
//

///TEST COMIT FOR GITHUB
#include "SPI.h"
#include "TLC_lib.h"
#include "MUX_lib.h"

//keyboard library
// Core library for code-sense - IDE-based
#if defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(ESP8266) // ESP8266 specific
#include "Arduino.h"
#elif defined(ARDUINO) // Arduino 1.8 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

#include "helperFunctions.h"
#include "subApplicationFunctions.h"
#include "PS2Keyboard.h"
//#include "PS2Keyboard.h"
//hello from xcode
enum mainStates {ANIMATIONS, INTERACTIVEANIMATIONS, GAMES, TOP};
enum directions {UP, FORWARD, LEFT, BACKWARD, RIGHT, DOWN};

typedef struct{
    uint8_t x;
    uint8_t y;
    uint8_t z;
} coord_t;



/**************************************************************************
 *
 *
 *
 *  Declare all global functions below
 *
 *
 *
 **************************************************************************/
void set_led( uint8_t x, uint8_t y, uint8_t z, uint16_t r, uint16_t g, uint16_t b);
void drawBall( int x, int y, int z, uint16_t color);
void bounceBall(int iterations);
void dodgeGame();
void lightCube();
void snow();
void snakeGame();
void someThing();
void lights();
void test();
void swirl();
void pong();
void randomColors();
#define GAMEAMOUNT 3
#define ANIMATIONAMOUNT 5
#define INTERACTIVEANIMATIONSAMOUNT 1
//store all functions in here after declaration
void (*appFunctions[3][5])() {{lightCube, snow, randomColors, swirl, test}, {someThing},  {dodgeGame, snakeGame, pong}};
//String appFunctions[] = {"dodgeGame"};
/**************************************************************************
 *
 *
 *
 *  Decalare all global variables below
 *
 *
 *
 **************************************************************************/
//escape corispondes to the

//mainStates state = TOP;

PS2Keyboard keyboard;
volatile uint8_t     gs_buf[NUM_BYTES];  //Buffer written to TLCs over SPI (12 bit color values)
volatile uint16_t    px_buf[NUM_LEDS];   //Pixel buffer storing each LED color as a 16 bit value ( RRRRRGGGGGGBBBBB )

uint8_t phase = 0; //strictly for the color shifting of the snake game


//uint8_t x;
//uint8_t y;
//uint8_t z;
//uint16_t blue = 0x000F;
uint32_t white = 0xFFFF;
uint32_t timer0;



int count = 0;


/**************************************************************************
 *
 *
 *
 *  Setup below
 *
 *
 *
 **************************************************************************/

void setup()
{
    delay(1000);
    keyboard.begin(15, 21);
    Serial.begin(9800);
    Serial.println("hello");


    delay(200);
    mux_init();
    init_TLC();
    memset((uint8_t *)gs_buf, 0xFF, NUM_BYTES);
    delay(100);
    write_dot_correction( (uint8_t *)gs_buf );
    memset((uint8_t *)gs_buf, 0x00, NUM_BYTES);
    memset((uint16_t *)px_buf, 0x0000, NUM_LEDS * 2);
    mux_begin();
    delay(200);

    pinMode(23, OUTPUT);
    digitalWrite( 23, HIGH);


}

/**************************************************************************
 *
 *
 *
 *  Functions to aid animations/games below
 *
 *
 *
 **************************************************************************/


//Coordinates and RGB values and buffer. RGB range 0-255
void set_led( uint8_t x, uint8_t y, uint8_t z, uint16_t r, uint16_t g, uint16_t b){
    px_buf[ z * NUM_LEDS_LYR + NUM_LEDS_DIM * x + y] = ((r & 0x00F8) << 8 ) |
    ((g & 0x00FC) << 3 ) |
    ((b & 0x00F8) >> 3 );
}


/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void d(uint16_t t){
    uint32_t temp = millis();
    while (millis() - temp < t) {
        if (keyboard.available()) {
            if (keyboard.read() == PS2_ESC) {
                break;
            }
        }
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//packs a rgb color into one number
uint16_t pk_color(uint16_t r, uint16_t g, uint16_t b) {

    return ((r & 0x00F8) << 8 ) |
    ((g & 0x00FC) << 3 ) |
    ((b & 0x00F8) >> 3 );
}


/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/
// red goes up to 31
//blue to 31
//green up to 63
uint16_t pk_low(uint16_t r, uint16_t g, uint16_t b) {
    return ((r & 0x001F) << 11 ) |
    ((g & 0x003F) << 5 ) |
    ((b & 0x001F));
}
/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/


uint16_t pk_coord(uint8_t x, uint8_t y, uint8_t z) {
    return (((z << 4) | y) << 4) | x;
}


/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

uint8_t shifter(uint8_t k, directions more, mainStates state) {
    uint8_t amount = 3; //for the top level
    if (state == GAMES) {
        amount = GAMEAMOUNT;
    }
    else if (state == ANIMATIONS) {
        amount = ANIMATIONAMOUNT;
    }
    else if (state == INTERACTIVEANIMATIONS) {
        
        amount = INTERACTIVEANIMATIONSAMOUNT;
    }
    
    if (more == RIGHT) {
        if (k == amount - 1) {
            k = 0;
        }
        else {
            k++;

        }
    }
    else if (more == LEFT){
        if (k == 0) {
            k = amount - 1;
        }
        else {
            k--;

        }
    }
    return k;
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//self explainatory
void clearCube(){
    memset((uint16_t *)px_buf, 0x0000, NUM_LEDS * 2);
}

//void adressToCoordinate

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//houses the edata for the whole cube
uint16_t LEDArray(uint8_t x, uint8_t y, uint8_t z){
    if (!(z > 11 || x > 11|| y > 11)) {
    return px_buf[ z * NUM_LEDS_LYR + NUM_LEDS_DIM * x + y];
    }
    else {
        return 0;
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//Set led with packed color value
void set_led_pk( uint8_t x, uint8_t y, uint8_t z, uint16_t c){
    px_buf[ z * NUM_LEDS_LYR + NUM_LEDS_DIM * x + y] = c;
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//Set led with packed color value
void setAdressLED( uint16_t address, uint16_t c){
    px_buf[address] = c;
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/


void setPackedPackedLED(uint16_t coord, uint16_t c) {
    //set_led_pk((coord & 0x000F), (coord & 0x00F0), (coord & 0x0F00), c);
    px_buf[ ((coord & 0x0F00) >> 8) * NUM_LEDS_LYR + NUM_LEDS_DIM * (coord & 0x000F) + ((coord & 0x00F0) >> 4)] = c;
}


/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//this function switches which orientation you may look at the cube, the  coordinates go as follows, the face you look at for each direction, the reletive 'z' coordinate is the third one, the reletive 'x' is either the true z or x direction, and the second coordinate is always the reletive 'y'
uint16_t directionalCubeArray(uint8_t firstCord, uint8_t secondCord, uint8_t thirdCord, directions direction, boolean setLED = false, uint16_t color = 0){
    
    
    if (setLED){
        switch (direction) { //you can set leds
            case UP:
                set_led_pk(firstCord, thirdCord, secondCord, color);
                break;
            case FORWARD:
                set_led_pk(firstCord, secondCord, thirdCord, color);
                break;
            case LEFT:
                set_led_pk(11 - thirdCord, secondCord, firstCord, color);
                break;
            case BACKWARD:
                set_led_pk(11 - firstCord, secondCord, 11 - thirdCord, color);
                break;
            case RIGHT:
                set_led_pk(thirdCord, secondCord, 11 - firstCord, color);
                break;
            case DOWN:
                set_led_pk(firstCord, 11 - thirdCord, secondCord, color);
                break;

            default:
                break;
        }
        return 0;
    }
    else {
        switch (direction) {//or you can call the array for the cube, it returns the value
            case UP:
                return LEDArray(firstCord, thirdCord, secondCord);
                break;
            case FORWARD:
                return LEDArray(firstCord, secondCord, thirdCord);
                break;
            case LEFT:
                return LEDArray(11 - thirdCord, secondCord, firstCord);
                break;
            case BACKWARD:
                return LEDArray(11 - firstCord, secondCord, 11 - thirdCord);
                break;
            case RIGHT:
                return LEDArray(thirdCord, secondCord, 11 - firstCord);
                break;
            case DOWN:
                return LEDArray(firstCord, 11 - thirdCord, secondCord);
                break;

            default:
                break;
        }
    }


}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void rotate(uint16_t t) {
    uint32_t timer  = millis();
    uint32_t R0Time = millis();
    uint32_t R1Time = millis();
    uint32_t R2Time = millis();
    uint32_t R3Time = millis();
    uint32_t R4Time = millis();
    uint32_t R5Time = millis();
    
    while (millis() - timer < t) {
        
    }

}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void moveRow(uint8_t firstCord, uint8_t secondCord,  directions direction,  uint16_t specificColor = 1, boolean collective = false, uint8_t start = 0, uint8_t end = 11, uint16_t color = 1){


    if (!collective && end == 11){
        directionalCubeArray(firstCord, secondCord, 11, direction, true);
    }
    for (int i = 10 - (11 - end); i >= 0 + start; i--) {
        if (specificColor == 1) {
            if (directionalCubeArray(firstCord, secondCord, i, direction) != 0){
                if (collective){
                    if (directionalCubeArray(firstCord, secondCord, i + 1, direction) == 0){
                        if (color == 1){
                            directionalCubeArray(firstCord, secondCord, i + 1,  direction, true, directionalCubeArray(firstCord, secondCord, i, direction));
                        }
                        else {
                            directionalCubeArray(firstCord, secondCord, i, direction, true, color);
                        }
                        directionalCubeArray(firstCord, secondCord, i, direction, true);
                    }

                }
                else {

                    if (color == 1){
                        directionalCubeArray(firstCord, secondCord, i + 1, direction, true, directionalCubeArray(firstCord, secondCord, i, direction));
                    }
                    else {
                        directionalCubeArray(firstCord, secondCord, i + 1, direction, true, color);
                    }
                    directionalCubeArray(firstCord, secondCord, i, direction, true);
                }
            }
        }
        else {
            if (directionalCubeArray(firstCord, secondCord, i, direction) != 0 && directionalCubeArray(firstCord, secondCord, i, direction) == specificColor){
                if (collective){
                    if (directionalCubeArray(firstCord, secondCord, i + 1, direction) != specificColor){
                        if (color == 1){
                            directionalCubeArray(firstCord, secondCord, i + 1, direction, true, directionalCubeArray(firstCord, secondCord, i, direction, false));
                        }
                        else {
                            directionalCubeArray(firstCord, secondCord, i, direction, true, color);
                        }
                        directionalCubeArray(firstCord, secondCord, i, direction, true);
                    }

                }
                else {

                    if (color == 1){
                        directionalCubeArray(firstCord, secondCord, i + 1,  direction, true, directionalCubeArray(firstCord, secondCord, i, direction));
                    }
                    else {
                        directionalCubeArray(firstCord, secondCord, i + 1, direction, true, color);
                    }
                    directionalCubeArray(firstCord, secondCord, i, direction, true);

                }
            }
        }
    }

}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void setRandomLED(uint8_t leds, directions direction, uint16_t color, boolean threeD = false) {
    if (!threeD) {
    for (uint8_t i = 0; i < leds; i++) {
        directionalCubeArray(rand() % 12, rand() % 12, 0, direction, true, color);
    }
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void drawNumber(int number, int x1, int y1, int z1, directions viewpoint, uint16_t color) {
    if (number == 0) {
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+1, z1, viewpoint, true, color);
    }
    else if (number == 1) {
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
    }
    else if (number == 2) {
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
    }
    else if (number == 3) {
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
    }
    else if (number == 4) {
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
    }
    else if (number == 5) {
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
    }
    else if (number == 6) {
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+1, z1, viewpoint, true, color);
    }
    else if (number == 7) {
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
    }
    else if (number == 8) {
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+1, z1, viewpoint, true, color);
    }
    else if (number == 9) {
        directionalCubeArray(x1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+1, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+2, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+3, z1, viewpoint, true, color);
        directionalCubeArray(x1+2, y1+4, z1, viewpoint, true, color);
        directionalCubeArray(x1+1, y1+4, z1, viewpoint, true, color);
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

//DrawFigure
//Coordinate 1: (x1,y2,z2)
//Coordinate 2: (x2,y2,z2);
void DrawFigure(int x1, int y1, int z1, int x2, int y2, int z2) {
    int xDist = abs(x1 - x2);
    int yDist = abs(y1 - y2);
    int zDist = abs(z1 - z2);
    if (x1 > x2){ //created in order to make the 'for' loops always accurate
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2){ // created in order to make the 'for' loop always accurate
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    if (z1 > z2) { // created in order to make the 'for' loop always accurate
        int temp = z1;
        z1 = z2;
        z2 = temp;
    }
    if (xDist == 0) { // if the x coordinates are the same, the shape will be a square will a constant x value
        for (int i = y1; i <= y2; i++){
            set_led(x1,i,z1,255,255,255);
            //delay(200);
            set_led(x1,i,z2,255,255,255);
            //delay(200);
        }
        for (int i = z1; i <= z2; i++) {
            set_led(x1,y1,i,255,255,255);
            //delay(200);
            set_led(x1,y2,i,255,255,255);
            //delay(200);
        }
    }
    else if (yDist == 0) { // if the y coordinates are the same, the shape will be a square will a constant y value
        for (int i = x1; i <= x2; i++){
            set_led(i,y1,z1,255,255,255);
            //delay(200);
            set_led(i,y1,z2,255,255,255);
            //delay(200);
        }
        for (int i = z1; i <= z2; i++) {
            set_led(x1,y1,i,255,255,255);
            //delay(200);
            set_led(x2,y1,i,255,255,255);
            //delay(200);
        }
    }
    else if (zDist == 0) { // if the z coordinates are the same, the shape will be a square will a constant z value
        for (int i = x1; i <= x2; i++){
            set_led(i,y1,z1,255,255,255);
            //delay(200);
            set_led(i,y2,z1,255,255,255);
            //delay(200);
        }
        for (int i = y1; i <= y2; i++) {
            set_led(x1,i,z1,255,255,255);
            //delay(200);
            set_led(x2,i,z1,255,255,255);
            //delay(200);
        }
    }
    else { // if none of the x/y/z coordinates match the other points' x/y/z, then the shape will be a cube.
        for (int i = x1; i <= x2; i++){
            set_led(i,y1,z1,255,255,255);
            //delay(200);
            set_led(i,y2,z1,255,255,255);
            //delay(200);
            set_led(i,y1,z2,255,255,255);
            //delay(200);
            set_led(i,y2,z2,255,255,255);
            //delay(200);
        }
        for (int i = y1; i <= y2; i++){
            set_led(x1,i,z1,255,255,255);
            //delay(200);
            set_led(x2,i,z1,255,255,255);
            //delay(200);
            set_led(x1,i,z2,255,255,255);
            //delay(200);
            set_led(x2,i,z2,255,255,255);
            //delay(200);
        }
        for (int i = z1; i <= z2; i++){
            set_led(x1,y1,i,255,255,255);
            //delay(200);
            set_led(x1,y2,i,255,255,255);
            //delay(200);
            set_led(x2,y1,i,255,255,255);
            //delay(200);
            set_led(x2,y2,i,255,255,255);
            //delay(200);
        }
    }
}
void clearMost(uint8_t howMany, directions backSide) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            for (int k = 11; k > 11-howMany; k--) {
                directionalCubeArray(i, j, k, backSide, true);
            }
        }
    }
}

/**************************************************************************
 *                                                                        *
 * ************************************************************************
 *                                                                        *
 *  Main Switch Function below                                            * ***************************
 *                                                                        *
 * ************************************************************************
 *                                                                        *
 **************************************************************************/

void mainSwitch(mainStates state) {
    int i = 0;
    uint8_t stateSwitch = 0;
    uint32_t backgroundTimer = millis();
    while(true) {
        
        if (millis() - backgroundTimer > 200) {
            
           // setRandomLED(3, DOWN, 0xFFFF);
            for (uint8_t j = 0; j < 12; j++) {
            for (uint8_t i = 0; i < 12; i++) {
             //   moveRow(i, j, DOWN);
            }
            backgroundTimer += 200;
        }
        }
        
        char c;
        
        if (keyboard.available()) {
            c = keyboard.read();
            //i = c - '0';
        }
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                for (int k = 0; k < 6; k++) {
                    set_led_pk(i, j, k + 6, 0);
                }
            }
        }
        
    switch (state) {
            //int y;
        case TOP:

            if (c == PS2_RIGHTARROW) {
                clearMost(6, BACKWARD);

                stateSwitch = shifter(stateSwitch, RIGHT, TOP);
            }
            if (c == PS2_LEFTARROW) {
                clearMost(6, BACKWARD);

                stateSwitch = shifter(stateSwitch, LEFT, TOP);

            }
            else if (c == PS2_ENTER) {
                state = mainStates(stateSwitch);
                clearMost(6, BACKWARD);
                i=0;
                break;
            }
            //y=0;
            drawNumber(stateSwitch + 1, 5, 5, 0, FORWARD, 0xF0F0);
            drawNumber(stateSwitch + 1, 5, 5, 1, FORWARD, 0xF0F0);

            break;
            
        case ANIMATIONS:

            if (c == PS2_ESC) {
                state = TOP;
                clearMost(6, BACKWARD);
                break;

            }
            
            else if (c == PS2_LEFTARROW) {
                i = shifter(i, LEFT, ANIMATIONS);
                clearMost(6, BACKWARD);

            }
            else if (c == PS2_RIGHTARROW) {
                i = shifter(i, RIGHT, ANIMATIONS);
                clearMost(6, BACKWARD);

            }
            
            else if (c == PS2_ENTER){
                appFunctions[0][i]();
            }
            drawNumber(i + 1, 5, 5, 0, FORWARD, 0xFFFF);
            drawNumber(i + 1, 5, 5, 1, FORWARD, 0xFFFF);

            break;
            
        case GAMES:

            if (c == PS2_ESC) {
                state = TOP;
                clearMost(6, BACKWARD);
                break;
            }
            
            else if (c == PS2_LEFTARROW) {
                i = shifter(i, LEFT, GAMES);
                clearMost(6, BACKWARD);
            }
            else if (c == PS2_RIGHTARROW) {
                i = shifter(i, RIGHT, GAMES);
                clearMost(6, BACKWARD);

            }
            
            else if (c == PS2_ENTER){
            appFunctions[2][i]();
            }
            drawNumber(i + 1, 5, 5, 0, FORWARD, 0xFFFF);
            drawNumber(i + 1, 5, 5, 1, FORWARD, 0xFFFF);

            break;
        
            case INTERACTIVEANIMATIONS:
            
            if (c == PS2_ESC) {
                state = TOP;
                clearMost(6, BACKWARD);
                break;
            }
            
            else if (c == PS2_LEFTARROW) {
                i = shifter(i, LEFT, INTERACTIVEANIMATIONS);
                clearMost(6, BACKWARD);
                
            }
            else if (c == PS2_RIGHTARROW) {
                i = shifter(i, RIGHT, INTERACTIVEANIMATIONS);
                clearMost(6, BACKWARD);
                
            }
            
            else if (c == PS2_ENTER){
                appFunctions[1][i]();
            }
            drawNumber(i + 1, 5, 5, 0, FORWARD, 0xFFFF);
            drawNumber(i + 1, 5, 5, 1, FORWARD, 0xFFFF);
            
            break;
            
    }
        c = ']';

}
}


/**************************************************************************
 *
 *
 *
 *  The loop call
 *
 *
 *
 **************************************************************************/



void loop()
{

    mainSwitch(TOP);
  //this is curently just a simple 'dodge' game

} //end loop


/**************************************************************************
 *
 *
 *
 *  all sub function below, all are self contained 'apps' called within the Main Switch function (must be declared within the two areas up top too.
 *
 *
 *
 **************************************************************************/


void dodgeGame() {
    
    uint32_t timer0 = millis();
    uint32_t dificultyTimer = millis();
    uint32_t time1 = millis();
    uint8_t ledCount = 3;
    uint8_t key = 0;
    boolean start = false;
    clearCube();
    while (true){
   
        if (millis() - dificultyTimer > 5000 && start) {
            
            ledCount++;
            Serial.print(ledCount);
            dificultyTimer += 5000;
            
        }
        uint16_t delayTime = 150;
    
    
    if (key != 7){
        set_led_pk(5, 5, 1, 0xF00);
        set_led_pk(6, 5, 1, 0xF00);
        set_led_pk(5, 6, 1, 0xF00);
        set_led_pk(6, 6, 1, 0xF00);
        set_led_pk(5, 5, 2, 0xF00);
        set_led_pk(6, 5, 2, 0xF00);
        set_led_pk(5, 6, 2, 0xF00);
        set_led_pk(6, 6, 2, 0xF00);
        
        
    }
    
    char c;
    
    if (keyboard.available()){
        start = true;
        c=keyboard.read();
        Serial.print(c);
        if (c == PS2_ESC) {
            break;
        }
    }
    
    if (c == PS2_UPARROW){
        for (int i = 0; i < 12; i++){
            moveRow(i, 1, UP, 0xF00, true);
            moveRow(i, 2, UP, 0xF00, true);
            c = '[';
        }
    }
    else if (c == PS2_DOWNARROW){
        for (int i = 0; i < 12; i++){
            moveRow(i, 1, DOWN, 0xF00, true);
            moveRow(i, 2, DOWN, 0xF00, true);
            c = '[';
        }
    }
    else if (c == PS2_LEFTARROW){
        for (int i = 0; i < 12; i++){
            moveRow(1, i, LEFT, 0xF00, true);
            moveRow(2, i, LEFT, 0xF00, true);
            c = '[';
        }
    }
    else if (c == PS2_RIGHTARROW){
        for (int i = 0; i < 12; i++){
            
            moveRow(10, i, RIGHT, 0xF00, true);
            moveRow(9, i, RIGHT, 0xF00, true);
            c = '[';
        }
    }
    
        if (start) {
    if (millis() - timer0 > delayTime){
        Serial.print(ledCount);
        setRandomLED(ledCount, BACKWARD, 0xFFF);

        
        for (int i = 0; i < 12; i++){
            for (int j = 0; j < 12; j++){
                
                moveRow(i, j, BACKWARD, 0xFFF);
            }
            
        }
        timer0 += delayTime;
    }
}
        else if (key != 7) {
            setRandomLED(ledCount, BACKWARD, 0xFFF);
            key = 7;
        }
    }
    clearCube();
}

/***************************************************************************
 *
 *   Sub Seperator t
 *
 ***************************************************************************/
void dropMatch() {
    for (int p = 0; p < 3; p++) {

    for (int k = 0; k < 12; k++) {
        for (int j = 0; j < 12; j++) {
            for (int i = 0; i < 12; i++) {
                if (LEDArray(i, k, j) == 0) {
        set_led_pk(i, j, k, 0xF800);
                }
            }
        }
    }
    d(250);
    clearCube();
        d(250);

    }
}
void lightMatch() {
    dropMatch();
}
void pourGasoline() {
    lightMatch();
}

uint16_t colorShifter(uint16_t currentColor) {
    boolean state = false;
    uint8_t r  = (currentColor & 0xF800) >> 11;
    uint16_t g = (currentColor & 0x07E0) >> 5;
    uint16_t b = (currentColor & 0x001F);
    if (state) {
        if (b != 31) { b++; } else if (r != 63) { r++; } else { state = true; }
    } else {
    if (b != 0) { b--; } else if (r != 0) { r--; } else {state = false;}
    }
    return pk_low(r, g, b);
}

void MoveLED(uint8_t x, uint8_t y, uint8_t z, uint16_t currentColor, boolean gainLength) {
    currentColor = colorShifter(currentColor);
    
//    Serial.println(0xFFFF);
//    Serial.println(colorShifter(0xFFFF));
//    Serial.println(LEDArray(x, y, z));

    if (LEDArray(x + 1, y, z) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        MoveLED(x + 1, y, z, currentColor, gainLength);
    }
    else if (LEDArray(x - 1, y, z) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        
        MoveLED(x - 1, y, z, currentColor, gainLength);
        
    }
    else if (LEDArray(x, y+1, z) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        
        MoveLED(x, y + 1, z, currentColor, gainLength);
        
    }
    else if (LEDArray(x, y-1, z) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        
        MoveLED(x, y-1, z, currentColor, gainLength);
        
    }
    else if (LEDArray(x, y, z+1) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        
        MoveLED(x, y, z+1, currentColor, gainLength);
        
    }
    else if (LEDArray(x, y, z-1) == currentColor) {
        set_led_pk(x, y, z, currentColor);
        //Serial.println("z-1");
        //uses underflow
        if (z-1 < 12) {
            MoveLED(x, y, z-1, currentColor, gainLength);
        }
    }
    else if (gainLength) {
        set_led_pk(x, y, z, currentColor);

    }
  else {
      set_led_pk(x, y, z, 0);
  }
    
}



void snakeGame() {
#define NUM_FOOD 10
    boolean start = false;
    uint16_t color = 0xFFFF;
    boolean toggle = false;
    uint8_t x = 5;
    uint8_t y = 5;
    uint8_t z = 4;
    uint8_t x2 = 5;
    uint8_t y2 = 3;
    uint8_t z2 = 6;
    uint16_t speed = 500;
    uint16_t length = 4;
//    uint8_t fx;
//    uint8_t fy;
//    uint8_t fx;
    directions direction = FORWARD;
    uint32_t foodTimer = millis();
    uint32_t moveTimer = millis();
    boolean addLength = false;
    coord_t food[NUM_FOOD];
    boolean endGame = false;
    
   // coord_t snake[ MAX_SNAKE_LEN ];
   // coord_t * sk_head = snake;
   // coord_t * sk_tail = snake;
    while (true) {
        char c = ']';
        
        if (keyboard.available()){
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
            
            if (c == PS2_LEFTARROW && direction == RIGHT) {
                c = ']';
            } else if (c == PS2_RIGHTARROW && direction == LEFT) {
                c = ']';
            } else if (c == PS2_DOWNARROW && direction == FORWARD) {
                c = ']';
            } else if (c == PS2_UPARROW && direction == BACKWARD) {
                c = ']';
            } else if (c == '1' && direction == DOWN) {
                c = ']';
            } else if (c == '0' && direction == UP) {
                c = ']';
            }
        }

    
        if (!start) {
            clearCube();
            for (int i = 0; i < 4; i++) {
                set_led_pk(5, 5, 4 - i, color);
                color = colorShifter(color);
            }
            for (int i = 0; i < NUM_FOOD; i++) {
                while (true) {
                food[i].x = rand() % 12;
                food[i].y = rand() % 12;
                food[i].z = rand() % 12;
                    if (LEDArray(food[i].x, food[i].y, food[i].z) == 0) {
                        break;
                    }
                }
            }
            start = true;
        }
        if (millis() - foodTimer > 900) {
            if (toggle) {
                for (int i = 0; i < NUM_FOOD; i++ ) {
                    set_led_pk(food[i].x, food[i].y, food[i].z, 0xF830);
                }
                toggle = !toggle;
            } else {
                for (int i = 0; i < NUM_FOOD; i++ ) {
                    set_led_pk(food[i].x, food[i].y, food[i].z, 0x031F);
                }
                toggle = !toggle;
            }
            foodTimer += 900;
        }
        
        switch (c) {
            case PS2_LEFTARROW:
                direction = LEFT;
                break;
            case PS2_RIGHTARROW:
                direction = RIGHT;
                break;
            case '0':
                direction = DOWN;
                break;
            case '1':
                direction = UP;
                break;
            case PS2_DOWNARROW:
                direction = BACKWARD;
                break;
            case PS2_UPARROW:
                direction = FORWARD;
                break;
            default:
                break;
        }

        
        x2 = x;
        y2 = y;
        z2 = z;
        
//        if (addLength && eatenFood != 100) {
//            food[eatenFood] = {
//
//
//            }
//        }
        

        if (millis() - moveTimer > speed && !endGame) {
            if (length >= 7 && speed > 200) {
                speed -= 90;
                length = 0;
            }
            if (!(x == 12 || z == 12 || y == 12)) {
        switch (direction) {
            case FORWARD:
                z++;
                break;
            case BACKWARD:
                z--;
                break;
            case UP:
                y++;
                break;
            case DOWN:
                y--;
                break;
            case LEFT:
                x--;
                break;
            case RIGHT:
                x++;
                break;
                
            default:
                break;
                    }
                
                for (int i = 0; i < NUM_FOOD; i++) {
                    if (food[i].x == x && food[i].y == y && food[i].z == z) {
                        while (true) {
                        food[i].x = rand() % 12;
                        food[i].y = rand() % 12;
                        food[i].z = rand() % 12;
                        addLength = true;
                            if (LEDArray(food[i].x, food[i].y, food[i].z) == 0) {
                                length++;
                                break;
                            }
                        }
                        
                    }
                    
                }
                if (LEDArray(x, y, z) != 0 && addLength != true || (x - 1 >= 11 || y - 1 >= 11 || z - 1 >= 11)) {
                    endGame = true;
                    pourGasoline();
                } else {
                set_led_pk(x, y, z, 0xFFFF);
                
        MoveLED(x2, y2, z2, 0xFFFF, addLength);
                addLength = false;
            moveTimer += speed;
                }
            }

        }
    
    }
#undef NUM_FOOD
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void lightCube() {
    boolean lightUp = true;
    uint32_t timer = millis();
    uint8_t i = 0;
    while (true) {
        char c;
        
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
                
            }
        }
        
        
        if (lightUp){
            if (millis() - timer > 200) {
            
            for (int l = 0; l < 12; l++){
                for (int j = 0; j < 12; j++){
                    for (int k = 0; k < 12; k++){
                        set_led(l, j, k, i * (255 / 10), i * (0 / 10), i * (255 / 10));
                        
                    }
                }
            }
        
            if (i == 11){
                lightUp = false;
                
            }
            else {
            i++;
            }
            timer += 200;
            }
        }
        else {
            if (millis() - timer > 200) {

            for (int l = 0; l < 12; l++){
                for (int j = 0; j < 12; j++){
                    for (int k = 0; k < 12; k++){
                        set_led(l, j, k, i * (255 / 10), i * (0 / 10), i * (255 / 10));
                    }
                }
            }
        
            if (i == 0){
                lightUp = true;
            }else {
            i--;
            }
            timer += 200;
        }
        }
}
    clearCube();
}
/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/



/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void someThing() {
    uint32_t resetTime = millis();
    uint32_t timer = millis();
    while (true) {
        char c;
        
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
        }
        if (millis() - resetTime > 20000) {
            for (int h = 0; h < 12; h++) {
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, DOWN);
                }
            }
                d(60);
            }
            resetTime += 20000;
        }
        if (millis() - timer > 150){
        set_led(rand() % 12, rand() % 12, rand() % 12, rand() % 255, rand() % 255, rand() % 255);
            timer += 150;
        }
        if (c == PS2_BACKSPACE) {
            resetTime = millis();
            uint8_t r = rand() % 2;
            uint8_t g = rand() % 2;
            uint8_t b = rand() % 2;
            if (r + b + g == 0){
                r = 1;
                b = 1;
            }
            for (int i = 1; i < 10; i++){
                for (int l = 0; l < 12; l++){
                    for (int j = 0; j < 12; j++){
                        for (int k = 0; k < 12; k++){
                            set_led(l, j, k, r * i * (255 / 10), g * i * (255 / 10),  b * i * (255 / 10));

                        }
                    }
                }
                d(12); //delay

            }
            for (int i = 9; i >= 0; i--){
                for (int l = 0; l < 12; l++){
                    for (int j = 0; j < 12; j++){
                        for (int k = 0; k < 12; k++){
                            set_led(l, j, k, r * i * (255 / 10), g * i * (255 / 10), b * i * (255 / 10));
                        }
                    }
                }
                d(12); //delay
            }
            c = ']';
        }

        if (c == PS2_UPARROW){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){

                moveRow(i, j, FORWARD, 1, true);
                c = '[';
            }
        }
        }
        else if (c == PS2_DOWNARROW){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, BACKWARD, 1, true);
                    c = '[';
                }
            }
        }
        

        else if (c == PS2_LEFTARROW){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, LEFT, 1, true);
                    c = '[';
                }
            }
        }
        

        else if (c == PS2_RIGHTARROW){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, RIGHT, 1, true);
                    c = '[';
                }
            }
        }
        else if (c == 'h'){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, DOWN, 1, true);
                    c = '[';
                }
            }
        }
        else if (c == 'y'){
            for (int i = 0; i < 12; i++){
                for (int j = 0; j < 12; j++){
                    
                    moveRow(i, j, UP, 1, true);
                    c = '[';
                }
            }
        }


            
            
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void lights() {
    
    while (true) {
        char c;
        
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
        }
        
        setPackedPackedLED(rand() % 4096 & 0x0777, 0xFFFF);
        clearCube();
        
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/
void snow() {
   
    uint32_t timer = millis();
    while (true) {
        char c;
        
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
        }
        
    if (millis() - timer > 260) {

    
    //int color = rand() % 64000;
    int count = 0;
    
   
    
        //delay(260);
    for (int i = 0; i < 12; i++){
        for (int f  = 0; f < 12; f++){
            count = 0;
            for (int g = 0; g < 4; g++){
                if (LEDArray(i, g, f) != 0){
                    count += 1;
                }
                if (count == 4){
                    set_led_pk(i, g, f, 0);
                }
                
            }
        }
    }
    
    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 12; j++){
            if (rand() % 25 != 1){
                moveRow(j, i, DOWN, 1, true);
            }
            else {
                moveRow(j, i, DOWN, 1, false);
            }
        }
    }
        int num = rand() % 12;
        int num1 = rand() % 12;
        int num2 = rand() % 12;
        set_led_pk(num, 11, num2, white);
        set_led_pk(num1, 11, num, white);
        set_led_pk(num2, 11, num1, white);
            timer += 260;
        }
    }
    clearCube();
}



/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/
void drawBall( int x, int y, int z, uint16_t color){
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            for(int k=-1; k<=1; k++){
                if( i*j*k == 0 )    //make sure one is zero so the corners don't get drawn
                    set_led_pk(x+i,y+j,z+k, color);
            }
        }
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void pong() {
    
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void randomColors() {
    while (true) {
        char c;
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
        }

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            for (int k = 0; k < 12; k++) {
                set_led_pk(i, j, k, rand() % 65535);
            }
        }
    }
    }
    clearCube();
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void bounceBall(int iterations){
    int x_vec = 1;
    int y_vec = 1;
    int z_vec = 1;
    int x = 6;
    int y = 7;
    int z = 1;

    memset((uint16_t *)px_buf, 0x0000, NUM_LEDS * 2);

    for( int q = 0; q< iterations; q++){

        drawBall( x, y, z, 0);

        if(x+x_vec >= 11 || x+x_vec <= 0){
            x += (x_vec = -x_vec);
        }
        else{ x += x_vec; }

        if(y+y_vec >= 11 || y+y_vec <= 0){
            y += (y_vec = -y_vec);
        }
        else{ y += y_vec; }

        if(z+z_vec >= 11 || z+z_vec <= 0){
            z += (z_vec = -z_vec);
        }
        else{ z += z_vec; }

        drawBall(x,y,z, 0xFFFF);
        ////delay(200);
        //test of the github
    }
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void spiral() {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++) {
      set_led(j, i, 0, 255, 20*i, 9*i);
      delay(30);
    }
    for (int j = 0; j < 12; j++) {
      set_led(11, i, j, 255, 20*i, 9*i);
      delay(30);
    }
    for (int j = 11; j >= 0; j--) {
      set_led(j, i, 11, 255, 20*i, 9*i);
      delay(30);
    }
    for (int j = 11; j >= 0; j--) {
      set_led(0, i, j, 255, 20*i, 9*i);
      delay(30);
    }
  }
}
/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/

void swirl() {
    boolean start = false;
    clearCube();
    uint32_t timer = millis();
    while (true) {
        char c;

        if (keyboard.available()){
            c=keyboard.read();
            if (c == PS2_ESC) {
                break;
            }
        }
        if (!start) {
        for (int i = 0; i < 12; i++) {
            directionalCubeArray(i, i, 11, FORWARD, true, 0xF000);
            directionalCubeArray(i, i, 11, BACKWARD, true,  0xFF00);
            directionalCubeArray(i, i, 11, LEFT, true, 0xFFFF);
            directionalCubeArray(i, i, 11, RIGHT, true, 0xF00F);

        }
            
            for (int i = 1; i < 11; i++) {
                directionalCubeArray(i, 11-i, 10, FORWARD, true, 0xF800);
                directionalCubeArray(i, 11-i, 10, BACKWARD, true,  0x0770);
                directionalCubeArray(i, 11-i, 10, LEFT, true, 0x00EF);
                directionalCubeArray(i, 11-i, 10, RIGHT, true, 0xF00F);
            }
        
            start = true;
        }
        //uint16_t directionalCubeArray(uint8_t firstCord, uint8_t secondCord, uint8_t thirdCord, directions direction, boolean setLED = false, uint16_t color = 0){

        if (millis() - timer > 100) {
            for (int k = 0; k < 12; k++) {
                moveRow(11, k, FORWARD);
                moveRow(11, k, BACKWARD);
               moveRow(11, k, LEFT);
               moveRow(11, k, RIGHT);

            }
            //void moveRow(uint8_t firstCord, uint8_t secondCord,  directions direction,  uint16_t specificColor = 1, boolean collective = false, uint8_t start = 0, uint8_t end = 11, uint16_t color = 1){
            for (int i = 0; i < 4; i++) {
            for (int k = 1; k < 11; k++) {
                moveRow(1, k, directions(i+1), 1, false, 1, 10);
            }
            }
            
        
            timer = millis();
        }
    }
        //DO STUFF HERE
        
        
    
    clearCube();
}

/***************************************************************************
 *
 *   Sub Seperator
 *
 ***************************************************************************/


void test() {
    uint32_t timer = millis();
    uint16_t co = 0xFFFF;
    while (true) {
        char c;
        if (keyboard.available()){
            
            c=keyboard.read();
            if (c == PS2_ESC) {
                 break;
            }
        }
        if (millis() - timer > 50) {
            co = colorShifter(co);
            timer += 50;
            
            //Serial.println(((0xFFFF & 0xF800))>>8);

        }
        setPackedPackedLED(pk_coord(1, 1, 1), co);

        //DO STUFF HERE
        
    }
    clearCube();
}

/***************************************************************************
 *
 *
 *  BASIC 'APP' FUNCTION SETUP BELOW
 *
 *
 ***************************************************************************/
/*
 void NAME() {
 while (!kill) {
    char c;
    if (keyboard.available()){
 
        c=keyboard.read();
        if (c == PS2_ESC) {
            kill = true;
        }
    }

    //DO STUFF HERE
 
 
    }
 kill = false;
 clearCube();
 }
*/
