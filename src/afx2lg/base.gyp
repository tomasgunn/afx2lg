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
      'target_name': 'base',
      'type': 'static_library',
      'include_dirs': [
        '.',
      ],
      'sources': [
        'common_types.h',  # todo: move.
        'common/thread_loop.cc',  # todo: rename common->base
        'common/thread_loop.h',
      ],
    },
  ],
}
