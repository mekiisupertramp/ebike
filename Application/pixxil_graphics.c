/*
 * pixxil_graphics.c
 *
 *  Created on: 14 May 2021
 *      Author: mehmedblazevic
 * Description: This file contains the functions who show
 * the desired object on the screen. This file need the
 * pixxil_driver.h/c files. Here we show every objects needed by
 * the design requirements. If requirements change, this file
 * should be updated.
 */


#include "pixxil_graphics.h"
#include <stdint.h>


uint16_t LCD_WIDTH = 240;
uint16_t LCD_HEIGHT = 240;

// darken a colour from 100% (no change) to 0% (black)
int16_t smoothColour1(int16_t colour, int16_t smooth){
    int16_t r,g,b;
    int16_t result = 0x0000;

    r = (colour & 0xF800) >> 11;
    g = (colour & 0x07E0) >> 5;
    b = colour & 0x001F;

    r = (r*smooth)/100;
    g = (g*smooth)/100;
    b = (b*smooth)/100;

    result = (r<<11) | (g<<5) | b;
    return result;
}
typedef enum {No, Left, Right} Pt_dir;
typedef struct{
    Pixel pos;
    int16_t angle;
    Pt_dir dir;
}Point_t;

static Point_t pt;
// draw a point and erase it's previous position (just some pixels)
int8_t drawNormalizedPoint(Point_t p){
    int8_t res = OK;
    int16_t buffa = (p.angle+STARTA_PT)%360;
    int16_t buffb = buffa;
    Pixel pixs[3];

    if(p.dir == Left) buffb=(buffb-3)%360;
    else buffb=(buffb+3)%360;
    if(buffb<0)buffb+=360;

    pixs[0].x = GRAVITY_X_PT+((double)(RADIUS_PT-DIAM_PT-3)*cosi[buffb]);
    pixs[0].y = GRAVITY_Y_PT-((double)(RADIUS_PT-DIAM_PT-3)*sinu[buffb]);
    pixs[1].x = GRAVITY_X_PT+((double)(RADIUS_PT+DIAM_PT+3)*cosi[buffb]);
    pixs[1].y = GRAVITY_Y_PT-((double)(RADIUS_PT+DIAM_PT+3)*sinu[buffb]);

    if(p.dir == Left) buffb=(buffb-5)%360;
    else buffb=(buffb+5)%360;
    if(buffb<0)buffb+=360;

    pixs[2].x = GRAVITY_X_PT+((double)(RADIUS_PT)*cosi[buffb]);
    pixs[2].y = GRAVITY_Y_PT-((double)(RADIUS_PT)*sinu[buffb]);

    gfx_PolygonFilled(pixs,3,MTSC_BLACK);

    pt.pos.x = GRAVITY_X_PT+((double)(RADIUS_PT)*cosi[buffa]);
    pt.pos.y = GRAVITY_Y_PT-((double)(RADIUS_PT)*sinu[buffa]);

    // draw the point
    res = gfx_CircleFilled(pt.pos, DIAM_PT, PT_COLOR);
    return res;
}

// ang can go from 0 to 280
int8_t moveGPSDir(int16_t ang){
    int8_t res = 0;

    if((ang >= MINA_PT)&&(ang <= MAXA_PT)){
        if(ang>pt.angle)pt.dir = Left;
        else pt.dir = Right;

        while(pt.angle != ang){
            gfx_Circle(pt.pos,DIAM_PT+1,MTSC_BLACK);
            gfx_Circle(pt.pos,DIAM_PT,MTSC_BLACK);
            if(pt.dir == Left){
                pt.angle++;
            }else{
                pt.angle--;
            }
            res = drawNormalizedPoint(pt);
            if(res == ERR) return res;
        }
    }

    return res;
}

