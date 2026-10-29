#!/usr/bin/env bash
# ===========================================================================
#  diagnose.sh —— 一键诊断脚本
#
#  做三件事：
#    1. 用 -g 编译 tests/test_cpp.cpp（便于 valgrind 报出行号）
#    2. valgrind --leak-check=full --show-leak-kinds=all 运行它
#    3. 激活 .venv 并运行 tests/test_main.py
#
#  用法：
#    bash diagnose.sh                  全部三步
#    bash diagnose.sh --only valgrind  只做第 1、2 步
#    bash diagnose.sh --only python    只做第 3 步
#
#  说明：任何一步失败都不会中断后面的步骤，最后统一汇总退出码。
# ===========================================================================
set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR" || exit 1

BUILD_DIR="build"
TEST_SRC="tests/test_cpp.cpp"
TEST_BIN="${BUILD_DIR}/test_cpp"
VG_LOG="${BUILD_DIR}/valgrind.log"
PY_TEST="tests/test_main.py"
VENV_DIR=".venv"

CXX="${CXX:-g++}"
CXXFLAGS_DIAG="-std=c++17 -g -O0 -Wall -Wextra -Iinclude"
# 泄漏/内存错误会让 valgrind 以 99 退出（默认它总是返回 0，容易误判为"通过"）
VG_OPTS=(--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=99)

ONLY="all"
while [ $# -gt 0 ]; do
    case "$1" in
        --only)
            ONLY="${2:-all}"
            shift 2 || shift
            ;;
        -h|--help)
            sed -n '2,20p' "$0"
            exit 0
            ;;
        *)
            ONLY="$1"
            shift
            ;;
    esac
done

FAILED=0

hr() { printf '%s\n' "--------------------------------------------------------------------"; }

# ---------------------------------------------------------------------------
# 第 1、2 步：编译 + valgrind
# ---------------------------------------------------------------------------
run_valgrind() {
    mkdir -p "$BUILD_DIR"

    # 收集 src/ 下的实现文件（跳过绑定层 bindings.cpp：它依赖 Python 头文件，
    # 且与扩展模块重复，链接进测试程序既无必要也可能冲突）
    local src_files=()
    local f
    for f in src/*.cpp; do
        [ -e "$f" ] || continue
        case "$f" in
            */bindings.cpp) continue ;;
        esac
        src_files+=("$f")
    done

    hr
    echo "[1/3] 编译 C++ 测试文件（含 -g 调试符号）"
    echo "      测试源：$TEST_SRC"
    echo "      实现源：${src_files[*]:-(src/ 下暂无实现文件)}"
    echo "      命令  ：$CXX $CXXFLAGS_DIAG -o $TEST_BIN $TEST_SRC ${src_files[*]}"

    if [ ! -f "$TEST_SRC" ]; then
        echo "  !! 找不到 $TEST_SRC"
        return 1
    fi
    if [ ! -f "include/my_vector.hpp" ]; then
        echo "  !! 提示：include/my_vector.hpp 尚不存在，"
        echo "     $TEST_SRC 会因缺失头文件而编译失败（等你手写该头文件后自动恢复）。"
    fi

    if ! $CXX $CXXFLAGS_DIAG -o "$TEST_BIN" "$TEST_SRC" "${src_files[@]}"; then
        echo "  !! 编译失败，跳过 valgrind。请先修复上面的编译错误。"
        return 1
    fi
    echo "  -> 编译成功：$TEST_BIN"

    hr
    echo "[2/3] valgrind 内存检测"
    echo "      命令：valgrind ${VG_OPTS[*]} $TEST_BIN"
    echo "      日志：$VG_LOG"

    valgrind "${VG_OPTS[@]}" "$TEST_BIN" 2>&1 | tee "$VG_LOG"
    local vg_rc=${PIPESTATUS[0]}

    hr
    echo "  关键摘要（详见 $VG_LOG）："
    grep -E 'definitely lost|indirectly lost|possibly lost|Invalid (read|write)|ERROR SUMMARY' "$VG_LOG" 2>/dev/null | sed 's/^/    /'

    hr
    case "$vg_rc" in
        0)
            echo "  -> valgrind 未发现内存错误与泄漏"
            return 0
            ;;
        99)
            echo "  -> valgrind 检测到内存问题（泄漏或非法访问）"
            echo "     关注 'definitely lost'（直接泄漏）与 'Invalid read|write'（越界/悬垂指针）。"
            return 1
            ;;
        *)
            echo "  -> 测试程序异常结束（退出码 $vg_rc，可能崩溃），详见 $VG_LOG"
            return 1
            ;;
    esac
}

# ---------------------------------------------------------------------------
# 第 3 步：Python 测试
# ---------------------------------------------------------------------------
run_python() {
    hr
    echo "[3/3] Python 测试"
    echo "      脚本：$PY_TEST"

    if [ ! -f "${VENV_DIR}/bin/activate" ]; then
        echo "  !! 找不到 ${VENV_DIR}/bin/activate，请先创建虚拟环境。"
        return 1
    fi
    # shellcheck disable=SC1091
    source "${VENV_DIR}/bin/activate"

    if [ ! -f "$PY_TEST" ]; then
        echo "  !! 找不到 $PY_TEST"
        return 1
    fi

    if python "$PY_TEST"; then
        echo "  -> Python 侧运行成功"
        return 0
    fi
    echo "  !! Python 侧运行失败"
    return 1
}

# ---------------------------------------------------------------------------
# 主流程
# ---------------------------------------------------------------------------
case "$ONLY" in
    all)
        run_valgrind || FAILED=1
        run_python   || FAILED=1
        ;;
    valgrind|vg)
        run_valgrind || FAILED=1
        ;;
    python|py)
        run_python || FAILED=1
        ;;
    *)
        echo "未知参数：$ONLY"
        echo "用法：bash diagnose.sh [--only all|valgrind|python]"
        exit 2
        ;;
esac

hr
if [ "$FAILED" -eq 0 ]; then
    echo "=== 诊断通过：内存检查与 Python 调用均无异常 ==="
else
    echo "=== 诊断未通过：请查看上面标记为 '!!' 的段落 ==="
fi
exit "$FAILED"
