#!/bin/bash
# Compiler sans privilèges ; tous les produits restent dans build/.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
[[ $EUID != 0 ]] || { echo 'Compiler avec un utilisateur ordinaire dans la VM Debian.'; exit 1; }
for tool in cmake ninja c++ dpkg-shlibdeps; do command -v "$tool" >/dev/null; done
[[ ! -L build ]] || { echo 'Répertoire build symbolique refusé.'; exit 1; }
project=$(pwd -P)
cmake -S applications/suite -B "$project/build/native-suite" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build "$project/build/native-suite" --parallel "${KYRONEXT_BUILD_JOBS:-1}"
QT_QPA_PLATFORM=offscreen ctest --test-dir "$project/build/native-suite" --output-on-failure
DESTDIR="$project/build/native-stage" cmake --install "$project/build/native-suite"
echo 'Compilation, tests et installation de préparation terminés dans build/.'