int8_t showGPSView(){
    Pixel arrow[7];
    int8_t res=OK;

    // draw the arrow
    arrow[0].x = POS_X1_GPS_ARROW;
    arrow[0].y = POS_Y1_GPS_ARROW;
    arrow[1].x = POS_X2_GPS_ARROW;
    arrow[1].y = POS_Y2_GPS_ARROW;

    arrow[2].x = POS_X3_GPS_ARROW;
    arrow[2].y = POS_Y3_GPS_ARROW;
    arrow[3].x = POS_X4_GPS_ARROW;
    arrow[3].y = POS_Y4_GPS_ARROW;

    arrow[4].x = POS_X5_GPS_ARROW;
    arrow[4].y = POS_Y5_GPS_ARROW;
    arrow[5].x = POS_X6_GPS_ARROW;
    arrow[5].y = POS_Y6_GPS_ARROW;

    arrow[6].x = POS_X1_GPS_ARROW-1;
    arrow[6].y = POS_Y1_GPS_ARROW;

    res = gfx_PolygonFilled(arrow, 7, ARROW_COLOR);
    if(res == ERR) return res;

    // draw the arrow blurred edges
    arrow[0].y = POS_Y1_GPS_ARROW-1;
    arrow[1].y = POS_Y2_GPS_ARROW-1;
    arrow[2].y = POS_Y3_GPS_ARROW+1;
    arrow[3].y = POS_Y4_GPS_ARROW+1;
    arrow[4].x = POS_X5_GPS_ARROW-1;
    arrow[4].y = POS_Y5_GPS_ARROW+1;
    res = gfx_Polygon(arrow, 7, smoothColour1(ARROW_COLOR, 40));
    if(res == ERR) return res;

    // draw rectangle
    res = gfx_RectangleFilled(POS_X_GPS_SQUARE,POS_Y_GPS_SQUARE,POS_X2_GPS_SQUARE,POS_Y2_GPS_SQUARE,SQUARE_COLOR);
    if(res == ERR) return res;

    // init the point
    pt.dir = No;
    pt.angle = 52;
    pt.pos.x = GRAVITY_X_PT+((double)(RADIUS_PT)*cosi[(pt.angle+STARTA_PT)%360]);
    pt.pos.y = GRAVITY_Y_PT-((double)(RADIUS_PT)*sinu[(pt.angle+STARTA_PT)%360]);

    // draw the point
    res = gfx_CircleFilled(pt.pos, DIAM_PT, PT_COLOR);

    return res;
}
int8_t showGPSSpeed(int8_t speed, bool changed){

    static int8_t spd_mem_tens = -1;
    static int8_t spd_mem_units = -1;
    static int8_t numWidth = -1;
    static int16_t posx_mem = -1;
    //uint16_t ivkmhposx = (LCD_WIDTH/2)-(img_GetWord(ivkmh, IMAGE_WIDTH)/2);

    int16_t numbers[2];
    int8_t speed_tens = speed/10;
    int8_t speed_units = speed%10;

    // if something changes, update posx at first
    if((speed_tens != spd_mem_tens) || (speed_units != spd_mem_units) || changed){
        numbers[0] = getLetter((char)(speed_tens+0x30),Pol3);
        numbers[1] = getLetter((char)(speed_units+0x30),Pol3);


        if(((speed_tens != spd_mem_tens)&&(speed_units != spd_mem_units))||changed){
            numWidth = img_GetWord(getLetter((char)('0'),Pol3), IMAGE_WIDTH);
            //posx_mem = ((LCD_WIDTH-(numWidth*2))/2)-ivkmhposx;
            posx_mem = SPEED_GPS_KMH_POS_X-2*numWidth;

            gfx_RectangleFilled(posx_mem, POS_Y_GPS_SPEED, SPEED_GPS_KMH_POS_X-2, POS_Y2_GPS_SPEED, MTSC_BLACK);
            img_SetWord(numbers[0],IMAGE_YPOS,POS_Y_GPS_SPEED);
            img_SetWord(numbers[0],IMAGE_XPOS,posx_mem);
            img_Show(numbers[0]);
            img_SetWord(numbers[1],IMAGE_YPOS,POS_Y_GPS_SPEED);
            img_SetWord(numbers[1],IMAGE_XPOS,posx_mem+numWidth);
            img_Show(numbers[1]);
            spd_mem_tens = speed_tens;
            spd_mem_units = speed_units;
        }else{
            if(speed_tens != spd_mem_tens) {
                gfx_RectangleFilled(posx_mem, POS_Y_GPS_SPEED, posx_mem+numWidth, POS_Y2_GPS_SPEED, MTSC_BLACK);
                img_SetWord(numbers[0],IMAGE_YPOS,POS_Y_GPS_SPEED);
                img_SetWord(numbers[0],IMAGE_XPOS,posx_mem);
                img_Show(numbers[0]);
                spd_mem_tens = speed_tens;
            }
            if(speed_units != spd_mem_units){
                gfx_RectangleFilled(posx_mem+numWidth, POS_Y_GPS_SPEED, posx_mem+2*numWidth, POS_Y2_GPS_SPEED, MTSC_BLACK);
                img_SetWord(numbers[1],IMAGE_YPOS,POS_Y_GPS_SPEED);
                img_SetWord(numbers[1],IMAGE_XPOS,posx_mem+numWidth);
                img_Show(numbers[1]);
                spd_mem_units = speed_units;
            }
        }
    }

    img_SetWord(ivkmh, IMAGE_YPOS, POS_Y_GPS_SPEED+15);
    img_SetWord(ivkmh, IMAGE_XPOS, SPEED_GPS_KMH_POS_X+5);
    img_Show(ivkmh);
    return OK;
}

