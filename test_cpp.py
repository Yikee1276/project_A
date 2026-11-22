import sys
import os

# 1. 把 .pyd 文件所在的 build 目录加入到 Python 的搜索路径中
# 这样 Python 才能找到我们刚刚编译的库
build_path = os.path.join(os.getcwd(), "include", "cpp_ext", "build")
sys.path.append(build_path)

# 2. 直接 import 你的 C++ 模块！
# 注意：模块名就是 CMakeLists.txt 里定义的名字，不需要加 .pyd 后缀
import my_cpp_lib

# 3. 调用 C++ 里的函数（假设你在 hello.cpp 里写了 add 函数）
result = my_cpp_lib.hello("World")
print(f"C++ 计算结果: {result}")

# 4. 看看 C++ 模块里还有哪些东西
print("模块包含的内容:", dir(my_cpp_lib))