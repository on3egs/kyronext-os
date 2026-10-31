#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
BUILD_DIR="${KYRONEXT_BUILD_DIR:-$ROOT/build/suite-preview}"
BUILD_DIR="$(realpath -m -- "$BUILD_DIR")"

case "$BUILD_DIR" in
  "$ROOT"/build|"$ROOT"/build/*) ;;
  *) printf 'Erreur : le répertoire de compilation doit rester sous %s/build.\n' "$ROOT" >&2; exit 2 ;;
esac

for tool in cmake ninja ctest dbus-run-session dpkg-deb sha256sum realpath; do
  command -v "$tool" >/dev/null || { printf 'Outil manquant : %s\n' "$tool" >&2; exit 2; }
done

CONTROL="$ROOT/applications/suite/debian/control"
[[ -s "$CONTROL" ]] || { printf 'Contrôle Debian absent : %s\n' "$CONTROL" >&2; exit 2; }
mkdir -p -- "$BUILD_DIR"

JOBS="${KYRONEXT_BUILD_JOBS:-1}"
[[ "$JOBS" =~ ^[1-9][0-9]*$ ]] || { printf 'KYRONEXT_BUILD_JOBS doit être un entier positif.\n' >&2; exit 2; }

STAGE_DIR="$(mktemp -d "$BUILD_DIR/stage.XXXXXX")"
PACKAGE_ROOT="$(mktemp -d "$BUILD_DIR/package.XXXXXX")"
trap 'rm -rf -- "$STAGE_DIR" "$PACKAGE_ROOT"' EXIT

cmake -S "$ROOT/applications/suite" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$BUILD_DIR" --parallel "$JOBS"
dbus-run-session -- env QT_QPA_PLATFORM=offscreen \
  ctest --test-dir "$BUILD_DIR" --output-on-failure

DESTDIR="$STAGE_DIR" cmake --install "$BUILD_DIR" --prefix /usr
cp -a -- "$STAGE_DIR"/. "$PACKAGE_ROOT"/
install -d -- "$PACKAGE_ROOT/DEBIAN"
install -m 0644 -- "$CONTROL" "$PACKAGE_ROOT/DEBIAN/control"

VERSION="$(sed -n 's/^Version:[[:space:]]*//p' "$CONTROL" | head -n 1)"
ARCH="$(sed -n 's/^Architecture:[[:space:]]*//p' "$CONTROL" | head -n 1)"
[[ -n "$VERSION" && -n "$ARCH" ]] || { printf 'Version ou architecture manquante dans control.\n' >&2; exit 2; }
PACKAGE="kyronext-suite-preview_${VERSION}_${ARCH}.deb"
RELEASE_DIR="$ROOT/releases/apps-preview-$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p -- "$RELEASE_DIR"
dpkg-deb --build --root-owner-group "$PACKAGE_ROOT" "$RELEASE_DIR/$PACKAGE"
dpkg-deb --info "$RELEASE_DIR/$PACKAGE"
(cd "$RELEASE_DIR" && sha256sum "$PACKAGE" | tee SHA256SUMS)
printf '\nPaquet prêt : %s\n' "$RELEASE_DIR/$PACKAGE"
