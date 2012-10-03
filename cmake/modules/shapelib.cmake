# cmake/modules/shapefil.cmake
#
# Copyright (C) 2012  Andrew Ross
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#
# Configuration for SHAPELIB support in plplot.

option(
HAVE_SHAPELIB
"Enable driver options for using shapelib library for reading shapefile map data"
ON
)

# Look for shapelib libraries
if (HAVE_SHAPELIB)
  find_package(Shapelib)
  if(NOT SHAPELIB_FOUND)
    message(STATUS 
    "WARNING: SHAPELIB not found.  Setting HAVE_SHAPELIB to OFF."
    )
    set(HAVE_SHAPELIB OFF
    CACHE BOOL "Enable driver options for using SHAPELIB library for reading shapefile map data"
    FORCE
    )
  endif(NOT SHAPELIB_FOUND)
endif (HAVE_SHAPELIB)