int8_t showSpeed(int8_t speed, bool changed){
    static int8_t spd_mem_tens = -1;
    static int8_t spd_mem_units = -1;
    static int8_t numWidth = -1;
    static int16_t posx_mem = -1;
    uint16_t ivkmhposx = (LCD_WIDTH/2)-(img_GetWord(ivkmh, IMAGE_WIDTH)/2);

    int16_t numbers[2];
    int8_t speed_tens = speed/10;
    int8_t speed_units = speed%10;

    if((speed_tens != spd_mem_tens) || (speed_units != spd_mem_units) || changed){
        numbers[0] = getLetter((char)(speed_tens+0x30),Speed);
        numbers[1] = getLetter((char)(speed_units+0x30),Speed);


        if(((speed_tens != spd_mem_tens)&&(speed_units != spd_mem_units))||changed){
            numWidth = img_GetWord(getLetter((char)('0'),Speed), IMAGE_WIDTH);
            posx_mem = (LCD_WIDTH-(numWidth*2))/2;

            gfx_RectangleFilled(posx_mem, SPEED_Y_POS, posx_mem+2*numWidth, SPEED_Y_POS+SPEED_Y_HEIGHT, MTSC_BLACK);
            img_SetWord(numbers[0],IMAGE_YPOS,SPEED_Y_POS);
            img_SetWord(numbers[0],IMAGE_XPOS,posx_mem);
            img_Show(numbers[0]);
            img_SetWord(numbers[1],IMAGE_YPOS,SPEED_Y_POS);
            img_SetWord(numbers[1],IMAGE_XPOS,posx_mem+numWidth);
            img_Show(numbers[1]);
            spd_mem_tens = speed_tens;
            spd_mem_units = speed_units;
        }else{
            if(speed_tens != spd_mem_tens) {
                gfx_RectangleFilled(posx_mem, SPEED_Y_POS, posx_mem+numWidth, SPEED_Y_POS+SPEED_Y_HEIGHT, MTSC_BLACK);
                img_SetWord(numbers[0],IMAGE_YPOS,SPEED_Y_POS);
                img_SetWord(numbers[0],IMAGE_XPOS,posx_mem);
                img_Show(numbers[0]);
                spd_mem_tens = speed_tens;
            }
            if(speed_units != spd_mem_units){
                gfx_RectangleFilled(posx_mem+numWidth, SPEED_Y_POS, posx_mem+2*numWidth, SPEED_Y_POS+SPEED_Y_HEIGHT, MTSC_BLACK);
                img_SetWord(numbers[1],IMAGE_YPOS,SPEED_Y_POS);
                img_SetWord(numbers[1],IMAGE_XPOS,posx_mem+numWidth);
                img_Show(numbers[1]);
                spd_mem_units = speed_units;
            }
        }
    }
#ifndef DEBUG_TEMP
    img_SetWord(ivkmh, IMAGE_YPOS, SPEED_Y_POS+SPEED_Y_HEIGHT+10+24);
    img_SetWord(ivkmh, IMAGE_XPOS, ivkmhposx);
    img_Show(ivkmh);
#endif
    return OK;
}

int8_t clearSpeed(){
    return gfx_RectangleFilled(POS_X_SPEED,POS_Y_SPEED,POS_X2_SPEED,POS_Y2_SPEED,MTSC_BLACK);
}
int8_t showGPSVmode(Vmode vmode){
    int tmp=ERR;
    switch(vmode){
    case Normal: tmp = showGPSCardinal(North);
        break;
    case Eco: tmp = showGPSCardinal(East);
        break;
    case Sport:  tmp = showGPSCardinal(South);
        break;
    }
    return tmp;
}

