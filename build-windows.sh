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

    # Stage through the CMake install rules (exe, runtime DLLs, third-party
    # license texts) so the installer content has one source of truth; using a
    # staging dir instead of cpack's preinstall avoids building all targets.
    STAGING="${BUILD_DIR}/_cpack_staging"
    rm -rf "${STAGING}"
    cmake --install "${BUILD_DIR}" --prefix "${STAGING}"

    # Render the user manual to a self-contained HTML file (screenshots and
    # stylesheet embedded), staged next to bin/ for the installer to package.
    if ! command -v pandoc >/dev/null 2>&1; then
        echo "Error: pandoc is required to render the manual." >&2
        echo "       Install it (e.g. 'dnf install pandoc') and re-run." >&2
        exit 1
    fi
    pandoc "${SCRIPT_DIR}/doc/MANUAL.md" \
        --standalone --embed-resources \
        --resource-path "${SCRIPT_DIR}/doc" \
        --metadata pagetitle="Rumpu Manual" \
        --css "${SCRIPT_DIR}/doc/manual.css" \
        -o "${STAGING}/Rumpu Manual.html"

    cpack -G NSIS \
        -D "CPACK_INSTALL_CMAKE_PROJECTS=" \
        -D "CPACK_INSTALLED_DIRECTORIES=${STAGING};."
    echo "Installer: ${BUILD_DIR}/Rumpu-*.exe"
fi
