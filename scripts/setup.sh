#!/usr/bin/env bash
# Copyright 2024-2026 PipeCAD Contributors
# SPDX-License-Identifier: Apache-2.0

#
# setup.sh — PipeCAD 环境初始化脚本
#
# 用法:
#   bash scripts/setup.sh              # 完整安装 + 首次配置
#   bash scripts/setup.sh --verify     # 仅验证环境，不做安装
#   bash scripts/setup.sh --help       # 显示帮助
#
# 功能:
#   1. 检查并安装 pixi 包管理器
#   2. 安装项目依赖 (pixi install)
#   3. 检查本地预编译库 (OCCT/VSG)
#   4. 首次 CMake configure (Debug)
#   5. 输出环境摘要与常用命令
#

set -euo pipefail

# ── 颜色 ──────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# ── 项目根目录 ────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# ── 输出函数 ──────────────────────────────────────────────
info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()      { echo -e "${GREEN}[ OK ]${NC}  $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
fail()    { echo -e "${RED}[FAIL]${NC}  $*"; exit 1; }

step_header() {
    echo ""
    echo -e "${BOLD}────────────────────────────────────────────────────────${NC}"
    echo -e "${BOLD}  $1${NC}"
    echo -e "${BOLD}────────────────────────────────────────────────────────${NC}"
}

# ── 解析参数 ──────────────────────────────────────────────
VERIFY_ONLY=false

for arg in "$@"; do
    case "${arg}" in
        --verify|--verify-only) VERIFY_ONLY=true ;;
        --help|-h)
            echo "用法: bash scripts/setup.sh [选项]"
            echo ""
            echo "选项:"
            echo "  --verify, --verify-only  仅验证当前环境，不执行安装"
            echo "  --help, -h               显示帮助信息"
            exit 0
            ;;
        *) warn "未知参数: ${arg}（已忽略）" ;;
    esac
done

cd "${PROJECT_ROOT}"

# ══════════════════════════════════════════════════════════
#  步骤 1/5: 检查 / 安装 pixi
# ══════════════════════════════════════════════════════════
step_header "步骤 1/5: 检查 pixi 包管理器"

if command -v pixi &>/dev/null; then
    PIXI_VER="$(pixi --version 2>/dev/null || echo "unknown")"
    ok "pixi 已安装: ${PIXI_VER}"
else
    if [ "${VERIFY_ONLY}" = true ]; then
        fail "pixi 未安装（--verify 模式不执行安装）"
    fi

    info "pixi 未安装，正在通过官方脚本安装..."
    curl -fsSL https://pixi.sh/install.sh | bash

    # 让当前 shell 识别新安装的 pixi
    export PATH="${HOME}/.pixi/bin:${PATH}"

    if command -v pixi &>/dev/null; then
        PIXI_VER="$(pixi --version 2>/dev/null || echo "unknown")"
        ok "pixi 安装成功: ${PIXI_VER}"
    else
        fail "pixi 安装失败，请手动执行: curl -fsSL https://pixi.sh/install.sh | bash"
    fi
fi

# ══════════════════════════════════════════════════════════
#  步骤 2/5: 安装项目依赖 (pixi install)
# ══════════════════════════════════════════════════════════
step_header "步骤 2/5: 安装项目依赖"

if [ "${VERIFY_ONLY}" = true ]; then
    if [ -d ".pixi" ]; then
        ok "pixi 环境已就绪 (.pixi/)"
    else
        warn "pixi 环境尚未创建，请去掉 --verify 参数执行完整安装"
    fi
else
    info "执行 pixi install（首次运行将下载依赖包）..."
    pixi install
    ok "项目依赖安装完成"
fi

# ══════════════════════════════════════════════════════════
#  步骤 3/5: 检查本地预编译库 (OCCT / VSG / VTK)
# ══════════════════════════════════════════════════════════
step_header "步骤 3/5: 检查本地预编译库"

ALL_LIBS_OK=true
for LIB_NAME in occt vsg; do
    LIB_PATH="${PROJECT_ROOT}/lib/${LIB_NAME}"
    if [ -d "${LIB_PATH}" ]; then
        ok "${LIB_NAME} — lib/${LIB_NAME}/ ✓"
    else
        warn "${LIB_NAME} — lib/${LIB_NAME}/ 不存在"
        ALL_LIBS_OK=false
    fi
done

if [ "${ALL_LIBS_OK}" = true ]; then
    ok "所有本地预编译库就绪"
