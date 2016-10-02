# Collection of useful functions that are required by PLplot.

function(list_example_files path device suffix output_list)
  # Return list of files (with ${path}/ prepended to form the full path
  # name for each file) that are generated by plplot-test.sh for a
  # particular device and file suffix corresponding to front-end
  # language.  This list will be used for OUTPUT files of a custom
  # command so that these files will be properly deleted by "make
  # clean".  Thus, it doesn't matter if we miss a few examples or
  # pages that are only implemented for one of the languages.
  # However, if we specify a file that is not generated by
  # plplot-test.sh for the specified device and language, then that
  # custom command is never satisfied and will continue to regenerate
  # the files.  Therefore only specify examples and pages that you
  # _know_ are generated by all language bindings.
  # MAINTENANCE
  set(examples_pages_LIST
    x00:01
    x01:01
    x02:02
    x03:01
    x04:02
    x05:01
    x06:05
    x07:20
    # x08:10 when Ada and OCaml are propagated.
    x08:08
    x09:05
    x10:01
    x11:08
    x12:01
    x13:01
    x14:02
    x14a:02
    x15:03
    x16:05
    x17:01
    x18:08
    # x19:05 when Ada and OCaml are propagated.
    x19:04
    x20:06
    x21:03
    x22:04
    x23:16
    x24:01
    x25:08
    x26:02
    x27:20
    x28:05
    x29:10
    x30:02
    x31:01
    # 32 missing deliberately since that only implemented for C
    x33:04
    )

  # This list taken directly from plplot-test.sh.cmake.  Update as needed.
  # MAINTENANCE
  if(
      ${device} STREQUAL "png" OR
      ${device} STREQUAL "pngcairo" OR
      ${device} STREQUAL "epscairo" OR
      ${device} STREQUAL "jpeg" OR
      ${device} STREQUAL "xfig" OR
      ${device} STREQUAL "svg" OR
      ${device} STREQUAL "svgcairo" OR
      ${device} STREQUAL "bmpqt" OR
      ${device} STREQUAL "jpgqt" OR
      ${device} STREQUAL "pngqt" OR
      ${device} STREQUAL "ppmqt" OR
      ${device} STREQUAL "tiffqt" OR
      ${device} STREQUAL "svgqt" OR
      ${device} STREQUAL "epsqt" OR
      ${device} STREQUAL "pdfqt" OR
      ${device} STREQUAL "gif"
      )
    set(familying ON)
  else(
      ${device} STREQUAL "png" OR
      ${device} STREQUAL "pngcairo" OR
      ${device} STREQUAL "epscairo" OR
      ${device} STREQUAL "jpeg" OR
      ${device} STREQUAL "xfig" OR
      ${device} STREQUAL "svg" OR
      ${device} STREQUAL "svgcairo" OR
      ${device} STREQUAL "bmpqt" OR
      ${device} STREQUAL "jpgqt" OR
      ${device} STREQUAL "pngqt" OR
      ${device} STREQUAL "ppmqt" OR
      ${device} STREQUAL "tiffqt" OR
      ${device} STREQUAL "svgqt" OR
      ${device} STREQUAL "epsqt" OR
      ${device} STREQUAL "pdfqt" OR
      ${device} STREQUAL "gif"
      )
    set(familying OFF)
  endif(
      ${device} STREQUAL "png" OR
      ${device} STREQUAL "pngcairo" OR
      ${device} STREQUAL "epscairo" OR
      ${device} STREQUAL "jpeg" OR
      ${device} STREQUAL "xfig" OR
      ${device} STREQUAL "svg" OR
      ${device} STREQUAL "svgcairo" OR
      ${device} STREQUAL "bmpqt" OR
      ${device} STREQUAL "jpgqt" OR
      ${device} STREQUAL "pngqt" OR
      ${device} STREQUAL "ppmqt" OR
      ${device} STREQUAL "tiffqt" OR
      ${device} STREQUAL "svgqt" OR
      ${device} STREQUAL "epsqt" OR
      ${device} STREQUAL "pdfqt" OR
      ${device} STREQUAL "gif"
      )
  set(file_list)

  foreach(example_pages ${examples_pages_LIST})
    string(REGEX REPLACE "^(.*):.*$" "\\1" example ${example_pages})
    string(REGEX REPLACE "^.*:(.*)$" "\\1" pages ${example_pages})
    if(${suffix} STREQUAL "a")
      string(REGEX REPLACE "^x" "xtraditional" traditional_example ${example})
      string(REGEX REPLACE "^x" "xstandard" example ${example})
    else(${suffix} STREQUAL "a")
      set(traditional_example)
    endif(${suffix} STREQUAL "a")
    if(familying)
      foreach(famnum RANGE 1 ${pages})
	if(famnum LESS 10)
	  set(famnum 0${famnum})
	endif(famnum LESS 10)
	list(APPEND file_list ${path}/${example}${suffix}${famnum}.${device})
	if(traditional_example)
	  list(APPEND file_list ${path}/${traditional_example}${suffix}${famnum}.${device})
	endif(traditional_example)
      endforeach(famnum RANGE 1 ${pages})
    else(familying)
      list(APPEND file_list ${path}/${example}${suffix}.${device})
      if(traditional_example)
	list(APPEND file_list ${path}/${traditional_example}${suffix}.${device})
      endif(traditional_example)
    endif(familying)
    if(NOT ${example} MATCHES "x.*14a")
      list(APPEND file_list ${path}/${example}${suffix}_${device}.txt)
      if(traditional_example)
	list(APPEND file_list ${path}/${traditional_example}${suffix}_${device}.txt)
      endif(traditional_example)
    endif(NOT ${example} MATCHES "x.*14a")
  endforeach(example_pages ${examples_pages_LIST})

  set(${output_list} ${file_list} PARENT_SCOPE)
