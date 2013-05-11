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
      'target_name': 'base',
      'type': 'static_library',
      'include_dirs': [
        '..',
      ],
      'sources': [
        'common_types.h',
        'file_utils.cc',
        'file_utils.h',
        'thread_loop.cc',
        'thread_loop.h',
      ],
    },
  ],
}
