/*
 * pixxil_static.h
 *
 *  Created on: 14 May 2021
 *      Author: mehmedblazevic
 * Description: This file contains addresses of every graphical
 * objects which are displayed. Those objects are stored in the
 * Pixxil-LCD memory and should not be changed!
 */

#ifndef PIXXIL_STATIC_H_
#define PIXXIL_STATIC_H_

typedef enum {Name, Speed, Pol1, Pol2, Pol3, Cardinals, Trip} letters_type;

// offset of the images stored in the screen
#define iBONJOURTXT     0
#define isplash         134
//#define imtsc           134
//#define imtsc_png       135

#define in0             57    // name font
#define in1             58
#define in2             59
#define in3             60
#define in4             61
#define in5             66
#define in6             62
#define in7             63
#define in8             64
#define in9             65
#define ina             28
#define inA             2
#define inb             29
#define inB             3
#define inc             30
#define inC             4
#define ind             31
#define inD             5
#define ine             32
#define inE             6
#define inea            54
#define ineg            56
#define inf             33
#define inF             7
#define ing             34
#define inG             8
#define inh             35
#define inH             9
#define ini             36
#define inI             12
#define inj             37
#define inJ             11
#define ink             38
#define inK             10
#define inl             39
#define inL             15
#define inM             14
#define inm             41
#define inN             13
#define inn             40
#define inO             18
#define ino             42
#define inP             17
#define inp             43
#define inq             44
#define inQ             16
#define inr             45
#define inR             21
#define ins             46
#define inS             20
#define intt            47
#define inT             19
#define inu             48
#define inU             24
#define inv             49
#define inV             23
#define inw             50
#define inW             22
#define inx             51
#define inX             27
#define iny             52
#define inY             26
#define inz             53
#define inZ             25
#define intir           55

#define iv0             67      // speed font
#define iv1             68
#define iv2             69
#define iv3             70
#define iv4             71
#define iv5             72
#define iv6             73
#define iv7             74
#define iv8             75
#define iv9             76
#define ivkmh           77

#define ia0             96      // font 1
#define ia1             97
#define ia2             98
#define ia3             99
#define ia4             100
#define ia5             101
#define ia6             102
#define ia7             103
#define ia8             104
#define ia9             105
#define iap             117

#define ib0             106     // font 2
#define ib1             107
#define ib2             108
#define ib3             109
#define ib4             110
#define ib5             111
#define ib6             112
#define ib7             113
#define ib8             114
#define ib9             115
#define ibp             116

#define ic0             118     // font 3
#define ic1             119
#define ic2             120
#define ic3             121
#define ic4             122
#define ic5             123
#define ic6             124
#define ic7             125
#define ic8             126
#define ic9             127
#define icp             128
#define icpp            129

#define it0             82      // trip font
#define it1             83
#define it2             84
#define it3             85
#define it4             86
#define it5             87
#define it6             88
#define it7             89
#define it8             90
#define it9             91
#define ita             78
#define itb             79
#define itc             80
#define itkm            81

#define itE             92      // cardinal point font
#define itN             93
#define itO             95
#define itS             94

#define isnow           130     // snow logo
#define irain           131     // rain logo
#define isetting        132     // setting logo
#define ilightning      133     // lightning logo
#define ibLightning     135     // lightning log
#define iheadLight      136
#define ilock           137


#endif /* PIXXIL_STATIC_H_ */