int8_t showGPSCardinal(Direction dir){
    int16_t width;
    int16_t card;

    switch(dir){
    case North: card = itN;
        break;
    case South: card = itS;
        break;
    case West:  card = itO;
        break;
    case East:  card = itE;
        break;
    }

    width = img_GetWord(card, IMAGE_WIDTH);
    if(width == ERR) return ERR;

    if(img_SetWord(card, IMAGE_YPOS, POS_Y_GPS_CARD) == ERR) return ERR;
    if(img_SetWord(card, IMAGE_XPOS, (LCD_WIDTH-width)/2) == ERR) return ERR;
    if(img_Show(card) == ERR) return ERR;


    return OK;
}
int8_t showGPSTxt1(char *txt){
    int8_t strLen = mystrlen(txt);
    int16_t letters[strLen];
    int8_t letWidths[strLen];
    int i=0;
    int posx=POS_X_GPS_TXT1;

    if(getLettersTxt(letters, txt, strLen, Pol2) != OK) return ERR;

    // get letters width
    for(i=0 ; i<strLen ; i++){
        letWidths[i] = img_GetWord(letters[i], IMAGE_WIDTH);
        if(letWidths[i] == ERR) return ERR;
    }

    // show letters
    for(i=0 ; i<strLen ; i++){
        if(img_SetWord(letters[i], IMAGE_YPOS, POS_Y_GPS_TXT1) == ERR) return ERR;
        if(img_SetWord(letters[i], IMAGE_XPOS, posx) == ERR) return ERR;
        if(img_Show(letters[i]) == ERR) return ERR;
        posx +=letWidths[i];
    }

    return OK;
}
int8_t showGPSTxt2(char *txt){
    int8_t strLen = mystrlen(txt);
    int16_t letters[strLen];
    int8_t letWidths[strLen];
    int i=0;
    int posx=POS_X_GPS_TXT1;

    if(getLettersTxt(letters, txt, strLen, Pol1) != OK) return ERR;

    // get letters width
    for(i=0 ; i<strLen ; i++){
        letWidths[i] = img_GetWord(letters[i], IMAGE_WIDTH);
        if(letWidths[i] == ERR) return ERR;
    }

    // show letters
    for(i=0 ; i<strLen ; i++){
        if(img_SetWord(letters[i], IMAGE_YPOS, POS_Y_GPS_TXT2) == ERR) return ERR;
        if(img_SetWord(letters[i], IMAGE_XPOS, posx) == ERR) return ERR;
        if(img_Show(letters[i]) == ERR) return ERR;
        posx +=letWidths[i];
    }

    return OK;
}

int8_t clearGPSView(){
    int8_t res = gfx_RectangleFilled(POS_X_GPS_SQUARE,POS_Y_GPS_SQUARE,POS_X2_GPS_SQUARE,POS_Y2_GPS_SQUARE,MTSC_BLACK);
    if(res == ERR) return res;
    res = gfx_RectangleFilled(POS_X1_GPS_ARROW-55,POS_Y1_GPS_ARROW-2,POS_X1_GPS_ARROW+55,POS_Y3_GPS_ARROW+1,MTSC_BLACK);
    return res;
}
int8_t clearGPSSpeed(){
    return gfx_RectangleFilled(POS_X_GPS_SPEED-2,POS_Y_GPS_SPEED+8,POS_X2_GPS_SPEED,POS_Y2_GPS_SPEED,MTSC_BLACK);
}
int8_t clearGPSDir(){
    return gfx_CircleFilled(pt.pos, DIAM_PT+2, MTSC_BLACK);
}
int8_t clearGPSTxt1(){
    return gfx_RectangleFilled(POS_X_GPS_TXT1-2,POS_Y_GPS_TXT1,POS_X2_GPS_TXT1,POS_Y2_GPS_TXT1,MTSC_BLACK);
}
int8_t clearGPSTxt2(){
    return gfx_RectangleFilled(POS_X_GPS_TXT1-2,POS_Y_GPS_TXT2,POS_X2_GPS_TXT1,POS_Y2_GPS_TXT2,MTSC_BLACK);
}
int8_t clearGPSVmode(){return clearGPSCardinal();}
int8_t clearGPSCardinal(){
    return gfx_RectangleFilled(POS_X_GPS_CARD,POS_Y_GPS_CARD+2,POS_X2_GPS_CARD,POS_Y2_GPS_CARD,MTSC_BLACK);
}

static char cursor;

// for each pixel's position return a percentage from 0 to 100 (corresponds to progressBar)
int16_t pos_to_perc(Pb_antia_pxs pxs){
    float posx = (float)pxs.posx-120.0;
    float posy = 120.0-(float)pxs.posy;
    int16_t degrees = (int16_t)((atanf(posy/posx)*(180.0/M_PI))+0.5);
    return -1*(degrees-49);
}

