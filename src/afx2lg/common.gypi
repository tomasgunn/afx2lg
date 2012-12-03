#!/usr/bin/python
# Copyright (c) 2012 Tomas Gunnarsson. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Mostly borrowed from Chrome's common.gypi file.
{
  'variables': {
    # TODO(tommi): fix the below.
    'os_posix%': 0,
    # 'windows_sdk_path': 'C:/Program Files (x86)/Microsoft SDKs/Windows/v8.0A',
    'msvs_use_common_linker_extras': 1,
    'build_dir_prefix%': '',
    'msvs_use_common_release': 1,
    'mac_release_optimization%': '3', # Use -O3 unless overridden
    'mac_debug_optimization%': '0',   # Use -O0 unless overridden
    'release_extra_cflags%': '',
    'win_debug_extra_cflags%': '',
    'msvs_debug_link_incremental%': 1,
    'debug_extra_cflags%': '',
  },
  'target_defaults': {
    'variables': {

      # See http://msdn.microsoft.com/en-us/library/aa652360(VS.71).aspx
      'win_release_Optimization%': '1', # 2 = optimizeMinSpace
      'win_debug_Optimization%': '0',   # 0 = optimizeDisabled

      # See http://msdn.microsoft.com/en-us/library/2kxx5t2c(v=vs.80).aspx
      # Tri-state: blank is default, 1 on, 0 off
      'win_release_OmitFramePointers%': '0',
      # Tri-state: blank is default, 1 on, 0 off
      'win_debug_OmitFramePointers%': '',

      # See http://msdn.microsoft.com/en-us/library/8wtf2dfz(VS.71).aspx
      'win_debug_RuntimeChecks%': '3',    # 3 = all checks enabled, 0 = off

      # See http://msdn.microsoft.com/en-us/library/47238hez(VS.71).aspx
      'win_debug_InlineFunctionExpansion%': '',    # empty = default, 0 = off,
      'win_release_InlineFunctionExpansion%': '2', # 1 = only __inline, 2 = max

      # VS inserts quite a lot of extra checks to algorithms like
      # std::partial_sort in Debug build which make them O(N^2)
      # instead of O(N*logN). This is particularly slow under memory
      # tools like ThreadSanitizer so we want it to be disablable.
      # See http://msdn.microsoft.com/en-us/library/aa985982(v=VS.80).aspx
      'win_debug_disable_iterator_debugging%': '0',


      # See http://msdn.microsoft.com/en-us/library/aa652367.aspx
      'win_release_RuntimeLibrary%': '0', # 0 = /MT (nondebug static)
      'win_debug_RuntimeLibrary%': '1',   # 1 = /MTd (debug static)
    },
    'conditions': [
    ],  # conditions for 'target_defaults'
    'target_conditions': [
    ],  # target_conditions for 'target_defaults'

    'default_configuration': 'Debug',
    'configurations': {
      # VCLinkerTool LinkIncremental values below:
      #   0 == default
      #   1 == /INCREMENTAL:NO
      #   2 == /INCREMENTAL
      # Debug links incremental, Release does not.
      #
      # Abstract base configurations to cover common attributes.
      #
      'Common_Base': {
        'abstract': 1,
        'msvs_configuration_attributes': {
          'OutputDirectory': '<(DEPTH)\\build\\<(build_dir_prefix)$(ConfigurationName)',
          'IntermediateDirectory': '$(OutDir)\\obj\\$(ProjectName)',
          'CharacterSet': '1',
        },
      },
      'x86_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCLinkerTool': {
            'TargetMachine': '1',
          },
        },
        'msvs_configuration_platform': 'Win32',
      },
      'x64_Base': {
        'abstract': 1,
        'msvs_configuration_platform': 'x64',
        'msvs_settings': {
          'VCLinkerTool': {
            'TargetMachine': '17', # x86 - 64
            'AdditionalLibraryDirectories!': [
              # '<(windows_sdk_path)/Lib/win8/um/x86'
            ],
            'AdditionalLibraryDirectories': [
              # '<(windows_sdk_path)/Lib/win8/um/x64'
            ],
          },
          'VCLibrarianTool': {
            'AdditionalLibraryDirectories!': [
              # '<(windows_sdk_path)/Lib/win8/um/x86'
            ],
            'AdditionalLibraryDirectories': [
              # '<(windows_sdk_path)/Lib/win8/um/x64'
            ],
          },
        },
        'defines': [
        ],
      },
      'Debug_Base': {
        'abstract': 1,
        'defines': [
          'DYNAMIC_ANNOTATIONS_ENABLED=1',
          'WTF_USE_DYNAMIC_ANNOTATIONS=1',
        ],
        'xcode_settings': {
          'COPY_PHASE_STRIP': 'NO',
          'GCC_OPTIMIZATION_LEVEL': '<(mac_debug_optimization)',
          'OTHER_CFLAGS': [
            '<@(debug_extra_cflags)',
          ],
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': '<(win_debug_Optimization)',
            'PreprocessorDefinitions': ['_DEBUG'],
            'BasicRuntimeChecks': '<(win_debug_RuntimeChecks)',
            'RuntimeLibrary': '<(win_debug_RuntimeLibrary)',
            'conditions': [
              # According to MSVS, InlineFunctionExpansion=0 means
              # "default inlining", not "/Ob0".
              # Thus, we have to handle InlineFunctionExpansion==0 separately.
              ['win_debug_InlineFunctionExpansion==0', {
                'AdditionalOptions': ['/Ob0'],
              }],
              ['win_debug_InlineFunctionExpansion!=""', {
                'InlineFunctionExpansion':
                  '<(win_debug_InlineFunctionExpansion)',
              }],
              ['win_debug_disable_iterator_debugging==1', {
                'PreprocessorDefinitions': ['_HAS_ITERATOR_DEBUGGING=0'],
              }],

              # if win_debug_OmitFramePointers is blank, leave as default
              ['win_debug_OmitFramePointers==1', {
                'OmitFramePointers': 'true',
              }],
              ['win_debug_OmitFramePointers==0', {
                'OmitFramePointers': 'false',
                # The above is not sufficient (http://crbug.com/106711): it
                # simply eliminates an explicit "/Oy", but both /O2 and /Ox
                # perform FPO regardless, so we must explicitly disable.
                # We still want the false setting above to avoid having
                # "/Oy /Oy-" and warnings about overriding.
                'AdditionalOptions': ['/Oy-'],
              }],
            ],
            'AdditionalOptions': [ '<@(win_debug_extra_cflags)', ],
          },
          'VCLinkerTool': {
            'LinkIncremental': '<(msvs_debug_link_incremental)',
            # ASLR makes debugging with windbg difficult because Chrome.exe and
            # Chrome.dll share the same base name. As result, windbg will
            # name the Chrome.dll module like chrome_<base address>, where
            # <base address> typically changes with each launch. This in turn
            # means that breakpoints in Chrome.dll don't stick from one launch
            # to the next. For this reason, we turn ASLR off in debug builds.
            # Note that this is a three-way bool, where 0 means to pick up
            # the default setting, 1 is off and 2 is on.
            'RandomizedBaseAddress': 1,
          },
          'VCResourceCompilerTool': {
            'PreprocessorDefinitions': ['_DEBUG'],
          },
        },
        'conditions': [
          ['OS=="linux" or OS=="android"', {
            'target_conditions': [
              ['_toolset=="target"', {
                'cflags': [
                  '<@(debug_extra_cflags)',
                ],
              }],
            ],
          }],
        ],
      },
      'Release_Base': {
        'abstract': 1,
        'defines': [
          'NDEBUG',
        ],
        'xcode_settings': {
          'DEAD_CODE_STRIPPING': 'YES',  # -Wl,-dead_strip
          'GCC_OPTIMIZATION_LEVEL': '<(mac_release_optimization)',
          'OTHER_CFLAGS': [ '<@(release_extra_cflags)', ],
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': '<(win_release_RuntimeLibrary)',
            'Optimization': '<(win_release_Optimization)',
            'conditions': [
              # According to MSVS, InlineFunctionExpansion=0 means
              # "default inlining", not "/Ob0".
              # Thus, we have to handle InlineFunctionExpansion==0 separately.
              ['win_release_InlineFunctionExpansion==0', {
                'AdditionalOptions': ['/Ob0'],
              }],
              ['win_release_InlineFunctionExpansion!=""', {
                'InlineFunctionExpansion':
                  '<(win_release_InlineFunctionExpansion)',
              }],

              # if win_release_OmitFramePointers is blank, leave as default
              ['win_release_OmitFramePointers==1', {
                'OmitFramePointers': 'true',
              }],
              ['win_release_OmitFramePointers==0', {
                'OmitFramePointers': 'false',
                # The above is not sufficient (http://crbug.com/106711): it
                # simply eliminates an explicit "/Oy", but both /O2 and /Ox
                # perform FPO regardless, so we must explicitly disable.
                # We still want the false setting above to avoid having
                # "/Oy /Oy-" and warnings about overriding.
                'AdditionalOptions': ['/Oy-'],
              }],
            ],
          },
          'VCLinkerTool': {
            # LinkIncremental is a tri-state boolean, where 0 means default
            # (i.e., inherit from parent solution), 1 means false, and
            # 2 means true.
            'LinkIncremental': '1',
            # This corresponds to the /PROFILE flag which ensures the PDB
            # file contains FIXUP information (growing the PDB file by about
            # 5%) but does not otherwise alter the output binary. This
            # information is used by the Syzygy optimization tool when
            # decomposing the release image.
            #'Profile': 'true',
          },
        },
        'conditions': [
          ['msvs_use_common_release', {
            # 'includes': ['release.gypi'],
          }],
          ['OS=="linux" or OS=="android"', {
            'target_conditions': [
              ['_toolset=="target"', {
                'cflags': [
                  '<@(release_extra_cflags)',
                ],
              }],
            ],
          }],
        ],
      },
      #
      # Concrete configurations
      #
      'Debug': {
        'inherit_from': ['Common_Base', 'x86_Base', 'Debug_Base'],
      },
      'Release': {
        'inherit_from': ['Common_Base', 'x86_Base', 'Release_Base'],
      },
      'conditions': [
        [ 'OS=="win"', {
          # TODO(bradnelson): add a gyp mechanism to make this more graceful.
          'Debug_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Debug_Base'],
          },
          'Release_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Release_Base'],
          },
        }],
      ],
    },
  },
  'conditions': [
    ['os_posix==1', {
      'target_defaults': {
        'cflags': [
          '-fstack-protector',
          '--param=ssp-buffer-size=4',
        ],
        'ldflags': [
          '-Wl,-z,now',
          '-Wl,-z,relro',
        ],
        'conditions': [
        ],
      },
    }],
    ['os_posix==1 and OS!="mac" and OS!="ios"', {
      'target_defaults': {
        # Enable -Werror by default, but put it in a variable so it can
        # be disabled in ~/.gyp/include.gypi on the valgrind builders.
        'variables': {
          'werror%': '-Werror',
          'libraries_for_target%': '',
        },
        'defines': [
          '_FILE_OFFSET_BITS=64',
        ],
        'cflags': [
          '<(werror)',  # See note above about the werror variable.
          '-pthread',
          '-fno-strict-aliasing',  # See http://crbug.com/32204
          '-Wall',
          # Don't warn about unused function params.
          '-Wno-unused-parameter',
          # Don't warn about the "struct foo f = {0};" initialization pattern.
          '-Wno-missing-field-initializers',
          # Don't export any symbols (for example, to plugins we dlopen()).
          # Note: this is *required* to make some plugins work.
          '-fvisibility=hidden',
          '-pipe',
        ],
        'cflags_cc': [
          '-fno-rtti',
          '-fno-threadsafe-statics',
          # Make inline functions have hidden visiblity by default.
          # Surprisingly, not covered by -fvisibility=hidden.
          '-fvisibility-inlines-hidden',
        ],
        'ldflags': [
          '-pthread', '-Wl,-z,noexecstack',
        ],
        'libraries' : [
          '<(libraries_for_target)',
        ],
        'configurations': {
          'Debug_Base': {
            'variables': {
              'debug_optimize%': '0',
            },
            'defines': [
              '_DEBUG',
            ],
            'cflags': [
              '-O>(debug_optimize)',
              '-g',
            ],
            'conditions' : [
              ['OS=="android" and android_full_debug==0', {
                # Some configurations are copied from Release_Base to reduce
                # the binary size.
                'variables': {
                  'debug_optimize%': 's',
                },
                'cflags': [
                  '-fomit-frame-pointer',
                  '-fdata-sections',
                  '-ffunction-sections',
                ],
                'ldflags': [
                  '-Wl,-O1',
                  '-Wl,--as-needed',
                  '-Wl,--gc-sections',
                ],
              }],
            ],
          },
          'Release_Base': {
            'variables': {
              'release_optimize%': '2',
            },
            'cflags': [
              '-O<(release_optimize)',
              # Don't emit the GCC version ident directives, they just end up
              # in the .comment section taking up binary size.
              '-fno-ident',
              # Put data and code in their own sections, so that unused symbols
              # can be removed at link time with --gc-sections.
              '-fdata-sections',
              '-ffunction-sections',
            ],
            'ldflags': [
              # Specifically tell the linker to perform optimizations.
              # See http://lwn.net/Articles/192624/ .
              '-Wl,-O1',
              '-Wl,--as-needed',
            ],
            'conditions' : [
              ['no_gc_sections==0', {
                'ldflags': [
                  '-Wl,--gc-sections',
                ],
              }],
              ['OS=="android"', {
                'variables': {
                  'release_optimize%': 's',
                },
                'cflags': [
                  '-fomit-frame-pointer',
                ],
              }],
              ['profiling==1', {
                'cflags': [
                  '-fno-omit-frame-pointer',
                  '-g',
                ],
              }],
            ],
          },
        },
        'variants': {
          'coverage': {
            'cflags': ['-fprofile-arcs', '-ftest-coverage'],
            'ldflags': ['-fprofile-arcs'],
          },
          'profile': {
            'cflags': ['-pg', '-g'],
            'ldflags': ['-pg'],
          },
          'symbols': {
            'cflags': ['-g'],
          },
        },
        'conditions': [
          ['target_arch=="ia32"', {
            'target_conditions': [
              ['_toolset=="target"', {
                'asflags': [
                  # Needed so that libs with .s files (e.g. libicudata.a)
                  # are compatible with the general 32-bit-ness.
                  '-32',
                ],
                # All floating-point computations on x87 happens in 80-bit
                # precision.  Because the C and C++ language standards allow
                # the compiler to keep the floating-point values in higher
                # precision than what's specified in the source and doing so
                # is more efficient than constantly rounding up to 64-bit or
                # 32-bit precision as specified in the source, the compiler,
                # especially in the optimized mode, tries very hard to keep
                # values in x87 floating-point stack (in 80-bit precision)
                # as long as possible. This has important side effects, that
                # the real value used in computation may change depending on
                # how the compiler did the optimization - that is, the value
                # kept in 80-bit is different than the value rounded down to
                # 64-bit or 32-bit. There are possible compiler options to
                # make this behavior consistent (e.g. -ffloat-store would keep
                # all floating-values in the memory, thus force them to be
                # rounded to its original precision) but they have significant
                # runtime performance penalty.
                #
                # -mfpmath=sse -msse2 makes the compiler use SSE instructions
                # which keep floating-point values in SSE registers in its
                # native precision (32-bit for single precision, and 64-bit
                # for double precision values). This means the floating-point
                # value used during computation does not change depending on
                # how the compiler optimized the code, since the value is
                # always kept in its specified precision.
                'conditions': [
                  # Install packages have started cropping up with
                  # different headers between the 32-bit and 64-bit
                  # versions, so we have to shadow those differences off
                  # and make sure a 32-bit-on-64-bit build picks up the
                  # right files.
                  # For android build, use NDK headers instead of host headers
                  ['host_arch!="ia32" and OS!="android"', {
                    'include_dirs+': [
                      '/usr/include32',
                    ],
                  }],
                ],
                # -mmmx allows mmintrin.h to be used for mmx intrinsics.
                # video playback is mmx and sse2 optimized.
                'cflags': [
                  '-m32',
                  '-mmmx',
                ],
                'ldflags': [
                  '-m32',
                ],
              }],
            ],
          }],
          ['target_arch=="arm"', {
            'target_conditions': [
              ['_toolset=="target"', {
                'cflags_cc': [
                  # The codesourcery arm-2009q3 toolchain warns at that the ABI
                  # has changed whenever it encounters a varargs function. This
                  # silences those warnings, as they are not helpful and
                  # clutter legitimate warnings.
                  '-Wno-abi',
                ],
                'conditions': [
                  ['arm_thumb==1', {
                    'cflags': [
                    '-mthumb',
                    ]
                  }],
                  ['armv7==1', {
                    'cflags': [
                      '-march=armv7-a',
                      '-mtune=cortex-a8',
                      '-mfloat-abi=<(arm_float_abi)',
                    ],
                    'conditions': [
                      ['arm_neon==1', {
                        'cflags': [ '-mfpu=neon', ],
                      }, {
                        'cflags': [ '-mfpu=<(arm_fpu)', ],
                      }],
                    ],
                  }],
                  ['OS=="android"', {
                    # Most of the following flags are derived from what Android
                    # uses by default when building for arm, reference for which
                    # can be found in the following file in the Android NDK:
                    # toolchains/arm-linux-androideabi-4.4.3/setup.mk
                    'cflags': [
                      # The tree-sra optimization (scalar replacement for
                      # aggregates enabling subsequent optimizations) leads to
                      # invalid code generation when using the Android NDK's
                      # compiler (r5-r7). This can be verified using
                      # TestWebKitAPI's WTF.Checked_int8_t test.
                      '-fno-tree-sra',
                      '-fuse-ld=gold',
                      '-Wno-psabi',
                    ],
                    # Android now supports .relro sections properly.
                    # NOTE: While these flags enable the generation of .relro
                    # sections, the generated libraries can still be loaded on
                    # older Android platform versions.
                    'ldflags': [
                        '-Wl,-z,relro',
                        '-Wl,-z,now',
                        '-fuse-ld=gold',
                    ],
                    'conditions': [
                      ['arm_thumb == 1', {
                        # Android toolchain doesn't support -mimplicit-it=thumb
                        'cflags!': [ '-Wa,-mimplicit-it=thumb', ],
                        'cflags': [ '-mthumb-interwork', ],
                      }],
                      ['armv7==0', {
                        # Flags suitable for Android emulator
                        'cflags': [
                          '-march=armv5te',
                          '-mtune=xscale',
                          '-msoft-float',
                        ],
                        'defines': [
                          '__ARM_ARCH_5__',
                          '__ARM_ARCH_5T__',
                          '__ARM_ARCH_5E__',
                          '__ARM_ARCH_5TE__',
                        ],
                      }],
                      ['profiling==1', {
                        'cflags': [
                          '-marm', # Probably reduntant, but recommend by "perf" docs.
                          '-mapcs-frame', # Seems required by -fno-omit-frame-pointer.
                        ],
                      }],
                    ],
                    'target_conditions': [
                      # ndk-build copies .a's around the filesystem, breaking
                      # relative paths in thin archives.  Disable using thin
                      # archives to avoid problems until one of these is fixed:
                      # http://code.google.com/p/android/issues/detail?id=40302
                      # http://code.google.com/p/android/issues/detail?id=40303
                      ['_type=="static_library"', {
                        'standalone_static_library': 1,
                      }],
                    ],
                  }],
                ],
              }],
            ],
          }],
          ['linux_fpic==1', {
            'cflags': [
              '-fPIC',
            ],
            'ldflags': [
              '-fPIC',
            ],
          }],
          ['sysroot!=""', {
            'target_conditions': [
              ['_toolset=="target"', {
                'cflags': [
                  '--sysroot=<(sysroot)',
                ],
                'ldflags': [
                  '--sysroot=<(sysroot)',
                ],
              }]]
          }],
        ],
      },
    }],
    ['OS=="android"', {
      'variables': {
        'conditions': [
          # Use shared stlport library when system one used.
          # Figure this out early since it needs symbols from libgcc.a, so it
          # has to be before that in the set of libraries.
          ['use_system_stlport==1', {
            'android_stlport_library': 'stlport',
          }, {
            'android_stlport_library': 'stlport_static',
          }],
        ],

        # Placing this variable here prevents from forking libvpx, used
        # by remoting.  Remoting is off, so it needn't built,
        # so forking it's deps seems like overkill.
        # But this variable need defined to properly run gyp.
        # A proper solution is to have an OS==android conditional
        # in third_party/libvpx/libvpx.gyp to define it.
        'libvpx_path': 'lib/linux/arm',
      },
      'target_defaults': {
        'variables': {
          'release_extra_cflags%': '',
        },

        'target_conditions': [
          # Settings for building device targets using Android's toolchain.
          # These are based on the setup.mk file from the Android NDK.
          #
          # The NDK Android executable link step looks as follows:
          #  $LDFLAGS
          #  $(TARGET_CRTBEGIN_DYNAMIC_O)  <-- crtbegin.o
          #  $(PRIVATE_OBJECTS)            <-- The .o that we built
          #  $(PRIVATE_STATIC_LIBRARIES)   <-- The .a that we built
          #  $(TARGET_LIBGCC)              <-- libgcc.a
          #  $(PRIVATE_SHARED_LIBRARIES)   <-- The .so that we built
          #  $(PRIVATE_LDLIBS)             <-- System .so
          #  $(TARGET_CRTEND_O)            <-- crtend.o
          #
          # For now the above are approximated for executables by adding
          # crtbegin.o to the end of the ldflags and 'crtend.o' to the end
          # of 'libraries'.
          #
          # The NDK Android shared library link step looks as follows:
          #  $LDFLAGS
          #  $(PRIVATE_OBJECTS)            <-- The .o that we built
          #  -l,--whole-archive
          #  $(PRIVATE_WHOLE_STATIC_LIBRARIES)
          #  -l,--no-whole-archive
          #  $(PRIVATE_STATIC_LIBRARIES)   <-- The .a that we built
          #  $(TARGET_LIBGCC)              <-- libgcc.a
          #  $(PRIVATE_SHARED_LIBRARIES)   <-- The .so that we built
          #  $(PRIVATE_LDLIBS)             <-- System .so
          #
          # For now, assume that whole static libraries are not needed.
          #
          # For both executables and shared libraries, add the proper
          # libgcc.a to the start of libraries which puts it in the
          # proper spot after .o and .a files get linked in.
          #
          # TODO: The proper thing to do longer-tem would be proper gyp
          # support for a custom link command line.
          ['_toolset=="target"', {
            'cflags!': [
              '-pthread',  # Not supported by Android toolchain.
            ],
            'cflags': [
              '-ffunction-sections',
              '-funwind-tables',
              '-g',
              '-fstack-protector',
              '-fno-short-enums',
              '-finline-limit=64',
              '-Wa,--noexecstack',
              '<@(release_extra_cflags)',
            ],
            'defines': [
              'ANDROID',
              '__GNU_SOURCE=1',  # Necessary for clone()
              'USE_STLPORT=1',
              '_STLP_USE_PTR_SPECIALIZATIONS=1',
            ],
            'ldflags!': [
              '-pthread',  # Not supported by Android toolchain.
            ],
            'ldflags': [
              '-nostdlib',
              '-Wl,--no-undefined',
              # Don't export symbols from statically linked libraries.
              '-Wl,--exclude-libs=ALL',
            ],
            'libraries': [
              '-l<(android_stlport_library)',
              # Manually link the libgcc.a that the cross compiler uses.
              '<!(<(android_toolchain)/*-gcc -print-libgcc-file-name)',
              '-lc',
              '-ldl',
              '-lstdc++',
              '-lm',
            ],
            'conditions': [
              ['android_upstream_bringup==1', {
                'defines': ['ANDROID_UPSTREAM_BRINGUP=1',],
              }],
              ['android_build_type==0', {
                'defines': [
                  # The NDK has these things, but doesn't define the constants
                  # to say that it does. Define them here instead.
                  'HAVE_SYS_UIO_H',
                ],
                'cflags': [
                  '--sysroot=<(android_ndk_sysroot)',
                ],
                'ldflags': [
                  '--sysroot=<(android_ndk_sysroot)',
                ],
              }],
              ['android_build_type==1', {
                'include_dirs': [
                  # OpenAL headers from the Android tree.
                  '<(android_src)/frameworks/wilhelm/include',
                ],
                'cflags': [
                  # Android predefines this as 1; undefine it here so Chromium
                  # can redefine it later to be 2 for chromium code and unset
                  # for third party code. This works because cflags are added
                  # before defines.
                  '-U_FORTIFY_SOURCE',
                  # Chromium builds its own (non-third-party) code with
                  # -Werror to make all warnings into errors. However, Android
                  # enables warnings that Chromium doesn't, so some of these
                  # extra warnings trip and break things.
                  # For now, we leave these warnings enabled but prevent them
                  # from being treated as errors.
                  #
                  # Things that are part of -Wextra:
                  '-Wno-error=extra', # Enabled by -Wextra, but no specific flag
                  '-Wno-error=ignored-qualifiers',
                  '-Wno-error=type-limits',
                  # Other things unrelated to -Wextra:
                  '-Wno-error=non-virtual-dtor',
                  '-Wno-error=sign-promo',
                ],
                'cflags_cc': [
                ],
              }],
              ['android_build_type==1 and chromium_code==0', {
                'cflags': [
                  # There is a class of warning which:
                  #  1) Android always enables and also treats as errors
                  #  2) Chromium ignores in third party code
                  # For now, I am leaving these warnings enabled but preventing
                  # them from being treated as errors here.
                  '-Wno-error=address',
                  '-Wno-error=format-security',
                  '-Wno-error=non-virtual-dtor',
                  '-Wno-error=return-type',
                  '-Wno-error=sequence-point',
                ],
              }],
              ['target_arch == "arm"', {
                'ldflags': [
                  # Enable identical code folding to reduce size.
                  '-Wl,--icf=safe',
                ],
              }],
              # NOTE: The stlport header include paths below are specified in
              # cflags rather than include_dirs because they need to come
              # after include_dirs. Think of them like system headers, but
              # don't use '-isystem' because the arm-linux-androideabi-4.4.3
              # toolchain (circa Gingerbread) will exhibit strange errors.
              # The include ordering here is important; change with caution.
              ['use_system_stlport==1', {
                'cflags': [
                  # For libstdc++/include, which is used by stlport.
                  '-I<(android_src)/bionic',
                  '-I<(android_src)/external/stlport/stlport',
                ],
              }, { # else: use_system_stlport!=1
                'cflags': [
                  '-I<(android_ndk_root)/sources/cxx-stl/stlport/stlport',
                ],
                'conditions': [
                  ['target_arch=="arm" and armv7==1', {
                    'ldflags': [
                      '-L<(android_ndk_root)/sources/cxx-stl/stlport/libs/armeabi-v7a',
                      '-L<(android_ndk_root)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a',
                    ],
                  }],
                  ['target_arch=="arm" and armv7==0', {
                    'ldflags': [
                      '-L<(android_ndk_root)/sources/cxx-stl/stlport/libs/armeabi',
                      '-L<(android_ndk_root)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi',
                    ],
                  }],
                  ['target_arch=="ia32"', {
                    'ldflags': [
                      '-L<(android_ndk_root)/sources/cxx-stl/stlport/libs/x86',
                      '-L<(android_ndk_root)/sources/cxx-stl/gnu-libstdc++/4.6/libs/x86',
                    ],
                  }],
                ],
              }],
              ['target_arch=="ia32"', {
                # The x86 toolchain currently has problems with stack-protector.
                'cflags!': [
                  '-fstack-protector',
                ],
                'cflags': [
                  '-fno-stack-protector',
                ],
              }],
            ],
            'target_conditions': [
              ['_type=="executable"', {
                'ldflags': [
                  '-Bdynamic',
                  '-Wl,-dynamic-linker,/system/bin/linker',
                  '-Wl,--gc-sections',
                  '-Wl,-z,nocopyreloc',
                  # crtbegin_dynamic.o should be the last item in ldflags.
                  '<(android_ndk_lib)/crtbegin_dynamic.o',
                ],
                'libraries': [
                  # crtend_android.o needs to be the last item in libraries.
                  # Do not add any libraries after this!
                  '<(android_ndk_lib)/crtend_android.o',
                ],
                'conditions': [
                  ['asan==1', {
                    'cflags': [
                      '-fPIE',
                    ],
                    'ldflags': [
                      '-pie',
                    ],
                  }],
                ],
              }],
              ['_type=="shared_library" or _type=="loadable_module"', {
                'ldflags': [
                  '-Wl,-shared,-Bsymbolic',
                  # crtbegin_so.o should be the last item in ldflags.
                  '<(android_ndk_lib)/crtbegin_so.o',
                ],
                'libraries': [
                  # crtend_so.o needs to be the last item in libraries.
                  # Do not add any libraries after this!
                  '<(android_ndk_lib)/crtend_so.o',
                ],
              }],
            ],
          }],
          # Settings for building host targets using the system toolchain.
          ['_toolset=="host"', {
            'cflags!': [
              '-w',  # http://crbug.com/162783
            ],
            'ldflags!': [
              '-fsanitize=address',
              '-Wl,-z,noexecstack',
              '-Wl,--gc-sections',
              '-Wl,-O1',
              '-Wl,--as-needed',
            ],
            'sources/': [
              ['exclude', '_android(_unittest)?\\.cc$'],
              ['exclude', '(^|/)android/']
            ],
          }],
        ],
      },
    }],
    ['OS=="mac" or OS=="ios"', {
      'target_defaults': {
        'mac_bundle': 0,
        'xcode_settings': {
          'ALWAYS_SEARCH_USER_PATHS': 'NO',
          # Don't link in libarclite_macosx.a, see http://crbug.com/156530.
          'CLANG_LINK_OBJC_RUNTIME': 'NO',          # -fno-objc-link-runtime
          'GCC_C_LANGUAGE_STANDARD': 'c99',         # -std=c99
          'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
          'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
          # GCC_INLINES_ARE_PRIVATE_EXTERN maps to -fvisibility-inlines-hidden
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
          'GCC_OBJC_CALL_CXX_CDTORS': 'YES',        # -fobjc-call-cxx-cdtors
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',      # -fvisibility=hidden
          'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
          'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES',    # -Werror
          'GCC_VERSION': '4.2',
          'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES',  # -Wnewline-eof
          'USE_HEADERMAP': 'NO',
          'WARNING_CFLAGS': [
            '-Wall',
            '-Wendif-labels',
            '-Wextra',
            # Don't warn about unused function parameters.
            '-Wno-unused-parameter',
            # Don't warn about the "struct foo f = {0};" initialization
            # pattern.
            '-Wno-missing-field-initializers',
          ],
        },
        'target_conditions': [
          ['_type!="static_library"', {
            'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-search_paths_first']},
            'conditions': [
            ],
          }],
          ['_mac_bundle', {
            'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-ObjC']},
          }],
        ],  # target_conditions
      },  # target_defaults
    }],  # OS=="mac" or OS=="ios"
    ['OS=="mac"', {
      'target_defaults': {
        'variables': {
          # These should end with %, but there seems to be a bug with % in
          # variables that are intended to be set to different values in
          # different targets, like these.
          'mac_pie': 1,        # Most executables can be position-independent.
          'mac_real_dsym': 0,  # Fake .dSYMs are fine in most cases.
          # Strip debugging symbols from the target.
          'mac_strip': '<(mac_strip_release)',
        },
        'xcode_settings': {
          'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
                                                    # (Equivalent to -fPIC)
          # MACOSX_DEPLOYMENT_TARGET maps to -mmacosx-version-min
          'MACOSX_DEPLOYMENT_TARGET': '<(mac_deployment_target)',
          # Keep pch files below xcodebuild/.
          'SHARED_PRECOMPS_DIR': '$(CONFIGURATION_BUILD_DIR)/SharedPrecompiledHeaders',
          'OTHER_CFLAGS': [
            '-fno-strict-aliasing',  # See http://crbug.com/32204
          ],
        },
        'target_conditions': [
          ['_type=="executable"', {
            'postbuilds': [
              {
                # Arranges for data (heap) pages to be protected against
                # code execution when running on Mac OS X 10.7 ("Lion"), and
                # ensures that the position-independent executable (PIE) bit
                # is set for ASLR when running on Mac OS X 10.5 ("Leopard").
                'variables': {
                  # Define change_mach_o_flags in a variable ending in _path
                  # so that GYP understands it's a path and performs proper
                  # relativization during dict merging.
                  'change_mach_o_flags_path':
                      'mac/change_mach_o_flags_from_xcode.sh',
                  'change_mach_o_flags_options%': [
                  ],
                  'target_conditions': [
                    ['mac_pie==0 or release_valgrind_build==1', {
                      # Don't enable PIE if it's unwanted. It's unwanted if
                      # the target specifies mac_pie=0 or if building for
                      # Valgrind, because Valgrind doesn't understand slide.
                      # See the similar mac_pie/release_valgrind_build check
                      # below.
                      'change_mach_o_flags_options': [
                        '--no-pie',
                      ],
                    }],
                  ],
                },
                'postbuild_name': 'Change Mach-O Flags',
                'action': [
                  '<(change_mach_o_flags_path)',
                  '>@(change_mach_o_flags_options)',
                ],
              },
            ],
            'conditions': [
              ['asan==1', {
                'variables': {
                 'asan_saves_file': 'asan.saves',
                },
                'xcode_settings': {
                  'CHROMIUM_STRIP_SAVE_FILE': '<(asan_saves_file)',
                },
              }],
            ],
            'target_conditions': [
              ['mac_pie==1 and release_valgrind_build==0', {
                # Turn on position-independence (ASLR) for executables. When
                # PIE is on for the Chrome executables, the framework will
                # also be subject to ASLR.
                'xcode_settings': {
                  'OTHER_LDFLAGS': [
                    '-Wl,-pie',  # Position-independent executable (MH_PIE)
                  ],
                },
              }],
            ],
          }],
          ['(_type=="executable" or _type=="shared_library" or \
             _type=="loadable_module") and mac_strip!=0', {
            'target_conditions': [
              ['mac_real_dsym == 1', {
                # To get a real .dSYM bundle produced by dsymutil, set the
                # debug information format to dwarf-with-dsym.  Since
                # strip_from_xcode will not be used, set Xcode to do the
                # stripping as well.
                'configurations': {
                  'Release_Base': {
                    'xcode_settings': {
                      'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
                      'DEPLOYMENT_POSTPROCESSING': 'YES',
                      'STRIP_INSTALLED_PRODUCT': 'YES',
                      'target_conditions': [
                        ['_type=="shared_library" or _type=="loadable_module"', {
                          # The Xcode default is to strip debugging symbols
                          # only (-S).  Local symbols should be stripped as
                          # well, which will be handled by -x.  Xcode will
                          # continue to insert -S when stripping even when
                          # additional flags are added with STRIPFLAGS.
                          'STRIPFLAGS': '-x',
                        }],  # _type=="shared_library" or _type=="loadable_module"'
                      ],  # target_conditions
                    },  # xcode_settings
                  },  # configuration "Release"
                },  # configurations
              }, {  # mac_real_dsym != 1
                # To get a fast fake .dSYM bundle, use a post-build step to
                # produce the .dSYM and strip the executable.  strip_from_xcode
                # only operates in the Release configuration.
                'postbuilds': [
                  {
                    'variables': {
                      # Define strip_from_xcode in a variable ending in _path
                      # so that gyp understands it's a path and performs proper
                      # relativization during dict merging.
                      'strip_from_xcode_path': 'mac/strip_from_xcode',
                    },
                    'postbuild_name': 'Strip If Needed',
                    'action': ['<(strip_from_xcode_path)'],
                  },
                ],  # postbuilds
              }],  # mac_real_dsym
            ],  # target_conditions
          }],  # (_type=="executable" or _type=="shared_library" or
               #  _type=="loadable_module") and mac_strip!=0
        ],  # target_conditions
      },  # target_defaults
    }],  # OS=="mac"
    ['OS=="ios"', {
      'target_defaults': {
        'xcode_settings' : {
          'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',

          # This next block is mostly common with the 'mac' section above,
          # but keying off (or setting) 'clang' isn't valid for iOS as it
          # also seems to mean using the custom build of clang.

          # Don't use -Wc++0x-extensions, which Xcode 4 enables by default
          # when building with clang. This warning is triggered when the
          # override keyword is used via the OVERRIDE macro from
          # base/compiler_specific.h.
          'CLANG_WARN_CXX0X_EXTENSIONS': 'NO',
          # Warn if automatic synthesis is triggered with
          # the -Wobjc-missing-property-synthesis flag.
          'CLANG_WARN_OBJC_MISSING_PROPERTY_SYNTHESIS': 'YES',
          'WARNING_CFLAGS': [
            '-Wheader-hygiene',
            # Don't die on dtoa code that uses a char as an array index.
            # This is required solely for base/third_party/dmg_fp/dtoa.cc.
            '-Wno-char-subscripts',
            # Clang spots more unused functions.
            '-Wno-unused-function',
            # See comments on this flag higher up in this file.
            '-Wno-unnamed-type-template-args',
            # This (rightfully) complains about 'override', which we use
            # heavily.
            '-Wno-c++11-extensions',
          ],
        },
        'target_conditions': [
          ['_type=="executable"', {
            'configurations': {
              'Release_Base': {
                'xcode_settings': {
                  'DEPLOYMENT_POSTPROCESSING': 'YES',
                  'STRIP_INSTALLED_PRODUCT': 'YES',
                },
              },
              'Debug_Base': {
                'xcode_settings': {
                  # Remove dSYM to reduce build time.
                  'DEBUG_INFORMATION_FORMAT': 'dwarf',
                },
              },
            },
            'xcode_settings': {
              'conditions': [
                ['chromium_ios_signing', {
                  # iOS SDK wants everything for device signed.
                  'CODE_SIGN_IDENTITY[sdk=iphoneos*]': 'iPhone Developer',
                }, {
                  'CODE_SIGNING_REQUIRED': 'NO',
                  'CODE_SIGN_IDENTITY[sdk=iphoneos*]': '',
                }],
              ],
            },
          }],
        ],  # target_conditions
      },  # target_defaults
    }],  # OS=="ios"
    ['OS=="win"', {
      'target_defaults': {
        'defines': [
          '_WIN32_WINNT=0x0602',
          'WINVER=0x0602',
          'WIN32',
          '_WINDOWS',
          'NOMINMAX',
          'PSAPI_VERSION=1',
          '_CRT_RAND_S',
          'CERT_CHAIN_PARA_HAS_EXTRA_FIELDS',
          'WIN32_LEAN_AND_MEAN',
          '_ATL_NO_OPENGL',
        ],
        'defines': [
        ],
        'msvs_system_include_dirs': [
          # '<(windows_sdk_path)/Include/shared',
          # '<(windows_sdk_path)/Include/um',
          # '<(windows_sdk_path)/Include/winrt',
          # '$(VSInstallDir)/VC/atlmfc/include',
        ],
        'msvs_cygwin_dirs': ['<(DEPTH)/third_party/cygwin'],
        'msvs_disabled_warnings': [
          # TODO(tommi): Add as needed.
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': ['/MP'],
            'MinimalRebuild': 'false',
            'BufferSecurityCheck': 'true',
            'EnableFunctionLevelLinking': 'true',
            'RuntimeTypeInfo': 'false',
            'WarningLevel': '4',
            'WarnAsError': 'true',
            'DebugInformationFormat': '3',
          },
          'VCLibrarianTool': {
            'AdditionalOptions': ['/ignore:4221'],
            'AdditionalLibraryDirectories': [
              # '<(windows_sdk_path)/Lib/win8/um/x86',
            ],
          },
          'VCLinkerTool': {
            'AdditionalDependencies': [
            ],

            'conditions': [
            ],
            'AdditionalLibraryDirectories': [
              # '<(windows_sdk_path)/Lib/win8/um/x86',
            ],
            'GenerateDebugInformation': 'true',
            'MapFileName': '$(OutDir)\\$(TargetName).map',
            'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
            'FixedBaseAddress': '1',
            # SubSystem values:
            #   0 == not set
            #   1 == /SUBSYSTEM:CONSOLE
            #   2 == /SUBSYSTEM:WINDOWS
            # Most of the executables we'll ever create are tests
            # and utilities with console output.
            'SubSystem': '1',
          },
          'VCMIDLTool': {
            'GenerateStublessProxies': 'true',
            'TypeLibraryName': '$(InputName).tlb',
            'OutputDirectory': '$(IntDir)',
            'HeaderFileName': '$(InputName).h',
            'DLLDataFileName': '$(InputName).dlldata.c',
            'InterfaceIdentifierFileName': '$(InputName)_i.c',
            'ProxyFileName': '$(InputName)_p.c',
          },
          'VCResourceCompilerTool': {
            'Culture' : '1033',
            'AdditionalIncludeDirectories': [
              '<(DEPTH)',
              '<(SHARED_INTERMEDIATE_DIR)',
            ],
          },
        },
      },
    }],
    ['OS=="win" and msvs_use_common_linker_extras', {
      'target_defaults': {
        'msvs_settings': {
          'VCLinkerTool': {
            'DelayLoadDLLs': [
            ],
          },
        },
        'configurations': {
          'x86_Base': {
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalOptions': [
                  '/safeseh',
                  '/dynamicbase',
                  '/ignore:4199',
                  '/ignore:4221',
                  '/nxcompat',
                ],
              },
            },
          },
          'x64_Base': {
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalOptions': [
                  # safeseh is not compatible with x64
                  '/dynamicbase',
                  '/ignore:4199',
                  '/ignore:4221',
                  '/nxcompat',
                ],
              },
            },
          },
        },
      },
    }],
    ['OS=="android"', {
      # Hardcode the compiler names in the Makefile so that
      # it won't depend on the environment at make time.
      'make_global_settings': [
        ['CC', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <(android_toolchain)/*-gcc)'],
        ['CXX', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <(android_toolchain)/*-g++)'],
        ['LINK', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <(android_toolchain)/*-gcc)'],
        ['CC.host', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <!(which gcc))'],
        ['CXX.host', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <!(which g++))'],
        ['LINK.host', '<!(/bin/echo -n ${ANDROID_GOMA_WRAPPER} <!(which g++))'],
      ],
    }],
  ],
  'xcode_settings': {
    # DON'T ADD ANYTHING NEW TO THIS BLOCK UNLESS YOU REALLY REALLY NEED IT!
    # This block adds *project-wide* configuration settings to each project
    # file.  It's almost always wrong to put things here.  Specify your
    # custom xcode_settings in target_defaults to add them to targets instead.

    'conditions': [
      # In an Xcode Project Info window, the "Base SDK for All Configurations"
      # setting sets the SDK on a project-wide basis. In order to get the
      # configured SDK to show properly in the Xcode UI, SDKROOT must be set
      # here at the project level.
      ['OS=="mac"', {
        'conditions': [
          ['mac_sdk_path==""', {
            'SDKROOT': 'macosx<(mac_sdk)',  # -isysroot
          }, {
            'SDKROOT': '<(mac_sdk_path)',  # -isysroot
          }],
        ],
      }],
      ['OS=="ios"', {
        'conditions': [
          ['ios_sdk_path==""', {
            'SDKROOT': 'iphoneos<(ios_sdk)',  # -isysroot
          }, {
            'SDKROOT': '<(ios_sdk_path)',  # -isysroot
          }],
        ],
      }],
      ['OS=="ios"', {
        'ARCHS': '$(ARCHS_UNIVERSAL_IPHONE_OS)',
        # Just build armv7, until armv7s is correctly tested.
        'VALID_ARCHS': 'armv7 i386',
        'IPHONEOS_DEPLOYMENT_TARGET': '<(ios_deployment_target)',
        # Target both iPhone and iPad.
        'TARGETED_DEVICE_FAMILY': '1,2',
      }],
    ],

    # The Xcode generator will look for an xcode_settings section at the root
    # of each dict and use it to apply settings on a file-wide basis.  Most
    # settings should not be here, they should be in target-specific
    # xcode_settings sections, or better yet, should use non-Xcode-specific
    # settings in target dicts.  SYMROOT is a special case, because many other
    # Xcode variables depend on it, including variables such as
    # PROJECT_DERIVED_FILE_DIR.  When a source group corresponding to something
    # like PROJECT_DERIVED_FILE_DIR is added to a project, in order for the
    # files to appear (when present) in the UI as actual files and not red
    # red "missing file" proxies, the correct path to PROJECT_DERIVED_FILE_DIR,
    # and therefore SYMROOT, needs to be set at the project level.
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
