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

# Set the params for the OpenZL codec
# TODO

# Define the compression and decompression parameters for Blosc2.
cparams = {
    'codec': blosc2.Codec.OPENZL,
    'filters': [],
    'codec_meta': 7,
}

# Read the image
im = Image.open("examples/kodim23.png")
# Convert the image to a numpy array
np_array = np.asarray(im)

# Transform the numpy array to a blosc2 array. This is where compression happens, and
# the OpenZL codec is called.
bl_array = blosc2.asarray(
    np_array,
    chunks=np_array.shape,
    blocks=np_array.shape,
    cparams=cparams,
    urlpath="examples/kodim23.b2nd",
    mode="w",
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


## Notes
When using `blosc2_openzl`, there are some restrictions that you have
to keep in mind.

## More examples
See the [examples](examples/) directory for more examples.

## Thanks

Thanks to Luke Shaw, Marta Iborra, J. David Ibáñez and Francesc Alted from the Blosc Development Team, for making this plugin possible. Thanks also to the OpenZL team for authoring of the [OpenZL library](https://github.com/facebook/openzl).

That's all folks!

The Blosc Development Team
