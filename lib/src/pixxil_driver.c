/*
 * pixxil_driver.c
 *
 *  Created on: 11 May 2021
 *      Author: mehmedblazevic
 * Description: This file contains the functions needed to control
 * the PixxiLCD-13P2. Not every functions of the display are implemented.
 * The functions here are needed for our purpose only.
 */

#include "pixxil_driver.h"
#include "string.h"

int8_t setupHeights(); //fefined in pixxi_graphics.c

// pre-calculated sinus points (0 to 360°)
const double sinu[] = {
0.0 ,
0.01745240643728351 ,
0.03489949670250097 ,
0.052335956242943835 ,
0.0697564737441253 ,
0.08715574274765817 ,
0.10452846326765347 ,
0.12186934340514748 ,
0.13917310096006544 ,
0.15643446504023087 ,
0.17364817766693033 ,
0.1908089953765448 ,
0.20791169081775934 ,
0.224951054343865 ,
0.24192189559966773 ,
0.25881904510252074 ,
0.27563735581699916 ,
0.29237170472273677 ,
0.3090169943749474 ,
0.3255681544571567 ,
0.3420201433256687 ,
0.35836794954530027 ,
0.374606593415912 ,
0.39073112848927377 ,
0.4067366430758002 ,
0.42261826174069944 ,
0.4383711467890774 ,
0.45399049973954675 ,
0.4694715627858908 ,
0.48480962024633706 ,
0.49999999999999994 ,
0.5150380749100542 ,
0.5299192642332049 ,
0.5446390350150271 ,
0.5591929034707469 ,
0.573576436351046 ,
0.5877852522924731 ,
0.6018150231520483 ,
0.6156614753256583 ,
0.6293203910498374 ,
0.6427876096865393 ,
0.6560590289905073 ,
0.6691306063588582 ,
0.6819983600624985 ,
0.6946583704589973 ,
0.7071067811865475 ,
0.7193398003386511 ,
0.7313537016191705 ,
0.7431448254773942 ,
0.7547095802227719 ,
0.766044443118978 ,
0.7771459614569708 ,
0.7880107536067219 ,
0.7986355100472928 ,
0.8090169943749473 ,
0.8191520442889917 ,
0.8290375725550416 ,
0.838670567945424 ,
0.848048096156426 ,
0.8571673007021123 ,
0.8660254037844386 ,
0.8746197071393957 ,
0.8829475928589269 ,
0.8910065241883678 ,
0.898794046299167 ,
0.9063077870366499 ,
0.9135454576426009 ,
0.9205048534524404 ,
0.9271838545667874 ,
0.9335804264972017 ,
0.9396926207859083 ,
0.9455185755993167 ,
0.9510565162951535 ,
0.9563047559630354 ,
0.9612616959383189 ,
0.9659258262890683 ,
0.9702957262759965 ,
0.9743700647852352 ,
0.9781476007338056 ,
0.981627183447664 ,
0.984807753012208 ,
0.9876883405951378 ,
0.9902680687415704 ,
0.992546151641322 ,
0.9945218953682733 ,
0.9961946980917455 ,
0.9975640502598242 ,
0.9986295347545738 ,
0.9993908270190958 ,
0.9998476951563913 ,
1.0 ,
0.9998476951563913 ,
0.9993908270190958 ,
0.9986295347545738 ,
0.9975640502598242 ,
0.9961946980917455 ,
0.9945218953682733 ,
0.9925461516413221 ,
0.9902680687415704 ,
0.9876883405951378 ,
0.984807753012208 ,
0.981627183447664 ,
0.9781476007338057 ,
0.9743700647852352 ,
0.9702957262759965 ,
0.9659258262890683 ,
0.9612616959383189 ,
0.9563047559630355 ,
0.9510565162951536 ,
0.9455185755993168 ,
0.9396926207859084 ,
0.9335804264972017 ,
0.9271838545667874 ,
0.9205048534524403 ,
0.9135454576426009 ,
0.90630778703665 ,
0.8987940462991669 ,
0.8910065241883679 ,
0.8829475928589269 ,
0.8746197071393959 ,
0.8660254037844388 ,
0.8571673007021123 ,
0.8480480961564261 ,
0.838670567945424 ,
0.8290375725550418 ,
0.8191520442889917 ,
0.8090169943749475 ,
0.7986355100472928 ,
0.788010753606722 ,
0.7771459614569711 ,
0.7660444431189781 ,
0.7547095802227721 ,
0.7431448254773942 ,
0.7313537016191707 ,
0.7193398003386511 ,
0.7071067811865476 ,
0.6946583704589971 ,
0.6819983600624985 ,
0.6691306063588583 ,
0.6560590289905073 ,
0.6427876096865395 ,
0.6293203910498374 ,
0.6156614753256584 ,
0.6018150231520482 ,
0.5877852522924732 ,
0.5735764363510459 ,
0.5591929034707469 ,
0.5446390350150273 ,
0.5299192642332049 ,
0.5150380749100544 ,
0.49999999999999994 ,
0.48480962024633717 ,
0.4694715627858907 ,
0.45399049973954686 ,
0.4383711467890773 ,
0.4226182617406995 ,
0.40673664307580043 ,
0.39073112848927377 ,
0.37460659341591224 ,
0.3583679495453002 ,
0.3420201433256689 ,
0.3255681544571566 ,
0.3090169943749475 ,
0.2923717047227366 ,
0.2756373558169992 ,
0.258819045102521 ,
0.24192189559966773 ,
0.2249510543438652 ,
0.2079116908177593 ,
0.19080899537654497 ,
0.17364817766693025 ,
0.15643446504023098 ,
0.1391731009600653 ,
0.12186934340514755 ,
0.10452846326765373 ,
0.0871557427476582 ,
0.06975647374412552 ,
0.05233595624294381 ,
0.03489949670250114 ,
0.017452406437283435 ,
1.2246467991473532e-16 ,
-0.017452406437283637 ,
-0.03489949670250089 ,
-0.05233595624294356 ,
-0.06975647374412527 ,
-0.08715574274765794 ,
-0.10452846326765348 ,
-0.12186934340514731 ,
-0.13917310096006552 ,
-0.15643446504023076 ,
-0.17364817766693047 ,
-0.19080899537654475 ,
-0.2079116908177595 ,
-0.22495105434386498 ,
-0.2419218955996675 ,
-0.2588190451025208 ,
-0.275637355816999 ,
-0.2923717047227368 ,
-0.3090169943749473 ,
-0.3255681544571568 ,
-0.34202014332566866 ,
-0.35836794954530043 ,
-0.374606593415912 ,
-0.39073112848927355 ,
-0.4067366430758002 ,
-0.4226182617406993 ,
-0.43837114678907746 ,
-0.4539904997395467 ,
-0.46947156278589086 ,
-0.48480962024633695 ,
-0.5000000000000001 ,
-0.5150380749100542 ,
-0.5299192642332048 ,
-0.5446390350150271 ,
-0.5591929034707467 ,
-0.5735764363510462 ,
-0.587785252292473 ,
-0.6018150231520484 ,
-0.6156614753256582 ,
-0.6293203910498376 ,
-0.6427876096865393 ,
-0.6560590289905072 ,
-0.6691306063588582 ,
-0.6819983600624984 ,
-0.6946583704589974 ,
-0.7071067811865475 ,
-0.7193398003386512 ,
-0.7313537016191705 ,
-0.7431448254773942 ,
-0.7547095802227719 ,
-0.7660444431189779 ,
-0.7771459614569706 ,
-0.788010753606722 ,
-0.7986355100472928 ,
-0.8090169943749473 ,
-0.8191520442889916 ,
-0.8290375725550418 ,
-0.838670567945424 ,
-0.8480480961564258 ,
-0.8571673007021121 ,
-0.8660254037844384 ,
-0.8746197071393959 ,
-0.882947592858927 ,
-0.8910065241883678 ,
-0.8987940462991668 ,
-0.9063077870366502 ,
-0.913545457642601 ,
-0.9205048534524403 ,
-0.9271838545667873 ,
-0.9335804264972016 ,
-0.9396926207859084 ,
-0.9455185755993168 ,
-0.9510565162951535 ,
-0.9563047559630353 ,
-0.961261695938319 ,
-0.9659258262890683 ,
-0.9702957262759965 ,
-0.9743700647852351 ,
-0.9781476007338056 ,
-0.981627183447664 ,
-0.984807753012208 ,
-0.9876883405951377 ,
-0.9902680687415703 ,
-0.9925461516413221 ,
-0.9945218953682734 ,
-0.9961946980917455 ,
-0.9975640502598242 ,
-0.9986295347545738 ,
-0.9993908270190958 ,
-0.9998476951563913 ,
-1.0 ,
-0.9998476951563913 ,
-0.9993908270190958 ,
-0.9986295347545738 ,
-0.9975640502598243 ,
-0.9961946980917455 ,
-0.9945218953682734 ,
-0.992546151641322 ,
-0.9902680687415704 ,
-0.9876883405951378 ,
-0.9848077530122081 ,
-0.9816271834476639 ,
-0.9781476007338056 ,
-0.9743700647852352 ,
-0.9702957262759966 ,
-0.9659258262890684 ,
-0.9612616959383188 ,
-0.9563047559630354 ,
-0.9510565162951536 ,
-0.945518575599317 ,
-0.9396926207859083 ,
-0.9335804264972017 ,
-0.9271838545667874 ,
-0.9205048534524405 ,
-0.9135454576426011 ,
-0.9063077870366498 ,
-0.898794046299167 ,
-0.891006524188368 ,
-0.8829475928589271 ,
-0.8746197071393956 ,
-0.8660254037844386 ,
-0.8571673007021123 ,
-0.8480480961564262 ,
-0.8386705679454243 ,
-0.8290375725550416 ,
-0.8191520442889918 ,
-0.8090169943749476 ,
-0.7986355100472932 ,
-0.7880107536067218 ,
-0.7771459614569708 ,
-0.7660444431189781 ,
-0.7547095802227722 ,
-0.7431448254773947 ,
-0.7313537016191705 ,
-0.7193398003386512 ,
-0.7071067811865477 ,
-0.6946583704589976 ,
-0.6819983600624983 ,
-0.6691306063588581 ,
-0.6560590289905074 ,
-0.6427876096865396 ,
-0.6293203910498378 ,
-0.6156614753256582 ,
-0.6018150231520483 ,
-0.5877852522924732 ,
-0.5735764363510464 ,
-0.5591929034707466 ,
-0.544639035015027 ,
-0.529919264233205 ,
-0.5150380749100545 ,
-0.5000000000000004 ,
-0.48480962024633684 ,
-0.4694715627858908 ,
-0.45399049973954697 ,
-0.4383711467890778 ,
-0.42261826174069916 ,
-0.40673664307580015 ,
-0.3907311284892739 ,
-0.3746065934159123 ,
-0.35836794954530077 ,
-0.34202014332566855 ,
-0.3255681544571567 ,
-0.3090169943749476 ,
-0.2923717047227371 ,
-0.27563735581699894 ,
-0.2588190451025207 ,
-0.24192189559966787 ,
-0.22495105434386534 ,
-0.20791169081775987 ,
-0.19080899537654467 ,
-0.1736481776669304 ,
-0.15643446504023112 ,
-0.13917310096006588 ,
-0.12186934340514724 ,
-0.10452846326765342 ,
-0.08715574274765832 ,
-0.06975647374412565 ,
-0.05233595624294437 ,
-0.03489949670250082 ,
-0.01745240643728356
};
// pre-calculated cosinus points (0 to 360°)
const double cosi[] = {
1.0 ,
0.9998476951563913 ,
0.9993908270190958 ,
0.9986295347545738 ,
0.9975640502598242 ,
0.9961946980917455 ,
0.9945218953682733 ,
0.992546151641322 ,
0.9902680687415704 ,
0.9876883405951378 ,
0.984807753012208 ,
0.981627183447664 ,
0.9781476007338057 ,
0.9743700647852352 ,
0.9702957262759965 ,
0.9659258262890683 ,
0.9612616959383189 ,
0.9563047559630354 ,
0.9510565162951535 ,
0.9455185755993168 ,
0.9396926207859084 ,
0.9335804264972017 ,
0.9271838545667874 ,
0.9205048534524404 ,
0.9135454576426009 ,
0.9063077870366499 ,
0.898794046299167 ,
0.8910065241883679 ,
0.882947592858927 ,
0.8746197071393957 ,
0.8660254037844387 ,
0.8571673007021123 ,
0.848048096156426 ,
0.838670567945424 ,
0.8290375725550416 ,
0.8191520442889918 ,
0.8090169943749475 ,
0.7986355100472928 ,
0.7880107536067219 ,
0.7771459614569709 ,
0.7660444431189781 ,
0.754709580222772 ,
0.7431448254773942 ,
0.7313537016191705 ,
0.7193398003386512 ,
0.7071067811865476 ,
0.6946583704589974 ,
0.6819983600624985 ,
0.6691306063588582 ,
0.6560590289905074 ,
0.6427876096865394 ,
0.6293203910498375 ,
0.6156614753256583 ,
0.6018150231520484 ,
0.5877852522924732 ,
0.5735764363510462 ,
0.5591929034707469 ,
0.5446390350150271 ,
0.5299192642332049 ,
0.5150380749100542 ,
0.5000000000000001 ,
0.48480962024633717 ,
0.46947156278589086 ,
0.4539904997395468 ,
0.4383711467890774 ,
0.42261826174069944 ,
0.40673664307580015 ,
0.3907311284892737 ,
0.37460659341591196 ,
0.3583679495453004 ,
0.3420201433256688 ,
0.32556815445715676 ,
0.30901699437494745 ,
0.29237170472273677 ,
0.27563735581699916 ,
0.25881904510252074 ,
0.24192189559966767 ,
0.22495105434386492 ,
0.20791169081775945 ,
0.19080899537654492 ,
0.17364817766693041 ,
0.15643446504023092 ,
0.13917310096006547 ,
0.12186934340514748 ,
0.10452846326765344 ,
0.08715574274765812 ,
0.06975647374412523 ,
0.052335956242943966 ,
0.03489949670250108 ,
0.0174524064372836 ,
6.123233995736766e-17 ,
-0.017452406437283473 ,
-0.034899496702500955 ,
-0.05233595624294384 ,
-0.06975647374412534 ,
-0.08715574274765824 ,
-0.10452846326765355 ,
-0.12186934340514738 ,
-0.13917310096006535 ,
-0.1564344650402308 ,
-0.1736481776669303 ,
-0.1908089953765448 ,
-0.20791169081775934 ,
-0.22495105434386503 ,
-0.24192189559966779 ,
-0.25881904510252085 ,
-0.27563735581699905 ,
-0.29237170472273666 ,
-0.30901699437494734 ,
-0.32556815445715664 ,
-0.3420201433256687 ,
-0.35836794954530027 ,
-0.37460659341591207 ,
-0.3907311284892738 ,
-0.40673664307580026 ,
-0.42261826174069933 ,
-0.4383711467890775 ,
-0.45399049973954675 ,
-0.4694715627858909 ,
-0.48480962024633695 ,
-0.4999999999999998 ,
-0.5150380749100542 ,
-0.5299192642332048 ,
-0.5446390350150271 ,
-0.5591929034707467 ,
-0.5735764363510462 ,
-0.587785252292473 ,
-0.6018150231520484 ,
-0.6156614753256582 ,
-0.6293203910498373 ,
-0.6427876096865393 ,
-0.6560590289905072 ,
-0.6691306063588582 ,
-0.6819983600624984 ,
-0.6946583704589974 ,
-0.7071067811865475 ,
-0.7193398003386513 ,
-0.7313537016191705 ,
-0.743144825477394 ,
-0.754709580222772 ,
-0.7660444431189779 ,
-0.7771459614569709 ,
-0.7880107536067219 ,
-0.7986355100472929 ,
-0.8090169943749473 ,
-0.8191520442889919 ,
-0.8290375725550416 ,
-0.8386705679454239 ,
-0.848048096156426 ,
-0.8571673007021121 ,
-0.8660254037844387 ,
-0.8746197071393957 ,
-0.882947592858927 ,
-0.8910065241883678 ,
-0.898794046299167 ,
-0.9063077870366499 ,
-0.9135454576426008 ,
-0.9205048534524404 ,
-0.9271838545667873 ,
-0.9335804264972017 ,
-0.9396926207859083 ,
-0.9455185755993168 ,
-0.9510565162951535 ,
-0.9563047559630355 ,
-0.9612616959383189 ,
-0.9659258262890682 ,
-0.9702957262759965 ,
-0.9743700647852351 ,
-0.9781476007338057 ,
-0.981627183447664 ,
-0.9848077530122081 ,
-0.9876883405951377 ,
-0.9902680687415704 ,
-0.992546151641322 ,
-0.9945218953682733 ,
-0.9961946980917455 ,
-0.9975640502598242 ,
-0.9986295347545738 ,
-0.9993908270190958 ,
-0.9998476951563913 ,
-1.0 ,
-0.9998476951563913 ,
-0.9993908270190958 ,
-0.9986295347545738 ,
-0.9975640502598242 ,
-0.9961946980917455 ,
-0.9945218953682733 ,
-0.9925461516413221 ,
-0.9902680687415703 ,
-0.9876883405951378 ,
-0.984807753012208 ,
-0.981627183447664 ,
-0.9781476007338056 ,
-0.9743700647852352 ,
-0.9702957262759965 ,
-0.9659258262890683 ,
-0.9612616959383189 ,
-0.9563047559630354 ,
-0.9510565162951536 ,
-0.9455185755993167 ,
-0.9396926207859084 ,
-0.9335804264972016 ,
-0.9271838545667874 ,
-0.9205048534524404 ,
-0.9135454576426009 ,
-0.90630778703665 ,
-0.8987940462991669 ,
-0.8910065241883679 ,
-0.8829475928589269 ,
-0.8746197071393959 ,
-0.8660254037844386 ,
-0.8571673007021123 ,
-0.8480480961564261 ,
-0.838670567945424 ,
-0.8290375725550418 ,
-0.8191520442889917 ,
-0.8090169943749475 ,
-0.7986355100472928 ,
-0.788010753606722 ,
-0.7771459614569708 ,
-0.7660444431189781 ,
-0.7547095802227721 ,
-0.7431448254773942 ,
-0.7313537016191707 ,
-0.7193398003386511 ,
-0.7071067811865477 ,
-0.6946583704589973 ,
-0.6819983600624986 ,
-0.6691306063588581 ,
-0.6560590289905074 ,
-0.6427876096865396 ,
-0.6293203910498378 ,
-0.6156614753256582 ,
-0.6018150231520483 ,
-0.5877852522924732 ,
-0.5735764363510464 ,
-0.5591929034707466 ,
-0.544639035015027 ,
-0.529919264233205 ,
-0.5150380749100545 ,
-0.5000000000000004 ,
-0.48480962024633684 ,
-0.46947156278589075 ,
-0.4539904997395469 ,
-0.43837114678907774 ,
-0.4226182617406991 ,
-0.4067366430758001 ,
-0.3907311284892738 ,
-0.3746065934159123 ,
-0.3583679495453007 ,
-0.3420201433256685 ,
-0.32556815445715664 ,
-0.30901699437494756 ,
-0.29237170472273705 ,
-0.2756373558169989 ,
-0.25881904510252063 ,
-0.2419218955996678 ,
-0.22495105434386528 ,
-0.20791169081775981 ,
-0.1908089953765446 ,
-0.17364817766693033 ,
-0.15643446504023106 ,
-0.13917310096006583 ,
-0.12186934340514717 ,
-0.10452846326765335 ,
-0.08715574274765825 ,
-0.06975647374412558 ,
-0.052335956242944306 ,
-0.034899496702500754 ,
-0.017452406437283498 ,
-1.8369701987210297e-16 ,
0.01745240643728313 ,
0.03489949670250128 ,
0.052335956242943946 ,
0.06975647374412522 ,
0.08715574274765789 ,
0.104528463267653 ,
0.12186934340514768 ,
0.13917310096006544 ,
0.15643446504023067 ,
0.17364817766692997 ,
0.1908089953765451 ,
0.20791169081775943 ,
0.2249510543438649 ,
0.24192189559966742 ,
0.2588190451025203 ,
0.2756373558169994 ,
0.29237170472273677 ,
0.30901699437494723 ,
0.3255681544571563 ,
0.34202014332566905 ,
0.3583679495453004 ,
0.37460659341591196 ,
0.3907311284892735 ,
0.40673664307579976 ,
0.4226182617406996 ,
0.4383711467890774 ,
0.45399049973954664 ,
0.4694715627858904 ,
0.4848096202463372 ,
0.5 ,
0.515038074910054 ,
0.5299192642332047 ,
0.5446390350150266 ,
0.559192903470747 ,
0.573576436351046 ,
0.5877852522924729 ,
0.6018150231520479 ,
0.6156614753256585 ,
0.6293203910498375 ,
0.6427876096865393 ,
0.656059028990507 ,
0.6691306063588578 ,
0.6819983600624986 ,
0.6946583704589973 ,
0.7071067811865475 ,
0.7193398003386509 ,
0.7313537016191707 ,
0.7431448254773942 ,
0.7547095802227719 ,
0.7660444431189779 ,
0.7771459614569706 ,
0.788010753606722 ,
0.7986355100472928 ,
0.8090169943749473 ,
0.8191520442889916 ,
0.8290375725550418 ,
0.838670567945424 ,
0.8480480961564258 ,
0.8571673007021121 ,
0.8660254037844384 ,
0.8746197071393959 ,
0.882947592858927 ,
0.8910065241883678 ,
0.8987940462991668 ,
0.90630778703665 ,
0.913545457642601 ,
0.9205048534524403 ,
0.9271838545667873 ,
0.9335804264972015 ,
0.9396926207859084 ,
0.9455185755993168 ,
0.9510565162951535 ,
0.9563047559630353 ,
0.9612616959383189 ,
0.9659258262890683 ,
0.9702957262759965 ,
0.9743700647852351 ,
0.9781476007338056 ,
0.981627183447664 ,
0.984807753012208 ,
0.9876883405951377 ,
0.9902680687415703 ,
0.9925461516413221 ,
0.9945218953682733 ,
0.9961946980917455 ,
0.9975640502598242 ,
0.9986295347545738 ,
0.9993908270190958 ,
0.9998476951563913
};

