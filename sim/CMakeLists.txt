cmake_minimum_required(VERSION 3.10)
project(lvgl C CXX)

# Define options for LVGL with default values (OFF)
option(LV_USE_DRAW_SDL "Use SDL draw unit" ON)
option(LV_USE_LIBPNG "Use libpng to decode PNG" ON)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype library" OFF)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Find and include SDL2 library
find_package(SDL2 REQUIRED)

# Remove ARM-specific compile and linker options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

# Add compile definitions based on the selected options
add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# Add LVGL subdirectory
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

# Create the main executable and add source files
add_executable(game-clock
    ${PROJECT_SOURCE_DIR}/game-clock/main.c
    ${PROJECT_SOURCE_DIR}/game-clock/ui.cpp
    ${PROJECT_SOURCE_DIR}/game-clock/freertos_main.cpp
    ${PROJECT_SOURCE_DIR}/game-clock/mouse_cursor_icon.c
    ${PROJECT_SOURCE_DIR}/game-clock/FreeRTOS_Posix_Port.c
  )

# Define LVGL configuration as a simple include
target_compile_definitions(game-clock PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(game-clock lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread)

# Conditionally include and link SDL2_image if LV_USE_DRAW_SDL is enabled
if(LV_USE_DRAW_SDL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
    find_package(SDL2_image REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(game-clock ${SDL2_IMAGE_LIBRARIES})
endif()

# Conditionally include and link libpng if LV_USE_LIBPNG is enabled
if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(game-clock ${PNG_LIBRARIES})
endif()

# Conditionally include and link libjpeg-turbo if LV_USE_LIBJPEG_TURBO is enabled
if(LV_USE_LIBJPEG_TURBO)
    find_package(JPEG REQUIRED)
    target_include_directories(lvgl PUBLIC ${JPEG_INCLUDE_DIRS})
    target_link_libraries(game-clock ${JPEG_LIBRARIES})
endif()

# Conditionally include and link FFmpeg libraries if LV_USE_FFMPEG is enabled
if(LV_USE_FFMPEG)
  target_link_libraries(game-clock avformat avcodec avutil swscale z)
endif()

# Conditionally include and link FreeType if LV_USE_FREETYPE is enabled
if(LV_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(game-clock ${FREETYPE_LIBRARIES})
endif()

# Apply additional compile options if the build type is Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lvgl PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        # -Wmissing-prototypes
        -Wpointer-arith
        -Wmultichar
        -Wno-pedantic # ignored for now, we convert functions to pointers for properties table.
        -Wreturn-type
        -Wshadow
        -Wshift-negative-value
        -Wsizeof-pointer-memaccess
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
    )

    # Check if the compiler supports sanitizers (e.g., leak, address, undefined)
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag("-fsanitize=leak" HAS_SANITIZE_LEAK)
    check_cxx_compiler_flag("-fsanitize=address" HAS_SANITIZE_ADDRESS)
    check_cxx_compiler_flag("-fsanitize=undefined" HAS_SANITIZE_UNDEFINED)

    # Prepare the list of sanitizers
    set(SANITIZERS "")
    if (HAS_SANITIZE_ADDRESS)
        set(SANITIZERS "${SANITIZERS},address")
    endif()

    if (HAS_SANITIZE_UNDEFINED)
        set(SANITIZERS "${SANITIZERS},undefined")
    endif()

    if (HAS_SANITIZE_LEAK)
        set(SANITIZERS "${SANITIZERS},leak")
    endif()

    # Remove leading comma if there are any sanitizers
    string(REGEX REPLACE "^," "" SANITIZERS "${SANITIZERS}")

    # Add the sanitizers to the compile options if any are supported
    if (SANITIZERS)
        message(STATUS "Sanitizers enabled: ${SANITIZERS}")
        target_compile_options(main PRIVATE -fsanitize=${SANITIZERS})
        target_link_options(main PRIVATE -fsanitize=${SANITIZERS})
        target_compile_options(game-clock PRIVATE -fsanitize=${SANITIZERS})
        target_link_options(game-clock PRIVATE -fsanitize=${SANITIZERS})
    else()
        message(STATUS "Sanitizers not enabled")
    endif()
endif()
