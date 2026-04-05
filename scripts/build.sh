#!/usr/bin/env bash
# Copyright 2024-2026 PipeCAD Contributors
# SPDX-License-Identifier: Apache-2.0

#
# build.sh — PipeCAD 一站式构建 / 编译脚本
#
# 用法:
#   bash scripts/build.sh                 # 默认 Debug 构建
#   bash scripts/build.sh debug           # Debug 构建
#   bash scripts/build.sh release         # Release 构建
#   bash scripts/build.sh test            # Debug 构建 + 运行全部测试
#   bash scripts/build.sh test release    # Release 构建 + 运行全部测试
#   bash scripts/build.sh test -R Engine  # 仅运行匹配 Engine 的测试
#   bash scripts/build.sh clean           # 清除所有构建产物
#   bash scripts/build.sh full            # clean + debug 构建 + 测试
#   bash scripts/build.sh run             # Debug 构建并运行主程序
#   bash scripts/build.sh run release     # Release 构建并运行主程序
#   bash scripts/build.sh configure       # 仅 CMake configure (Debug)
#   bash scripts/build.sh configure rel   # 仅 CMake configure (Release)
#   bash scripts/build.sh status          # 检查构建状态
#   bash scripts/build.sh help            # 显示帮助
#
# 所有构建操作均通过 pixi 执行，确保环境隔离。
#

set -euo pipefail

# ── 颜色 ──────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BLUE='\033[0;34m'
BOLD='\033[1m'
DIM='\033[2m'
NC='\033[0m'

# ── 项目根目录 ────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# ── 默认配置 ──────────────────────────────────────────────
BUILD_DIR_NAME="debug"
BUILD_TYPE="Debug"
TEST_JOBS="1"

detect_cpu_count() {
    nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
}

detect_total_memory_gib() {
    if [ -r /proc/meminfo ]; then
        awk '/MemTotal:/ { printf "%d", ($2 + 1048575) / 1048576 }' /proc/meminfo
        return
    fi

    echo 8
}

is_positive_integer() {
    [[ "$1" =~ ^[1-9][0-9]*$ ]]
}

resolve_build_jobs() {
    if [ -n "${PIPECAD_BUILD_JOBS:-}" ] && is_positive_integer "${PIPECAD_BUILD_JOBS}"; then
        echo "${PIPECAD_BUILD_JOBS}"
        return
    fi

    # 默认 6 路并行构建。
    echo 6
}

resolve_test_jobs() {
    if [ -n "${PIPECAD_TEST_JOBS:-}" ] && is_positive_integer "${PIPECAD_TEST_JOBS}"; then
        echo "${PIPECAD_TEST_JOBS}"
        return
    fi

    echo 1
}

PARALLEL_JOBS="$(resolve_build_jobs)"
TEST_JOBS="$(resolve_test_jobs)"

# ── 输出函数 ──────────────────────────────────────────────
info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()      { echo -e "${GREEN}[ OK ]${NC}  $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
fail()    { echo -e "${RED}[FAIL]${NC}  $*"; }
step()    { echo -e "${BLUE}[STEP]${NC}  $*"; }

banner() {
    echo ""
    echo -e "${BOLD}────────────────────────────────────────────────────────${NC}"
    echo -e "${BOLD}  $1${NC}"
    echo -e "${BOLD}────────────────────────────────────────────────────────${NC}"
    echo ""
}

print_parallel_jobs() {
    case "${1:-build}" in
        build)
            resolve_build_jobs
            ;;
        test)
            resolve_test_jobs
            ;;
        *)
            fail "未知并发类型: ${1}"
            exit 1
            ;;
    esac
}

# ── 计时器 ────────────────────────────────────────────────
TIMER_START=""

timer_start() {
    TIMER_START=$(date +%s%N)
}

timer_stop() {
    local end=$(date +%s%N)
    local elapsed_ns=$((end - TIMER_START))
    local elapsed_ms=$((elapsed_ns / 1000000))
    local elapsed_s=$((elapsed_ms / 1000))
    local remaining_ms=$((elapsed_ms % 1000))
    echo "${elapsed_s}.$(printf '%03d' "${remaining_ms}")s"
}

