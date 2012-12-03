# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'test',
      'type': 'executable',
      'defines': [
        '_VARIADIC_MAX=10',
      ],
      'include_dirs': [
        '.',
        '../gtest/include',
      ],
      'dependencies': [
        'axefx.gyp:axefx',
        'lg/lg.gyp:lg',
        'base.gyp:base',
        'gtest.gyp:gtest',
        'midi.gyp:midi',
      ],
      'sources': [
        'common_types.h',
        'test/axefx_test.cc',
        'test/lg_test.cc',
        'test/main.cc',
        'test/midi_test.cc',
        'test/test_utils.cc',
        'test/test_utils.h',
        'test/thread_loop_test.cc',
      ],
    },
  ],
}
