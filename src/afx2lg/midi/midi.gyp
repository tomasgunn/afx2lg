# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'midi',
      'type': 'static_library',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'midi_in.cc',
        'midi_in.h',
        'midi_out.cc',
        'midi_out.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            'midi_in_win.cc',
            'midi_out_win.cc',
          ],
          'link_settings': {
            'libraries': [
              '-lwinmm.lib',
            ],
          },
        }],
      ],
    },
  ],
}