endfunction(list_example_files)

# CMake-2.6.x duplicates this list so work around that bug by removing
# those duplicates.
if(CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES)
  list(REMOVE_DUPLICATES CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES)
endif(CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES)

# Filter all CMAKE_<LANG>_IMPLICIT_LINK_DIRECTORIES list elements from
# rpath_in list.  Note, this uses variables that are only defined after
# languages have been enabled but according to the documentation the
# logic is only invoked when the function is invoked so this should be
# OK _if care is used that this function is invoked only after the
# languages have been enabled_.  C is enabled immediately so that will
# serve most purposes, but CXX and Fortran are enabled later so if
# you want those special system locations removed (unlikely but
# possible) then you are going to have to be somewhat more careful
# when this function is invoked.

function(filter_rpath rpath)
  #message("DEBUG: ${rpath} = ${${rpath}}")
  set(internal_rpath ${${rpath}})
  if(internal_rpath)
    # Remove duplicates and empty elements.
    list(REMOVE_DUPLICATES internal_rpath)
    list(REMOVE_ITEM internal_rpath "")
    set(directories_to_be_removed
      ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}
      ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}
      ${CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES}
      )

    if(directories_to_be_removed)
      list(REMOVE_DUPLICATES directories_to_be_removed)
    endif(directories_to_be_removed)

    if(directories_to_be_removed)
      list(REMOVE_ITEM internal_rpath ${directories_to_be_removed})
    endif(directories_to_be_removed)
  endif(internal_rpath)
  #message("DEBUG: (filtered) ${rpath} = ${internal_rpath}")
  set(${rpath} ${internal_rpath} PARENT_SCOPE)
endfunction(filter_rpath)

if(MINGW)
  # Useful function to convert Windows list of semicolon-delimited
  # PATHs to the equivalent list of MSYS PATHs (exactly like the
  # colon-delimited Unix list of PATHs except the driver letters are
  # specified as the initial one-character component of each of the
  # PATHs).  For example, this function will transform the Windows
  # list of PATHs, "z:\path1;c:\path2" to "/z/path1:/c/path2".
  function(determine_msys_path MSYS_PATH NATIVE_PATH)
    #message(STATUS "NATIVE_PATH = ${NATIVE_PATH}")
    string(REGEX REPLACE "^\([a-zA-z]\):" "/\\1" PATH  "${NATIVE_PATH}")
    string(REGEX REPLACE ";\([a-zA-z]\):" ";/\\1" PATH  "${PATH}")
    string(REGEX REPLACE ";" ":" PATH  "${PATH}")
    file(TO_CMAKE_PATH "${PATH}" PATH)
    #message(STATUS "MSYS_PATH = ${PATH}")
    set(${MSYS_PATH} ${PATH} PARENT_SCOPE)
  endfunction(determine_msys_path)
endif(MINGW)

