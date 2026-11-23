"""绘图程序：读取 tests/bench_results.json，输出性能对比图

用法：
    python tests/plot_bench.py

输出：
    docs/figures/vector_vs_pylist.png（Python list vs C++ Vector 三重对比）
"""

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # 无显示环境（SSH / WSL）下也能出图
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parent.parent
RESULT_FILE = ROOT / "tests" / "bench_results.json"
OUT_FILE = ROOT / "docs" / "figures" / "vector_vs_pylist.png"

OPS = ["append", "sum", "iterate"]
TITLES = {
    "append": "Per-element append",
    "sum": "Sum",
    "iterate": "Pure-Python loop",
}


def main():
    data = json.loads(RESULT_FILE.read_text(encoding="utf-8"))
    sizes = sorted(int(k) for k in data["append"])

    fig, axes = plt.subplots(1, 3, figsize=(15, 4.6))
    for ax, op in zip(axes, OPS):
        py = [data[op][str(n)]["python"] * 1e3 for n in sizes]
        cpp = [data[op][str(n)]["cpp"] * 1e3 for n in sizes]
        ax.plot(sizes, py, "o-", label="Python list")
        ax.plot(sizes, cpp, "s-", label="C++ Vector (fast_cpp)")
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("N (elements)")
        ax.set_ylabel("time (ms)")
        ax.set_title(TITLES[op])
        ax.grid(True, which="both", ls="--", alpha=0.4)
        ax.legend()

    fig.suptitle("Performance: Python list vs C++ Vector via pybind11")
    fig.tight_layout()
    OUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(OUT_FILE, dpi=150)
    print(f"saved: {OUT_FILE}")


if __name__ == "__main__":
    main()
