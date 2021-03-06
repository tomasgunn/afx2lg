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
      'target_name': 'axefx_types',
      'type': 'none',
      'sources': [
        'AxeFxII_9_2.axeml',
        'type_gen.py',
      ],
      'actions': [
        {
          'action_name': 'generate_types',
          'msvs_cygwin_shell': 0,
          'inputs': [
            'AxeFxII_9_2.axeml',
          ],
          'outputs': [
            'axefx_ii_ids.cc',
            'axefx_ii_ids.h',
          ],
          'action': [
            'python', 'type_gen.py', 'AxeFxII_9_2.axeml', './'
          ],
        },
      ],
    },
    {
      'target_name': 'axefx',
      'type': 'static_library',
      'include_dirs': [
        '..',
        '../../',
        '../../jsoncpp/include',
      ],
      'dependencies': [
        '../../bcl/bcl.gyp:bcl',
        '../jsoncpp/jsoncpp.gyp:*',
        'axefx_types',
      ],
      'sources': [
        'axe_fx_sysex_parser.cc',
        'axe_fx_sysex_parser.h',
        'axefx_ii_ids.cc',
        'axefx_ii_ids.h',
        'blocks.cc',
        'blocks.h',
        'ir_data.cc',
        'ir_data.h',
        'preset.cc',
        'preset.h',
        'preset_parameters.cc',
        'preset_parameters.h',
        'sysex_callback.h',
        'sysex_types.cc',
        'sysex_types.h',
      ],
    },
  ],
}
