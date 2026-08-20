/*
 * pixxil_graphics.h
 *
 *  Created on: 14 May 2021
 *      Author: mehmedblazevic
 * Description: This file contains the functions who show
 * the desired object on the screen. This file need the
 * pixxil_driver.h/c files. Here we show every objects needed by
 * the design requirements. If requirements change, this file
 * should be updated.
 */

#ifndef PIXXIL_GRAPHICS_H_
#define PIXXIL_GRAPHICS_H_

#include "pixxil_driver.h"
#include "pb_alias_pxs.h"
//#include <ti/drivers/dpl/ClockP.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>


//--------------- Graphical objects positions ---------------

#define MTSC_BLACK  0x2104
//#define LCD_WIDTH       240
//#define LCD_HEIGHT      240
#define SPEED_Y_POS     60+8
#define SPEED_Y_HEIGHT  100

extern uint16_t LCD_WIDTH;
extern uint16_t LCD_HEIGHT;
#define PIXXIL_BAUDRATE 256000

#define POL1_Y_HEIGHT   25
#define POL2_Y_HEIGHT   25


#define POS_Y_BJR   50+8

#define NAME_Y_POS  100+8 // name

#define POS_Y_CARD  105 // cardinal
#define POS_X_CARD  200
#define HEIGHT_CARD 50
#define WIDTH_CARD  30

#define POS_Y_TRIP  20  // trip
#define POS_X_TRIP  75
#define POS_X2_TRIP POS_X_TRIP+100
#define POS_Y2_TRIP POS_Y_TRIP+30

#define POS_X_STAT  180  // status
#define POS_Y_STAT  50
#define POS_X2_STAT POS_X_STAT+40
#define POS_Y2_STAT POS_Y_STAT+40

#define POS_Y_DATE  190
#define POS_X_DATE  75
#define POS_Y2_DATE POS_Y_DATE+40
#define POS_X2_DATE POS_X_DATE+95


#define POS_X_PROGB     120.0 // progressbar
#define POS_Y_PROGB     120.0
#define POS_X_PROGBINT  120
#define INRAD_PROGB     105.0
#define INRAD_PROGBINT  105
#define PBPIXS_X       (POS_X_PROGBINT-INRAD_PROGBINT+40)
#define PBPIXS_Y        240
#define OUTRAD_PROGB    112.0
#define STARTA_PROGB    130 // 130
#define ENDA_PROGB      230 // 230
#define WHITE_PROGB     0xffff
#define HINT_PROGB      0x630c
#define ORANGE_PROGB    0xf36d
#define STEP_PB         (ENDA_PROGB-STARTA_PROGB)/100

#define POS_Y_SPEED  80    // speed constants
#define POS_X_SPEED  60
#define POS_Y2_SPEED POS_Y_SPEED+110
#define POS_X2_SPEED POS_X_SPEED+120

#define POS_X_GPS_CARD  100  // cardinal (GPS)
#define POS_Y_GPS_CARD  207
#define POS_X2_GPS_CARD POS_X_GPS_CARD+35
#define POS_Y2_GPS_CARD POS_Y_GPS_CARD+40

#define POS_Y_GPS_SPEED     25 // speed (GPS)
#define POS_X_GPS_SPEED     80
#define POS_Y2_GPS_SPEED    POS_Y_GPS_SPEED+35
#define POS_X2_GPS_SPEED    POS_X_GPS_SPEED+83
#define SPEED_GPS_Y_HEIGHT  35
#define SPEED_GPS_KMH_POS_X POS_X_GPS_SPEED+40

#define POS_X_GPS_TXT1      110 // texts (GPS)
#define POS_Y_GPS_TXT1      150
#define POS_Y_GPS_TXT2      POS_Y_GPS_TXT1+25
#define POS_X2_GPS_TXT1     POS_X_GPS_TXT1+70
#define POS_Y2_GPS_TXT1     POS_Y_GPS_TXT1+25
#define POS_Y2_GPS_TXT2     POS_Y_GPS_TXT2+25

