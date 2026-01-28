############################################################################## 
# blosc2_openzl: OpenZL plugin for Blosc2
#
# Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

import ctypes
import os
import platform
from enum import Enum
from pathlib import Path
import atexit
import numpy as np

__version__ = "0.0.1.dev0"


class OpenZLProfile(Enum):
    """
    Available OpenZL profiles.
    """

    OZLPROF_ZSTD = 0
    OZLPROF_LZ4 = 1
    OZLPROF_SH_ZSTD = 2
    OZLPROF_SH_LZ4 = 3
    OZLPROF_SH_BD_ZSTD = 6
    OZLPROF_SH_BD_LZ4 = 7
    OZLPROF_SH_BD_SPLIT_ZSTD = 14
    OZLPROF_SH_BD_SPLIT_LZ4 = 15
    # With checksum enabled #
    OZLPROF_ZSTD_CS = OZLPROF_ZSTD + 16
    OZLPROF_LZ4_CS = OZLPROF_LZ4 + 16
    OZLPROF_SH_ZSTD_CS = OZLPROF_SH_ZSTD + 16
    OZLPROF_SH_LZ4_CS = OZLPROF_SH_LZ4 + 16
    OZLPROF_SH_BD_ZSTD_CS = OZLPROF_SH_BD_ZSTD + 16
    OZLPROF_SH_BD_LZ4_CS = OZLPROF_SH_BD_LZ4 + 16
    OZLPROF_SH_BD_SPLIT_ZSTD_CS = OZLPROF_SH_BD_SPLIT_ZSTD + 16
    OZLPROF_SH_BD_SPLIT_LZ4_CS = OZLPROF_SH_BD_SPLIT_LZ4 + 16
  

def get_libpath():
    system = platform.system()
    if system in "Linux":
        libname = "libblosc2_openzl.so"
    elif system == "Darwin":
        # in future, on macOS may compile to .os (e.g. if blosc2 funcs added to blosc2_openzl.c)
        libname = "libblosc2_openzl.dylib"
    elif system == "Windows":
        libname = "blosc2_openzl.dll"
    else:
        raise RuntimeError("Unsupported system: ", system)
    return os.path.abspath(Path(__file__).parent / libname)


libpath = get_libpath()
lib = ctypes.cdll.LoadLibrary(libpath)


def print_libpath():
    print(libpath, end="")

if __name__ == "__main__":
    print_libpath()
