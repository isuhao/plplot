# For now, must set this by hand before each release.  This value affects
# at least the documentation build and perhaps other parts of the release.
set(RELEASE_DATE "2013-09-30")

# Version data that need review and possible modification for each release.

# Overall PLplot version number.
set(PLPLOT_VERSION 5.9.10)

# CPack version numbers for release tarball name.
set(CPACK_PACKAGE_VERSION_MAJOR 5)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 10)

# PLplot library version information.

# Rules: 
# (1) If a backwards incompatible API change has been made in the library
#     API (e.g., if old compiled and linked applications will no longer work)
#     then increment SOVERSION and zero the corresponding minor and patch
#     numbers just before release.
# (2) If the library changes are limited to additions to the API, then
#     then leave SOVERSION alone, increment the minor number and zero the
#     patch number just before release.
# (3) If the library changes are limited to implementation changes with 
#     no API changes at all, then leave SOVERSION and minor number alone, and
#     increment the patch number just before the release.
# (4) If there are no library source code changes at all, then leave all
#     library version numbers the same for the release.

set(plplotqt_SOVERSION 1)
set(plplotqt_VERSION ${plplotqt_SOVERSION}.0.0)

set(nistcd_SOVERSION 0)
set(nistcd_VERSION ${nistcd_SOVERSION}.0.1)

set(csirocsa_SOVERSION 0)
set(csirocsa_VERSION ${csirocsa_SOVERSION}.0.1)

set(csironn_SOVERSION 0)
set(csironn_VERSION ${csironn_SOVERSION}.0.1)

set(qsastime_SOVERSION 0)
set(qsastime_VERSION ${qsastime_SOVERSION}.0.1)

set(plplot_SOVERSION 12)
set(plplot_VERSION ${plplot_SOVERSION}.0.0)

set(plplotcxx_SOVERSION 11)
set(plplotcxx_VERSION ${plplotcxx_SOVERSION}.0.0)

set(plplotdmd_SOVERSION 1)
set(plplotdmd_VERSION ${plplotdmd_SOVERSION}.0.0)

set(plplotf95_SOVERSION 10)
set(plplotf95_VERSION ${plplotf95_SOVERSION}.0.0)

set(plplotgnome2_SOVERSION 0)
set(plplotgnome2_VERSION ${plplotgnome2_SOVERSION}.0.0)

set(tclmatrix_SOVERSION 9)
set(tclmatrix_VERSION ${tclmatrix_SOVERSION}.2.0)

set(plplottcltk_SOVERSION 10)
set(plplottcltk_VERSION ${plplottcltk_SOVERSION}.0.0)

set(plplottcltk_Main_SOVERSION 0)
set(plplottcltk_Main_VERSION ${plplottcltk_Main_SOVERSION}.0.0)

set(plplotwxwidgets_SOVERSION 0)
set(plplotwxwidgets_VERSION ${plplotwxwidgets_SOVERSION}.0.0)

set(plplotada_SOVERSION 1)
set(plplotada_VERSION ${plplotada_SOVERSION}.0.0)
