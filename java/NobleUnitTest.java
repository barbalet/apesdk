/****************************************************************

 NobleUnitTest.java

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

import java.io.*;
import java.util.*;

public class NobleUnitTest
{

    public static void check_root(long value, long squared)
    {
		NobleMath nobleMath = new NobleMath();
        long result = nobleMath.newton_root(squared);
        if (result != value)
        {
            System.out.println("squared " + squared + " expects value " + value + ", instead " + result);
        }
    }

	public static void compare48(long full48, long first24, long next24)
	{
		long  localFirst24 = full48 >> 24;
		long  localNext24  = full48 & ((1 << 24) - 1);
		
		if ((first24 != localFirst24) || (next24 != localNext24))
		{
			System.out.println("localFirst24 " + localFirst24 + " localNext24 " + localNext24);
		}
	}
    
	public static void check_hash()
	{
		NobleMath nobleMath = new NobleMath();

		long value1 = nobleMath.hash("when you are at home", 20);
		long value2 = nobleMath.hash("playing my nation - ", 20);
		long value3 = nobleMath.hash("ask YOURSELF are YOU", 20);
		long value4 = nobleMath.hash("a monkey in a simulation?", 25);
		
		compare48(value1, 10254033, 8974960);
		compare48(value2, 14445625, 924468);
		compare48(value3, 3443992, 14170421);
		compare48(value4, 6876686, 14488999);
	}
    
    public static void check_math()
    {
        int          loop = 0;
        NobleVector2 sum_vect = new NobleVector2();
        
        check_root(1234, 1522756);
        check_root(4, 17);
        check_root(4, 16);
        check_root(4, 15);
        check_root(3, 14);
        
        while (loop < 256)
        {
            NobleVector2 each_vect = new NobleVector2();
            
            each_vect.direction(loop, 1);
            sum_vect.delta(each_vect);
            
            loop++;
        }

        if (sum_vect.nonzero())
        {
            System.out.println("vect sum expecting 0, 0 instead " + sum_vect.x + ", " + sum_vect.y);
        }
    }

    
    public static void main(String[] args)
    {
        check_math();
		check_hash();
    }
}

