# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'dependencies': [
        '../bcl/bcl.gyp:*',
        'axe_backup/axe_backup.gyp:*',
        'axe_http/axe_http.gyp:*',
        'axe_loader/axe_loader.gyp:*',
        'axys/axys.gyp:*',
        'main/afx2lg.gyp:*',
        'test/test.gyp:*',
      ],
    }
  ]
}

