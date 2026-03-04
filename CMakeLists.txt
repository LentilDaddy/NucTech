# $Id: CMakeLists.txt 86065 2014-11-07 08:51:15Z gcosmo $

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.8 FATAL_ERROR) 
project(NucTech)

#----------------------------------------------------------------------------
# Find Geant4 package
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Find ROOT package (REQUIRED for the energy spectrum sampling)
#----------------------------------------------------------------------------
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Setup Geant4 and ROOT include directories
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable
add_executable (nucTech nucTech.cc ${sources} ${headers})

#----------------------------------------------------------------------------
# Link the executable to Geant4 AND ROOT libraries
# This resolves the "undefined reference" errors for TFile and TH1D
#----------------------------------------------------------------------------
target_link_libraries(nucTech ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Integrate clang-tidy for static code analysis
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
if(CLANG_TIDY_EXECUTABLE)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
endif()

#----------------------------------------------------------------------------
# Enable warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(nucTech PRIVATE -Wall -Wextra -pedantic -Wconversion -Wshadow -Wnon-virtual-dtor -Wunused -Woverloaded-virtual -Wold-style-cast)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(nucTech PRIVATE -Wall -Wextra -pedantic -Wconversion -Wshadow -Wnon-virtual-dtor -Wunused -Woverloaded-virtual -Wold-style-cast -Wdouble-promotion -Wnull-dereference -Wdocumentation)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(nucTech PRIVATE /Wall /WX /permissive-)
endif()

#----------------------------------------------------------------------------
# Copy scripts to build directory
set(nucTech_SCRIPTS
  quiet.mac
  vis.mac
  run.mac
  beam.mac
  ionBeam.mac
  radioactiveSrc.mac
  processSim.C
  )

foreach(_script ${nucTech_SCRIPTS})
    configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

#----------------------------------------------------------------------------
# Install and targets
add_custom_target(nucTech_exec DEPENDS nucTech)
install(TARGETS nucTech DESTINATION bin)