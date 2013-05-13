# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'afx2lg_cmd',
      'type': 'executable',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:axefx',
        '../common/base.gyp:*',
        '../lg/lg.gyp:lg',
      ],
      'sources': [
        'main.cc',
      ],
    }, {
      'target_name': 'Afx2LG',
      'type': 'executable',
      'defines': [
      ],
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../axefx/axefx.gyp:axefx',
        '../axys/axys.gyp:axys_lib',
        '../common/base.gyp:*',
        '../juce/juce.gyp:*',
        '../lg/lg.gyp:lg',
      ],
      'sources': [
        'main_app.cc',
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
              # TODO: See if we can trim this.
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
              # '$(SDKROOT)/System/Library/Frameworks/CoreMIDI.framework',
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
    },
  ],
}
