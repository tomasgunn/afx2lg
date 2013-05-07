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
        'overrides/src',
        'source/src',
      ],
      'sources': [
        # We only use the Huffman encoder/decoder.
        'overrides/src/huffman.h',
        'overrides/src/huffman.c',
      ],
    },
  ],
}
