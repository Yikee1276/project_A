"""压测程序：Python 原生 list  vs  手写 C++ Vector（fast_cpp 模块）

用法：
    python tests/stress_bench.py

输出：
    * 终端打印 Python / C++ 的耗时对比表（毫秒）；
    * 结果写入 tests/bench_results.json，供 tests/plot_bench.py 绘图。
"""

import json
import time
from pathlib import Path

import fast_cpp

SIZES = [10_000, 100_000, 1_000_000]
REPEAT = 3
RESULT_FILE = Path(__file__).resolve().parent / "bench_results.json"


def best_time(fn, repeat=REPEAT):
    """重复执行取最短耗时（秒），削弱调度抖动带来的干扰。"""
    best = float("inf")
    for _ in range(repeat):
        t0 = time.perf_counter()
        fn()
        best = min(best, time.perf_counter() - t0)
    return best


def bench_append(n):
    """逐元素追加：list.append  vs  IntVector.push_back。"""

    def py():
        lst = []
        ap = lst.append
        for i in range(n):
            ap(i)

    def cpp():
        v = fast_cpp.IntVector()
        pb = v.push_back
        for i in range(n):
            pb(i)

    return best_time(py), best_time(cpp)


def bench_sum(n):
    """求和：Python 内置 sum(list)  vs  C++ 原生循环 v.sum()。"""
    data = list(range(n))
    v = fast_cpp.IntVector.from_list(data)
    assert v.size() == n and sum(data) == v.sum()

    return best_time(lambda: sum(data)), best_time(lambda: v.sum())


def bench_iterate(n):
    """纯 Python 循环遍历求和：list  vs  C++ Vector 的 __iter__。"""
    data = list(range(n))
    v = fast_cpp.IntVector.from_list(data)

    def py():
        s = 0
        for x in data:
            s += x
        return s

    def cpp():
        s = 0
        for x in v:  # 通过 __iter__ 暴露的 C++ 迭代器
            s += x
        return s

    assert py() == cpp() == sum(data)
    return best_time(py), best_time(cpp)


def main():
    results = {"append": {}, "sum": {}, "iterate": {}}
    benches = {"append": bench_append, "sum": bench_sum, "iterate": bench_iterate}
    names = {
        "append": "逐元素追加（list.append vs IntVector.push_back）",
        "sum": "求和（sum(list) vs C++ 原生 v.sum()）",
        "iterate": "纯 Python 循环遍历求和（for x in ...）",
    }

    print(f"fast_cpp: {fast_cpp.__file__}")
    print(f"sizes = {SIZES}，repeat = {REPEAT}（取最短耗时）\n")

    for op, fn in benches.items():
        print(f"===== {names[op]} =====")
        print(f"{'N':>10}  {'Python(ms)':>12}  {'C++(ms)':>12}  {'C++/Python':>10}")
        for n in SIZES:
            py_s, cpp_s = fn(n)
            results[op][str(n)] = {"python": py_s, "cpp": cpp_s}
            ratio = cpp_s / py_s if py_s else float("nan")
            print(f"{n:>10}  {py_s * 1e3:>12.3f}  {cpp_s * 1e3:>12.3f}  {ratio:>10.2f}x")
        print()

    RESULT_FILE.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(f"结果已写入：{RESULT_FILE}")


if __name__ == "__main__":
    main()
