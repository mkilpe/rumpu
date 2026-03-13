#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-windows"

BUILD_INSTALLER=false
CMAKE_ARGS=()
for arg in "$@"; do
    if [[ "$arg" == "--installer" ]]; then
        BUILD_INSTALLER=true
    else
        CMAKE_ARGS+=("$arg")
    fi
done

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/toolchain-mingw64.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    "${CMAKE_ARGS[@]}"

cmake --build "${BUILD_DIR}" --target rumpu --parallel "$(nproc)"

echo "Build complete. Binary: ${BUILD_DIR}/bin/rumpu.exe"

if [[ "$BUILD_INSTALLER" == true ]]; then
    cd "${BUILD_DIR}"

    # Stage only the files we need (avoids cpack preinstall building all targets)
    STAGING="${BUILD_DIR}/_cpack_staging"
    rm -rf "${STAGING}"
    mkdir -p "${STAGING}/bin"
    cp "${BUILD_DIR}/bin/rumpu.exe" "${STAGING}/bin/"

    # Bundle MinGW runtime DLLs
    SYSROOT=$(x86_64-w64-mingw32-gcc -print-sysroot 2>/dev/null || echo "/usr/x86_64-w64-mingw32/sys-root")
    for dll in libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll; do
        found=$(find "${SYSROOT}" /usr/x86_64-w64-mingw32 -name "$dll" 2>/dev/null | head -1)
        if [[ -n "$found" ]]; then
            cp "$found" "${STAGING}/bin/"
        fi
    done

    cpack -G NSIS \
        -D "CPACK_INSTALL_CMAKE_PROJECTS=" \
        -D "CPACK_INSTALLED_DIRECTORIES=${STAGING};."
    echo "Installer: ${BUILD_DIR}/Rumpu-*.exe"
fi
