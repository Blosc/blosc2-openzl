/*********************************************************************
* blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
*
* Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
* https://blosc.org
* License: GNU Affero General Public License v3.0 (see LICENSE.txt)

Test program demonstrating use of the Blosc filter from C code.
Compile this program with cmake and run:

$ ./test_grok2
image size width 256, height 256
Compress OK:	cratio: 2.891 x
Going to decompress
Decompressing buffer

Image Info
Width: 256
Height: 256
Number of components: 3
Precision of component 0 : 8
Precision of component 1 : 8
Precision of component 2 : 8
Number of tiles: 1
Component 0 : dimensions (256,256) at precision 8
Component 1 : dimensions (256,256) at precision 8
Component 2 : dimensions (256,256) at precision 8
Decompress OK

**********************************************************************/

#include "b2nd.h"
#include "blosc2.h"
#include "blosc2/codecs-registry.h"
#include "blosc2_openzl.h"

int comp_decomp() {
    return BLOSC2_ERROR_SUCCESS;
}

int main(void) {
    // Initialization
    blosc2_init();

    int error = comp_decomp();

    blosc2_destroy();
    return error;
}
