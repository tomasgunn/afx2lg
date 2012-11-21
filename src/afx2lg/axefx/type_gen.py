 #!/usr/bin/python
 # Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
 # Use of this source code is governed by a BSD-style license that can be
 # found in the LICENSE file.

import os, sys
import xml.parsers.expat

OUTPUT_H = "axefx_ii_ids.h"
OUTPUT_CC = "axefx_ii_ids.cc"

HEADER_FILE_TEMPLATE = """// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

// WARNING: Do not edit, this file is generated!
#pragma once
#ifndef __AXEFX_II_GENERATED_TYPE_IDS__
#define __AXEFX_II_GENERATED_TYPE_IDS__

namespace axefx {

%s

AxeFxBlockType GetBlockTypeFromID(AxeFxIIBlockID id);
int GetBlockBypassParamID(AxeFxBlockType type);

}  // namespace axefx

#endif
"""

SOURCE_FILE_TEMPLATE = """// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx_ii_ids.h"

// WARNING: Do not edit, this file is generated!

namespace axefx {

AxeFxBlockType GetBlockTypeFromID(AxeFxIIBlockID id) {
  switch (id) {
%s
  }
  return BLOCK_TYPE_INVALID;
}

int GetBlockBypassParamID(AxeFxBlockType type) {
  switch (type) {
%s
  }
  return -1;
}

}  // namespace axefx
"""

BLOCK_TYPE_TEMPLATE = """enum AxeFxBlockType {
  BLOCK_TYPE_INVALID = -1,
  %s
};"""

BLOCK_ID_TEMPLATE = """enum AxeFxIIBlockID {
  %s
};"""

PARAM_ID_TEMPLATE = """enum %sParamID {
  %s
};"""

class AxeMlParser:
  parser = None
  block_types = []
  block_ids = []
  block_type_bypass_ids = {}
  param_ids = []
  current_entries = []
  current_block = None
  current_type_name = None
  current_bypass_id = None
  block_to_type_id = {}
  type_id_to_name = {}

  def __init__(self):
    self.parser = xml.parsers.expat.ParserCreate()
    self.parser.CharacterDataHandler = self.onCharData
    self.parser.StartElementHandler = self.onStartElement
    self.parser.EndElementHandler = self.onEndElement

  def parse(self, xml_file):
    self.parser.ParseFile(open(xml_file, "rb"))

  def onCharData(self, data):
    pass

  def onStartElement(self, name, attrs):
    if name == "EffectPoolInstance":
      if "typeID" in attrs:
        block_name = "BLOCK_%s" % (attrs["name"].replace(' ', '_')\
            .replace('/','_').upper())
        self.block_ids += ["%s = %s" % (block_name, attrs["id"])]
        self.block_to_type_id[block_name] = attrs["typeID"]
    elif name == "EffectParameters":
      if "typeID" in attrs:
        self.current_type_name = "BLOCK_TYPE_%s" % (attrs["name"].upper())
        self.block_types += ["%s = %s" % \
            (self.current_type_name, attrs["typeID"])]
        self.current_block = attrs["name"]
        self.type_id_to_name[attrs["typeID"]] = self.current_type_name
        self.current_bypass_id = attrs["bypassParam"]
    elif name == "EffectParameter":
      self.current_entries += ["%s = %s" % (attrs["name"], attrs["id"])]
      if attrs["id"] == self.current_bypass_id:
        self.block_type_bypass_ids[self.current_type_name] = attrs["name"]

  def onEndElement(self, name):
    if name == "EffectParameters":
      if self.current_block != None:
        self.param_ids += \
          [PARAM_ID_TEMPLATE % (self.current_block,
                                ",\n  ".join(self.current_entries))]
      self.current_block = None
      self.current_type_name = None
      self.current_entries = []
      self.current_bypass_id = None

  def GenerateBlockTypeFromID(self):
    ret = ""
    for b, t in self.block_to_type_id.items():
      ret += "    case %s:\n      return %s;\n" % (b, self.type_id_to_name[t])
    return ret

  def GenerateBlockBypassParamID(self):
    ret = ""
    for n in self.type_id_to_name.values():
      if n in self.block_type_bypass_ids:
        ret += "    case %s:\n      return %s;\n" % \
               (n, self.block_type_bypass_ids[n])
    return ret

def WriteIfChanged(path, contents):
  if os.path.exists(path):
    if open(path, 'r').read() == contents:
      print "%s is up to date." % path
      return True
  print "Generating %s" % path
  f = open(path, 'w')
  f.write(contents)
  f.close()

def main(args):
  # args[0]: this script.
  # args[1]: '--clean' or input file.
  # args[2]: output folder
  if len(args) != 3:
    print >> sys.stderr, "Missing argument"
    print args
    sys.exit(-1)

  output_folder = os.path.normcase(args[2])
  cc_file = os.path.join(output_folder, OUTPUT_CC)
  h_file = os.path.join(output_folder, OUTPUT_H)
  if args[1].lower() == "--clean":
    print "Deleting source files."
    try:
      os.unlink(cc_file)
    except:
      print >> os.stderr, "%s doesn't exist" % cc_file
    try:
      os.unlink(h_file)
    except:
      print >> os.stderr, "%s doesn't exist" % h_file
    sys.exit(0)

  input_file = os.path.normcase(args[1])
  if not os.path.exists(input_file):
    print >> os.stderr, "%s doesn't exist" % input_file
    sys.exit(-1)

  x = AxeMlParser()
  x.parse(input_file)
  header = BLOCK_TYPE_TEMPLATE % (",\n  ".join(x.block_types)) + \
           "\n\n" + (BLOCK_ID_TEMPLATE % (",\n  ".join(x.block_ids)))
  header += "\n\n"
  header += "\n\n".join(x.param_ids)

  header = HEADER_FILE_TEMPLATE % (header)

  source = SOURCE_FILE_TEMPLATE % (x.GenerateBlockTypeFromID(),
                                   x.GenerateBlockBypassParamID())
  WriteIfChanged(h_file, header)
  WriteIfChanged(cc_file, source)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
