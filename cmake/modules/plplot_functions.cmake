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
  set(examples_pages_LIST
    x01:01
    x02:02
    x03:01
    x04:02
    x05:01
    x06:05
    x07:20
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
      string(REGEX REPLACE "^x" "xthick" thick_example ${example})
    else(${suffix} STREQUAL "a")
      set(thick_example)
    endif(${suffix} STREQUAL "a")
    if(familying)
      foreach(famnum RANGE 1 ${pages})
	if(famnum LESS 10)
	  set(famnum 0${famnum})
	endif(famnum LESS 10)
	list(APPEND file_list ${path}/${example}${suffix}${famnum}.${device})
	if(thick_example)
	  list(APPEND file_list ${path}/${thick_example}${suffix}${famnum}.${device})
	endif(thick_example)
      endforeach(famnum RANGE 1 ${pages})
    else(familying)
      list(APPEND file_list ${path}/${example}${suffix}.${device})
      if(thick_example)
	list(APPEND file_list ${path}/${thick_example}${suffix}.${device})
      endif(thick_example)
    endif(familying)
    if(NOT ${example} STREQUAL "x14a")
      list(APPEND file_list ${path}/${example}${suffix}_${device}.txt)
      if(thick_example)
	list(APPEND file_list ${path}/${thick_example}${suffix}_${device}.txt)
      endif(thick_example)
    endif(NOT ${example} STREQUAL "x14a")
  endforeach(example_pages ${examples_pages_LIST})

  set(${output_list} ${file_list} PARENT_SCOPE)
endfunction(list_example_files )

function(TRANSFORM_VERSION numerical_result version)
  # internal_version ignores everything in version after any character that
  # is not 0-9 or ".".  This should take care of the case when there is
  # some non-numerical data in the patch version.
  #message(STATUS "DEBUG: version = ${version}")
  string(REGEX REPLACE "^([0-9.]+).*$" "\\1" internal_version ${version})

  # internal_version is normally a period-delimited triplet string of the form
  # "major.minor.patch", but patch and/or minor could be missing.
  # Transform internal_version into a numerical result that can be compared.
  string(REGEX REPLACE "^([0-9]*).+$" "\\1" major ${internal_version})
  string(REGEX REPLACE "^[0-9]*\\.([0-9]*).*$" "\\1" minor ${internal_version})
  string(REGEX REPLACE "^[0-9]*\\.[0-9]*\\.([0-9]*)$" "\\1" patch ${internal_version})

  if(NOT patch MATCHES "[0-9]+")
    set(patch 0)
  endif(NOT patch MATCHES "[0-9]+")
  
  if(NOT minor MATCHES "[0-9]+")
    set(minor 0)
  endif(NOT minor MATCHES "[0-9]+")
  
  if(NOT major MATCHES "[0-9]+")
    set(major 0)
  endif(NOT major MATCHES "[0-9]+")
  #message(STATUS "DEBUG: internal_version = ${internal_version}")
  #message(STATUS "DEBUG: major = ${major}")
  #message(STATUS "DEBUG: minor= ${minor}")
  #message(STATUS "DEBUG: patch = ${patch}")
  math(EXPR internal_numerical_result
    "${major}*1000000 + ${minor}*1000 + ${patch}"
    )
  #message(STATUS "DEBUG: ${numerical_result} = ${internal_numerical_result}")
  set(${numerical_result} ${internal_numerical_result} PARENT_SCOPE)
endfunction(TRANSFORM_VERSION)

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
