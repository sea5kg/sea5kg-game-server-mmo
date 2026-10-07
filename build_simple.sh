#!/bin/bash
##################################################################################
#
# ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# ::                                   oooooo 8                                 ::
# ::                                   8      8                                 ::
# ::              .oPYo. .oPYo. .oPYo. 8pPYo. 8  .o  .oPYo.                     ::
# ::              Yb..   8oooo8 .oooo8     `8 8oP'   8    8                     ::
# ::                'Yb. 8.     8    8     .P 8 `b.  8    8                     ::
# ::              `YooP' `Yooo' `YooP8 `YooP' 8  `o. `YooP8                     ::
# :::::::::::::::::.....::.....::.....::.....:..::...:....8 ::::::::::::::::::::::
# :::::::::::::::::::::::::::::::::::::::::::::::::::::ooP'.::::::::::::::::::::::
# :: .oPYo. .oPYo. ooYoYo. .oPYo.       .oPYo. .oPYo. oPYo. o    o .oPYo. oPYo. ::
# :: 8    8 .oooo8 8' 8  8 8oooo8 ooooo Yb..   8oooo8 8  `' Y.  .P 8oooo8 8  `' ::
# :: 8    8 8    8 8  8  8 8.             'Yb. 8.     8     `b..d' 8.     8     ::
# :: `YooP8 `YooP8 8  8  8 `Yooo'       `YooP' `Yooo' 8      `YP'  `Yooo' 8     ::
# ::::....8 :.....:..:..:..:.....::::::::.....::.....:..::::::...:::.....:..::::::
# :::::ooP'.:::::::ooYoYo. ooYoYo. .oPYo.                                       ::
# ::               8' 8  8 8' 8  8 8    8                                       ::
# ::               8  8  8 8  8  8 8    8                                       ::
# ::               8  8  8 8  8  8 `YooP'                                       ::
# :::::::::::::::::..:..:....:..:..:.....:::::::::::::::::::::::::::::::::::::::::
# ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
# MIT License
#
# Copyright (c) 2018-2026 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Original repository: https://github.com/sea5kg/sea5kg-game-server-mmo
#
##################################################################################

check_ret() {
    if [ $1 -ne 0 ]; then
        echo ""
        echo "!!! FAIL: $2"
        echo "********************************************************************************"
        echo ""
        exit $1
    else
        echo ""
        echo "*** SUCCESS: $2"
        echo "********************************************************************************"
        echo ""
    fi
}

cmake -H. -B./tmp/release -DCMAKE_BUILD_TYPE=Release
check_ret $? "configure"

cmake --build ./tmp/release --config Release
check_ret $? "build"

cd ./tmp/release && ctest --output-on-failure
