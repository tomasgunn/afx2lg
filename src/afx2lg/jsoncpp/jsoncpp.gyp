# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'jsoncpp',
      'type': 'static_library',
      'include_dirs': [
        '../../jsoncpp/src',
        '../../jsoncpp/include',
      ],
      'sources': [
        # include
        '../../jsoncpp/include/json/assertions.h',
        '../../jsoncpp/include/json/autolink.h',
        '../../jsoncpp/include/json/config.h',
        '../../jsoncpp/include/json/features.h',
        '../../jsoncpp/include/json/forwards.h',
        '../../jsoncpp/include/json/json.h',
        '../../jsoncpp/include/json/reader.h',
        '../../jsoncpp/include/json/value.h',
        '../../jsoncpp/include/json/writer.h',

        # src
        '../../jsoncpp/src/lib_json/json_batchallocator.h',
        '../../jsoncpp/src/lib_json/json_internalarray.inl',
        '../../jsoncpp/src/lib_json/json_internalmap.inl',
        '../../jsoncpp/src/lib_json/json_reader.cpp',
        '../../jsoncpp/src/lib_json/json_tool.h',
        '../../jsoncpp/src/lib_json/json_value.cpp',
        '../../jsoncpp/src/lib_json/json_valueiterator.inl',
        '../../jsoncpp/src/lib_json/json_writer.cpp',
      ],
    },
  ],
}
