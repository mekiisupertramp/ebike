/*
 * bms.c
 *
 *  Created on: 28 nov. 2022
 *      Author: Mehmed Blazevic
 */

#include "bigBerta.h"
#include "tiny_BMS.h"
#include "vesc_can.h"
#include "config.h"
#include "OCV_25degC.h"

//calculate the SOC of the battery by integration.
#define MAIN_VOLTAGE   48.0
#define ACCES_VOLTAGE  12.0

#define TINYBMS_ACTIVE_POWER (0.005*MAIN_VOLTAGE)
#define BB_LB_VEILLE_POWER (0.008*MAIN_VOLTAGE)
#define BB_LB_ACTIVE_POWER (0.021*MAIN_VOLTAGE)
#define VESC_ACTIVE_POWER (0.044*MAIN_VOLTAGE)
#define FRONT_LIGHT_POWER (0.034*MAIN_VOLTAGE)
#define REAR_LIGHT_LOW_POWER (0.012*MAIN_VOLTAGE)
#define REAR_LIGHT_HIGH_POWER (0.034*MAIN_VOLTAGE)
#define HORN_POWER (1.4*ACCES_VOLTAGE)
#define TRACY_MOD_LOW (0.005*ACCES_VOLTAGE)
#define TRACY_MOD_HIGH (0.050*ACCES_VOLTAGE)

#define NB_CELLS    13
#define BATCAPACITY 17000

volatile float soc; //0 to 1

void bmsLbInit() //initialize SOC call in bmsLbInteg (wait valid data from BMS)
{
    int l;
    float uCel=bms_get_ubat()/NB_CELLS;
    for(l=0;l<sizeof(OCV25deg)/sizeof(OCV25deg[0]);l++) //estimate soc with voltage use OCV25deg table this tabe cantain open circuit voltage of battery
    {
        if(uCel<OCV25deg[l])
            break;
    }
    soc=(float)l/(sizeof(OCV25deg)/sizeof(OCV25deg[0]));  //fixme ICI PLUTOT RECUPERER LE SOC DU TINYBMS
}

void bmsLbInteg(bool active, float dt) // call each time we read the SOC from BMS (or fixed period) -> dt periode between 2 call of this function
{
    float ubat=bms_get_ubat();
    float iTinyBms=bms_get_ibat();

    //fix bad current mesurement on tinybms (occasionally tinyBms measure small current when the bike is off)
    if(!active) //if bike is OFF
    {
        if(iTinyBms<0 && iTinyBms>-1) //if the surrent is betwhen 0 and -1A
        {
            iTinyBms=0;
        }
    }

    if(ubat>20)//bms have valide data
    {
        if(soc<0.01 && ubat/NB_CELLS>OCV25deg[200] && iTinyBms<1 && iTinyBms>-1) //is soc is >1 % but tension say > 20% and we have a low current
        {
            bmsLbInit(); //comput soc from voltage
        }

        float iestimate=-TINYBMS_ACTIVE_POWER/ubat;

        if(active)//if bike is ON
        {
            for(int l=0;l<NB_VESC;l++) //estimate current by aditioning current of all elements
            {
                iestimate-=vesc_get_value(l).current_in;
            }
            iestimate-=(BB_LB_ACTIVE_POWER+VESC_ACTIVE_POWER)/ubat; //every time active in active mode
            if(bigBertaGetLight())
            {
                iestimate-=(FRONT_LIGHT_POWER+REAR_LIGHT_LOW_POWER*2)/ubat; //add light
                if(bigBertaGetBrakeLight())
                    iestimate-=(REAR_LIGHT_HIGH_POWER-REAR_LIGHT_LOW_POWER)*2/ubat;//add brake light and remove rear light
            }
            else
            {
                if(bigBertaGetBrakeLight())
                    iestimate-=(REAR_LIGHT_HIGH_POWER)*2/ubat;
            }
            if(bigBertaGetHorn())
            {
                iestimate-=(HORN_POWER)/ubat;
            }

            iestimate-=(REAR_LIGHT_HIGH_POWER*bigBertaGetNbBlink())/ubat; //add blink current bigBertaGetNbBlink return 1 if right or left 2 if warning we have 2 or 4 blink but ON half time
        }
        else
        {
            //if bike is of only LB and BB consumes
            iestimate-=BB_LB_VEILLE_POWER/ubat;
        }

        if(iTinyBms<iestimate || iTinyBms>0 ) //if current estimate and bms are not the same take the biger discharge current or if in charge take TinyBms current
        {
            iestimate=iTinyBms;
        }

        soc+=iestimate/(BATCAPACITY/1000.f)*dt/60/60; //integration of current for compute SOC

        if(soc>1) //limite soc bethwen 1 and 0
            soc=1;
        if(soc<0)
            soc=0;
        if(iTinyBms<2 && (ubat/NB_CELLS>4.1) && (bms_get_status()==BMS_S_CHARGING)) //if in charge but low current and voltage ~=max voltage
            soc=1;//full charged
    }
}

float bmsLbGetSoc() //Accessors
{
    return soc*100;
}
