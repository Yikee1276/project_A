# 第 3 课完整笔记：原始内存管理（allocator + placement new）

## 目录

- [总对比表格（两个总表）](#tables)
- [1. 为什么 Vector 不能用 new T[n]](#why-not-new-array)
- [2. 声明、定义、初始化](#decl-def-init)
- [3. delete 与析构函数、释放内存的区别](#delete-dtor-free)
- [4. operator 关键字的两种身份](#operator-keyword)
- [5. ::operator new / ::operator delete](#global-operator-new)
- [6. std::allocator<T>](#allocator)
- [7. placement new](#placement-new)
- [8. 四步口诀与对象完整生命周期](#lifecycle)
- [9. allocator 路线与 ::operator new 路线对比](#two-routes)
- [10. 易错点清单](#pitfalls)
- [11. 关键演示代码片段](#code)
- [12. 核心问答概括](#qa-summary)
- [13. 本课检查点](#checkpoints)

---

## <a id="tables"></a >总对比表格

### 表 1：原始内存管理工具总表

| 工具 | 负责什么 | 用法格式 | 头文件 | 返回类型 |
|---|---|---|---|---|
| `new T(...)` | 分配内存 + 构造对象（绑在一起） | `T* p = new T(args);`<br>`T* arr = new T[n];` | 内置 | `T*` |
| `::operator new(n)` | 只分配原始内存（字节级） | `void* raw = ::operator new(n * sizeof(T));` | `<new>` | `void*` |
| `std::allocator<T>::allocate(n)` | 只分配原始内存（类型感知） | `std::allocator<T> alloc;`<br>`T* p = alloc.allocate(n);` | `<memory>` | `T*` |
| placement new：`new (ptr) T(...)` | 只在已有内存上构造对象 | `new (ptr) T(args);`<br>或 `std::construct_at(ptr, args);` | `<new>` / `<memory>` | `T*` |
| `ptr->~T()` | 手动析构对象 | `ptr->~T();`<br>`data_[i].~T();` | 内置 | `void` |
| `::operator delete(ptr)` | 只释放原始内存（字节级） | `::operator delete(raw);` | `<new>` | `void` |
| `allocator<T>::deallocate(ptr, n)` | 只释放原始内存（类型感知） | `alloc.deallocate(ptr, n);` | `<memory>` | `void` |

### 表 2：配对关系表（不能交叉使用）

| 分配 | 释放 | 说明 |
|---|---|---|
| `new T` | `delete p` | 表达式配对，管“内存 + 生命周期” |
| `new T[n]` | `delete[] p` | 数组表达式配对 |
| `::operator new(n)` | `::operator delete(p)` | 字节级函数配对，不析构 |
| `alloc.allocate(n)` | `alloc.deallocate(p, n)` | allocator 配对，`n` 必须一致，不析构 |
| `new (ptr) T(...)` | `ptr->~T()` | placement new 配对手动析构 |

> 交叉使用 = 未定义行为。

---
## <a id="why-not-new-array"></a >1. 为什么 Vector 不能用 new T[n]

`T* data_ = new T[capacity];` 做了两件事：申请内存 + 调用 `capacity` 次默认构造函数。

| 问题 | 说明 |
|---|---|
| 浪费性能 | `size_ = 0` 却提前构造 n 个对象；`T` 复杂时白白分配内部资源 |
| 限制类型 | 要求 `T` 有默认构造函数；只有 `Foo(int, int)` 的类无法存入 |
| 崩溃风险 | `delete[]` 会析构整个数组所有槽位；`size_ < capacity_` 时析构未构造的空地，UB |

结论：`Vector` 必须把“分配内存”和“构造对象”拆开。

---

## <a id="decl-def-init"></a >2. 声明、定义、初始化

| 概念 | 含义 | 例子 |
|---|---|---|
| 声明 Declaration | 说明存在，不给内存 | `extern int a;` |
| 定义 Definition | 给内存 | `int a;` |
| 初始化 Initialization | 获得内存时赋初值 | `int a = 10;` 或 `Foo f(42);` |

`Tracked a(1);` 属于**定义 + 初始化**。

---

## <a id="delete-dtor-free"></a >3. delete 与析构函数、释放内存的区别

`delete p;` 是 delete 表达式，大致等价于：

```cpp
p->~T();              // 1. 调用析构函数
::operator delete(p); // 2. 释放对象自身的内存
```

| 操作 | 负责什么 |
|---|---|
| 析构函数 `~T()` | 只清理对象内部申请的额外资源；不退还对象自身内存 |
| `deallocate` / `operator delete` / `free` | 只退还内存，绝不调用析构函数 |

为什么拆开：`Vector` 有 10 块地皮（`capacity`），只盖 3 栋房子（`size`）。只能手动析构 3 次，最后统一 `deallocate` 退 10 块地皮。

---

## <a id="operator-keyword"></a >4. operator 关键字的两种身份

### 4.1 运算符重载函数名

```cpp
bool operator==(const Foo& a, const Foo& b);
int& operator[](std::size_t i);
void operator()();
operator bool() const;
```

`operator==` 是函数名；调用时仍写 `a == b`。`operator` 不能省。

### 4.2 组成 operator new / operator delete 函数名

```cpp
void* raw = ::operator new(sizeof(Foo));
::operator delete(raw);
```

全局函数名，只管原始内存。`::` 表示全局命名空间。

### 4.3 表达式关键字

```cpp
Foo* p = new Foo(42);   // new 表达式：分配 + 构造
delete p;               // delete 表达式：析构 + 释放
new (ptr) Foo(42);      // placement new 表达式：只在已有内存上构造
```

没有 `::new` 这种写法。

---

## <a id="global-operator-new"></a >5. ::operator new / ::operator delete

| 函数 | 作用 |
|---|---|
| `::operator new(n)` | 申请 n 字节原始内存，返回 `void*`，不构造 |
| `::operator delete(p)` | 释放原始内存，不析构 |

为什么返回 `void*`：

- 全局通用分配函数，只接收字节数，不知道也不绑定具体类型。
- `void*` 表示“未初始化原始存储的通用地址”，此时还不是 `T` 对象。
- 使用前必须 `static_cast<T*>`，再 placement new 构造。

```cpp
void* raw = ::operator new(n * sizeof(T));
T* p = static_cast<T*>(raw);
new (p) T(...); // placement new 表达式
```

对比：`new T` 分配 + 构造，返回 `T*`；`allocator<T>::allocate` 知道 `T`，返回 `T*`。

核心一句话：`::operator new` 返回 `void*`，因为它是只分配原始内存、不构造对象的通用分配函数；类型和构造由使用者后续负责。

---

## <a id="allocator"></a >6. std::allocator<T>

| 成员 | 作用 |
|---|---|
| `alloc.allocate(n)` | 分配 n 个 T 的原始内存，返回 `T*`，不构造 |
| `alloc.deallocate(ptr, n)` | 释放内存，不析构；`n` 必须和 `allocate` 时一致 |


```cpp
std::allocator<Foo> alloc;
Foo* data = alloc.allocate(3); // 只拿内存
// ... placement new 构造 ...
alloc.deallocate(data, 3);     // 只还内存，必须传回 3
```

和 `::operator new` 区别：

| | `::operator new(n)` | `allocator<T>::allocate(n)` |
|---|---|---|
| 分配单位 | 字节 | n 个 T |
| 返回类型 | `void*` | `T*` |
| 是否知道类型 | 不知道 | 知道 T |
| 参数含义 | 总字节数 | 元素个数 |
| 释放 | `::operator delete(ptr)` | `alloc.deallocate(ptr, n)`，必须传回 n |
| 可定制 | 可全局重载 | 容器可接受自定义 allocator |

典型实现大致是 `static_cast<T*>(::operator new(n * sizeof(T)))`，但这是实现近似，不是标准保证。

`Vector` 优先用 allocator：类型安全、语义清楚、支持自定义分配器、和标准库容器接口一致。

---

## <a id="placement-new"></a >7. placement new

```cpp
new (ptr) T(args);
```

在已有内存上原地构造，不分配内存。需要 `<new>`。

```cpp
void* raw = ::operator new(sizeof(Foo));
Foo* p = new (raw) Foo(42); // 只构造

p->~Foo();               // 手动析构
::operator delete(raw);  // 释放裸内存
```

关键规则：

- placement new 构造的对象不能用 `delete` 释放。
- 必须手动 `p->~T()` 析构，再单独释放内存。
- C++17 可用 `std::construct_at(ptr, args)` 替代 placement new。

---

## <a id="lifecycle"></a >8. 四步口诀与对象完整生命周期

```text
allocate   —— 只拿内存（生肉）
construct  —— 只构造（placement new / construct_at）
destroy    —— 只析构（->~T()）
deallocate —— 只还内存
```

allocator 路线核心四行：

```cpp
T* data_ = alloc.allocate(10); // 1. 只拿内存
new (data_ + 0) T("hello");    // 2. 只构造
data_[0].~T();                 // 3. 只析构
alloc.deallocate(data_, 10);   // 4. 只还内存
```

析构函数正确顺序：

```cpp
for (std::size_t i = 0; i < size_; ++i)
    data_[i].~T();              // 先析构已构造的 size_ 个
alloc_.deallocate(data_, N);    // 再还 N 块内存，不是 size_
```

顺序不能反：对象还活着时先还内存，析构函数访问已释放内存是 UB。

---

## <a id="two-routes"></a >9. allocator 路线与 ::operator new 路线对比

| 步骤 | allocator 路线 | ::operator new 路线 |
|---|---|---|
| 拿内存 | `T* p = alloc.allocate(n);` | `void* raw = ::operator new(n * sizeof(T));` |
| 转类型 | 不需要 | `T* p = static_cast<T*>(raw);` |
| 构造 | `new (p + i) T(args);` | `new (p + i) T(args);` |
| 析构 | `p[i].~T();` | `p[i].~T();` |
| 还内存 | `alloc.deallocate(p, n);` | `::operator delete(raw);` |
| 特点 | 类型安全、支持自定义 allocator | 字节级、返回 `void*`、需手动强转 |

两条路线目的相同：**分配、构造、析构、释放四步分离**。

---

## <a id="pitfalls"></a >10. 易错点清单

| 错误写法 | 问题 |
|---|---|
| `delete data_;` 释放 `alloc.allocate` 的内存 | 配对错误，UB |
| `alloc.deallocate(data_, size_)` | 应传 `capacity` / `N`，不是 `size_` |
| `deallocate` 前不析构 | 对象资源泄漏，析构函数没跑 |
| `data_[size_] = value;` 代替 placement new | 在未构造内存上赋值，UB |
| `delete p` 释放 placement new 的对象 | 内存不是 new 表达式分配的，UB |
| 对同一指针 `deallocate` 两次 | 双重释放，UB |
| 模板类成员函数定义放 `.cpp` | 链接错误，应全放 `.h` |
| `::new (p) T(...)` | 语法错误；应写 `new (p) T(...)` |

---

## <a id="code"></a >11. 关键演示代码片段

### 11.1 allocator + placement new 四步

```cpp
std::allocator<Foo> alloc;
Foo* data = alloc.allocate(3);      // 只拿内存

new (data + 0) Foo(1);              // 只构造
new (data + 1) Foo(2);
new (data + 2) Foo(3);

data[0].~Foo();                     // 只析构
data[1].~Foo();
data[2].~Foo();

alloc.deallocate(data, 3);          // 只还内存
```

### 11.2 FixedStack 关键三处

```cpp
FixedStack() : alloc_(), data_(alloc_.allocate(N)), size_(0) {} // 只分配

void push(const T& value) {
    new (data_ + size_) T(value);   // placement new 构造
    ++size_;
}

void pop() {
    --size_;
    data_[size_].~T();              // 手动析构
}

~FixedStack() {
    for (std::size_t i = 0; i < size_; ++i)
        data_[i].~T();              // 只析构 size_ 个
    alloc_.deallocate(data_, N);    // 只还 N 块内存
}
```

### 11.3 ::operator new 路线关键几行

```cpp
void* raw = ::operator new(sizeof(Tracked) * 3); // 只拿内存
Tracked* p = static_cast<Tracked*>(raw);

new (p + 0) Tracked(10);                         // 只构造
new (p + 1) Tracked(20);
new (p + 2) Tracked(30);

p[0].~Tracked();                                 // 手动析构
p[1].~Tracked();
p[2].~Tracked();

::operator delete(raw);                          // 只还内存
```

验收：3 次构造对应 3 次析构；ASan 0 error；编译 0 warning；全文件搜索不到 `new T[N]`、`delete[]`、`delete data_`。

---

## <a id="qa-summary"></a >12. 核心问答概括

**Q1：为什么 `new T[n]` 不能用于 Vector 底层？**  
A：浪费性能（提前构造 n 个对象）、限制类型（要求默认构造）、崩溃风险（`delete[]` 析构未构造的空地，UB）。

**Q2：`delete p` 和析构函数、释放内存什么关系？**  
A：`delete p` 大致等于 `p->~T(); ::operator delete(p);`。析构只清理对象内部资源；`deallocate` / `operator delete` 只还内存，不调用析构。

**Q3：`allocator::allocate` 和 `::operator new` 一样吗？**  
A：目的相似，都只拿原始内存不构造；但 `::operator new(n)` 按字节返回 `void*`，`alloc.allocate(n)` 按元素个数返回 `T*`，释放时必须传回相同 `n`。allocator 支持自定义分配器。

**Q4：`operator` 加了和不加有什么区别？**  
A：`new T` / `delete p` 是表达式，管“内存 + 生命周期”；`::operator new` / `::operator delete` 是函数，只管原始内存。`operator==` 是运算符重载函数名，`==` 是运算符记号。`allocator::allocate` 是普通成员函数，名字里没有 `operator`。

**Q5：`::operator delete` 和 `allocator::deallocate` 有什么区别？**  
A：前者全局字节级，只认指针，配 `::operator new`；后者类型感知成员函数，必须传回元素个数 `n`，配同一个 allocator 的 `allocate`。两者都不析构；不能交叉配对。

**Q6：placement new 是什么？**  
A：`new (ptr) T(args);` 在已有内存上原地构造，不分配内存。必须手动 `ptr->~T()` 析构，不能用 `delete`。C++17 可用 `std::construct_at`。

**Q7：析构函数里为什么先析构再 deallocate？**  
A：对象还活着时不能先还内存，否则析构访问已释放内存是 UB。先循环 `data_[i].~T()` 只析构 `size_` 个，再 `deallocate(data_, N)` 还全部内存。

**Q8：声明、定义、初始化怎么区分？**  
A：声明 `extern int a;` 说明存在；定义 `int a;` 给内存；初始化 `int a = 10;` 获得内存时赋初值。`Tracked a(1);` 是定义 + 初始化。

**Q9：为什么 `::operator new` 返回 `void*`？**  
A：它是全局通用分配函数，只接收字节数，不绑定类型。`void*` 表示未初始化原始存储地址；使用前需 `static_cast<T*>`，再 placement new 构造。对比 `allocator<T>::allocate` 知道 `T`，返回 `T*`。注意不要写 `::new (p) T(...)`，正确是 `new (p) T(...)`。

**Q10：模板类成员函数为什么放头文件？**  
A：编译器在实例化时才生成代码；定义在 `.cpp` 时其他翻译单元只看到声明，链接时找不到实例化版本，导致 undefined reference。

**Q11：.h / .cpp 怎么命名？**  
A：练习用 `lessonN.cpp`；项目用 `my_vector.h` / `my_vector.cpp`，全小写 + 下划线，避免 `vector.h` 与标准库混淆。头文件保护用 `#pragma once`。模板实现全放头文件。

**Q12：关于 `::operator new` 返回 `void*` 的回答修正**  
A：回答基本正确，但需注意：不要写 `::new (p) T(...)`，正确写法是 `new (p) T(...)`；可补充 `void*` 与 `char*` 的区别（`void*` 表示未指定类型的原始地址，`char*` 会误导为字符内存），以及 `allocator<T>::allocate` 因知道 `T` 而直接返回 `T*` 的对比。核心一句话：`::operator new` 返回 `void*`，因为它是只分配原始内存、不构造对象的通用分配函数；类型和构造由使用者后续负责。

---

## <a id="checkpoints"></a >13. 本课检查点

- [ ] 能说清为什么 `new T[n]` 不能用于 `Vector` 底层（三个致命问题）。
- [ ] 知道 `new` / `delete` 是表达式，管“内存 + 生命周期”。
- [ ] 知道 `::operator new` / `::operator delete` 是函数，只管原始内存。
- [ ] 知道 `allocator::allocate` 只拿内存不构造，且 `deallocate` 必须传回相同数量。
- [ ] 知道 placement new 只在已有内存上构造。
- [ ] 知道通过 placement new 构造的对象必须手动 `->~T()` 析构。
- [ ] 知道 `operator` 既是运算符重载函数名的组成部分，也是 `operator new` / `operator delete` 函数名的组成部分。
- [ ] 知道 `::operator new` 返回 `void*` 的原因，以及和 `allocator<T>::allocate` 返回 `T*` 的区别。
- [ ] 练习跑通，3 次构造对应 3 次析构，ASan 报告干净，编译 0 warning。