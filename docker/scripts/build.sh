#!/bin/bash

WORK_DIR=/tmp/workspace
BUILD_DIR="${WORK_DIR}/cmake-build-test"

cd "${WORK_DIR}" || exit

git_modifications=$(git status --porcelain)
git_remotes=$(git remote -v | grep -q cable)

if [[ -z ${git_modifications} && -z ${git_remotes} ]]; then
  git remote add cable https://github.com/ethereum/cable
fi

if [[ -z ${git_modifications} ]]; then
  git subtree pull --prefix cmake/cable cable master --squash
fi

cd "${BUILD_DIR}" || exit

/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -G "CodeBlocks - Unix Makefiles" "${WORK_DIR}"
/usr/bin/cmake --build "${BUILD_DIR}" --target all -- -j 4
