#!/bin/bash
set -eu

SCRIPT_DIR="$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)"
cd "${SCRIPT_DIR}"

ALL_TARGETS=(gflags libuv asio flatbuffers spdlog cnats termbox)

#-------------------------------------------------------------------------------
# misc
#-------------------------------------------------------------------------------

utils.is_cmd() { type "$1" >/dev/null 2>&1; }
utils.exec_subcmd() {
  local subcmd="$1"; shift
  local receiver="${FUNCNAME[1]}"
  local cmd="${receiver}.${subcmd}"
  if ! utils.is_cmd "$cmd"; then
    log.error "Unknown command '${cmd}'\n"
    return 1
  fi
  "$cmd" "$@"
}
log() {
  local level="$1"; shift
  echo -e "$(date "+%Y-%m-%d %H:%M:%S") ${level} $*";
}
log.info() { log INFO "$*"; }
log.error() { log ERROR "$*"; }

#-------------------------------------------------------------------------------
# app
#-------------------------------------------------------------------------------

app() {
  if (($# == 0)); then
    app.help "$@"
    exit 1
  fi
  local subcmd="$1"; shift
  utils.exec_subcmd "$subcmd" "$@"
}

app.help() {
  if (($# > 0)); then
    local subcmd="$1"; shift
    utils.exec_subcmd "$subcmd"
  fi
  cat <<EOT
Usage: $0 <commands> [<options>] [<arguments...>]

Commands:
    build
    clean
    help
EOT
}

app.help.build() { log.info 'TODO'; }
app.help.clean() { log.info 'TODO'; }
app.help.help() { log.info 'TODO'; }

app.build() {
  local targets=("$@")
  if [[ $# == 0 ]]; then
    targets=("${ALL_TARGETS[@]}")
  fi

  mkdir -p include lib bin

  for target in "${targets[@]}"; do
    log.info "===== BUILD: ${target} ====="
    local basecmd="deps.${target}"
    if "${basecmd}.is_built"; then
      log.info "${target} is already built, so skipped."
    else
      "${basecmd}.build"
      log.info "done."
    fi
  done
}

app.clean() {
  case "${1:---help}" in
    -h | --help )
      app.help.clean
      return
      ;;
    -a | --all )
      log.info "Clean all..."
      rm -rf include lib bin
      log.info "done."
      return
      ;;
  esac
  local targets=("$@")
  for target in "${targets[@]}"; do
    log.info "Clean ${target}..."
    local basecmd="deps.${target}"
    "${basecmd}.clean"
    log.info "done."
  done
}

#-------------------------------------------------------------------------------
# deps
#-------------------------------------------------------------------------------

deps.gflags.is_built() {
  [[ -d include/gflags && -f lib/libgflags.a ]]
}

deps.gflags.build() {
  pushd src/gflags
  mkdir -p build-ninja
  cd build-ninja
  cmake .. -GNinja
  ninja
  cp -R include/* ../../../include/
  cp lib/* ../../../lib/
  popd
}

deps.gflags.clean() {
  rm lib/libgflags.a lib/libgflags_nothreads.a
  rm -r include/gflags
}

#---

deps.libuv.is_built() {
  [[ -d include/uv.h && -f lib/libuv.a ]]
}

deps.libuv.build() {
  pushd src/libuv
  ./gyp_uv.py -f ninja
  ninja -C out/Release # or ninja -C out/Debug
  cp out/Release/libuv.a ../../lib/
  cp -R include/* ../../include/
  popd
}

deps.libuv.clean() {
  rm lib/libuv.a
  rm include/uv-* tree.h android-ifaddrs.h pthread-barrier.h stdint-msvc2008.h
}

#---

deps.asio.is_built() {
  [[ -d include/asio && -f include/asio.hpp ]]
}

deps.asio.build() {
  cp -R src/asio/asio/include/asio include/asio
  cp -R src/asio/asio/include/asio.hpp include/
}

deps.asio.clean() {
  rm -r include/asio
  rm include/asio.hpp
}

#---

deps.flatbuffers.is_built() {
  [[ -f bin/flatc && -d include/flatbuffers && -f lib/libflatbuffers.a ]]
}

deps.flatbuffers.build() {
  pushd src/flatbuffers
  cmake -GNinja
  ninja
  cp flatc ../../bin/
  cp libflatbuffers.a ../../lib/
  cp -R include/flatbuffers ../../include/
  git add .
  git reset --hard
  popd
}

deps.flatbuffers.clean() {
  rm bin/flatc lib/libflatbuffers.a
  rm -r include/flatbuffers
}

#---

deps.spdlog.is_built() {
  [[ -d include/spdlog ]]
}

deps.spdlog.build() {
  cp -R src/spdlog/include/spdlog include/spdlog
}

deps.spdlog.clean() {
  rm -r include/spdlog
}

#---

deps.cnats.is_built() {
  [[ -f lib/libnats_static.a && -d include/nats && -f include/nats.h ]]
}

deps.cnats.build() {
  pushd src/cnats
  mkdir -p build
  cd build
  cmake .. -GNinja -DNATS_BUILD_WITH_TLS=OFF -DCMAKE_INSTALL_PREFIX=../../..
  ninja
  ninja install
  popd
}

deps.cnats.clean() {
  rm lib/libnats_static.a include/nats.h
  rm -r include/nats
}

#---

deps.termbox.is_built() {
  [[ -f lib/libtermbox.a && -f include/termbox.h ]]
}

deps.termbox.build() {
  pushd src/termbox
  ./waf --prefix=../.. distclean configure clean build install
  popd
}

deps.termbox.clean() {
  pushd src/termbox
  ./waf --prefix=../.. configure uninstall distclean
  popd
}

#-------------------------------------------------------------------------------

app "$@"
