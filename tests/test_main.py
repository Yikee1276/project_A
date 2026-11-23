"""Python 侧测试入口：验证 fast_cpp 绑定可用（供 make test / diagnose.sh 调用）。

用法：
    make test
    bash diagnose.sh --only python
"""

import sys

import fast_cpp


def main() -> int:
    print(f"Python 版本 : {sys.version.split()[0]}")
    print(f"fast_cpp    : {fast_cpp.__file__}")

    # 1) 冒烟函数
    assert fast_cpp.add(20, 22) == 42

    # 2) 手写 Vector<int> 绑定
    v = fast_cpp.IntVector()
    for i in range(10):
        v.push_back(i)
    assert len(v) == 10
    assert v[0] == 0 and v[9] == 9
    assert v.sum() == 45
    assert v.to_list() == list(range(10))
    assert list(v) == list(range(10))

    # 3) from_list / DoubleVector
    w = fast_cpp.IntVector.from_list(range(100))
    assert w.size() == 100 and w.sum() == 4950

    d = fast_cpp.DoubleVector.from_list([1.5, 2.5, 3.5])
    assert abs(d.sum() - 7.5) < 1e-9

    print("IntVector / DoubleVector 冒烟通过（push_back / [] / len / sum / 迭代 / from_list）")
    print("\n[OK] Python 侧脚手架就绪。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
