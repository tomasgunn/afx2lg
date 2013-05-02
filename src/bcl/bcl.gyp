# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'bcl',
      'type': 'static_library',
      'defines': [
      ],
      'include_dirs': [
        'source/src',
      ],
      'sources': [
        # We only use the Huffman encoder/decoder.
        'source/src/huffman.h',
        'source/src/huffman.c',
      ],
    },
  ],
}