else
    warn "部分预编译库缺失，CMake configure 可能失败"
    warn "请将缺失的库放置到 lib/ 目录，或等待后续迁入 pixi 管理"
fi

# ══════════════════════════════════════════════════════════
#  步骤 4/5: 验证构建工具链
# ══════════════════════════════════════════════════════════
step_header "步骤 4/5: 验证构建工具链 (pixi 环境)"

TOOLS_OK=true
for TOOL in cmake ninja clang++; do
    TOOL_PATH="$(pixi run which "${TOOL}" 2>/dev/null || true)"
    if [ -n "${TOOL_PATH}" ]; then
        TOOL_VER="$("${TOOL_PATH}" --version 2>/dev/null | head -1 || echo "unknown")"
        ok "${TOOL} → ${TOOL_PATH} (${TOOL_VER})"
    else
        warn "${TOOL} 未找到"
        TOOLS_OK=false
    fi
done

if [ "${TOOLS_OK}" = false ] && [ "${VERIFY_ONLY}" = false ]; then
    warn "部分工具缺失，请确认 pixi install 已正确完成"
fi

# ══════════════════════════════════════════════════════════
#  步骤 5/5: 首次 CMake Configure (Debug)
# ══════════════════════════════════════════════════════════
step_header "步骤 5/5: 首次 CMake Configure (Debug)"

if [ "${VERIFY_ONLY}" = true ]; then
    if [ -f "${PROJECT_ROOT}/build/debug/build.ninja" ]; then
        ok "Debug 构建目录已配置: build/debug/"
    else
        warn "Debug 构建目录尚未配置"
    fi
else
    info "执行 pixi run configure-debug ..."
    if pixi run configure-debug; then
        ok "CMake configure 完成: build/debug/"

        # 为 IDE 提供 compile_commands.json
        if [ -f "${PROJECT_ROOT}/build/debug/compile_commands.json" ]; then
            ln -sf "${PROJECT_ROOT}/build/debug/compile_commands.json" \
                   "${PROJECT_ROOT}/compile_commands.json"
            ok "compile_commands.json 已链接到项目根目录（IDE 支持）"
        fi
    else
        warn "CMake configure 失败 — 请检查上方错误信息"
        warn "常见原因: 本地预编译库 (lib/occt, lib/vsg) 缺失"
        warn "修复后可重新执行: pixi run configure-debug"
    fi
fi

# ══════════════════════════════════════════════════════════
#  环境状态摘要
# ══════════════════════════════════════════════════════════
echo ""
echo -e "${BOLD}════════════════════════════════════════════════════════${NC}"
echo -e "${BOLD}  环境状态摘要${NC}"
echo -e "${BOLD}════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "  项目:   ${BOLD}qml-vsg-occt (PipeCAD)${NC}"
echo -e "  目录:   ${PROJECT_ROOT}"
echo -e "  工具链: pixi + CMake + Ninja + Clang, C++17/C++20"
echo ""
echo -e "  ${BOLD}常用 pixi 命令:${NC}"
echo -e "    ${CYAN}pixi run build-debug${NC}      Configure + 编译 Debug 版本"
echo -e "    ${CYAN}pixi run build-release${NC}    Configure + 编译 Release 版本"
echo -e "    ${CYAN}pixi run test${NC}             编译 Debug + 运行全部测试"
echo -e "    ${CYAN}pixi run clean${NC}            清除构建目录 (build/)"
echo -e "    ${CYAN}pixi run configure-debug${NC}  仅执行 CMake configure"
echo -e "    ${CYAN}pixi shell${NC}                进入 pixi 交互式环境"
echo ""
echo -e "  ${BOLD}快捷脚本:${NC}"
echo -e "    ${CYAN}bash scripts/build.sh${NC}                 默认 Debug 构建"
echo -e "    ${CYAN}bash scripts/build.sh release${NC}         Release 构建"
echo -e "    ${CYAN}bash scripts/build.sh test${NC}            构建 + 运行测试"
echo -e "    ${CYAN}bash scripts/build.sh clean${NC}           清除构建产物"
echo -e "    ${CYAN}bash scripts/build.sh run${NC}             构建并运行主程序"
echo ""

if [ "${ALL_LIBS_OK}" = true ]; then
    ok "环境初始化完成，可以开始开发！"
else
    warn "部分预编译库缺失，构建可能会失败"
    warn "请将 OCCT/VSG 预编译库放置到 lib/ 目录后重新运行本脚本"
fi

echo ""
