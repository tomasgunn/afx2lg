# Copyright (c) 2013 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'axys_lib',
      'type': 'static_library',
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:*',
        '../common/base.gyp:*',
        '../juce/juce.gyp:*',
      ],
      'sources': [
        'tree_preset_item.cc',
        'tree_preset_item.cc',
        'tree_root_item.cc',
        'tree_root_item.h',
      ],
    },
    {
      'target_name': 'axys',
      'product_name': 'Axys',
      'type': 'executable',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:*',
        '../common/base.gyp:*',
        '../juce/juce.gyp:*',
        '../midi/midi.gyp:*',
        'axys_lib',
      ],
      'sources': [
        'main.cc',
        'main_view.cc',
        'main_view.h',
        'MainWnd.cpp',
        'MainWnd.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCLinkerTool': {
              # /SUBSYSTEM:WINDOWS
              'SubSystem': '2',
            },
          },
        }],
        ['OS=="mac"', {
          'mac_bundle': 1,
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreMIDI.framework',
              '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
            ],
          },
          'mac_bundle_resources': [
            # TODO?
            #'TestApp/English.lproj/InfoPlist.strings',
            #'TestApp/English.lproj/MainMenu.xib',
          ],
          'xcode_settings': {
            # TODO?
            #'INFOPLIST_FILE': 'TestApp/TestApp-Info.plist',
          },
        }],
      ],
    },  # axys
  ],
}
