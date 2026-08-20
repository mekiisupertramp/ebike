/*
 * pixxil_driver.h
 *
 *  Created on: 11 May 2021
 *      Author: mehmedblazevic
 * Description: This file contains the functions needed to control
 * the PixxiLCD-13P2. Not every functions of the display are implemented.
 * The functions here are needed for our purpose only.
 */

#ifndef PIXXIL_DRIVER_H_
#define PIXXIL_DRIVER_H_

#include "pixxil_static.h"
#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/dpl/ClockP.h>
#include <math.h>
#include <stdint.h>
#include "ti_drivers_config.h"


#define OK              0
#define ERR             -1
#define ERR_WIDTH       -2

#define ACK         0x6

#define TIMEOUT_RX_SCREEN 10000 //10=1ms timout for stop wait read on screen

#define IMAGE_XPOS    2   // WORD image location X
#define IMAGE_YPOS    3   // WORD image location Y
#define IMAGE_WIDTH   4   // WORD image width
#define IMAGE_HEIGHT  5   // WORD image height
#define IMAGE_FLAGS   6   // WORD image flags
#define IMAGE_DELAY   7   // WORD inter frame delay
#define IMAGE_FRAMES  8   // WORD number of frames
#define IMAGE_INDEX   9   // WORD current frame

#define PIXXIL_PAQUET_SIZE 512
#define PIXXI_FLASH_SIZE (16*1024*1024)

extern const double sinu[];   // must be declared somewhere!
extern const double cosi[];   // same!

typedef struct {
    uint8_t x;
    uint8_t y;
    //int16_t colour;
} Pixel;

typedef enum {
    PB_110,
    PB_300,
    PB_600,
    PB_1200,
    PB_2400,
    PB_4800,
    PB_9600,
    PB_14400,
    PB_19200,
    PB_31250,
    PB_38400,
    PB_56000,
    PB_57600,
    PB_115200,
    PB_128000,
    PB_256000,
    PB_300000,
    PB_375000,
    PB_500000,
    PB_600000,
} pixiBaudrate;


/*!
 *  @brief  Function which initialize the Pixxil Display
 *
 *  @param  baudrate    The uart baudrate
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t init_Pixxil(uint32_t lLcdWidth,uint32_t lLcdHeight);

void close_Pixxil();

/*!
 *  @brief  Function which resets the screen
 *
 */
void rst_Pixxil();

/*!
 *  @brief  Function change baudrate of screen
 *
 */
void setBaudrate(pixiBaudrate baudrate);

/*!
 *  @brief  Pixxil's inner function. Initialize the memory-space of the screen.
 *
 */
void media_init();
/*!
 *  @brief  Pixxil's inner function. This function loads and return
 *  the handler for the screen. The handler is a memory pointer to the
 *  screen objects.
 *
 *  @return the handler if everything is fine, or -1 if something is wrong
 */
int16_t LoadImageControl();
/*!
 *  @brief  Pixxil's inner function. This function returns the image entry
 *  in the list. An image control must be created before by calling
 *  LoadImageControl() function. With the offset parameter, user can
 *  read those image's entry:
 *
 *
 *   IMAGE_LOWORD    0      WORD image address LO
 *   IMAGE_HIWORD    1      WORD image address HI
 *   IMAGE_XPOS      2      WORD image location X
 *   IMAGE_YPOS      3      WORD image location Y
 *   IMAGE_WIDTH     4      WORD image width
 *   IMAGE_HEIGHT    5      WORD image height
 *   IMAGE_FLAGS     6      WORD image flags
 *   IMAGE_DELAY     7      WORD inter frame delay
 *   IMAGE_FRAMES    8      WORD number of frames
 *   IMAGE_INDEX     9      WORD current frame
 *   IMAGE_CLUSTER  10      WORD image start cluster pos (for FAT16 only)
 *   IMAGE_SECTOR   11      WORD image start sector in cluster pos (for FAT16 only)
 *   IMAGE_TAG      12      WORD user variable #1
 *   IMAGE_TAG2     13      WORD user variable #2
 *
 *  @param  index       Index of the image in the list
 *  @param  offset      The offset of the required word in the image entry
 *
 *  @return the image entry if everything is fine, or -1 if something is wrong
 */
