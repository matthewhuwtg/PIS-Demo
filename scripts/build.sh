#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; BLUE='\033[0;34m'; NC='\033[0m'
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_ok() { echo -e "${GREEN}[OK]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

check_deps() {
    echo ""; echo "=== Checking Dependencies ==="; echo ""
    command -v g++ &>/dev/null && log_ok "g++: $(g++ --version | head -1)" || { log_error "g++ not found"; return 1; }
    command -v make &>/dev/null && log_ok "make: $(make --version 2>&1 | head -1)" || { log_error "make not found"; return 1; }
    command -v python3 &>/dev/null && log_ok "python3: $(python3 --version)" || log_info "python3 not found"
    command -v pkg-config &>/dev/null || log_info "pkg-config not found"
    pkg-config --exists dbus-1 2>/dev/null && log_ok "D-Bus: $(pkg-config --modversion dbus-1)" || log_info "D-Bus: not found (mock)"
    pkg-config --exists gstreamer-1.0 2>/dev/null && log_ok "GStreamer: $(pkg-config --modversion gstreamer-1.0)" || log_info "GStreamer: not found (simulation)"
    pkg-config --exists Qt5Widgets 2>/dev/null && log_ok "Qt5: $(pkg-config --modversion Qt5Widgets)" || log_info "Qt5: not found (GUI disabled)"
    echo ""
}

do_build() {
    echo "=== Building PIS Demo ==="; echo ""
    cd "$PROJECT_DIR"
    if [ "${1:-}" = "--clean" ]; then make clean; fi
    make -j$(nproc) && log_ok "Build successful" || { log_error "Build failed"; return 1; }
}

do_test() {
    echo "=== Running Tests ==="; echo ""
    cd "$PROJECT_DIR" && make test && log_ok "Tests passed" || { log_error "Tests failed"; return 1; }
}

do_run() {
    cd "$PROJECT_DIR"
    if [ -f bin/main ]; then log_info "Starting PIS demo..."; ./bin/main "$@"; else log_error "Binary not found"; return 1; fi
}

case "${1:-default}" in
    --check) check_deps ;;
    --clean) check_deps; do_build --clean; do_test ;;
    --test-only) do_test ;;
    --run) check_deps; do_build; do_run "${@:2}" ;;
    --all) check_deps; do_build --clean; do_test; log_ok "Complete! Run: ./scripts/build.sh --run" ;;
    --help|-h) echo "Usage: $0 [--check|--clean|--test-only|--run|--all|--help]";;
    *) check_deps; do_build; do_test ;;
esac