int8_t showPB(ProgressBar pb){

    cursor = pb.progValue;

    int i=0;
    for(i=0 ; i<1546 ; i++){
        if(pos_to_perc(antiA_px[i]) < cursor){
            gfx_PutPixel1(antiA_px[i].posx, antiA_px[i].posy, smoothColour1(pb.progColour, antiA_px[i].hint));
        }else{
            gfx_PutPixel1(antiA_px[i].posx, antiA_px[i].posy, smoothColour1(pb.progHintCol, antiA_px[i].hint));
        }
    }
    return OK;
}
int8_t showPB2(ProgressBar pb){

    cursor = pb.progValue;

    int i=0;
    for(i=1546 ; i>-1 ; i--){
        if(pos_to_perc(antiA_px[i]) < cursor){
            gfx_PutPixel1(antiA_px[i].posx, antiA_px[i].posy, smoothColour1(pb.progColour, antiA_px[i].hint));
        }else{
            gfx_PutPixel1(antiA_px[i].posx, antiA_px[i].posy, smoothColour1(pb.progHintCol, antiA_px[i].hint));
        }
    }
    return OK;
}
int8_t updatePB(ProgressBar pb){
//    static int16_t hintCol = HINT_PROGB;
//
//    if(hintCol != pb.progHintCol){
//        if(pb.progHintCol == (short)ORANGE_PROGB){
//            showPB(pb);
//        }
////        }else{
////            showPB2(pb);
////        }
//        hintCol = pb.progHintCol;
//    }else{
        // if battery increasing
        if(pb.progValue > cursor){
            showPB2(pb);
        }else{ // if battery decreasing
            if(pb.progValue < cursor){
                showPB(pb);
            }
        }
   // }
    cursor = pb.progValue;
    return OK;
}

int8_t clearPB(){
      return gfx_RectangleFilled(0,0,POS_X_PROGB-INRAD_PROGB+40,LCD_HEIGHT,MTSC_BLACK);
}

int8_t clearWeather(); // useless?
int8_t showWeather(Weather weath){
    uint8_t width=0;
    uint8_t height=0;
    switch(weath){
        case Rain:
            width = img_GetWord(irain, IMAGE_WIDTH);
            height = img_GetWord(irain, IMAGE_HEIGHT);
            img_SetWord(irain, IMAGE_XPOS, (LCD_WIDTH-width)/2);
            img_SetWord(irain, IMAGE_YPOS, (LCD_HEIGHT-height)/2);
            img_Show(irain);
            break;
        case Snow:
            width = img_GetWord(isnow, IMAGE_WIDTH);
            height = img_GetWord(isnow, IMAGE_HEIGHT);
            img_SetWord(isnow, IMAGE_XPOS, (LCD_WIDTH-width)/2);
            img_SetWord(isnow, IMAGE_YPOS, (LCD_HEIGHT-height)/2);
            img_Show(isnow);
            break;
        case Storm:
            // nothing for now
            return 2;
            break;
        case Sun:
            return 2;
            // nothing for now
            break;
    }
    return OK;
}

int8_t clearStatus(){
    return gfx_RectangleFilled(POS_X_STAT-2, POS_Y_STAT-2, POS_X2_STAT, POS_Y2_STAT, MTSC_BLACK);
}

int8_t showStatus(Status stat){
    static Status oldStat = None;
//    if(oldStat != stat){
        switch(stat){
            case None:
                clearStatus();
                break;
            case Charging:
                img_Show(ibLightning);
                break;
            case Balancing:
                img_SetWord(ilightning, IMAGE_XPOS, POS_X_STAT);
                img_SetWord(ilightning, IMAGE_YPOS, POS_Y_STAT);
                img_Show(ilightning);
                break;
            case Settings:
                img_SetWord(isetting, IMAGE_XPOS, POS_X_STAT);
                img_SetWord(isetting, IMAGE_YPOS, POS_Y_STAT);
                img_Show(isetting);
                break;
        }
        oldStat = stat;
//    }
    return OK;
}
int8_t clearDate(){
    return gfx_RectangleFilled(POS_X_DATE, POS_Y_DATE-2, POS_X2_DATE, POS_Y2_DATE, MTSC_BLACK);
}
int8_t clearTrip(){
    return gfx_RectangleFilled(POS_X_TRIP, POS_Y_TRIP-5, POS_X2_TRIP, POS_Y2_TRIP, MTSC_BLACK);
}
// count the number of zeros, don't show useless ones
uint8_t tripNZeros(char *trip){
    int i=0;
    for(i=0 ; i<4 ; i++){if(trip[i] != '0') return i;}
    return 3;
}