int16_t img_GetWord(int16_t index, int16_t offset);
/*!
 *  @brief  Pixxil's inner function. This function will set the image entry.
 *  The image entry can be chosen with the offset parameter and it can
 *  set those parameters:
 *
 *  IMAGE_XPOS      2       WORD image location X
 *  IMAGE_YPOS      3       WORD image location Y
 *  IMAGE_FLAGS     6       WORD image flags
 *  IMAGE_DELAY     7       WORD inter frame delay
 *  IMAGE_INDEX     9       WORD current frame
 *  IMAGE_TAG      12       WORD user variable #1
 *  IMAGE_TAG2     13       WORD user variable #2
 *
 *  @param  index       Index of the image in the list
 *  @param  offset      The offset of the required word in the image entry
 *  @param  value       The value to write in the register
 *
 *  @return 1 if everything is fine, or -1 if something is wrong
 */
int16_t img_SetWord(int16_t index, int16_t offset, int16_t value);
/*!
 *  @brief  Pixxil's inner function. Display the image entry in the
 *  image control.
 *
 *  @param  index       Index of the image in the list
 *
 *  @return 1 if everything is fine, or -1 if something is wrong
 */
int16_t img_Show(int16_t index);

/*!
 *  @brief  Display a text from a string in the desired position and in
 *  a specific font.
 *
 *  @param  txt       The string to show
 *  @param  x         X position of the text
 *  @param  y         Y position of the text
 *  @param  type      The font Type: Name, Speed, Pol1, Pol2, Pol3, Cardinals or Trip
 *
 *  @return x position of end of text
 */
int16_t img_showTxt(char *txt, uint8_t x, uint8_t y, letters_type type);
int16_t img_showTxtCentred(char *txt, uint8_t x, uint8_t y, letters_type type, uint32_t maxWidth);
int16_t img_showTxtRightJust(char *txt, uint8_t x, uint8_t y, letters_type type);

int16_t img_showTxtwithLetters(int16_t *letters, uint8_t nbrLetters, uint8_t x, uint8_t y, letters_type type); // useless?
/*!
 *  @brief  Custom function to count the number of letters in a string.
 *
 *  @param  name       The string to process
 *
 *  @return pos x of end of text
 */
int8_t mystrlen(char *name);
/*!
 *  @brief  Get the memory pointer (image entry) of every letter of a string
 *  for a specific font. Each pointer is the memory location of a specific
 *  letter in the screen memory. Those pointers will allow the user to show
 *  the letters stored in the screen.
 *
 *  @param  letters   The array where every entry will be stored
 *  @param  txt       The string to get every entry from
 *  @param  nbrL      The number of letters
 *  @param  type      The font Type: Name, Speed, Pol1, Pol2, Pol3, Cardinals or Trip
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t getLettersTxt(int16_t *letters, char *txt, int8_t nbrL, letters_type type);
/*!
 *  @brief  Get the memory pointer (image entry) from a specific letter. This entry
 *  allow the user to show the letter or modify the letter's parameters.
 *
 *  @param  l         The letter to get the entry from
 *  @param  type      The font Type: Name, Speed, Pol1, Pol2, Pol3, Cardinals or Trip
 *
 *  @return the entry pointer if everything is fine, or -1 if something is wrong
 */
int16_t getLetter(char l,letters_type type);
/*!
 *  @brief  Display a text from a string in the desired position and in
 *  a specific font.
 *
 *  @param  name       The string to get the length from
 *
 *  @return the number of letters
 */
int8_t mystrlen(char *name);
/*!
 *  @brief  Convert a string to int
 *
 *  @param  val       The value to convert
 *
 *  @return the value converted
 */
int8_t myatoi(char *val);

/*!
 *  @brief  Clear the screen.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */

