# project_A —— C++ / Python 混合开发工程

手写 C++ 容器类（`MyString` / `MyVector`），通过 pybind11 暴露给 Python 调用，
并用 valgrind 做内存正确性检查。

---

## 一、工程结构

```
project_A/
├── include/                  # 【你手写】C++ 头文件
│   ├── my_string.h           #   MyString 类声明
│   └── my_vector.hpp         #   MyVector 模板类（模板实现须写在头文件里）
├── src/                      # 【你手写】C++ 实现 + 绑定层
│   ├── my_string.cpp         #   MyString 实现
│   ├── my_vector.cpp         #   （可选）非模板的辅助实现
│   └── bindings.cpp          #   绑定骨架，TODO 区留给你暴露类给 Python
├── scripts/
│   └── CMakeLists.txt        # 构建配置：自动收录 src/*.cpp（不含 bindings.cpp）
├── tests/
│   ├── test_cpp.cpp          # valgrind 专用 C++ 测试入口（main 留空待填）
│   └── test_main.py          # Python 调用 / 测速入口（调用区留空待填）
├── build/                    # 构建产物（已被 .gitignore 忽略）
├── Makefile                  # 工程化入口：build / test / valgrind / clean
├── diagnose.sh               # 一键诊断：编译 + valgrind + Python 测试
└── .venv/                    # Python 虚拟环境（pybind11 / matplotlib 已装好）
```

---

## 二、构建系统如何工作

`scripts/CMakeLists.txt` 用 `file(GLOB ... CONFIGURE_DEPENDS "src/*.cpp")` 自动收集源文件：

- `src/bindings.cpp` 单独显式加入（它是绑定层，必须存在）；
- `src/` 下**其它任何 `.cpp` 都会被自动纳入编译**，新增文件不需要改 CMakeLists；
- `CONFIGURE_DEPENDS` 让 `make` 在检测到新增/删除源文件时自动重跑 cmake；
- 模板类（如 `MyVector<T>`）请把实现放在 `include/*.hpp` 里，否则链接会失败。

Python 模块名为 `fast_cpp`，产物输出到 `build/`：
`build/fast_cpp.cpython-314-x86_64-linux-gnu.so`。

> 模块路径已通过 `.venv/lib/python3.14/site-packages/fast_cpp.pth` 加入 `sys.path`，
> 因此在项目任意目录下 `import fast_cpp` 都能直接用，无需设置 `PYTHONPATH`。

---

## 三、Makefile 用法

| 命令 | 作用 |
|---|---|
| `make` / `make help` | 列出所有目标 |
| `make build` | 增量编译；若 `build/Makefile` 不存在（刚 clean 过）会自动先执行 `cmake ../scripts` |
| `make configure` | 仅重新执行 cmake 配置 |
| `make test` | 激活 `.venv` 并运行 `python tests/test_main.py` |
| `make valgrind` | 编译 `tests/test_cpp.cpp`（`-g`）并用 valgrind 检测内存 |
| `make diagnose` | 一键诊断 = valgrind + Python 测试（等价于 `bash diagnose.sh`） |
| `make clean` | 清理 `build/` 下的构建产物 |

典型日常流程：

```bash
cd /home/CH/workspace/project_A
source .venv/bin/activate

# 改完 include/ 或 src/ 里的代码后
make build

# 跑 Python 侧验证
make test
```

---

## 四、内存检查（valgrind）

C++ 侧测试写在 `tests/test_cpp.cpp` 的 `main()` 中。运行：

```bash
make valgrind          # 等价于 bash diagnose.sh --only valgrind
```

它实际执行（测试文件会与 `src/*.cpp` 的实现一起编译，`bindings.cpp` 除外）：

```bash
g++ -std=c++17 -g -O0 -Wall -Wextra -Iinclude \
    -o build/test_cpp tests/test_cpp.cpp src/my_string.cpp
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
         --error-exitcode=99 build/test_cpp
```

`--error-exitcode=99` 很关键：valgrind 默认**总是返回 0**，不加这个参数会把泄漏误判成"通过"。
脚本据此给出结论：退出码 `0` = 干净；`99` = 有泄漏或非法访问；其它 = 程序自身崩溃。

完整输出会同时写入 `build/valgrind.log`，方便对照行号排查。

读报告的关键段落：

| 段落 | 含义 | 期望结果 |
|---|---|---|
| `All heap blocks were freed -- no leaks are possible` | 全部释放 | ✅ 目标 |
| `definitely lost` | 确定泄漏（典型：`new` 没有对应 `delete`） | 必须为 0 |
| `indirectly lost` | 因上面泄漏而连带丢失 | 必须为 0 |
| `Invalid read/write of size N` | 越界或 use-after-free | 必须为 0 |
| `Conditional jump depends on uninitialised value` | 未初始化内存 | 尽量为 0 |

清理 `MyString` 深拷贝、`MyVector` 扩容（`new[]` 旧缓冲区未 `delete[]`）这类问题，
valgrind 会直接指到源码行。

---

## 五、一键诊断脚本

```bash
bash diagnose.sh                   # 全流程：编译 + valgrind + Python 测试
bash diagnose.sh --only valgrind   # 只做 C++ 内存检查
bash diagnose.sh --only python     # 只做 Python 测试
```

脚本设计为"任一步失败也继续跑后续步骤"，最后统一汇总；退出码非 0 表示有问题。

---

## 六、当前状态说明（重要）

- `include/my_vector.hpp` **尚未创建**。在它出现之前，`make valgrind` / `bash diagnose.sh`
  会在第一步报 `fatal error: my_vector.hpp: No such file or directory` —— 这是预期行为，
  等你手写完该头文件后自动恢复。
- `src/bindings.cpp` 目前只有模块骨架和 `add()` 冒烟函数，业务类绑定区已用 `TODO` 标出。
- `make build` 与 `make test` 现在是通的（绑定层不含任何手写类，因此不依赖未完成的代码）。
- 若需要临时移除模块搜索路径：`rm .venv/lib/python3.14/site-packages/fast_cpp.pth`。
