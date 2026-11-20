#  手写 VectorBug 排查与修复笔记

## 一、 核心 Bug 诊断与修复

### 1. 拷贝赋值运算符（最严重，直接段错误）
*   **Bug 1：指针与引用比较语法错误**
    *    **错误写法**：`if (this != other)` （`this` 是指针，`other` 是引用，两者不能直接比较）。
    *    **正确写法**：`if (this != &other)` （必须对 `other` 取地址）。
*   **Bug 2：析构函数调用缺少括号**
    *    **错误写法**：`data_[i].~T;`
    *    **正确写法**：`data_[i].~T();` （显式调用析构函数必须带括号）。
*   **Bug 3（致命）：漏写重新分配内存（`allocate`）**
    *    **错误后果**：在释放旧内存后，没有为新数据分配内存，直接往悬空指针 `data_` 上执行 `placement new`，程序必崩。
    *    **正确写法**：在深拷贝循环前，必须先执行 `data_ = alloc_.allocate(capacity_);`。

### 2. `const` 修饰完全失效（违背只读契约）
*   **Bug 4：`const` 迭代器返回非 `const` 指针**
    *    **错误写法**：`T* begin() const { return data_; }` （外部可通过返回的指针修改 `const` 对象的内部数据）。
    *    **正确写法**：`const T* begin() const { return data_; }`。
*   **Bug 5：`const` 下标访问返回非 `const` 引用**
    *    **错误写法**：`T& operator[](size_t index) const { return data_[index]; }`
    *    **正确写法**：`const T& operator[](size_t index) const { return data_[index]; }`。

### 3. 移动赋值运算符语法错误
*   **Bug 6：指针与引用比较语法错误**
    *    **错误写法**：`if (this != other)`
    *    **正确写法**：`if (this != &other)` （同 Bug 1）。

### 4. 测试代码的“终极陷阱”（异常安全与退化机制）
*   **Bug 7：禁用拷贝/移动导致扩容编译失败**
    *    **错误场景**：如果 `Foo` 类同时禁用了拷贝构造和移动构造（`= delete`），当 `emplace_back` 触发 `reserve` 扩容时，内部的 `std::move_if_noexcept` 找不到移动构造，会退化为拷贝构造，进而导致编译报错（无路可走）。
    *    **正确验证策略**：将 `Foo` 的拷贝/移动构造函数改为**计数器模式**（统计 `copy_count` 和 `move_count`），既允许触发扩容，又能精准验证 `emplace_back` 的 0 拷贝特性。

---

## 二、 命名规范化修正

*   `push` ️ `push_back` （对齐标准库命名规范）。
*   `emplace` ️ `emplace_back` （对齐标准库命名规范）。

---

## 三、 验证预期（计数器测试）

在使用带计数器的 `Foo` 类触发扩容测试时：
*   **Copies 预期值：0** （`emplace_back` 原地构造，且移动构造函数标记了 `noexcept`，`std::move_if_noexcept` 优先触发移动，绝不拷贝）。
*   **Moves 预期值：1** （插入第 1 个元素不触发扩容，0次移动；插入第 2 个元素触发扩容，旧的第 1 个元素被移动到新内存，计 1 次移动；第 2 个元素为原地构造，不计移动）。