int16_t gfx_showTxt(uint8_t x, uint8_t y, char *txt);
/*!
 *  @brief  Pixxil's inner function. Draw a rectangle.
 *
 *  @param  x1       x position
 *  @param  y1       y position
 *  @param  text     show text in gfx mode (no anti aliazing)
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */

int8_t gfx_CLS(uint16_t color);
/*!
 *  @brief  Pixxil's inner function. Draw a rectangle.
 *
 *  @param  x1       The first x position
 *  @param  y1       The first y position
 *  @param  x2       The second x position
 *  @param  y2       The second y position
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a filled rectangle.
 *
 *  @param  x1       The first x position
 *  @param  y1       The first y position
 *  @param  x2       The second x position
 *  @param  y2       The second y position
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_RectangleFilled(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a line.
 *
 *  @param  x1       The first x position
 *  @param  y1       The first y position
 *  @param  x2       The second x position
 *  @param  y2       The second y position
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a filled polygon from an array
 *  of pixels. The lines are drawed from pixel to following pixel.
 *
 *  @param  pixels   The array of pixels
 *  @param  n        The number of pixels
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_PolygonFilled(Pixel *pixels, uint16_t n, uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a polygon from an array
 *  of pixels. The lines are drawed from pixel to following pixel.
 *
 *  @param  pixels   The array of pixels
 *  @param  n        The number of pixels
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_Polygon(Pixel *pixels, uint16_t n, uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Set the background color. The
 *  background color will change after a gfx_CLS()
 *
 *  @param  color   The desired color
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_BGcolour(uint16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a pixel in the
 *  desired position.
 *
 *  @param  x        The x position of the pixel
 *  @param  y        The y position of the pixel
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_PutPixel1(uint8_t x, uint8_t y, int16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a pixel in the
 *  desired position.
 *
 *  @param  pix      The pixel to draw
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_PutPixel(Pixel pix, int16_t colour);
/*!
 *  @brief  Draw pixels from a given array.
 *
 *  @param  pixels   Array containing every pixels to draw
 *  @param  number   The number of pixels
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_PutPixels(Pixel *pixels, int16_t number, int16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a filled circle.
 *
 *  @param  pix      The position of the center
 *  @param  rad      The radius of the circle
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_CircleFilled(Pixel pix, int16_t rad, int16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a circle.
 *
 *  @param  pix      The position of the center
 *  @param  rad      The radius of the circle
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_Circle(Pixel pix, int16_t rad, int16_t colour);
/*!
 *  @brief  Pixxil's inner function. Draw a ring segment.
 *
 *  @param  x        The x position of the center of the ring segment
 *  @param  y        The y position of the center of the ring segment
 *  @param  inrad    The inner radius of the ring segment
 *  @param  outrad   The outer radius of the ring segment
 *  @param  start    The start angle of the ring segment
 *  @param  enda     The end angle of the ring segment
 *  @param  colour   The colour desired
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t gfx_RingSegment(uint8_t x, uint8_t y, uint8_t inrad, uint8_t outrad, uint16_t starta, uint16_t enda, int16_t colour);

//set color use for transparance
int8_t gfx_TransparentColour(uint16_t  Color);

//enable transparence
int8_t gfx_TransparentOn();

//set a rect where the screen is update when Cliping is enable
int8_t gfx_SetClipingWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

//see gfx_SetClipingWindows
int8_t gfx_SetCliping(bool  enable);

//see 5.2.31 Contrast
int8_t gfx_SetBackLight(uint8_t  ligthing);

//see 5.5.1 Contrast
int8_t gfx_SetSleep(uint16_t s);

//convert RGB to 16bit color
uint16_t rgbToRGB565(uint8_t red,uint8_t green,uint8_t blue);

void uart0ReadCallback(UART_Handle handle, void *rxBuf, size_t size);

void UARTWriteReadScreen(uint8_t* txBuffer, size_t txSize,uint8_t* rxBuffer, size_t rxSize);

void UARTWriteReadScreenNoTimeout(uint8_t* txBuffer, size_t txSize,uint8_t* rxBuffer, size_t rxSize);

#endif /* PIXXIL_DRIVER_H_ */
