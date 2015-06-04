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

#include "monkeymind_emotion.h"

/* convert a set of neurotransmitter levels into a single value
   indicating the type of emotion */
n_byte mm_neuro_to_emotion(n_uint serotonin,
                           n_uint dopamine,
                           n_uint noradrenaline,
                           n_uint neurotransmitter_max)
{
    n_uint threshold = neurotransmitter_max>>1;
    n_byte emotion = 0;

    if (noradrenaline >= threshold) emotion |= 1;
    if (dopamine >= threshold) emotion |= 2;
    if (serotonin >= threshold) emotion |= 4;
    return emotion;
}

/* convert an emotion into a set of neurotransmitter levels */
void mm_emotion_to_neuro(n_byte emotion,
                         n_uint * serotonin,
                         n_uint * dopamine,
                         n_uint * noradrenaline,
                         n_uint neurotransmitter_max)
{
    *serotonin = 0;
    *dopamine = 0;
    *noradrenaline = 0;

    if (emotion & 1) *noradrenaline = neurotransmitter_max;
    if (emotion & 2) *dopamine = neurotransmitter_max;
    if (emotion & 4) *serotonin = neurotransmitter_max;
}
