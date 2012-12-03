# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'afx2lg',
      'type': 'executable',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx.gyp:axefx',
        '../lg/lg.gyp:lg',
      ],
      'sources': [
        '../common_types.h',
        '../main.cc',
      ],
    },
  ],
}
