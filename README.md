# Blosc2 OpenZL

A dynamic codec plugin for Blosc2 that allows one to compress
and decompress data using the OpenZL compression library. For details, check out the
[project README](https://github.com/facebook/openzl). With this plugin, you can use it as yet another codec in applications using Blosc2.  See an example of use at: https://github.com/Blosc/blosc2-openzl/blob/main/tests/test_meta.py

## Installation

For using `blosc2_openzl` you will first have to install its wheel:

```shell
pip install blosc2-openzl -U
```

## Usage

```python
import blosc2
import numpy as np
import blosc2_openzl

# Define the compression and decompression parameters for Blosc2.
cparams = {
    'codec': blosc2.Codec.OPENZL,
    'filters': [],
    'codec_meta': 7,
}

# Create array to be compressed
np_array = np.arange(1000).reshape((10,100))

# Transform the numpy array to a blosc2 array. This is where compression happens, and
# the OpenZL codec is called.
bl_array = blosc2.asarray(
    np_array,
    cparams=cparams
)

# Print information about the array, see the compression ratio (cratio)
print(bl_array.info)
```

## Parameters for compression
One may set the OpenZL graph using `codec_meta`, an 8-bit integer corresponding to meta information relating to codecs, filters and other nodes for the compression graph. Starting from the Least-Significant-Bit (LSB), setting the bits tells OpenZL how to build the graph:
  CODEC | SHUFFLE | DELTA | SPLIT | CRC | x | x | x |

  - CODEC - If set, use LZ4. Else ZSTD.
  - SHUFFLE - If set, use shuffle (outputs a stream for every byte of input data typesize)
  - DELTA - If set, apply a bytedelta (to all streams if necessary)
  - SPLIT - If set, do not recombine the bytestreams
  - CRC - If set, store a checksum during compression and check it during decompression

The remaining bits may be used in the future.

## Thanks

Thanks to Luke Shaw, Marta Iborra, J. David Ibáñez and Francesc Alted from the Blosc Development Team, for making this plugin possible. Thanks also to the OpenZL team for authoring of the [OpenZL library](https://github.com/facebook/openzl).

That's all folks!

The Blosc Development Team
