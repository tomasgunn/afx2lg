#!/usr/bin/python
# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import os
import shlex
import subprocess
import sys

# Get the GYP source into the path before importing gyp.
script_dir = os.path.dirname(os.path.realpath(__file__))
src_dir = os.path.abspath(os.path.join(script_dir, os.pardir))
sys.path.insert(0, os.path.join(src_dir, 'tools', 'gyp', 'pylib'))
import gyp

def main(args):
  args = sys.argv[1:]  # strip away this script

  if sys.platform == 'win32':
    # We need Visual Studio 2012 on Windows.
    os.environ['GYP_MSVS_VERSION'] = "2012"

  gyp_file = os.path.normpath(os.path.join(script_dir, '../afx2lg/all.gyp'))
  args.append(gyp_file)

  common_gypi = os.path.normpath(os.path.join(script_dir, '../afx2lg/common.gypi'))
  args.extend(['-I' + i for i in [common_gypi]])

  return gyp.main(args)

if __name__ == '__main__':
  sys.exit(main(sys.argv))
