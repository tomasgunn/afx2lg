# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'axeloader',
      'type': 'executable',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:*',
        '../common/base.gyp:*',
        '../midi/midi.gyp:*',
      ],
      'sources': [
        '../common/common_types.h',
        'main.cc',
      ],
    },
  ],
}