int8_t showDatestr(char *date, letters_type type){
    int8_t strLen = mystrlen(date);
    int16_t letters[strLen];
    int8_t letWidths[strLen];
    int16_t widthTot=0;
    int i=0;
    int posx=0;

    if(getLettersTxt(letters, date, strLen, type) != OK) return ERR;

    // get letters width
    for(i=0 ; i<strLen ; i++){
        letWidths[i] = img_GetWord(letters[i], IMAGE_WIDTH);
        if(letWidths[i] == ERR) return ERR;
        widthTot += letWidths[i];
    }
    posx = (LCD_WIDTH-widthTot)/2;

    // show letters
    for(i=0 ; i<strLen ; i++){
        if(img_SetWord(letters[i], IMAGE_YPOS, POS_Y_DATE) == ERR) return ERR;
        if(img_SetWord(letters[i], IMAGE_XPOS, posx) == ERR) return ERR;
        if(img_Show(letters[i]) == ERR) return ERR;
        posx +=letWidths[i];
    }

    return OK;

}
int8_t showDate(uint8_t mounth, uint8_t day, letters_type type){
    char date[] = {mounth/10+0x30, mounth%10+0x30, ':',day/10+0x30, day%10+0x30, 0};
    return showDatestr(date, type);
}
int8_t showTripP(int16_t trip, char p){
    char tripString[] = {p,(trip/1000)+0x30, ((trip/100)%10)+0x30, ((trip/10)%10)+0x30, (trip%10)+0x30, 0};
    uint8_t posZero = tripNZeros(&tripString[1]);
    int8_t strLen = mystrlen(tripString);
    int16_t letters[strLen];
    int8_t letWidths[strLen];
    int16_t widthTot=0;
    int i=0;
    int posx=0;

    if(getLettersTxt(letters, tripString, 1, Trip) != OK) return ERR; // get first char
    if(getLettersTxt(&letters[1], &tripString[1+posZero], 4-posZero, Trip) != OK) return ERR; // then others without useless zeros

    // get letters width
    for(i=0 ; i<4-posZero+1 ; i++){
        letWidths[i] = img_GetWord(letters[i], IMAGE_WIDTH);
        if(letWidths[i] == ERR) return ERR;
        widthTot += letWidths[i];
    }
    widthTot += img_GetWord(itkm, IMAGE_WIDTH);
    posx = (LCD_WIDTH-widthTot)/2;

    // show first letter
    if(img_SetWord(letters[0], IMAGE_YPOS, POS_Y_TRIP+5) == ERR) return ERR;
    if(img_SetWord(letters[0], IMAGE_XPOS, posx) == ERR) return ERR;
    if(img_Show(letters[0]) == ERR) return ERR;
    posx+=letWidths[0]+5;

    // show letters
    for(i=1 ; i<4-posZero+1 ; i++){
        if(img_SetWord(letters[i], IMAGE_YPOS, POS_Y_TRIP) == ERR) return ERR;
        if(img_SetWord(letters[i], IMAGE_XPOS, posx) == ERR) return ERR;
        if(img_Show(letters[i]) == ERR) return ERR;
        posx +=letWidths[i];
    }
    // show "km"
    if(img_SetWord(itkm, IMAGE_YPOS, POS_Y_TRIP+5) == ERR) return ERR;
    if(img_SetWord(itkm, IMAGE_XPOS, posx+5) == ERR) return ERR;
    if(img_Show(itkm) == ERR) return ERR;

    return OK;
}
int8_t showTrip(int16_t trip){
    char tripString[] = {(trip/1000)+0x30, ((trip/100)%10)+0x30, ((trip/10)%10)+0x30, (trip%10)+0x30, 0};
    uint8_t posZero = tripNZeros(tripString);
    int16_t letters[4-tripNZeros(tripString)];
    int8_t letWidths[4-tripNZeros(tripString)];
    int16_t widthTot=0;
    int i=0;
    int posx=0;

    if(getLettersTxt(letters, &tripString[posZero], 4-posZero, Trip) != OK) return ERR;

    // get letters width
    for(i=0 ; i<4-posZero ; i++){
        letWidths[i] = img_GetWord(letters[i], IMAGE_WIDTH);
        if(letWidths[i] == ERR) return ERR;
        widthTot += letWidths[i];
    }
    widthTot += img_GetWord(itkm, IMAGE_WIDTH);
    posx = (LCD_WIDTH-widthTot)/2;

    // show letters
    for(i=0 ; i<4-posZero ; i++){
        if(img_SetWord(letters[i], IMAGE_YPOS, POS_Y_TRIP) == ERR) return ERR;
        if(img_SetWord(letters[i], IMAGE_XPOS, posx) == ERR) return ERR;
        if(img_Show(letters[i]) == ERR) return ERR;
        posx +=letWidths[i];
    }
    // show "km"
#ifndef DEBUG_TEMP
    if(img_SetWord(itkm, IMAGE_YPOS, POS_Y_TRIP+5) == ERR) return ERR;
    if(img_SetWord(itkm, IMAGE_XPOS, posx+5) == ERR) return ERR;
    if(img_Show(itkm) == ERR) return ERR;
#endif

    return OK;
}

