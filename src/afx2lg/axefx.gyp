# Copyright (c) 2012 The Chromium Authors. All rights reserved.
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
        'axefx/AxeFxII_7.axeml',
        'axefx/type_gen.py',
      ],
      'actions': [
        {
          'action_name': 'generate_types',
          'msvs_cygwin_shell': 0,
          'inputs': [
            'axefx/AxeFxII_7.axeml',
          ],
          'outputs': [
            'axefx/axefx_ii_ids.cc',
            'axefx/axefx_ii_ids.h',
          ],
          'action': [
            'python.exe', 'axefx/type_gen.py', 'axefx/AxeFxII_7.axeml', 'axefx/'
          ],
        },
      ],
    },
    {
      'target_name': 'axefx',
      'type': 'static_library',
      'include_dirs': [
        '.',
      ],
      'dependencies': [
        'axefx_types',
      ],
      'sources': [
        'axefx/axe_fx_sysex_parser.cc',
        'axefx/axe_fx_sysex_parser.h',
        'axefx/axefx_ii_ids.cc',
        'axefx/axefx_ii_ids.h',
        'axefx/blocks.cc',
        'axefx/blocks.h',
        'axefx/preset.cc',
        'axefx/preset.h',
        'axefx/preset_parameters.cc',
        'axefx/preset_parameters.h',
        'axefx/sysex_types.cc',
        'axefx/sysex_types.h',
      ],
    },
  ],
}
