/****************************************************************

 NobleMath.java

 =============================================================

 Copyright 1996-2017 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*! \file   math.c
 *  \brief  This covers vector math, the hash/random mathematics,
   (Newton) square root, the Bresenham's line algorithm, the bilinear
   interpolation and other abstract math that is used in Noble Ape.
 */


import java.io.*;
import java.util.*;

public class NobleMath
{
    public static void random3(int local[], int val)
    {
        int[] values;
        
        values = new int[2];
        
        values[0] = local[val];
        values[1] = local[val + 1];
        
        random(values);
        random(values);
        random(values);
        
        local[val] = values[0];
        local[val + 1] = values[1];
    }
    
    public static int random(int local[])
    {
        int tmp0 = local[0];
        int tmp1 = local[1];
        local[0] = tmp1;
        switch (tmp0 & 7)
        {
            case 0:
                local[1] = (tmp1 ^ (tmp0 >> 1) ^ 53288);
                break;
            case 3:
                local[1] = (tmp1 ^ (tmp0 >> 2) ^ 44552);
                break;
            case 7:
                local[1] = (tmp1 ^ (tmp0 >> 3) ^ 25376);
                break;
            default:
                local[1] = (tmp1 ^ (tmp0 >> 1));
                break;
        }
        return (tmp1);
    }
    
    public static long	newton_root(long squ)
    {
        long attempt = 1;
        long second_attempt = 0;
        long count = 0;
        while ((attempt != second_attempt) && (count ++ < 256)) {
            second_attempt = attempt;
            if (attempt == 0)
                return 0;
            attempt = (attempt + (squ / attempt)) >> 1;
        }
        if (count > 255)
            return newton_root(squ + 1);
        return attempt;
    }
    
    public static long hash(String values, int length)
    {
        /* this is a 48-bit implementation of the hash algorithm to avoid negative values */
        int	  loop = 0;
        int[] round;
        long  return_value, long_carry;
        round = new int[4];
        
        round[0] = 64120;
        round[1] = 64215;
        round[2] = 21479;
        round[3] = 42792;
        
        while( loop < length)
        {
            round[0] ^= round[3];
            round[1] ^= values.charAt(loop++);
            
            random3(round, 0);
            random3(round, 1);
            random3(round, 2);
        }
        
        long_carry = round[2];
        return_value = long_carry;
        
        long_carry = round[1];
        return_value = (return_value* 65536) + long_carry;
        
        long_carry = round[0];
        return_value = (return_value* 65536) + long_carry;
        
        return return_value;
    }
    
//n_byte math_turn_towards(n_vect2 * p, n_byte fac, n_byte turn)
//{
//    n_int track[NUMBER_TURN_TOWARDS_POINTS] =
//    {
//        64, 64, 32, 16, 8, 4, 2, 1
//    };
//    n_vect2 vector_facing;
//    n_int best_p;
//    n_int best_f = fac;
//    n_int loop = turn;
//
//    NA_ASSERT(p, "p NULL");
//
//    vect2_direction(&vector_facing, best_f, 32);
//
//    best_p = vect2_dot(p, &vector_facing, 1, 1);
//
//    while (loop < NUMBER_TURN_TOWARDS_POINTS)
//    {
//        n_int loc_track = track[loop];
//        n_int loc_f = (best_f + loc_track) & 255;
//        n_int project1;
//
//        vect2_direction(&vector_facing, loc_f, 32);
//        project1 = vect2_dot(p, &vector_facing, 1, 1);
//
//        if (project1 > best_p)
//        {
//            best_f = loc_f;
//            best_p = project1;
//        }
//        else
//        {
//            n_int loc_f2 = (best_f + 256 - loc_track) & 255;
//            n_int project2;
//
//            vect2_direction(&vector_facing, loc_f, 32);
//            project2 = vect2_dot(p, &vector_facing, 1, 1);
//
//            if (project2 > best_p)
//            {
//                best_f = loc_f2;
//                best_p = project2;
//            }
//        }
//        loop++;
//    }
//    return (n_byte)best_f;
//}

}