# ── 解析构建类型 ──────────────────────────────────────────
parse_build_type() {
    case "$1" in
        Debug|debug|d)       BUILD_DIR_NAME="debug";   BUILD_TYPE="Debug"   ;;
        Release|release|r|rel) BUILD_DIR_NAME="release"; BUILD_TYPE="Release" ;;
        *)
            warn "未知构建类型 '$1'，使用默认 Debug"
            BUILD_DIR_NAME="debug"
            BUILD_TYPE="Debug"
            ;;
    esac
}

# ══════════════════════════════════════════════════════════
#  前置检查
# ══════════════════════════════════════════════════════════
preflight_check() {
    if ! command -v pixi &>/dev/null; then
        fail "pixi 未安装或不在 PATH 中"
        echo ""
        info "安装 pixi:"
        echo -e "  ${CYAN}curl -fsSL https://pixi.sh/install.sh | bash${NC}"
        echo ""
        info "或运行环境初始化脚本:"
        echo -e "  ${CYAN}bash scripts/setup.sh${NC}"
        exit 1
    fi

    if [ ! -d "${PROJECT_ROOT}/.pixi" ]; then
        warn "pixi 环境尚未安装，正在执行 pixi install ..."
        pixi install
    fi
}

# ══════════════════════════════════════════════════════════
#  CMake Configure
# ══════════════════════════════════════════════════════════
do_configure() {
    local build_dir="${PROJECT_ROOT}/build/${BUILD_DIR_NAME}"

    banner "CMake Configure (${BUILD_TYPE})"

    step "构建目录: ${build_dir}"
    step "推荐编译并行: ${PARALLEL_JOBS}"
    step "推荐测试并行: ${TEST_JOBS}"
    echo ""

    timer_start

    pixi run cmake -B "${build_dir}" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_PREFIX_PATH="${CONDA_PREFIX:-$(pixi run printenv CONDA_PREFIX 2>/dev/null || echo "")}" \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        2>&1 || {
        fail "CMake configure 失败"
        echo ""
        info "排查建议:"
        echo "  1. 确认 lib/occt、lib/vsg、lib/vtk 预编译库已就位"
        echo "  2. 运行 pixi install 确保依赖完整"
        echo "  3. 查看上方 CMake 错误信息定位问题"
        exit 1
    }

    local elapsed
    elapsed="$(timer_stop)"

    # 为 IDE 链接 compile_commands.json
    if [ -f "${build_dir}/compile_commands.json" ]; then
        ln -sf "${build_dir}/compile_commands.json" \
               "${PROJECT_ROOT}/compile_commands.json" 2>/dev/null || true
    fi

    ok "CMake configure 完成 (${elapsed})"
}

# ══════════════════════════════════════════════════════════
#  CMake Build
# ══════════════════════════════════════════════════════════
do_build() {
    local build_dir="${PROJECT_ROOT}/build/${BUILD_DIR_NAME}"

    # 未 configure 则先 configure
    if [ ! -f "${build_dir}/build.ninja" ]; then
        do_configure
    fi

    banner "编译构建 (${BUILD_TYPE}, ${PARALLEL_JOBS} 并行)"

    timer_start

    pixi run cmake --build "${build_dir}" --parallel "${PARALLEL_JOBS}" 2>&1 || {
        fail "编译失败"
        echo ""
        info "排查建议:"
        echo "  1. 检查上方的编译错误信息"
        echo "  2. 清理后重新构建: bash scripts/build.sh clean && bash scripts/build.sh"
        echo "  3. 链接错误请检查依赖库版本"
        exit 1
    }

    local elapsed
    elapsed="$(timer_stop)"

    ok "编译完成 (${elapsed})"

    # 输出构建产物摘要
    echo ""
    info "构建产物:"
    local main_exe="${build_dir}/src/pipecad"
    if [ -f "${main_exe}" ]; then
        local exe_size
        exe_size="$(du -h "${main_exe}" | cut -f1)"
        echo -e "  ${GREEN}●${NC} pipecad  ${exe_size}  ${DIM}${main_exe}${NC}"
    else
        echo -e "  ${YELLOW}○${NC} pipecad  (未生成)"
    fi

    local lib_count=0
    while IFS= read -r -d '' _lib; do
        lib_count=$((lib_count + 1))
    done < <(find "${build_dir}" -name "*.a" -print0 2>/dev/null)
    echo -e "  ${GREEN}●${NC} 静态库 x${lib_count}"
}

