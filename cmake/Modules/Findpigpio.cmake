################################################################################
### Find the pigpio includes and shared libraries.
################################################################################

# Find the path to the pigpio includes.
find_path(pigpio_INCLUDE_DIR 
	NAMES pigpio.h pigpiod_if.h pigpiod_if2.h
	HINTS /usr/local/include)
	
# Find the path to the pigpio libraries.
find_library(pigpio_LIBRARY 
	NAMES libpigpio.so libpigpiod_if.so libpigpiod_if2.so
	HINTS /usr/local/lib)
    
# Set the pigpio variables to plural form to make them accessible for 
# the paramount cmake modules.
set(pigpio_INCLUDE_DIRS ${pigpio_INCLUDE_DIR})
set(pigpio_INCLUDES     ${pigpio_INCLUDE_DIR})
set(pigpio_LIBRARIES    ${pigpio_LIBRARY})

# Handle REQUIRED, QUIET, and version arguments 
# and set the <packagename>_FOUND variable.
find_package_handle_standard_args(pigpio 
    DEFAULT_MSG 
    pigpio_INCLUDE_DIR pigpio_LIBRARY)
