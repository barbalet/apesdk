#!/bin/bash
#	test.sh
#
#	=============================================================
#
#   Copyright 1996-2024 Tom Barbalet. All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use,
#   copy, modify, merge, publish, distribute, sublicense, and/or
#   sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following
#   conditions:
#
#   The above copyright notice and this permission notice shall be
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.
#
#   This software is a continuing work of Tom Barbalet, begun on
#   13 June 1996. No apes or cats were harmed in the writing of
#   this software.

cd ..
./apesdk-json.sh
mv apesdk-json json
cd json

./apesdk-json example_four_two_vect2.json
diff example_four_two_vect2.json 2xample_four_two_vect2.json

./apesdk-json example_urban.json
diff example_urban.json 2xample_urban.json

./apesdk-json example_road.json
diff example_road.json 2xample_road.json

./apesdk-json example0c.json
diff example0c.json 2xample0c.json

./apesdk-json example1.json
diff example1.json 2xample1.json

./apesdk-json example1b.json
diff example1b.json 2xample1b.json

./apesdk-json example1c.json
diff example1c.json 2xample1c.json

./apesdk-json example2.json
diff example2.json 2xample2.json

./apesdk-json example2c.json
diff example2c.json 2xample2c.json

./apesdk-json example3.json
diff example3.json 2xample3.json

./apesdk-json example3c.json
diff example3c.json 2xample3c.json

./apesdk-json example3d.json
diff example3d.json 2xample3d.json

./apesdk-json example3e.json
diff example3e.json 2xample3e.json

./apesdk-json example4.json
diff example4.json 2xample4.json

./apesdk-json example4b.json
diff example4b.json 2xample4b.json

./apesdk-json example5.json
diff example5.json 2xample5.json

./apesdk-json battle.json
diff battle.json 2attle.json

./apesdk-json sim.json
diff sim.json 2im.json

./apesdk-json example6.json
diff example6.json 2xample6.json

./apesdk-json example7.json
diff example7.json 2xample7.json

./apesdk-json random.json
diff random.json 2andom.json

./apesdk-json randomb.json
diff randomb.json 2andomb.json

./apesdk-json randomc.json
diff randomc.json 2andomc.json

./apesdk-json random2.json
diff random2.json 2andom2.json

./apesdk-json neighborhood.json
diff neighborhood.json 2eighborhood.json

./apesdk-json new_ashford.json
diff new_ashford.json 2ew_ashford.json

./apesdk-json new_folkestone.json
diff new_folkestone.json 2ew_folkestone.json

rm 2*.json
rm apesdk-json
