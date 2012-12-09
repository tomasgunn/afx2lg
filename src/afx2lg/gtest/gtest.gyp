# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'gtest',
      'type': 'static_library',
      'defines': [
        '_VARIADIC_MAX=10',
      ],
      'include_dirs': [
        '../../gtest',
        '../../gtest/include',
      ],
      'sources': [
        '../../gtest/src/gtest-all.cc',
      ],
    },
  ],
}