// global variables for uart communication
UART_Handle uart;
static UART_Params uartParams;
volatile bool flagRead=0;
void uart0ReadCallback(UART_Handle handle, void *rxBuf, size_t size);

// handler used for accessing the Pixxil-LCD objects
static int16_t hndlr;

uint32_t lcdWidth;
uint32_t lcdHeight;
bool error=0; //error with the scren (connection issue ...)

int8_t init_Pixxil(uint32_t lLcdWidth,uint32_t lLcdHeight){

    error=0;
    lcdHeight=lLcdHeight;
    lcdWidth=lLcdWidth;
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 9600;
    uartParams.readCallback=uart0ReadCallback;
    uartParams.readMode       = UART_MODE_CALLBACK;
    //uartParams.readTimeout=10;



    uart = UART_open(PIXXI_UART, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    GPIO_setConfig(LCD_RST, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW); // not needed i think, already called in gpio_init

    // reset the screen
    rst_Pixxil();

    //while(1)
        setBaudrate(PB_256000); //if screen are in 9600 baud change it to 256000

    UART_close(uart);
    uartParams.baudRate = 256000;
    uart = UART_open(PIXXI_UART, &uartParams);

    // first function to call
    media_init();

    hndlr = LoadImageControl(); // create the handler to access the screen's memory

    setupHeights(); // setup heights

    return 0;
}

void UARTWriteReadScreen(uint8_t* txBuffer, size_t txSize,uint8_t* rxBuffer, size_t rxSize)
{
    flagRead=0;
    UART_read(uart, rxBuffer, rxSize); //prepare to read
    UART_write(uart,(const void *)txBuffer,txSize); //send commande
    int timeout=0;
    while(flagRead==0)//wait end of read
    {
        if((timeout>TIMEOUT_RX_SCREEN) || error)
        {
            error=1;
            UART_readCancel(uart);
            break;
        }
        timeout++;
        ClockP_usleep(100);
    }
}

void UARTWriteReadScreenNoTimeout(uint8_t* txBuffer, size_t txSize,uint8_t* rxBuffer, size_t rxSize)
{
    flagRead=0;
    UART_read(uart, rxBuffer, rxSize); //prepare to read
    UART_write(uart,(const void *)txBuffer,txSize); //send commande
    while(flagRead==0)//wait end of read
    {
        ClockP_usleep(1000);
    }
}

void setBaudrate(pixiBaudrate baudrate)
{
    uint8_t ack[3];
    uint8_t cmd[] = {0x00, 0x26, 0x00, baudrate};
    UART_read(uart, ack, sizeof(ack)); //prepare to read
    UART_write(uart,(const void *)cmd,sizeof(cmd)); //send commande
    ClockP_usleep(100000);//wait ready can not be read because bad baudrate
    UART_readCancel(uart);//
}

void uart0ReadCallback(UART_Handle handle, void *rxBuf, size_t size)
{
        flagRead=1;
}

void close_Pixxil(){
    UART_close(uart);
}

int16_t img_showTxt(char *txt, uint8_t x, uint8_t y, letters_type type){
    int8_t nbrLetters=mystrlen(txt);
    int16_t letters[nbrLetters];
    int8_t i=0;
    int16_t posx=x;

    if(getLettersTxt(letters, txt, nbrLetters, type) != 0) return ERR;

    for(i=0 ; i<nbrLetters ; i++){
        img_SetWord(letters[i], IMAGE_YPOS, y);
        img_SetWord(letters[i], IMAGE_XPOS, posx);
        img_Show(letters[i]);
        posx += img_GetWord(letters[i], IMAGE_WIDTH);
    }
    return posx;
}

int16_t img_showTxtCentred(char *txt, uint8_t x, uint8_t y, letters_type type, uint32_t maxWidth){
    int8_t nbrLetters=mystrlen(txt);
    int16_t letters[nbrLetters];
    int8_t i=0;
    int16_t length=0;

    if(getLettersTxt(letters, txt, nbrLetters, type) != 0) return ERR;

    for(i=0 ; i<nbrLetters ; i++){
        int charWidth = img_GetWord(letters[i], IMAGE_WIDTH);
        if(charWidth<0)
        	return x;
        length += charWidth;
        if(length>maxWidth)
        {
            length-=charWidth;
            img_showTxtCentred(txt+i,x,y+img_GetWord(letters[i], IMAGE_HEIGHT),type,maxWidth);
            nbrLetters=i;
            break;
        }
    }

    int pos=x-length/2;
    if(pos<0)
        pos=0;
    return img_showTxtwithLetters(letters,nbrLetters,pos,y,type);
}

int16_t img_showTxtRightJust(char *txt, uint8_t x, uint8_t y, letters_type type){
    int8_t nbrLetters=mystrlen(txt);
    int16_t letters[nbrLetters];
    int8_t i=0;
    int16_t posx=x;

    if(getLettersTxt(letters, txt, nbrLetters, type) != 0) return ERR;

    for(i=nbrLetters-1 ; i>=0 ; i--){
        posx -= img_GetWord(letters[i], IMAGE_WIDTH);
        img_SetWord(letters[i], IMAGE_YPOS, y);
        img_SetWord(letters[i], IMAGE_XPOS, posx);
        img_Show(letters[i]);
    }
    return posx;
}

int16_t img_showTxtwithLetters(int16_t *letters, uint8_t nbrLetters, uint8_t x, uint8_t y, letters_type type){
    int8_t i=0;
    int16_t posx = x;

    for(i=0 ; i<nbrLetters ; i++){
        img_SetWord(letters[i], IMAGE_YPOS, y);
        img_SetWord(letters[i], IMAGE_XPOS, posx);
        img_Show(letters[i]);
        posx += img_GetWord(letters[i], IMAGE_WIDTH);
    }
    return posx;
}

int8_t getLettersTxt(int16_t *letters, char *txt, int8_t nbrL, letters_type type){
    int8_t i = 0;
    for(i=0 ; i<nbrL ; i++){
        letters[i] = getLetter(txt[i],type);
        if(letters[i] == ERR) return ERR;
    }
    return 0;
}
int8_t mystrlen(char *name){
    int8_t cpt = 1;
    while(name[cpt] != 0) cpt++;
    return cpt;
}
int8_t myatoi(char *str){
   int8_t res = 0;
   int i;
   for (i = 0; str[i] != '\0'; ++i)
       res = res * 10 + str[i] - '0';

   return res;
}
volatile int debugUart=2;

int16_t img_GetWord(int16_t index, int16_t offset){
    uint8_t ack[3];
    uint8_t cmd[] = {0xFF, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = hndlr >> 8;
    cmd[3] = hndlr;
    cmd[4] = index >> 8;
    cmd[5] = index;
    cmd[6] = offset >> 8;
    cmd[7] = offset;
    debugUart=22;
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    debugUart=21;
    //ClockP_usleep(1);
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
    debugUart=20;
    return (ack[0] == ACK) ? (((int16_t)ack[1]<<8) | ack[2]) : ERR;
}


int16_t img_SetWord(int16_t index, int16_t offset, int16_t value){
    uint8_t ack[3];
    uint8_t cmd[] = {0xFF, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = hndlr >> 8;
    cmd[3] = hndlr;
    cmd[4] = index >> 8;
    cmd[5] = index;
    cmd[6] = offset >> 8;
    cmd[7] = offset;
    cmd[8] = value >> 8;
    cmd[9] = value;
    debugUart=12;
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    debugUart=11;
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
    debugUart=10;
    return (ack[0] == ACK) ? ack[2] : ERR;
}

int16_t img_Show(int16_t index){
    uint8_t ack[3];
    uint8_t cmd[] = {0xFF, 0x47, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = hndlr >> 8;
    cmd[3] = hndlr;
    cmd[4] = index >> 8;
    cmd[5] = index;
    debugUart=2;
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    debugUart=1;
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
    debugUart=0;

    return (ack[0] == ACK) ? ack[2] : ERR;
}


int16_t LoadImageControl(){
    uint8_t ack[3];
    uint8_t cmd[] = {0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
    return (ack[0] == ACK) ? (((int16_t)ack[1]<<8) | ack[2]) : ERR;
}
void media_init(){
    uint8_t ack[3];
    uint8_t cmd[] = {0xFF, 0x89};
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
}
void rst_Pixxil(){
    // assert reset for the screen
    GPIO_write(LCD_RST, 0);
    ClockP_usleep(10000);
    GPIO_write(LCD_RST, 1);
    ClockP_sleep(3);    // need to wait at least 3 sec for screen to be ready
}

int16_t gfx_showTxt(uint8_t x, uint8_t y, char *txt){

	uint8_t ack[3] = {0};
	uint8_t cmd[] = {0xFF, 0xE9, 0x00, 0x00, 0x00, 0x00};
	cmd[2] = 0;
	cmd[3] = y;
	cmd[4] = 0;
	cmd[5] = x;
	UARTWriteReadScreen(cmd,sizeof(cmd),ack,1);
	if(ack[0] != ACK)
		return ERR;

	uint8_t cmdString[] = {0x00, 0x18};
	UART_write(uart,(const void *)cmdString,sizeof(cmdString));
	UARTWriteReadScreen((uint8_t*)txt,strlen(txt)+1,ack,sizeof(ack));

	return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_CLS(uint16_t color){
    // resets everything, not a good idea!
    /*uint8_t ack = 0;
    const uint8_t cmd[] = {0xFF, 0xCD};
    UART_write(uart,(const void *)cmd,sizeof(cmd));
    MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;*/
    return gfx_RectangleFilled(0,0,lcdWidth,lcdHeight,color);
}
int8_t gfx_PutPixels(Pixel *pixels, int16_t number, int16_t colour){
    int16_t cpt=0;
    while(cpt<number){
        gfx_PutPixel(pixels[cpt], colour);
        cpt++;
    }
    return OK;
}
int8_t gfx_PutPixel1(uint8_t x, uint8_t y, int16_t colour){
    Pixel pix = {.x = x, .y = y};
    return gfx_PutPixel(pix,colour);
}
int8_t gfx_PutPixel(Pixel pix, int16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = 0;
    cmd[3] = pix.x;
    cmd[4] = 0;
    cmd[5] = pix.y;
    cmd[6] = colour>>8;
    cmd[7] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}
int8_t gfx_CircleFilled(Pixel pix, int16_t rad, int16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = 0;
    cmd[3] = pix.x;
    cmd[4] = 0;
    cmd[5] = pix.y;
    cmd[6] = rad>>8;
    cmd[7] = rad&0x00FF;
    cmd[8] = colour>>8;
    cmd[9] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}

int8_t gfx_Circle(Pixel pix, int16_t rad, int16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[2] = 0;
    cmd[3] = pix.x;
    cmd[4] = 0;
    cmd[5] = pix.y;
    cmd[6] = rad>>8;
    cmd[7] = rad&0x00FF;
    cmd[8] = colour>>8;
    cmd[9] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}

int16_t myabs(int16_t val){
    if(val<0) return val+360;
    return val;
}
// darker a colour from 100% (no change) to 0% (black)
int16_t smoothColour(int16_t colour, int16_t smooth){
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

int8_t gfx_RingSegment(uint8_t x, uint8_t y, uint8_t inrad, uint8_t outrad, uint16_t starta, uint16_t enda, int16_t colour){
   Pixel pxs[myabs(enda-starta)];
   int i=0;
   uint16_t angle = starta;
   int8_t radius = inrad+((outrad-inrad)/2);
   int8_t pen_rad = (outrad-inrad)/2;
   int16_t col_darker = smoothColour(colour,20);
   int16_t col_dark = smoothColour(colour,60);
   //Pixel tmp;

   // 2 loops would be maybe smoother when drawing?
  for(i=0 ; i<myabs(enda-starta) ; i++){
      // draw the point
      pxs[i].x = x+((double)radius*cosi[angle]);
      pxs[i].y = y-((double)radius*sinu[angle]);
      //gfx_CircleFilled(pxs[i], pen_rad, colour);

      angle = (angle+1)%360;
  }
  for(i=0 ; i<myabs(enda-starta) ; i++){
    gfx_CircleFilled(pxs[i], pen_rad, colour);

    angle = (angle+1)%360;
  }
  angle = starta;

  // "aliasing"
 /* for(i=0 ; i<myabs(enda-starta) ; i++){

      tmp.x = x+((double)(inrad)*cosi[angle]);
      tmp.y = y-((double)(inrad )*sinu[angle]);
      gfx_PutPixel(tmp, col_dark);

      tmp.x = x+((double)(outrad-1)*cosi[angle]);
      tmp.y = y-((double)(outrad-1)*sinu[angle]);
      gfx_PutPixel(tmp, col_dark);

      // draw "aliasing" on each side
      tmp.x = x+((double)(inrad-1)*cosi[angle]);
      tmp.y = y-((double)(inrad-1)*sinu[angle]);
      gfx_PutPixel(tmp, col_darker);

      tmp.x = x+((double)(outrad)*cosi[angle]);
      tmp.y = y-((double)(outrad)*sinu[angle]);
      gfx_PutPixel(tmp, col_darker);

      angle = (angle+1)%360;
  }*/

  return OK;

}
int8_t gfx_PolygonFilled(Pixel *pixels, uint16_t n, uint16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[4*n+6]; //= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i=0;
    int pix=0;
    cmd[0] = 0x00;
    cmd[1] = 0x14;
    cmd[2] = n>>8;
    cmd[3] = n&0x00FF;

    pix=0;
    for(i=4 ; i<(n*2)+4 ; i=i+2){
        cmd[i] = pixels[pix].x>>8;
        cmd[i+1] = pixels[pix].x&0x00FF;
        pix++;
    }
    pix=0;
    for(i=(n*2)+4 ; i<(n*4)+4 ; i=i+2){
        cmd[i] = pixels[pix].y>>8;
        cmd[i+1] = pixels[pix].y&0x00FF;
        pix++;
    }

    cmd[n*4+4] = colour>>8;
    cmd[n*4+5] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}
int8_t gfx_Polygon(Pixel *pixels, uint16_t n, uint16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[4*n+6]; //= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i=0;
    int pix=0;
    cmd[0] = 0x00;
    cmd[1] = 0x13;
    cmd[2] = n>>8;
    cmd[3] = n&0x00FF;

    pix=0;
    for(i=4 ; i<(n*2)+4 ; i=i+2){
        cmd[i] = pixels[pix].x>>8;
        cmd[i+1] = pixels[pix].x&0x00FF;
        pix++;
    }
    pix=0;
    for(i=(n*2)+4 ; i<(n*4)+4 ; i=i+2){
        cmd[i] = pixels[pix].y>>8;
        cmd[i+1] = pixels[pix].y&0x00FF;
        pix++;
    }

    cmd[n*4+4] = colour>>8;
    cmd[n*4+5] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}

int8_t gfx_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[3] = x1;
    cmd[5] = y1;
    cmd[7] = x2;
    cmd[9] = y2;
    cmd[10] = colour>>8;
    cmd[11] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}

int8_t gfx_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[3] = x1;
    cmd[5] = y1;
    cmd[7] = x2;
    cmd[9] = y2;
    cmd[10] = colour>>8;
    cmd[11] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}
int8_t gfx_RectangleFilled(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[3] = x1;
    cmd[5] = y1;
    cmd[7] = x2;
    cmd[9] = y2;
    cmd[10] = colour>>8;
    cmd[11] = (colour&0x00FF);
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}
int8_t gfx_BGcolour(uint16_t colour){
    uint8_t ack[3];
    uint8_t cmd[] = {0xFF, 0xA4, 0x00, 0x00};
    cmd[2] = (colour>>8);
    cmd[3] = colour;
    UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
    //UART_write(uart,(const void *)cmd,sizeof(cmd));
    //MyUARTRead(uart,(void *)ack,sizeof(ack));
    return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_TransparentColour(uint16_t  Color)
{
  uint8_t ack[3];
  uint8_t cmd[] = {0xFF, 0xA1, Color>>8, Color&0xFF};
  UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
  //UART_write(uart,(const void *)cmd,sizeof(cmd));
  //MyUARTRead(uart,(void *)ack,sizeof(ack));
  return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_TransparentOn()
{
  uint8_t ack[3];
  uint8_t cmd[] = {0xFF, 0xA0, 0x00, 0x01};
  UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
  //UART_write(uart,(const void *)cmd,sizeof(cmd));
  //MyUARTRead(uart,(void *)ack,sizeof(ack));
  return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_SetClipingWindow(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
    uint8_t ack = 0;
    uint8_t cmd[] = {0xFF, 0xB5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[3] = x1;
    cmd[5] = y1;
    cmd[7] = x2;
    cmd[9] = y2;
    UARTWriteReadScreen(cmd,sizeof(cmd),&ack,sizeof(ack));
    return ack = ACK ? OK : ERR;
}

int8_t gfx_SetCliping(bool  enable)
{
  uint8_t ack[3];
  uint8_t cmd[] = {0xFF, 0xA2, 0, enable};
  UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
  return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_SetBackLight(uint8_t  ligthing)
{
  uint8_t ack[3];
  uint8_t cmd[] = {0xFF, 0x9C, 0, ligthing};
  UARTWriteReadScreen(cmd,sizeof(cmd),ack,sizeof(ack));
  return ack[0] = ACK ? OK : ERR;
}

int8_t gfx_SetSleep(uint16_t s)
{
  uint8_t ack[3];
  uint8_t cmd[] = {0xFF, 0x3B, s>>8, s&0xFF};
  UARTWriteReadScreenNoTimeout(cmd,sizeof(cmd),ack,sizeof(ack));
  return ack[0] = ACK ? OK : ERR;
}


uint16_t rgbToRGB565(uint8_t red,uint8_t green,uint8_t blue)
{
    return (red >> 3 << 11) + (green >> 2 << 5) + (blue >> 3);

}
