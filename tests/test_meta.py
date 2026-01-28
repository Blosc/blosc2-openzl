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
@pytest.mark.parametrize('dtype', (np.int64, np.float32, np.float64, np.int32, np.uint32))
@pytest.mark.parametrize('meta', test_list)
@pytest.mark.parametrize(('shape', 'chunks'), [((N_ITEMS, N_ITEMS), (N_ITEMS // 10, N_ITEMS // 6)), ((N_ITEMS, ), (N_ITEMS // 6,))])
def test_meta(dtype, meta, shape, chunks):
    # Convert the image to a numpy array
    np_array = np.arange(np.prod(shape), dtype=dtype).reshape(shape)

    # Set the parameters that will be used by the codec
    cparams = {
        'codec': blosc2.Codec.OPENZL,
        'codec_meta': meta.value,
        'filters': [],
    }

    bl_array = blosc2.asarray(
        np_array,
        chunks=chunks,
        cparams=cparams,
    )

    assert bl_array.cratio > 1
    assert bl_array.chunks == chunks
    np.testing.assert_allclose(np_array, bl_array[:])