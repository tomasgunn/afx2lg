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
        '..',
        '../../gtest/include',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:axefx',
        '../common/base.gyp:base',
        '../gtest.gyp:gtest',
        '../lg/lg.gyp:lg',
        '../midi/midi.gyp:midi',
      ],
      'sources': [
        'axefx_test.cc',
        'lg_test.cc',
        'main.cc',
        'midi_test.cc',
        'test_utils.cc',
        'test_utils.h',
        'thread_loop_test.cc',
      ],
    },
  ],
}
