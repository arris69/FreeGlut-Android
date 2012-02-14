# CMake toolchain file, cf. README.mingw_cross
SET(CMAKE_SYSTEM_NAME Windows)
include(CMakeForceCompiler)
IF(NOT "${GNU_HOST}")
	SET(GNU_HOST i586-mingw32msvc)
ENDIF()
# Prefix detection only works with compiler id "GNU"
CMAKE_FORCE_C_COMPILER(${GNU_HOST}-gcc GNU)
SET(CMAKE_RC_COMPILER ${GNU_HOST}-windres)
