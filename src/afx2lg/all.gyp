# Copyright 2011 the V8 project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'dependencies': [
        'axefx/axefx.gyp:*',
        'common/base.gyp:*',
        'gtest.gyp:*',
        'lg/lg.gyp:*',
        'main/afx2lg.gyp:*',
        'midi/midi.gyp:*',
        'test/test.gyp:*',
      ],
    }
  ]
}

