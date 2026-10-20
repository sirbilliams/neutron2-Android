cmake_minimum_required(VERSION 3.16)
project(omt_port CXX C)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_EXTENSIONS ON)

# --------------------------------------------------------------------------- sources
file(GLOB_RECURSE OMT_SRC OMTClasses/*.cpp)
list(FILTER OMT_SRC EXCLUDE REGEX "OMTClasses/OS/(MSWindows|MacOS)/")
file(GLOB SDL_SRC sdl/*.cpp)
list(FILTER SDL_SRC EXCLUDE REGEX "OMediaSDLMain\\.cpp$")      # compiled into each app, see below
set(OMT_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/sdl/OMediaSDLMain.cpp)
file(GLOB_RECURSE OMT_HDR_DIRS LIST_DIRECTORIES true OMTClasses/*)
set(OMT_INC "")
foreach(d ${OMT_HDR_DIRS})
  if(IS_DIRECTORY ${d} AND NOT d MATCHES "OS/(MSWindows|MacOS)")
    list(APPEND OMT_INC ${d})
  endif()
endforeach()

# --------------------------------------------------------------------------- options
option(OMT_GL4ES "Render through gl4es on an OpenGL ES 2 context (always on for Android)" OFF)
set(GL4ES_DIR "" CACHE PATH "gl4es source dir (for include/)")
set(GL4ES_LIB "" CACHE FILEPATH "gl4es static lib, if not built as the 'GL' target")
if(ANDROID)
  set(OMT_GL4ES ON)
endif()

# --------------------------------------------------------------------------- SDL2
# An existing target (Android: SDL added as a subproject first) or an installed package.
if(NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2 AND NOT TARGET SDL2::SDL2-static)
  find_package(SDL2 REQUIRED)
endif()
if(TARGET SDL2::SDL2)
  set(OMT_SDL SDL2::SDL2)
elseif(TARGET SDL2)
  set(OMT_SDL SDL2)
elseif(TARGET SDL2::SDL2-static)
  set(OMT_SDL SDL2::SDL2-static)
else()
  set(OMT_SDL ${SDL2_LIBRARIES})
endif()

# --------------------------------------------------------------------------- GL
find_package(ZLIB REQUIRED)
if(OMT_GL4ES)
  if(TARGET GL)
    set(OMT_GL_LIBS GL dl m)              # gl4es added as a subproject
  else()
    set(OMT_GL_LIBS ${GL4ES_LIB} dl m)
  endif()
  set(OMT_GL_INC ${GL4ES_DIR}/include)
else()
  find_package(OpenGL REQUIRED)
  set(OMT_GL_LIBS OpenGL::GL)
  set(OMT_GL_INC "")
endif()

# --------------------------------------------------------------------------- engine
add_library(omt STATIC ${OMT_SRC} ${SDL_SRC})
target_include_directories(omt PUBLIC ${OMT_INC} sdl ${SDL2_INCLUDE_DIRS} ${OMT_GL_INC})
target_compile_options(omt PRIVATE -fpermissive -Wno-deprecated -Wno-write-strings -Werror=int-to-pointer-cast -fno-strict-aliasing)
target_link_libraries(omt PUBLIC ${OMT_SDL} ${OMT_GL_LIBS} ZLIB::ZLIB)
if(OMT_GL4ES)
  target_compile_definitions(omt PUBLIC OMT_USE_GL4ES)
endif()
if(ANDROID)
  target_link_libraries(omt PUBLIC android log)
endif()

# --------------------------------------------------------------------------- game importer
# Installs the game files from the game's CD image (.iso): ISO 9660 + InstallShield cabinet
# (unshield, MIT, fetched by fetch-deps.sh). Android: libimporter.so for the setup screen.
set(UNSHIELD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../android/app/jni/unshield" CACHE PATH "unshield source dir")
if(EXISTS ${UNSHIELD_DIR}/lib/libunshield.h)
  file(GLOB US_SRC ${UNSHIELD_DIR}/lib/*.c)
  add_library(negimport STATIC importer/NegImport.c ${US_SRC}
              ${UNSHIELD_DIR}/lib/convert_utf/ConvertUTF.c ${UNSHIELD_DIR}/lib/md5/md5c.c)
  target_include_directories(negimport PUBLIC importer PRIVATE importer/config ${UNSHIELD_DIR}/lib ${UNSHIELD_DIR})
  target_compile_options(negimport PRIVATE -w)
  set_target_properties(negimport PROPERTIES POSITION_INDEPENDENT_CODE ON C_STANDARD 99)
  target_link_libraries(negimport PUBLIC ZLIB::ZLIB)
  if(ANDROID)
    add_library(importer SHARED importer/NegImportJNI.c)
    target_link_libraries(importer negimport log)
  else()
    add_executable(neg_import importer/neg_import_main.c)
    target_link_libraries(neg_import negimport)
  endif()
endif()

# --------------------------------------------------------------------------- apps
if(ANDROID)
  # SDLActivity loads libmain.so and calls SDL_main (OMediaSDLMain.cpp's main).
  add_library(main SHARED viewer/OMTViewer.cpp ${OMT_MAIN})
  target_link_libraries(main omt)
  target_compile_options(main PRIVATE -w -fpermissive)
else()
  add_executable(omt_viewer viewer/OMTViewer.cpp ${OMT_MAIN})
  target_link_libraries(omt_viewer omt)
  target_compile_options(omt_viewer PRIVATE -w -fpermissive)

  add_executable(omt_datatest tests/omt_datatest.cpp ${OMT_MAIN})
  target_link_libraries(omt_datatest omt)
  target_compile_options(omt_datatest PRIVATE -w -fpermissive)

  # The reconstructed game (../game/src), desktop build
  file(GLOB_RECURSE NEG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../game/src/*.cpp)
  if(NEG_SRC)
    add_executable(negatron ${NEG_SRC} ${OMT_MAIN})
    target_link_libraries(negatron omt)
    target_compile_options(negatron PRIVATE -Wno-multichar -Wno-write-strings -Werror=narrowing -Wfloat-conversion -fno-strict-aliasing)
  endif()

  # OMT's own samples, built unmodified (after tools/portify.py) against the SDL layer
  foreach(s Cube Teapot)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples/${s}.cpp)
      add_executable(sample_${s} samples/${s}.cpp ${OMT_MAIN})
      target_link_libraries(sample_${s} omt)
      target_compile_options(sample_${s} PRIVATE -w -fpermissive)
    endif()
  endforeach()
endif()
