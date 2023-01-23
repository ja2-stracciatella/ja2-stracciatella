# \file dependencies/lib-stracciatella/CMakeLists.txt

set(STRACCIATELLA_DIR "${CMAKE_BINARY_DIR}/lib-stracciatella")
set(STRACCIATELLA_HEADER "${STRACCIATELLA_DIR}/include/stracciatella.h")
set(STRACCIATELLA_LIB "${STRACCIATELLA_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}stracciatella${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(STRACCIATELLA_BIN_ja2-resource-pack "${STRACCIATELLA_DIR}/bin/ja2-resource-pack${CMAKE_EXECUTABLE_SUFFIX}")

# find cargo and rustc
file(READ "${CMAKE_SOURCE_DIR}/rust-toolchain" MIN_RUST_VERSION)
string(STRIP "${MIN_RUST_VERSION}" MIN_RUST_VERSION)
find_package(Cargo ${MIN_RUST_VERSION} REQUIRED)
find_package(Rustc ${MIN_RUST_VERSION} REQUIRED)
if (MINGW)
    set(RUSTC_STATICLIB_PREFIX "lib")
    set(RUSTC_STATICLIB_SUFFIX ".a")
else()
    set(RUSTC_STATICLIB_PREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
    set(RUSTC_STATICLIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
set(RUSTC_BIN_SUFFIX "${CMAKE_EXECUTABLE_SUFFIX}")

# setup cargo env
set(CARGO_BUILD_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/target")
set(CARGO_BUILD_TARGET "${RUSTC_HOST}" CACHE STRING "The target platform triple for rustc.")
if (ANDROID)
    if (ANDROID_ABI STREQUAL "armeabi-v7a")
        set(CARGO_BUILD_TARGET "armv7-linux-androideabi" CACHE STRING "The target platform triple for rustc." FORCE)
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
        set(CARGO_BUILD_TARGET "aarch64-linux-android" CACHE STRING "The target platform triple for rustc." FORCE)
    elseif(ANDROID_ABI STREQUAL "x86")
        set(CARGO_BUILD_TARGET "i686-linux-android" CACHE STRING "The target platform triple for rustc." FORCE)
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(CARGO_BUILD_TARGET "x86_64-linux-android" CACHE STRING "The target platform triple for rustc." FORCE)
    else()
        message(FATAL_ERROR "target android abi is not supported: '${ANDROID_ABI}'")
    endif()
endif()
set(CARGO_BUILD_RUSTFLAGS "--print=native-static-libs")
set(CARGO_ENV
    "HEADER_LOCATION=${STRACCIATELLA_HEADER}"
    "EXTRA_DATA_DIR=${EXTRA_DATA_DIR}"
    "RUST_BACKTRACE=1"
    "CARGO_BUILD_TARGET=${CARGO_BUILD_TARGET}"
    "CARGO_BUILD_TARGET_DIR=${CARGO_BUILD_TARGET_DIR}"
    "CARGO_BUILD_RUSTFLAGS=${CARGO_BUILD_RUSTFLAGS}"
)
if (SCCACHE_PROGRAM)
    set(CARGO_ENV
        ${CARGO_ENV}
        "RUSTC_WRAPPER=${SCCACHE_PROGRAM}"
    )
endif()
if (MINGW OR ANDROID)
    string(MAKE_C_IDENTIFIER "CARGO_TARGET_${CARGO_BUILD_TARGET}_LINKER" var)
    string(TOUPPER "${var}" var)
    list(APPEND CARGO_ENV "${var}=${CMAKE_C_COMPILER}")
endif()

# create stamp
set(STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/rust.stamp")
set(STAMP_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/update-stamp.cmake")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/update-stamp.cmake.in"
    "${STAMP_SCRIPT_FILE}"
    @ONLY ESCAPE_QUOTES
)

# build
set(OUT_PROFILE "$<$<CONFIG:Debug>:debug>$<$<NOT:$<CONFIG:Debug>>:release>") # XXX only use this where generator expressions are supported
set(OUT_DIR "${CARGO_BUILD_TARGET_DIR}/${CARGO_BUILD_TARGET}/${OUT_PROFILE}")
set(OUT_LIB "${OUT_DIR}/${RUSTC_STATICLIB_PREFIX}stracciatella_c_api${RUSTC_STATICLIB_SUFFIX}")
set(OUT_BIN_ja2-resource-pack "${OUT_DIR}/ja2-resource-pack${RUSTC_BIN_SUFFIX}")
add_custom_target(
    stracciatella-update-stamp
    COMMAND ${CMAKE_COMMAND} -P "${STAMP_SCRIPT_FILE}"
    BYPRODUCTS ${STAMP_FILE}
)

set(RUST_BUILD_OUTPUTS "${STRACCIATELLA_HEADER}" "${STRACCIATELLA_LIB}")
if(WITH_RUST_BINARIES)
    list(APPEND RUST_BUILD_OUTPUTS "${STRACCIATELLA_BIN_ja2-resource-pack}")
endif()
set(COPY_BINARIES_COMMAND echo "Skipping copy of rust binaries")
if (WITH_RUST_BINARIES)
    set(COPY_BINARIES_COMMAND copy_if_different "${OUT_BIN_ja2-resource-pack}" "${STRACCIATELLA_BIN_ja2-resource-pack}")
endif()
set(CARGO_WORKSPACE_FLAGS "--all")
if (NOT WITH_RUST_BINARIES)
    list(APPEND CARGO_WORKSPACE_FLAGS "--exclude stracciatella_bin")
endif()
add_custom_command(
    OUTPUT ${RUST_BUILD_OUTPUTS}
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target "cargo-build-${OUT_PROFILE}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT_LIB}" "${STRACCIATELLA_LIB}"
    COMMAND ${CMAKE_COMMAND} -E ${COPY_BINARIES_COMMAND}
    DEPENDS "${STAMP_FILE}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/rust"
    COMMENT "Building stracciatella"
    VERBATIM
)
add_custom_target(stracciatella-build DEPENDS stracciatella-update-stamp ${RUST_BUILD_OUTPUTS})

# interface library
if (UNIX AND NOT MINGW AND NOT ANDROID)
    if(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        set(ADDITIONAL_LIBS pthread)
    else()
        set(ADDITIONAL_LIBS dl pthread)
    endif()
elseif(MINGW)
    set(ADDITIONAL_LIBS bcrypt ws2_32 shell32 advapi32 userenv gcc_eh pthread)
elseif(MSVC)
    set(ADDITIONAL_LIBS bcrypt ws2_32 shell32 advapi32 userenv)
else()
    set(ADDITIONAL_LIBS)
endif()
add_library(stracciatella INTERFACE)
add_dependencies(stracciatella stracciatella-build)
target_include_directories(stracciatella INTERFACE "${STRACCIATELLA_DIR}/include")
target_link_libraries(stracciatella INTERFACE "${STRACCIATELLA_LIB}" ${ADDITIONAL_LIBS})
set(STRACCIATELLA_LIBRARIES stracciatella PARENT_SCOPE)
set(STRACCIATELLA_EXECUTABLES "" PARENT_SCOPE)
if (WITH_RUST_BINARIES)
    set(STRACCIATELLA_EXECUTABLES "${STRACCIATELLA_BIN_ja2-resource-pack}" PARENT_SCOPE)
endif()

# auxiliary targets
function (add_cargo_target target command)
    if ("${command}" MATCHES "^cargo (.*)$")
        string(REPLACE " " ";" args "${CMAKE_MATCH_1}") # FIXME this only works when no argument has spaces
        add_custom_target(${target}
            COMMAND ${CMAKE_COMMAND} -E env ${CARGO_ENV} ${CARGO_EXECUTABLE} ${args}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/rust"
            COMMENT "${command}"
            VERBATIM
        )
    else()
        message(FATAL_ERROR "not a cargo command: ${command}")
    endif()
endfunction()
add_cargo_target(cargo-build-debug "cargo build --verbose ${CARGO_WORKSPACE_FLAGS}")
add_cargo_target(cargo-build-release "cargo build --verbose --release ${CARGO_WORKSPACE_FLAGS}")
add_cargo_target(cargo-fmt "cargo fmt --verbose")
add_cargo_target(cargo-test-debug "cargo test --verbose")
add_cargo_target(cargo-test-release "cargo test --verbose --release")
add_cargo_target(cargo-fmt-check "cargo fmt --verbose -- --check")
add_cargo_target(cargo-clippy-debug "cargo clippy --all-targets --all-features --verbose -- -D warnings")
add_cargo_target(cargo-clippy-release "cargo clippy --all-targets --all-features --verbose --release -- -D warnings")
add_custom_target(cargo-test COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target "cargo-test-${OUT_PROFILE}")
add_custom_target(cargo-build COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target "cargo-build-${OUT_PROFILE}")
add_custom_target(cargo-clippy COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target "cargo-clippy-${OUT_PROFILE}")