int8_t clearCardinal(){
    return gfx_RectangleFilled(POS_X_CARD, POS_Y_CARD, POS_X_CARD+WIDTH_CARD, POS_Y_CARD+HEIGHT_CARD, MTSC_BLACK);
}
int8_t setCardinalPos(int pix_card){
    if(img_SetWord(pix_card, IMAGE_YPOS, POS_Y_CARD) == ERR) return ERR;
    if(img_SetWord(pix_card, IMAGE_XPOS, POS_X_CARD) == ERR) return ERR;
    if(img_Show(pix_card) == ERR) return ERR;
    return OK;
}
int8_t clearVmode(){
    return gfx_RectangleFilled(POS_X_CARD, POS_Y_CARD, POS_X_CARD+WIDTH_CARD, POS_Y_CARD+HEIGHT_CARD, MTSC_BLACK);
}
int8_t showVmode(Vmode vmode){
    int tmp=ERR;
    switch(vmode){
    case Normal: tmp = setCardinalPos(itN);
        break;
    case Eco: tmp = setCardinalPos(itE);
        break;
    case Sport:  tmp = setCardinalPos(itS);
        break;
    }
    return tmp;
}
int8_t showCardinal(Direction dir){
    int tmp=ERR;
    switch(dir){
    case North: tmp = setCardinalPos(itN);
        break;
    case South: tmp = setCardinalPos(itS);
        break;
    case West:  tmp = setCardinalPos(itO);
        break;
    case East:  tmp = setCardinalPos(itE);
        break;
    }
    return tmp;
}
int8_t showBonjour(char *name){
    int16_t posx;
    int i=0;
    int16_t totWidth=0;

    // get total width of the name
    for(i=0 ; i<mystrlen(name) ; i++){
        totWidth += img_GetWord(getLetter(name[i], Name), IMAGE_WIDTH);
    }
    // calculate if it can be showed on the screen (error if not)
    if(totWidth > LCD_WIDTH-5) return ERR_WIDTH;

    posx = (LCD_WIDTH-totWidth)/2;

    img_SetWord(iBONJOURTXT, IMAGE_YPOS, POS_Y_BJR);
    img_Show(iBONJOURTXT);
    img_showTxt(name, posx, NAME_Y_POS, Name);
    return OK;
}

int8_t showSplash(){
    int16_t frames = img_GetWord(isplash, IMAGE_FRAMES);
    int i;

    for (i = 0; i < frames; i++) {
        img_SetWord(isplash, IMAGE_INDEX, i);
        img_Show(isplash);
        //ClockP_usleep(5000);
    }
    //ClockP_sleep(1);
    //gfx_CLS();
    return OK;
}

