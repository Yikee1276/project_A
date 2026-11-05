<a name="top"></a >
# 🧠 Day 2 笔记：移动语义（简洁版）

## 目录

- [1. 左值、右值、右值引用](#sec1)
  - [1.1 左值与右值](#sec1-1)
  - [1.2 引用就是别名](#sec1-2)
  - [1.3 三种引用绑定规则](#sec1-3)
- [2. `std::move`](#sec2)
- [3. 三种构造函数](#sec3)
- [4. 移动构造函数](#sec4)
- [5. 移动赋值运算符](#sec5)
- [6. Copy-and-Swap](#sec6)
- [7. `print()` 与测试](#sec7)
- [8. 内存检测](#sec8)
- [9. 核心总结](#sec9)
- [附：问答概括](#sec10)

---

<a name="sec1"></a >
## 1. 左值、右值、右值引用

<a name="sec1-1"></a >
### 1.1 左值与右值

- **左值**：有名字、能取地址，如 `a`。
- **右值**：临时值、通常不能取地址，如 `10`、`a + 20`。

<a name="sec1-2"></a >
### 1.2 引用就是别名

引用是已有对象的别名，必须初始化，不能改绑。

```cpp
int a = 10;
int& r = a;   // r 是 a 的别名
r = 20;       // a 也变 20
```

<a name="sec1-3"></a >
### 1.3 三种引用绑定规则

| 引用类型 | 能否修改 | 绑定谁 | 原因 |
| :--- | :--- | :--- | :--- |
| `T&` | 能改 | 只能左值 | 改临时值不安全 |
| `const T&` | 只读 | 左值 + 右值 | 不改，临时值寿命被延长 |
| `T&&` | 可接管资源 | 只能右值 | 防止偷走还要用的左值 |

> 口诀：能改只绑左值；只读两边都绑；偷资源只绑右值。

[⬆️ 返回顶部](#top)

---

<a name="sec2"></a >
## 2. `std::move`

`std::move` **不移动资源**，只把左值转成右值引用，授权调用移动构造/移动赋值。

```cpp
MyString a("hello");
MyString b(std::move(a));   // 调用移动构造
// 之后 a 的指针被置空，不要假设它还是 "hello"
```

本质近似：`static_cast<MyString&&>(a)`。

[⬆️ 返回顶部](#top)

---

<a name="sec3"></a >
## 3. 三种构造函数

```cpp
class MyString {
public:
    MyString(const char* str);              // 带参构造
    MyString(const MyString& other);        // 拷贝构造
    MyString(MyString&& other) noexcept;    // 移动构造
};
```

| 对比项 | 带参构造 | 拷贝构造 | 移动构造 |
| :--- | :--- | :--- | :--- |
| 参数 | `const char*` | `const MyString&` | `MyString&&` |
| 参数含义 | 原始数据 | 同类左值 | 同类右值 |
| 典型调用 | `MyString a("hi");` | `MyString b(a);` | `MyString c(std::move(a));` |
| 资源处理 | 新申请 + 拷贝 | 新申请 + 拷贝 | 直接接管指针 |
| 原对象 | 不涉及 | 不变 | 被置空 |
| 风险 | 空指针 | 浅拷贝 double free | 忘记置空 double free |

> `MyString c = a;` 是新定义对象，调用**拷贝构造**，不是赋值。

[⬆️ 返回顶部](#top)

---

<a name="sec4"></a >
## 4. 移动构造函数

```cpp
MyString::MyString(MyString&& other) noexcept {
    data = other.data;        // 接管指针
    size = other.size;        // 接管大小
    other.data = nullptr;     // 对方放弃所有权
    other.size = 0;
}
```

- 必须置空对方，否则双方析构同一块内存 → `double free`。
- 移动构造里**不能** `delete[] data;`，此时 `this->data` 未初始化。

[⬆️ 返回顶部](#top)

---

<a name="sec5"></a >
## 5. 移动赋值运算符

| 对比项 | 移动构造 | 移动赋值 |
| :--- | :--- | :--- |
| 时机 | 创建新对象 | 给已有对象赋值 |
| 自己有无旧资源 | 没有 | 有 |
| 是否释放旧资源 | 不要 | 必须 `delete[] data` |
| 是否检查自赋值 | 不需要 | 需要 `if (this != &other)` |
| 对方置空 | 要 | 要 |
| 返回值 | 无 | `*this` |

```cpp
MyString& MyString::operator=(MyString&& other) noexcept {
    if (this != &other) {
        delete[] data;          // 释放自己的旧内存
        data = other.data;      // 接管对方资源
        size = other.size;
        other.data = nullptr;   // 对方放弃所有权
        other.size = 0;
    }
    return *this;
}
```

关键：
- `delete[] data` 释放的是**指向的内存**；`data = nullptr` 改的是**指针本身**。
- 移动后对方是 `nullptr`，不是悬空指针。
- 不检查自赋值时 `c = std::move(c);` 会释放后读到野指针。

[⬆️ 返回顶部](#top)

---

<a name="sec6"></a >
## 6. Copy-and-Swap

```cpp
MyString& operator=(MyString other) {   // 按值传递
    std::swap(data, other.data);
    std::swap(size, other.size);
    return *this;
}
```

| 传入 | `other` 如何构造 | 等价于 |
| :--- | :--- | :--- |
| 左值 `a` | 拷贝构造 | 拷贝赋值 |
| 右值 `std::move(a)` | 移动构造 | 移动赋值 |

- 参数必须按值，不能写 `MyString& other`，否则 `other` 是调用者别名，交换会改坏原对象且旧资源泄漏。
- 函数结束时 `other` 析构，自动释放旧资源。
- 自赋值安全，不需要 `if (this != &other)`。

[⬆️ 返回顶部](#top)

---

<a name="sec7"></a >
## 7. `print()` 与测试

```cpp
void print() const {
    if (data != nullptr) std::cout << data << std::endl;
    else std::cout << "(null)" << std::endl;
}
```

测试覆盖：
1. 带参构造
2. 拷贝构造（原对象不变）
3. 移动构造（原对象变 `(null)`）
4. 拷贝赋值（原对象不变）
5. 移动赋值（原对象变 `(null)`）
6. 自赋值 `b = b` 不崩溃
7. 程序结束不崩溃

[⬆️ 返回顶部](#top)

---

<a name="sec8"></a >
## 8. 内存检测

```bash
g++ -std=c++11 -g -O0 -Wall -Wextra main.cpp -o main_vg
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main_vg
```

目标：
- `ERROR SUMMARY: 0 errors`
- `definitely lost: 0 bytes`
- 无 `double-free`、`Invalid free`

[⬆️ 返回顶部](#top)

---

<a name="sec9"></a >
## 9. 核心总结

1. `T&` 绑左值，`const T&` 都绑，`T&&` 只绑右值。
2. `std::move` 只转类型，不移动资源。
3. 移动构造：从无到有，接管指针，对方置空，不 delete 自己的。
4. 移动赋值：旧换新，先 delete 自己的旧内存，再接管，检查自赋值。
5. 悬空指针指向已释放内存；`nullptr` 不指向任何内存。
6. Copy-and-Swap：参数按值，交换资源，副本析构释放旧资源，一个函数覆盖拷贝和移动。
7. 检测目标：0 errors，0 definitely lost，无 double-free。

[⬆️ 返回顶部](#top)

---

<a name="sec10"></a >
## 附：问答概括

| 问题 | 概括 |
| :--- | :--- |
| 引用和别名一样吗？ | 基本是别名，但必须初始化、不能改绑、不是独立变量。 |
| 三种引用为什么绑定规则不同？ | `T&` 要改只能绑左值；`const T&` 只读且延长临时值寿命；`T&&` 用于偷资源，只能绑右值。 |
| 拷贝构造和带参构造区别？ | 带参构造用原始数据；拷贝构造用同类对象。`MyString c = a;` 调拷贝构造。 |
| 移动构造和移动赋值区别？ | 构造从无到有只接管；赋值旧换新，先释放旧内存再接管，并检查自赋值。 |
| 移动赋值为什么不能 delete 原 data？ | 必须 delete 自己的旧 data；不能 delete 对方的 data；移动构造不能 delete 未初始化的 data。 |
| 接管后对方是悬空指针吗？ | 置空前是别名，有 double free 风险；置空后是 `nullptr`，不是悬空指针。 |
| delete 与置空分别操作什么？ | `delete[]` 释放指向的内存；`= nullptr` 改指针本身。标准写法两步。 |
| `a.print()` 是什么？ | 调用对象 `a` 的成员函数 `print()`。 |
| `operator<<` 的 `os` 可以不学吗？ | 可以，用 `print()` 替代即可。 |
| 一共几步？能在 VSCode 做吗？ | 共 7 步；VSCode 只是编辑器，用终端 `g++` 编译运行即可。 |
| 给出移动构造和移动赋值代码 | 见第 4、5 节代码。 |

[⬆️ 返回顶部](#top)
