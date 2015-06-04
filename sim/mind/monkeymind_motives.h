/****************************************************************

 Monkeymind - an experimental cogitive architecture

 =============================================================

 Copyright 2013-2014 Bob Mottram

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the followingp
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

****************************************************************/

#ifndef MONKEYMIND_MOTIVES_H
#define MONKEYMIND_MOTIVES_H

#include <stdio.h>
#include <string.h>
#include "monkeymind_object.h"

enum motive_categories {
    MOTIVE_AMBITION = 1,
    MOTIVE_MATERIALISM,
    MOTIVE_STATUS,
    MOTIVE_POWER,
    MOTIVE_AFFECTION,
    MOTIVE_INFORMATION,
    MOTIVE_CATEGORIES
};

enum motive_types {
    MOTIVE_NONE = 0,
    MOTIVE_ACHIEVEMENT,
    MOTIVE_RECOGNITION,
    MOTIVE_EXHIBITION,
    MOTIVE_ACQUISITION,
    MOTIVE_ORDER,
    MOTIVE_RETENTION,
    MOTIVE_CONSTRUCTION,
    MOTIVE_INFAVOIDANCE,
    MOTIVE_DEFENDANCE,
    MOTIVE_COUNTERACTION,
    MOTIVE_DOMINANCE,
    MOTIVE_DEFERENCE,
    MOTIVE_AUTONOMY,
    MOTIVE_CONTRARIANCE,
    MOTIVE_AGGRESSION,
    MOTIVE_ABASEMENT,
    MOTIVE_BLAME_AVOIDANCE,
    MOTIVE_HARM_AVOIDANCE,
    MOTIVE_AFFILIATION,
    MOTIVE_SEX,
    MOTIVE_REJECTION,
    MOTIVE_NURTURANCE,
    MOTIVE_SUCCORANCE,
    MOTIVE_PLAY,
    MOTIVE_SENTIENCE,
    MOTIVE_COGNIZANCE,
    MOTIVE_EXPOSITION,
    MOTIVES
};

n_byte mm_get_motive_category(n_byte motive);

#endif