int16_t getLetter(char l,letters_type type){
    switch(type){
        case Name:
                switch(l){
                    case 'a': return ina;
                    case 'b': return inb;
                    case 'c': return inc;
                    case 'd': return ind;
                    case 'e': return ine;
                    case 'f': return inf;
                    case 'g': return ing;
                    case 'h': return inh;
                    case 'i': return ini;
                    case 'j': return inj;
                    case 'k': return ink;
                    case 'l': return inl;
                    case 'm': return inm;
                    case 'n': return inn;
                    case 'o': return ino;
                    case 'p': return inp;
                    case 'q': return inq;
                    case 'r': return inr;
                    case 's': return ins;
                    case 't': return intt;
                    case 'u': return inu;
                    case 'v': return inv;
                    case 'w': return inw;
                    case 'x': return inx;
                    case 'y': return iny;
                    case 'z': return inz;
                    case 'é': return inea;
                    case 'è': return ineg;
                    case '-': return intir;

                    case 'A': return inA;
                    case 'B': return inB;
                    case 'C': return inC;
                    case 'D': return inD;
                    case 'E': return inE;
                    case 'F': return inF;
                    case 'G': return inG;
                    case 'H': return inH;
                    case 'I': return inI;
                    case 'J': return inJ;
                    case 'K': return inK;
                    case 'L': return inL;
                    case 'M': return inM;
                    case 'N': return inN;
                    case 'O': return inO;
                    case 'P': return inP;
                    case 'Q': return inQ;
                    case 'R': return inR;
                    case 'S': return inS;
                    case 'T': return inT;
                    case 'U': return inU;
                    case 'V': return inV;
                    case 'W': return inW;
                    case 'X': return inX;
                    case 'Y': return inY;
                    case 'Z': return inZ;

                    case '0': return in0;
                    case '1': return in1;
                    case '2': return in2;
                    case '3': return in3;
                    case '4': return in4;
                    case '5': return in5;
                    case '6': return in6;
                    case '7': return in7;
                    case '8': return in8;
                    case '9': return in9;

                    default: return ERR;
                }
        case Speed:
            switch(l){
                    case '0': return iv0;
                    case '1': return iv1;
                    case '2': return iv2;
                    case '3': return iv3;
                    case '4': return iv4;
                    case '5': return iv5;
                    case '6': return iv6;
                    case '7': return iv7;
                    case '8': return iv8;
                    case '9': return iv9;

                    default: return ERR;
                }

        case Pol1:
            switch(l){
                    case '0': return ia0;
                    case '1': return ia1;
                    case '2': return ia2;
                    case '3': return ia3;
                    case '4': return ia4;
                    case '5': return ia5;
                    case '6': return ia6;
                    case '7': return ia7;
                    case '8': return ia8;
                    case '9': return ia9;
                    case '.': return iap;

                    default: return ERR;
                }
        case Pol2:
            switch(l){
                    case '0': return ib0;
                    case '1': return ib1;
                    case '2': return ib2;
                    case '3': return ib3;
                    case '4': return ib4;
                    case '5': return ib5;
                    case '6': return ib6;
                    case '7': return ib7;
                    case '8': return ib8;
                    case '9': return ib9;
                    case '.': return ibp;

                    default: return ERR;
                }
        case Pol3:
            switch(l){
                    case '0': return ic0;
                    case '1': return ic1;
                    case '2': return ic2;
                    case '3': return ic3;
                    case '4': return ic4;
                    case '5': return ic5;
                    case '6': return ic6;
                    case '7': return ic7;
                    case '8': return ic8;
                    case '9': return ic9;
                    case '.': return icp;
                    case ':': return icpp;

                    default: return ERR;
                }
        case Cardinals:
            switch(l){
                    case 'N': return itN;
                    case 'S': return itS;
                    case 'O': return itO;
                    case 'E': return itE;

                    default: return ERR;
                }
        case Trip:
            switch(l){
                    case '0': return it0;
                    case '1': return it1;
                    case '2': return it2;
                    case '3': return it3;
                    case '4': return it4;
                    case '5': return it5;
                    case '6': return it6;
                    case '7': return it7;
                    case '8': return it8;
                    case '9': return it9;
                    case 'a': return ita;
                    case 'b': return itb;
                    case 'c': return itc;

                    default: return ERR;
                }
    }
    return ERR;
}

int8_t setupHeights(){
    // set height for each speed font (avoid colition with "km/h" string)
    img_SetWord(iv0,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv1,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv2,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv3,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv4,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv5,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv6,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv7,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv8,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
    img_SetWord(iv9,IMAGE_HEIGHT,SPEED_Y_HEIGHT);
   /* img_SetWord(iv0,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv1,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv2,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv3,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv4,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv5,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv6,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv7,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv8,IMAGE_WIDTH,SPEED_X_WIDTH);
    img_SetWord(iv9,IMAGE_WIDTH,SPEED_X_WIDTH);*/
    // set height for each pol1 font, which avoid collisions
    img_SetWord(ia0,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia1,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia2,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia3,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia4,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia5,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia6,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia7,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia8,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(ia9,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    img_SetWord(iap,IMAGE_HEIGHT,POL1_Y_HEIGHT);
    // same for pol2
    img_SetWord(ib0,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib1,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib2,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib3,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib4,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib5,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib6,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib7,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib8,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ib9,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    img_SetWord(ibp,IMAGE_HEIGHT,POL2_Y_HEIGHT);
    return OK;
}
