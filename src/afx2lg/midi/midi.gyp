# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
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
        ['OS=="mac"', {
          'sources': [
            'midi_in_mac.cc',
            'midi_mac.cc',
            'midi_mac.h',
            'midi_out_mac.cc',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreMIDI.framework',
            ],
          },
        }],
      ],
    },
  ],
}