// GPS remaining (arrow, point, ...)
#define POS_X_GPS_SQUARE        70
#define POS_Y_GPS_SQUARE        202
#define POS_X2_GPS_SQUARE       170
#define POS_Y2_GPS_SQUARE       205
#define SQUARE_COLOR            0xffff
#define ARROW_COLOR             0xffff
#define ARROW_WIDTH             5
#define POS_X1_GPS_ARROW        120
#define POS_Y1_GPS_ARROW        65
#define POS_X2_GPS_ARROW        170
#define POS_Y2_GPS_ARROW        140
#define POS_X3_GPS_ARROW        POS_X2_GPS_ARROW-ARROW_WIDTH
#define POS_Y3_GPS_ARROW        POS_Y2_GPS_ARROW+ARROW_WIDTH
#define POS_X4_GPS_ARROW        POS_X1_GPS_ARROW
#define POS_Y4_GPS_ARROW        POS_Y1_GPS_ARROW+ARROW_WIDTH+ARROW_WIDTH
#define POS_X5_GPS_ARROW        POS_X2_GPS_ARROW-100+ARROW_WIDTH+1
#define POS_Y5_GPS_ARROW        POS_Y3_GPS_ARROW
#define POS_X6_GPS_ARROW        POS_X2_GPS_ARROW-100
#define POS_Y6_GPS_ARROW        POS_Y2_GPS_ARROW

#define DIAM_PT                 7
#define GRAVITY_X_PT            120
#define GRAVITY_Y_PT            120
#define RADIUS_PT               105
#define PT_COLOR                0xffff
#define STARTA_PT               310
#define MINA_PT                 0
#define MAXA_PT                 280



#define ERR     -1
#define OK      0

typedef enum {North, South, East, West} Direction;
typedef enum {Normal, Eco, Sport} Vmode;
typedef enum {Rain, Sun, Snow, Storm} Weather;
typedef enum {None, Charging, Settings, Balancing} Status;

extern Pb_antia_pxs antiA_px[];

typedef struct{
    int8_t progValue;
    int16_t progColour;
    int8_t progHintVal;
    int16_t progHintCol;
}ProgressBar;


int16_t getLetter(char l,letters_type type);
int8_t setupHeights();

