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

AxeFxBlockType GetBlockType(AxeFxIIBlockID id);
const char* GetBlockTypeName(AxeFxBlockType type);
const char* GetBlockName(AxeFxIIBlockID id);
int GetBlockBypassParamID(AxeFxBlockType type);
const char* GetParamName(AxeFxBlockType type, int param_id);
const char* GetAmpName(int index);
const char* GetCabName(int index);

// Forward declarations for block parameter lookups.
%s

}  // namespace axefx

#endif
"""

SOURCE_FILE_TEMPLATE = """// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "axefx_ii_ids.h"

// WARNING: Do not edit, this file is generated!

namespace axefx {

AxeFxBlockType GetBlockType(AxeFxIIBlockID id) {
  switch (id) {
    default:
      break;
%s  }
  return BLOCK_TYPE_INVALID;
}

const char* GetBlockTypeName(AxeFxBlockType type) {
  switch (type) {
    default:
      break;
%s  }
  return "";
}

const char* GetBlockName(AxeFxIIBlockID id) {
  switch (id) {
    default:
      break;
%s  }
  return "";
}

int GetBlockBypassParamID(AxeFxBlockType type) {
  switch (type) {
    default:
      break;
%s  }
  return -1;
}

const char* GetParamName(AxeFxBlockType type, int param_id) {
  switch (type) {
    default:
      break;
%s  }
  return "";
}

const char* GetAmpName(int index) {
  switch (index) {
    default:
      break;
%s  }
  return "";
}

const char* GetCabName(int index) {
  switch (index) {
    default:
      break;
%s  }
  return "";
}

// Implementations of block parameter lookup functions.

%s

}  // namespace axefx
"""

BLOCK_TYPE_TEMPLATE = """enum AxeFxBlockType {
  BLOCK_TYPE_INVALID = -1,
  %s
};"""

BLOCK_ID_TEMPLATE = """enum AxeFxIIBlockID {
  BLOCK_INVALID = 0,  // Use 0 and not -1 to be compatible with the matrix.
  BLOCK_SHUNT_200 = 200,
  %s
};"""

PARAM_ID_TEMPLATE = """enum %sParamID {
  %s
};"""

PARAM_ID_LOOKUP_FUNCTION_FWD_TEMPLATE = \
  "const char* Get%sParamName(%sParamID id);"

PARAM_ID_LOOKUP_FUNCTION_TEMPLATE = \
"""
const char* Get%sParamName(%sParamID id) {
  switch (id) {
    default:
      break;
%s  }
  return "";
}"""

class AxeMlParser:
  amp_names = {}
  block_ids = []
  block_to_type_id = {}
  block_type_bypass_ids = {}
  block_types = []
  cab_names = {}
  current_block = None
  current_bypass_id = None
  current_type_name = None
  effect_parameter_names = []
  effect_parameters = []
  param_ids = []
  param_lookup_fn_fwd = []
  param_lookup_fn_impl = []
  parser = None
  type_id_name = {}
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
        self.block_to_type_id[block_name] = [attrs["typeID"], attrs["name"]]
    elif name == "EffectParameters":
      if "typeID" in attrs:
        self.current_type_name = "BLOCK_TYPE_%s" % (attrs["name"].upper())
        self.block_types += ["%s = %s" % \
            (self.current_type_name, attrs["typeID"])]
        self.current_block = attrs["name"]
        self.type_id_to_name[attrs["typeID"]] = self.current_type_name
        self.type_id_name[self.current_type_name] = self.current_block
        if "bypassParam" in attrs:
          self.current_bypass_id = attrs["bypassParam"]
    elif name == "EffectParameter":
      self.effect_parameters += ["%s = %s" % (attrs["name"], attrs["id"])]
      self.effect_parameter_names += [attrs["name"]]
      if attrs["id"] == self.current_bypass_id:
        self.block_type_bypass_ids[self.current_type_name] = attrs["name"]
    elif name == "Amp":
      self.amp_names[int(attrs["id"])] = attrs["name"]
    elif name == "Cab":
      self.cab_names[int(attrs["id"])] = attrs["name"]

  def onEndElement(self, name):
    if name == "EffectParameters":
      if self.current_block != None:
        self.param_ids += \
          [PARAM_ID_TEMPLATE % (self.current_block,
                                ",\n  ".join(self.effect_parameters))]
        self.param_lookup_fn_fwd += \
          [PARAM_ID_LOOKUP_FUNCTION_FWD_TEMPLATE %
           (self.current_block, self.current_block)]
        self.param_lookup_fn_impl += \
          [PARAM_ID_LOOKUP_FUNCTION_TEMPLATE %
           (self.current_block, self.current_block,
            self.GenerateCaseLabelsWithReturn(self.effect_parameter_names))]
      self.current_block = None
      self.current_type_name = None
      self.effect_parameters = []
      self.effect_parameter_names = []
      self.current_bypass_id = None

  def GenerateCaseLabelsWithReturn(self, entries):
    ret = ""
    for e in entries:
      ret += '    case %s:\n      return "%s";\n' % (e, e.lower())
    return ret

  def GenerateBlockTypeFromID(self):
    ret = ""
    for b, t in self.block_to_type_id.items():
      ret += "    case %s:\n      return %s;\n" % (b, self.type_id_to_name[t[0]])
    return ret

  def GenerateBlockTypeName(self):
    ret = ""
    for t in self.type_id_name.items():
      ret += "    case %s:\n      return \"%s\";\n" % (t[0], t[1])
    return ret

  def GenerateBlockNameFromID(self):
    ret = ""
    for b, t in self.block_to_type_id.items():
      ret += '    case %s:\n      return "%s";\n' % (b, t[1])
    return ret

  def GenerateBlockBypassParamID(self):
    ret = ""
    for n in self.type_id_to_name.values():
      if n in self.block_type_bypass_ids:
        ret += "    case %s:\n      return %s;\n" % \
               (n, self.block_type_bypass_ids[n])
    return ret

  def GenerateParamNamesForBlocks(self):
    ret = ""
    for t in self.type_id_to_name.values():
      block_name = self.type_id_name[t]
      ret += "    case %s:\n      return Get%sParamName(static_cast<%sParamID>(param_id));\n" % \
        (t, block_name, block_name)
    return ret

  def GenerateAmpNames(self):
    ret = ""
    for i, n in sorted(self.amp_names.items()):
      ret += '    case %s:\n      return "%s";\n' %  (i, n)
    return ret

  def GenerateCabNames(self):
    ret = ""
    for i, n in sorted(self.cab_names.items()):
      ret += '    case %s:\n      return "%s";\n' %  (i, n)
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

  header = HEADER_FILE_TEMPLATE % (header, "\n".join(x.param_lookup_fn_fwd))

  source = SOURCE_FILE_TEMPLATE % (x.GenerateBlockTypeFromID(),
                                   x.GenerateBlockTypeName(),
                                   x.GenerateBlockNameFromID(),
                                   x.GenerateBlockBypassParamID(),
                                   x.GenerateParamNamesForBlocks(),
                                   x.GenerateAmpNames(),
                                   x.GenerateCabNames(),
                                   "\n".join(x.param_lookup_fn_impl))
  WriteIfChanged(h_file, header)
  WriteIfChanged(cc_file, source)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
