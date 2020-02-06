/****************************************************************

 common.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

#ifndef    _WIN32

#include "../toolkit/toolkit.h"

#else

#include "..\toolkit\toolkit.h"

#endif

#include "sim.h"
#include "common.h"

n_string example_json = "{\"information\":{\"signature\":20033,\"version\":699,\"copyright\":\"Copyright Tom Barbalet, 1996-2020.\",\"date\":\"Jun 26 2020\"},\"land\":{\"date\":30,\"genetics\":[60759,44960],\"time\":599},\"exit\":5,\"beings\":[{\"name\":\"Alfred Morton-Price\",\"facing\":161,\"location\":[32482,9546],\"velocity\":19,\"energy\":2981,\"state\":17,\"genetics\":[2464410267,61210982,2771567728,251944452],\"identification\":997421449},{\"name\":\"Alfred Morton-Price\",\"facing\":63,\"location\":[31732,17783],\"velocity\":14,\"energy\":1605,\"state\":21,\"genetics\":[2769573207,2122291008,3645579776,3802468996],\"identification\":3766874771},{\"name\":\"Alfred Morton-Price\",\"facing\":59,\"location\":[26374,12012],\"velocity\":22,\"energy\":2118,\"state\":17,\"genetics\":[1631492238,2788038956,116722445,1964307392],\"identification\":3035724143},{\"name\":\"Alfred Morton-Price\",\"facing\":100,\"location\":[26570,22807],\"velocity\":33,\"energy\":1876,\"state\":17,\"genetics\":[1431610647,2539486880,202116235,2609210080],\"identification\":1434498524},{\"name\":\"Alfred Morton-Price\",\"facing\":102,\"location\":[14886,32260],\"velocity\":19,\"energy\":3027,\"state\":17,\"genetics\":[1267869487,312208016,908664621,2360616],\"identification\":1862588346},{\"name\":\"Alfred Morton-Price\",\"facing\":189,\"location\":[26627,21375],\"velocity\":0,\"energy\":1681,\"state\":11,\"genetics\":[2769769815,2267897408,721034,2425385],\"identification\":571738548},{\"name\":\"Alfred Morton-Price\",\"facing\":186,\"location\":[18068,29498],\"velocity\":11,\"energy\":3852,\"state\":17,\"genetics\":[1534493358,114165134,2186835520,1294103966],\"identification\":2462221566},{\"name\":\"Alfred Morton-Price\",\"facing\":253,\"location\":[14901,7346],\"velocity\":37,\"energy\":3353,\"state\":17,\"genetics\":[1577268395,9764882,2256491842,112354000],\"identification\":3730506539},{\"name\":\"Alfred Morton-Price\",\"facing\":128,\"location\":[16298,13906],\"velocity\":14,\"energy\":2433,\"state\":21,\"genetics\":[1243785810,100272123,1900583,1129460320],\"identification\":211263470},{\"name\":\"Alfred Morton-Price\",\"facing\":220,\"location\":[7643,22587],\"velocity\":18,\"energy\":1770,\"state\":21,\"genetics\":[3038556782,358329280,2491285,89000299],\"identification\":2771325776},{\"name\":\"Alfred Morton-Price\",\"facing\":225,\"location\":[14459,22201],\"velocity\":0,\"energy\":2308,\"state\":11,\"genetics\":[2641127718,15335519,1272097368,156640533],\"identification\":3741509172},{\"name\":\"Alfred Morton-Price\",\"facing\":182,\"location\":[28811,10682],\"velocity\":0,\"energy\":1270,\"state\":11,\"genetics\":[1418406583,34210664,1772817226,3594826128],\"identification\":3916337925},{\"name\":\"Alfred Morton-Price\",\"facing\":191,\"location\":[25365,19397],\"velocity\":17,\"energy\":2758,\"state\":17,\"genetics\":[2482668786,34734544,912662565,2895155840],\"identification\":186529671},{\"name\":\"Alfred Morton-Price\",\"facing\":49,\"location\":[11694,30624],\"velocity\":39,\"energy\":3060,\"state\":17,\"genetics\":[372068426,1369812921,1321615216,260770671],\"identification\":113626092},{\"name\":\"Alfred Morton-Price\",\"facing\":230,\"location\":[25786,19031],\"velocity\":0,\"energy\":3324,\"state\":11,\"genetics\":[2045135658,1459139224,175178378,3298662798],\"identification\":3790758582},{\"name\":\"Alfred Morton-Price\",\"facing\":41,\"location\":[8892,30852],\"velocity\":0,\"energy\":3273,\"state\":11,\"genetics\":[340415022,601562468,15728742,1535532796],\"identification\":1827082704},{\"name\":\"Alfred Morton-Price\",\"facing\":69,\"location\":[30639,3643],\"velocity\":6,\"energy\":1218,\"state\":19,\"genetics\":[1195263294,169508164,1514691136,109249680],\"identification\":300902058}]}";
