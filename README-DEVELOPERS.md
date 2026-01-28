# blosc2_openzl

For using blosc2_openzl you will first have to create and install its wheel.


## Download the repository

```shell
git clone https://github.com/Blosc/blosc2_openzl.git
cd blosc2_openzl
```

## Create the wheel

For Linux:

```shell
python -m cibuildwheel --only 'cp313-manylinux_x86_64'
```

For Mac x86_64:

```shell
CMAKE_OSX_ARCHITECTURES=x86_64 python -m cibuildwheel --only 'cp313-macosx_x86_64'
```

For Mac arm64:

```shell
CMAKE_OSX_ARCHITECTURES=arm64 python -m cibuildwheel --only 'cp313-macosx_arm64'
```

## Install the wheel

```shell
pip install wheelhouse/blosc2_openzl-*.whl --force-reinstall
```

## Debugging

If you would like to debug and run an example from C getting to track the problem through the C functions, you can use
the codec as a local registered codec. For that you will have to do the following:

```
// In blosc2_openzl_public.h
// Comment out the info
//BLOSC2_OPENZL_EXPORT codec_info info = {
//    .encoder=(char *)"blosc2_openzl_encoder",
//    .decoder=(char *)"blosc2_openzl_decoder"
//};

// In your example, include the blosc2_openzl_public.h header and add the function pointers
// to the codec struct before registering it.
#include "blosc2_openzl_public.h"
// Some code in between
blosc2_codec openzl_codec = {0};
openzl_codec.compname = (char *)"openzl";
openzl_codec.compcode = 160;
openzl_codec.complib = 1;
openzl_codec.version = 0;
openzl_codec.encoder = &blosc2_openzl_encoder;
openzl_codec.decoder = &blosc2_openzl_decoder;
int rc = blosc2_register_codec(&openzl_codec);
```

That's all folks!
