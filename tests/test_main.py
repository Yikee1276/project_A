"""Python 侧测试 / 测速入口。

运行方式：
    make test            （推荐，会自动激活 .venv）
    bash diagnose.sh     （连 valgrind 一起跑）
    python tests/test_main.py

调用区域留给你填写：验证 C++ 绑定是否可用、做性能对比等。
"""

import sys

import fast_cpp


def main() -> int:
    print(f"Python 版本 : {sys.version.split()[0]}")
    print(f"fast_cpp    : {fast_cpp.__file__}")

    exported = [name for name in dir(fast_cpp) if not name.startswith("_")]
    print(f"已导出符号  : {exported if exported else '(无)'}")

    # ------------------------------------------------------------------
    # TODO: 在这里填写你的调用与性能测试代码，例如：
    #
    #   s = fast_cpp.MyString("hello")
    #   print(s.c_str(), len(s))
    #
    #   v = fast_cpp.IntVector()
    #   for i in range(10_000):
    #       v.push_back(i)
    #   print(len(v))
    #
    #   import timeit
    #   print(timeit.timeit(lambda: fast_cpp.add(1, 2), number=100_000))
    # ------------------------------------------------------------------

    print("\n[OK] Python 侧脚手架就绪（尚未填写调用代码）。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
