##############################################################################
# blosc2_grok: Grok (JPEG2000 codec) plugin for Blosc2
#
# Copyright (c) 2023  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

"""
Benchmark for compressing a dataset of images with the grok codec.

This uses the second partition in Blosc2 for encoding the JPEG2000 data.

Data can be downloaded from: http://www.silx.org/pub/nabu/data/compression/lung_raw_2000-2100.h5
"""

import h5py
import blosc2
import blosc2_openzl
import numpy as np
from tqdm import tqdm
from time import time


if __name__ == '__main__':
    # Define the compression and decompression parameters. Disable the filters and the
    # splitmode, because these don't work with the codec.
    cparams = {
        'codec': blosc2.Codec.OPENZL,
        'codec_meta': 1, # LZ4 codec
        'nthreads': 4,
    }
    cparams_blosc2 = blosc2.cparams_dflts.copy()
    cparams_blosc2['codec'] = blosc2.Codec.LZ4
    # Open the dataset
    f = h5py.File('/Users/faltet/Downloads/lung_raw_2000-2100.h5', 'r')
    dset = f['/data']
    print(f"*** Compressing dataset of {dset.shape} images ...")

    for cratio in range(1, 11):
        print(f"*** Compressing with cratio={cratio}x ...")
        # for i in tqdm(range(dset.shape[0])):
        for i in range(1):  #dset.shape[0]):
            im = dset[i:i+1, ...]
            # Transform the numpy array to a blosc2 array. This is where compression happens.
            t0 = time()
            #blocks = (1, im.shape[1] // 4, im.shape[2] // 4)
            blocks = (1, im.shape[1], im.shape[2])
            b2im = blosc2.asarray(im, chunks=im.shape, blocks=blocks, cparams=cparams)
            t1 = time() - t0

            # Use a lossless codec for the original data
            t0 = time()
            b2im2 = blosc2.asarray(im, cparams=cparams_blosc2)
            t2 = time() - t0
            im2 = b2im[:]

            if i == 0:
                # Compare with original
                cratio1 = b2im.schunk.cratio
                speed1 = (im.shape[1] * im.shape[2] * im.shape[0] * im.dtype.itemsize) / t1 / 1e6
                # Compare with lossless over original image
                cratio2 = b2im2.schunk.cratio
                speed2 = (im.shape[1] * im.shape[2] * im.shape[0] * im.dtype.itemsize) / t2 / 1e6
        print(f"cratio (openzl): {cratio1:.2f}")
        print(f"cspeed (openzl): {speed1:.3f} MB/s")
        print(f"cratio (lz4 on orig image): {cratio2:.2f}")
        print(f"cspeed (lz4 on orig image): {speed2:.3f} MB/s")

    f.close()
