##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

import numpy as np
import pytest
from pathlib import Path
import blosc2
from blosc2_openzl import OpenZLProfile
test_list = list(OpenZLProfile)

N_ITEMS = 1000

project_dir = Path(__file__).parent.parent
@pytest.mark.parametrize('meta', test_list)
@pytest.mark.parametrize(('shapes', 'chunks'), [((N_ITEMS, N_ITEMS), (N_ITEMS // 10, N_ITEMS // 6)), ((N_ITEMS, ), (N_ITEMS // 6))])
def test_meta(meta, shape, chunks):
    # Convert the image to a numpy array
    np_array = np.arange(N_ITEMS).shape

    # Set the parameters that will be used by the codec
    cparams = {
        'codec': blosc2.Codec.OPENZL,
        'codec_meta': meta,
        'filters': [],
    }

    bl_array = blosc2.asarray(
        np_array,
        chunks=chunks,
        cparams=cparams,
    )

    assert bl_array.cratio > 1
    assert bl_array.chunks == chunks