if(${CMAKE_VERSION} VERSION_LESS 3.2 AND (CYGWIN OR CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  # N.B. file(GENERATE ...) has a bug (fixed in CMake 3.2)
  # where permissions were not copied from INPUT to OUTPUT.
  # So for CMake version less than 3.2 implement a permissions fixup
  # at run-time using the --reference option for chmod which is a GNU
  # extension for chmod only available on Cygwin and Linux.
  # Thus, for all other platforms the minimum CMake version must be 3.2, see
  # top-level CMakeLists.txt file.
  # Establish GLOBAL property which will be appended to each time
  # the configure_file_generate function is called below.
  set(WORKAROUND_FILE_GENERATE_BUG ON)
  set_property(GLOBAL PROPERTY FILES_WORKAROUND_FILE_GENERATE_BUG)
endif(${CMAKE_VERSION} VERSION_LESS 3.2 AND (CYGWIN OR CMAKE_SYSTEM_NAME STREQUAL "Linux"))

# Must set this variable globally because also used outside configure_file_generate
# when WORKAROUND_FILE_GENERATE_BUG is ON.
set(configure_file_generate_suffix "_cf_only")
function(configure_file_generate)
  # Configure files that contain both normal items
  # to configure (referenced as ${VAR} or @VAR@) as well as
  # generator expressions (referenced as $<...>).
  # The arguments of this function have exactly the
  # same syntax and meaning as configure_file.

  list(GET ARGV 0 input_file)
  list(GET ARGV 1 output_file)
  set(intermediate_file ${output_file}${configure_file_generate_suffix})

  # Locally modify ARGV so that output file for configure file is
  # redirected to intermediate_file.
  list(REMOVE_AT ARGV 1)
  list(INSERT ARGV 1 ${intermediate_file})

  # Configure all but generator expressions.
  configure_file(${ARGV})

  # Configure generator expressions.
  # N.B. these ${output_file} results will only be available
  # at generate time.
  file(GENERATE
    OUTPUT ${output_file}
    INPUT ${intermediate_file}
    )
  if(WORKAROUND_FILE_GENERATE_BUG)
    # Collect all filenames whose permissions must be updated
    # from corresponding file name with ${configure_file_generate_suffix}
    # appended.
    set_property(GLOBAL APPEND
      PROPERTY FILES_WORKAROUND_FILE_GENERATE_BUG
      ${output_file}
      )
  endif(WORKAROUND_FILE_GENERATE_BUG)

endfunction(configure_file_generate)

function(set_library_properties library)
  # Set properties that are normally required by PLplot
  # for a library.

  # Sanity check that needed variables have been assigned values.
  # (Note that ${library}_SOVERSION and ${library}_VERSION should be
  # assigned values in cmake/modules/plplot_version.cmake and LIB_DIR
  # should be assigned a value in cmake/modules/instdirs.cmake.)
  if(NOT(${library}_SOVERSION OR ${library}_VERSION OR LIB_DIR))
    message(STATUS "${library}_SOVERSION = ${${library}_SOVERSION}")
    message(STATUS "${library}_VERSION = ${${library}_VERSION}")
    message(STATUS "LIB_DIR = ${LIB_DIR}")
    message(FATAL_ERROR "${library}_SOVERSION, ${library}_VERSION, and/or LIB_DIR are not properly defined")
  endif(NOT(${library}_SOVERSION OR ${library}_VERSION OR LIB_DIR))

  # The INSTALL_RPATH property is only set if BOTH USE_RPATH is true
  # and LIB_INSTALL_RPATH is defined.
  if(USE_RPATH AND LIB_INSTALL_RPATH)
    filter_rpath(LIB_INSTALL_RPATH)
    set_target_properties(
      ${library}
      PROPERTIES
      INSTALL_RPATH "${LIB_INSTALL_RPATH}"
      )
  else(USE_RPATH AND LIB_INSTALL_RPATH)
    set_target_properties(
      ${library}
      PROPERTIES
      INSTALL_NAME_DIR "${LIB_DIR}"
      )
  endif(USE_RPATH AND LIB_INSTALL_RPATH)

  set_target_properties(
    ${library}
    PROPERTIES
    SOVERSION ${${library}_SOVERSION}
    VERSION ${${library}_VERSION}
    # This allows PLplot static library builds to
    # be linked by shared libraries.
    POSITION_INDEPENDENT_CODE ON
    )

endfunction(set_library_properties library)