# ══════════════════════════════════════════════════════════
#  运行测试
# ══════════════════════════════════════════════════════════
do_test() {
    local build_dir="${PROJECT_ROOT}/build/${BUILD_DIR_NAME}"
    local extra_args=("$@")

    banner "运行测试 (${BUILD_TYPE} 构建, ${TEST_JOBS} 并行)"

    # 确保已编译
    if [ ! -f "${build_dir}/build.ninja" ]; then
        do_build
    fi

    timer_start

    local ctest_cmd="pixi run ctest --test-dir ${build_dir} --output-on-failure --parallel ${TEST_JOBS}"

    # 支持传递额外的 ctest 参数（如 -R <pattern>）
    if [ ${#extra_args[@]} -gt 0 ]; then
        ctest_cmd="${ctest_cmd} ${extra_args[*]}"
        info "附加参数: ${extra_args[*]}"
    fi

    echo ""
    step "正在执行测试..."
    echo ""

    if eval "${ctest_cmd}" 2>&1; then
        local elapsed
        elapsed="$(timer_stop)"
        ok "全部测试通过 (${elapsed})"
    else
        local elapsed
        elapsed="$(timer_stop)"
        fail "部分测试失败 (${elapsed})"
        echo ""
        info "调试建议:"
        echo "  1. 查看上方失败测试的详细输出"
        echo "  2. 单独运行失败的测试:"
        echo -e "     ${CYAN}pixi shell${NC}"
        echo -e "     ${CYAN}cd ${build_dir} && ctest -R <TestName> -V${NC}"
        echo "  3. 直接运行测试二进制:"
        echo -e "     ${CYAN}${build_dir}/tests/test_<name>${NC}"
        exit 1
    fi
}

# ══════════════════════════════════════════════════════════
#  运行主程序
# ══════════════════════════════════════════════════════════
do_run() {
    local build_dir="${PROJECT_ROOT}/build/${BUILD_DIR_NAME}"
    local main_exe="${build_dir}/src/pipecad"

    banner "运行 PipeCAD (${BUILD_TYPE})"

    # 确保已编译
    if [ ! -f "${main_exe}" ]; then
        do_build
    fi

    if [ ! -f "${main_exe}" ]; then
        fail "主程序不存在: ${main_exe}"
        exit 1
    fi

    local exe_size
    exe_size="$(du -h "${main_exe}" | cut -f1)"
    info "可执行文件: ${main_exe} (${exe_size})"
    info "启动中..."
    echo ""

    pixi run "${main_exe}" "$@"
}

# ══════════════════════════════════════════════════════════
#  清理构建产物
# ══════════════════════════════════════════════════════════
do_clean() {
    banner "清理构建产物"

    local cleaned=0

    if [ -d "${PROJECT_ROOT}/build" ]; then
        local build_size
        build_size="$(du -sh "${PROJECT_ROOT}/build" 2>/dev/null | cut -f1 || echo "?")"
        info "正在删除 build/ 目录 (${build_size})..."
        rm -rf "${PROJECT_ROOT}/build"
        ok "已删除 build/ 目录"
        cleaned=1
    fi

    if [ -L "${PROJECT_ROOT}/compile_commands.json" ] || [ -f "${PROJECT_ROOT}/compile_commands.json" ]; then
        rm -f "${PROJECT_ROOT}/compile_commands.json"
        ok "已删除 compile_commands.json"
        cleaned=1
    fi

    if [ "${cleaned}" -eq 0 ]; then
        info "没有需要清理的构建产物"
    else
        ok "清理完成"
    fi
}

# ══════════════════════════════════════════════════════════
#  全量构建 (clean + build + test)
# ══════════════════════════════════════════════════════════
do_full() {
    banner "全量构建 (clean → build → test)"

    local full_start
    full_start=$(date +%s)

    do_clean
    echo ""
    do_build
    echo ""
    do_test

    local full_end
    full_end=$(date +%s)
    local total=$((full_end - full_start))
    local mins=$((total / 60))
    local secs=$((total % 60))

    banner "全量构建完成"
    ok "总耗时: ${mins}m ${secs}s"
}

# ══════════════════════════════════════════════════════════
#  构建状态
# ══════════════════════════════════════════════════════════
do_status() {
    banner "构建状态"

    # pixi 环境
    if command -v pixi &>/dev/null; then
        local pixi_ver
        pixi_ver="$(pixi --version 2>/dev/null || echo "unknown")"
        echo -e "  pixi:            ${GREEN}已安装${NC} (${pixi_ver})"
    else
        echo -e "  pixi:            ${RED}未安装${NC}"
    fi

    if [ -d "${PROJECT_ROOT}/.pixi" ]; then
        echo -e "  pixi 环境:       ${GREEN}已安装${NC} (.pixi/)"
    else
        echo -e "  pixi 环境:       ${RED}未安装${NC} (运行 pixi install)"
    fi

    echo ""

    # 各构建配置状态
    for bt in debug release; do
        local build_dir="${PROJECT_ROOT}/build/${bt}"

        if [ -f "${build_dir}/build.ninja" ]; then
            local build_size
            build_size="$(du -sh "${build_dir}" 2>/dev/null | cut -f1 || echo "?")"

            echo -e "  ${bt} 构建:       ${GREEN}已配置${NC} (${build_size})"

            local main_exe="${build_dir}/src/pipecad"
            if [ -f "${main_exe}" ]; then
                local exe_size exe_mtime
                exe_size="$(du -h "${main_exe}" | cut -f1)"
                exe_mtime="$(stat -c '%y' "${main_exe}" 2>/dev/null | cut -d'.' -f1 || echo "unknown")"
                echo -e "    pipecad:       ${GREEN}存在${NC} (${exe_size}, ${exe_mtime})"
            else
                echo -e "    pipecad:       ${YELLOW}未生成${NC}"
            fi
        else
            echo -e "  ${bt} 构建:       ${DIM}未配置${NC}"
        fi
    done

    # 测试统计
    local debug_dir="${PROJECT_ROOT}/build/debug"
    if [ -f "${debug_dir}/build.ninja" ]; then
        echo ""
        local test_summary
        test_summary="$(pixi run ctest --test-dir "${debug_dir}" -N 2>&1 | tail -1 || true)"
        if [ -n "${test_summary}" ]; then
            echo -e "  测试:            ${test_summary}"
        fi
    fi

    echo ""
    echo -e "  推荐编译并行:    ${PARALLEL_JOBS}"
    echo -e "  推荐测试并行:    ${TEST_JOBS}"

    # 预编译库
    echo ""
    for lib in occt vsg vtk; do
        if [ -d "${PROJECT_ROOT}/lib/${lib}" ]; then
            echo -e "  lib/${lib}/:       ${GREEN}存在${NC}"
        else
            echo -e "  lib/${lib}/:       ${YELLOW}缺失${NC}"
        fi
    done

    echo ""
}

# ══════════════════════════════════════════════════════════
#  帮助信息
# ══════════════════════════════════════════════════════════
show_help() {
    echo ""
    echo -e "${BOLD}PipeCAD 构建脚本 (build.sh)${NC}"
    echo ""
    echo -e "${BOLD}用法:${NC}"
    echo "  bash scripts/build.sh <命令> [选项]"
    echo ""
    echo -e "${BOLD}命令:${NC}"
    echo ""
    echo -e "  ${CYAN}debug${NC}              Configure + 编译 Debug 版本 (默认命令)"
    echo -e "  ${CYAN}release${NC}            Configure + 编译 Release 版本"
    echo -e "  ${CYAN}configure${NC} [类型]   仅执行 CMake configure (默认 Debug)"
    echo -e "  ${CYAN}test${NC} [类型|参数]   编译 + 运行全部测试 (默认 Debug)"
    echo -e "  ${CYAN}run${NC} [类型]         编译 + 运行主程序 pipecad"
    echo -e "  ${CYAN}clean${NC}             清除所有构建产物 (build/)"
    echo -e "  ${CYAN}full${NC}              clean + debug 编译 + 测试 (全量构建)"
    echo -e "  ${CYAN}status${NC}            显示当前构建状态"
    echo -e "  ${CYAN}help${NC}              显示本帮助信息"
    echo ""
    echo -e "${BOLD}构建类型 (用于 configure/test/run 后的参数):${NC}"
    echo "  debug | d           Debug 构建 (默认)"
    echo "  release | r | rel   Release 构建"
    echo ""
    echo -e "${BOLD}测试参数 (用于 test 命令后):${NC}"
    echo "  -R <pattern>        仅运行匹配 pattern 的测试"
    echo "  -V                  详细输出模式"
    echo "  --verbose           详细输出模式"
    echo ""
    echo -e "${BOLD}并发控制环境变量:${NC}"
    echo "  PIPECAD_BUILD_JOBS=<N>   覆盖默认编译并发"
    echo "  PIPECAD_TEST_JOBS=<N>    覆盖默认测试并发"
    echo "  默认策略: 编译并发 = 1，测试并发 = 1"
    echo "  如需更高吞吐，请显式设置 PIPECAD_BUILD_JOBS 和 PIPECAD_TEST_JOBS"
    echo ""
    echo -e "${BOLD}示例:${NC}"
    echo "  bash scripts/build.sh                       # 默认 Debug 构建"
    echo "  bash scripts/build.sh release               # Release 构建"
    echo "  bash scripts/build.sh test                  # Debug 构建 + 全部测试"
    echo "  bash scripts/build.sh test release          # Release 构建 + 全部测试"
    echo "  bash scripts/build.sh test -R Engine        # 仅运行 Engine 相关测试"
    echo "  bash scripts/build.sh test -R Geometry -V   # 详细模式运行 Geometry 测试"
    echo "  bash scripts/build.sh run                   # Debug 构建并运行"
    echo "  bash scripts/build.sh run release           # Release 构建并运行"
    echo "  bash scripts/build.sh full                  # 全量构建 (clean+build+test)"
    echo "  bash scripts/build.sh clean                 # 清除构建目录"
    echo "  bash scripts/build.sh status                # 查看构建状态"
    echo "  PIPECAD_BUILD_JOBS=2 bash scripts/build.sh  # 强制使用 2 路编译"
    echo ""
    echo -e "${BOLD}等价的 pixi 命令:${NC}"
    echo "  pixi run configure-debug     # CMake configure (Debug)"
    echo "  pixi run configure-release   # CMake configure (Release)"
    echo "  pixi run build-debug         # Configure + 编译 (Debug)"
    echo "  pixi run build-release       # Configure + 编译 (Release)"
    echo "  pixi run test                # 编译 Debug + 运行全部测试"
    echo "  pixi run clean               # 清除 build/ 目录"
    echo "  pixi shell                   # 进入 pixi 交互式环境"
    echo ""
}

# ══════════════════════════════════════════════════════════
#  主入口
# ══════════════════════════════════════════════════════════
main() {
    cd "${PROJECT_ROOT}"

    local command="${1:-debug}"
    shift || true

    case "${command}" in
        debug|d)
            preflight_check
            parse_build_type "debug"
            do_build
            ;;
        release|r|rel)
            preflight_check
            parse_build_type "release"
            do_build
            ;;
        configure|config|cfg)
            preflight_check
            # configure 后面可以跟构建类型
            if [ $# -gt 0 ]; then
                parse_build_type "$1"
            fi
            do_configure
            ;;
        test|t)
            preflight_check
            # test 后面可能是构建类型，也可能是 ctest 参数
            if [ $# -gt 0 ]; then
                case "$1" in
                    debug|d|Debug)    parse_build_type "debug";  shift || true ;;
                    release|r|rel|Release) parse_build_type "release"; shift || true ;;
                    -*) ;; # ctest 参数，不改变默认 Debug
                esac
            fi
            do_test "$@"
            ;;
        run)
            preflight_check
            if [ $# -gt 0 ]; then
                case "$1" in
                    debug|d|Debug)         parse_build_type "debug";    shift || true ;;
                    release|r|rel|Release) parse_build_type "release";  shift || true ;;
                esac
            fi
            do_run "$@"
            ;;
        clean|c)
            do_clean
            ;;
        full|f)
            preflight_check
            do_full
            ;;
        status|s|stat)
            do_status
            ;;
        jobs)
            print_parallel_jobs "${1:-build}"
            ;;
        help|h|--help|-h)
            show_help
            ;;
        *)
            echo -e "${RED}未知命令: ${command}${NC}"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

main "$@"
