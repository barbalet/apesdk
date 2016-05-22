/****************************************************************

 NobleVector2.java

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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

import java.io.*;
import java.util.*;

public class NobleVector2
{
    public int x;
    public int y;
    
    private int[] new_sd = new int[] {
    0, 659, 1318, 1977, 2634, 3290, 3944, 4595, 5244, 5889, 6531, 7169, 7802, 8431, 9055, 9673,
    10286, 10892, 11492, 12085, 12671, 13249, 13819, 14380, 14933, 15477, 16012, 16537, 17052, 17557, 18051, 18534,
    19007, 19467, 19916, 20353, 20778, 21190, 21590, 21976, 22349, 22709, 23055, 23387, 23706, 24009, 24299, 24573,
    24833, 25078, 25308, 25523, 25722, 25906, 26074, 26226, 26363, 26484, 26589, 26677, 26750, 26807, 26847, 26871,
    26880, 26871, 26847, 26807, 26750, 26677, 26589, 26484, 26363, 26226, 26074, 25906, 25722, 25523, 25308, 25078,
    24833, 24573, 24299, 24009, 23706, 23387, 23055, 22709, 22349, 21976, 21590, 21190, 20778, 20353, 19916, 19467,
    19007, 18534, 18051, 17557, 17052, 16537, 16012, 15477, 14933, 14380, 13819, 13249, 12671, 12085, 11492, 10892,
    10286, 9673, 9055, 8431, 7802, 7169, 6531, 5889, 5244, 4595, 3944, 3290, 2634, 1977, 1318, 659,
    0, -659, -1318, -1977, -2634, -3290, -3944, -4595, -5244, -5889, -6531, -7169, -7802, -8431, -9055, -9673,
    -10286, -10892, -11492, -12085, -12671, -13249, -13819, -14380, -14933, -15477, -16012, -16537, -17052, -17557, -18051, -18534,
    -19007, -19467, -19916, -20353, -20778, -21190, -21590, -21976, -22349, -22709, -23055, -23387, -23706, -24009, -24299, -24573,
    -24833, -25078, -25308, -25523, -25722, -25906, -26074, -26226, -26363, -26484, -26589, -26677, -26750, -26807, -26847, -26871,
    -26880, -26871, -26847, -26807, -26750, -26677, -26589, -26484, -26363, -26226, -26074, -25906, -25722, -25523, -25308, -25078,
    -24833, -24573, -24299, -24009, -23706, -23387, -23055, -22709, -22349, -21976, -21590, -21190, -20778, -20353, -19916, -19467,
    -19007, -18534, -18051, -17557, -17052, -16537, -16012, -15477, -14933, -14380, -13819, -13249, -12671, -12085, -11492, -10892,
    -10286, -9673, -9055, -8431, -7802, -7169, -6531, -5889, -5244, -4595, -3944, -3290, -2634, -1977, -1318, -659
    };
    
    public void NobleVector2()
    {
        x = 0;
        y = 0;
    }

    public void populate(int ix, int iy)
    {
        x = ix;
        y = iy;
    }
    
    public void add(NobleVector2 initial, NobleVector2 second)
    {
        x = initial.x + second.x;
        y = initial.y + second.y;
    }


    public void center(NobleVector2 initial, NobleVector2 second)
    {
        add(initial, second);
        x = x / 2;
        y = y / 2;
    }


    public void subtract(NobleVector2 initial, NobleVector2 second)
    {
        x = initial.x - second.x;
        y = initial.y - second.y;
    }

    public void multiply(NobleVector2 initial, NobleVector2 second, int multiplier, int divisor)
    {
        if (divisor == 0) return;

        x = (multiplier * initial.x * second.x) / divisor;
        y = (multiplier * initial.y * second.y) / divisor;
    }


    public void rationalize(int multiplier, int divisor)
    {
        if (divisor == 0) return;

        x = ( (multiplier * x) / divisor);
        y = ( (multiplier * y) / divisor);
    }


    public int dot(NobleVector2 second, int multiplier, int divisor)
    {
        if (divisor == 0) return 0;

        return (multiplier * ((x * second.x) + (y * second.y))) / divisor;
    }


    public void rotate90()
    {
        int temp = y;
        y = 0 - x;
        x = temp;
    }


    public void direction(int direction, int divisor)
    {
        x = ((new_sd[((direction)+64)&255]) / (divisor));
        y = ((new_sd[(direction)&255]) / (divisor));
    }

    public void delta(NobleVector2 delta)
    {
        x += delta.x;
        y += delta.y;
    }

    public void offset(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    public void copy(NobleVector2 from)
    {
        x = from.x;
        y = from.y;
    }

    public void rotation(NobleVector2 rotation)
    {
        NobleVector2 temp = new NobleVector2();

        temp.x = ((x * rotation.x) + (y * rotation.y)) / 26880;
        temp.y = ((x * rotation.y) - (y * rotation.x)) / 26880;

        x = temp.x;
        y = temp.y;
    }

    public boolean nonzero()
    {
        return ((x != 0) || (y != 0));
    }

//n_vect2 * vect2_min_max_init(void)
//{
//    n_vect2 * min_max = io_new(2 * sizeof(n_vect2));
//    if (min_max == 0L)
//    {
//        return 0L;
//    }
//    vect2_populate(&min_max[0], 2147483647, 2147483647);
//    vect2_populate(&min_max[1], -2147483648, -2147483648);
//    return min_max;
//}
//
//void vect2_min_max(n_vect2 * points, n_int number, n_vect2 * maxmin)
//{
//    n_int loop = 0;
//
//    while(loop < number)
//    {
//        n_int px = points[loop].x;
//        n_int py = points[loop].y;
//        if (px < maxmin[0].x)
//        {
//            maxmin[0].x = px;
//        }
//        if (py < maxmin[0].y)
//        {
//            maxmin[0].y = py;
//        }
//
//        if (px > maxmin[1].x)
//        {
//            maxmin[1].x = px;
//        }
//        if (py > maxmin[1].y)
//        {
//            maxmin[1].y = py;
//        }
//        loop++;
//    }
//}

}

