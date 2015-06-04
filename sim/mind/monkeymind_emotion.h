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

#ifndef MONKEYMIND_EMOTION_H
#define MONKEYMIND_EMOTION_H

#include "noble.h"

/* emotions binary coded according to the Lövheim Cube
   Lövheim H. A new three-dimensional model for emotions and
   monoamine neurotransmitters. (2012). Med Hypotheses,
   78, 341-348. doi:10.1016/j.mehy.2011.11.016 PMID 22153577 */
#define MM_EMOTION_SHAME      0
#define MM_EMOTION_DISTRESS   1
#define MM_EMOTION_FEAR       2
#define MM_EMOTION_ANGER      3
#define MM_EMOTION_DISGUST    4
#define MM_EMOTION_SURPRISE   5
#define MM_EMOTION_JOY        6
#define MM_EMOTION_EXCITEMENT 7

n_byte mm_neuro_to_emotion(n_uint serotonin,
                           n_uint dopamine,
                           n_uint noradrenaline,
                           n_uint neurotransmitter_max);
void mm_emotion_to_neuro(n_byte emotion,
                         n_uint * serotonin,
                         n_uint * dopamine,
                         n_uint * noradrenaline,
                         n_uint neurotransmitter_max);

#endif