/*!
 *  @brief  Function which show the specific splash screen
 *
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showSplash();
/*!
 *  @brief  Show the "Bonjour" (Hello) message with the name of the
 *  person.
 *
 *  @param  name    The name of the person (or something more fun)
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showBonjour(char *name);
/*!
 *  @brief  Show the weather screen with the appropriate weather.
 *
 *  @param  weath    The weather to show: Rain, Sun, Snow or Storm
 *
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showWeather(Weather weath);
/*!
 *  @brief  Clear the weather view.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearWeather();

/*!
 *  @brief  This function show the normal view on the screen with
 *  default values. The speed, distance, progressbar, cardinals,
 *  status and date objects.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showNormalView(); // one function to call them all
/*!
 *  @brief  Show the speed on the screen. The function will show
 *  2 digits value and will update the one (or both) digit which
 *  changed compared to the previous display. This allows to
 *  speed up the display and reduces the blinks. The user can
 *  choose to force the update of both digits in the case of
 *  the first display of speed or if the screen is cleared.
 *
 *  @param  speed    The speed to show
 *  @param changed   Force the speed to be updated if true
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showSpeed(int8_t speed, bool changed);
/*!
 *  @brief  Shows the cardinal direction on the screen
 *
 *  @param  dir    The cardinal direction: North, South, East or West
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showCardinal(Direction dir);
int8_t showVmode(Vmode vmode);
/*!
 *  @brief  Show the trip distance remaining.
 *
 *  @param  trip    The value in kilometers
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showTrip(int16_t trip);
/*!
 *  @brief  Show the trip distance remaining with a letter.
 *
 *  @param  trip    The value in kilometers
 *  @param  p       The char to show (a, b or c)
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showTripP(int16_t trip, char p);
/*!
 *  @brief  Show the status of the bike. Actually there is 3
 *  main status. The normal where there is nothing particular
 *  to show despite the speed, the trip, etc. The charging status which
 *  shows the user when the bike is charging (or need to charge?).
 *  The last one is the settings status which shows when the bike
 *  need a service.
 *
 *  @param  stat    The status to display: None, Charging or Settings
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showStatus(Status stat);
/*!
 *  @brief  Show the date on the bottom of the screen.
 *
 *  @param  mounth    The mounth of the year
 *  @param  day       The day of the mounth
 *  @param  type      The font type Pol1, Pol2 or Pol3
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showDate(uint8_t mounth, uint8_t day, letters_type type);
/*!
 *  @brief  Show the date on the bottom of the screen.
 *
 *  @param  date    The date in string format. For exemple: "10.12"
 *  @param  type    The font type Pol1, Pol2 or Pol3
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showDatestr(char *date, letters_type type);

/*!
 *  @brief  Clear the cardinal object.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearCardinal();
int8_t clearVmode();
/*!
 *  @brief  Clear the speed object.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearSpeed();
/*!
 *  @brief  Clear the trip object.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearTrip();
/*!
 *  @brief  Clear the status object.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearStatus();
/*!
 *  @brief  Clear the date object.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearDate();


/*!
 *  @brief  Shows the GPS view which includes the big arrow, the
 *  navigation point and the rectangle on the bottom of the LCD.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showGPSView();
/*!
 *  @brief  Move the direction point to the desired angle position.
 *
 *  @param  ang    The angle of the direction point from 0° to 280°.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t moveGPSDir(int16_t ang);
/*!
 *  @brief  Shows the speed on the GPS view. This function is similar
 *  than showSpeed() function.
 *
 *  @param  speed    The speed to show
 *  @param  changed  If the speed changed
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showGPSSpeed(int8_t speed, bool changed);
/*!
 *  @brief  Show the cardinal direction in the GPS view.
 *
 *  @param  dir    The direction North, South, East or West
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showGPSCardinal(Direction dir);
int8_t showGPSVmode(Vmode vmode);
/*!
 *  @brief  Show the first txt on the GPS view.
 *
 *  @param  txt    The text to show for exemple: "23.12"
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */

int8_t showGPSTxt1(char *txt);
/*!
 *  @brief  Show the second text on the GPS view.
 *
 *  @param  txt    The text to show for exemple: "10.15"
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showGPSTxt2(char *txt);

/*!
 *  @brief  Clear the GPS view (navigation point, rectangle and arrow)
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSView();
/*!
 *  @brief  Clear the speed on the GPS view
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSSpeed();
/*!
 *  @brief  Clear the direction point on the GPS view
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSDir();
/*!
 *  @brief  Clear the first text on the GPS view
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSTxt1();
/*!
 *  @brief  Clear the second tet on the GPS view
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSTxt2();
/*!
 *  @brief  Clear the cardinal direction on the GPS view
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearGPSCardinal();
int8_t clearGPSVmode();

/*int8_t initPBAlia();
int8_t initPB(); */
/*!
 *  @brief  Show the progress bar (first time) on the normal view.
 *
 *  @param  pb    The progress bar values
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t showPB(ProgressBar pb);
int8_t showPB2(ProgressBar pb); // upside down
/*!
 *  @brief  Update the progress bar. Use this when the pb is
 *  already showed on the screen.
 *
 *  @param  pb    THe progress bar values
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t updatePB(ProgressBar pb);
/*!
 *  @brief  Clear the progress bar.
 *
 *  @return 0 if everything is fine, or -1 if something is wrong
 */
int8_t clearPB();


#endif /* PIXXIL_GRAPHICS_H_ */
