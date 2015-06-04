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

#include "monkeymind_motives.h"

const n_byte motives[] = {
    MOTIVE_NONE, MOTIVE_NONE,
    MOTIVE_ACHIEVEMENT, MOTIVE_AMBITION,
    MOTIVE_RECOGNITION, MOTIVE_AMBITION,
    MOTIVE_EXHIBITION, MOTIVE_AMBITION,
    MOTIVE_ACQUISITION, MOTIVE_MATERIALISM,
    MOTIVE_ORDER, MOTIVE_MATERIALISM,
    MOTIVE_RETENTION, MOTIVE_MATERIALISM,
    MOTIVE_CONSTRUCTION, MOTIVE_MATERIALISM,
    MOTIVE_INFAVOIDANCE, MOTIVE_STATUS,
    MOTIVE_DEFENDANCE, MOTIVE_STATUS,
    MOTIVE_COUNTERACTION, MOTIVE_STATUS,
    MOTIVE_DOMINANCE, MOTIVE_POWER,
    MOTIVE_DEFERENCE, MOTIVE_POWER,
    MOTIVE_AUTONOMY, MOTIVE_POWER,
    MOTIVE_CONTRARIANCE, MOTIVE_POWER,
    MOTIVE_AGGRESSION, MOTIVE_POWER,
    MOTIVE_ABASEMENT, MOTIVE_POWER,
    MOTIVE_BLAME_AVOIDANCE, MOTIVE_POWER,
    MOTIVE_HARM_AVOIDANCE, MOTIVE_POWER,
    MOTIVE_AFFILIATION, MOTIVE_AFFECTION,
    MOTIVE_SEX, MOTIVE_AFFECTION,
    MOTIVE_REJECTION, MOTIVE_AFFECTION,
    MOTIVE_NURTURANCE, MOTIVE_AFFECTION,
    MOTIVE_SUCCORANCE, MOTIVE_AFFECTION,
    MOTIVE_PLAY, MOTIVE_AFFECTION,
    MOTIVE_SENTIENCE, MOTIVE_INFORMATION,
    MOTIVE_COGNIZANCE, MOTIVE_INFORMATION,
    MOTIVE_EXPOSITION, MOTIVE_INFORMATION
};

n_byte mm_get_motive_category(n_byte motive)
{
    if (motive < MOTIVES) {
        return motives[motive*2 + 1];
    }
    return MOTIVE_NONE;
}
