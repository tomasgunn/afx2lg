# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
  },
  'target_defaults': {
  },
  'targets': [
    {
      'target_name': 'lg',
      'type': 'static_library',
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:axefx_types',
      ],
      'sources': [
        'lg_entry.cc',
        'lg_entry.h',
        'lg_parser.cc',
        'lg_parser.h',
        'lg_utils.cc',
        'lg_utils.h',
      ],
    },
  ],
}
