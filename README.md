# Build Your Own Lisp (Byol) - Implementation & DevLog

这是一个基于 C 语言实现的 Lisp 解释器。

本项目严格遵循 [Build Your Own Lisp](http://www.buildyourownlisp.com/) 教程进行学习与开发，并在此基础上完成了 **Chapter 4 至 Chapter 15 的所有 Bonus Marks** 挑战。

## 📂 项目结构说明 (Project Structure)

### 1. 目录说明 (Directories)

*   **`chapter/`**: 存放教程各章节的原始源代码 (`chapterX.c`)，作为学习过程的历史存档与参考。
*   **`test_function/`**: 存放测试相关的代码与资源。
    *   包含测试用的 Lisp 脚本（如 `hello.lspy`, `test_tco.lspy`）。
    *   包含测试编译生成的执行文件（如 `test_parser`, `test_parser_pool`）。

### 2. 核心文件说明 (Core Files)

以下是项目根目录下所有源文件及文档的详细功能说明：

#### 文档 (Documentation)
*   **`README.md`**: 项目主文档，包含开发日志、文件结构与功能说明。
*   **`STANDARD_LIBRARY.md`**: **标准库文档**。详细记录了 `prelude.lspy` 中定义的标准库函数（如 `map`, `filter`, `fold` 等）的用法与示例。

#### 解释器入口与核心逻辑 (Interpreter Core)
*   **`parsing.c`**: **主程序入口 (Main)**。实现了交互式编程环境 (REPL)，负责读取用户输入、调用解析器处理、并输出求值结果。
*   **`parser.c`**: **[核心组件] 手写解析器**。替代了教程原有的 `mpc` 库，实现了递归下降解析器，负责将源代码文本转换为抽象语法树 (AST)。
*   **`lval.c`**: **数据结构与求值**。
    *   定义了 Lisp Value (`lval`) 结构体（支持数字、符号、函数、S-Expr 等类型）。
    *   实现了核心求值函数 `lval_eval`。
    *   包含了自定义的字符串转义/反转义函数 (`lval_str_escape`)。
*   **`lenv.c`**: **环境管理**。实现了变量作用域 (`lenv`)，支持变量的绑定 (`put`)、查找 (`get`) 以及父环境的嵌套查找。
*   **`builtins.c`**: **内置函数库**。实现了 Lisp 的基础内置函数，包括：
    *   数学运算 (`+`, `-`, `*`, `/`)
    *   列表操作 (`head`, `tail`, `list`, `join`, `cons`...)
    *   逻辑比较 (`if`, `==`, `>`, `or`, `and`...)
    *   I/O 操作 (`print`, `load`, `read`)

#### 内存管理与工具 (Memory & Utils)
*   **`pool.c` / `pool.h`**: **[优化组件] 内存池**。实现了基于空闲链表 (Free List) 的内存池，用于高效分配和回收 `lval` 对象，替代系统频繁的 `malloc/free`，并提供内存使用统计日志。
*   **`vec.c`**: **动态数组**。一个简单的通用动态数组实现，作为辅助数据结构使用。
*   **`file_function.c`**: **文件操作**。封装了文件读取与写入相关的内置函数 (`fopen`, `fread`, `fwrite` 等)。

#### 配置与错误处理 (Config & Error)
*   **`config.h`**: **全局配置**。包含所有核心结构体的类型定义、函数前置声明（解决循环依赖）以及全局宏定义。
*   **`error.h`**: **错误处理**。定义了 `LASSERT` 等宏，用于简化参数检查和错误报告逻辑。

#### 测试与遗留文件 (Test & Legacy)
*   **`test_parser_main.c` / `test_parser_utils.c`**: **测试代码**。用于测试解析器和内存池功能的独立测试源文件。
*   **`mpc.c` / `mpc.h`**: **遗留依赖**。教程最初使用的组合子解析库。虽然本项目核心已迁移至手写解析器 (`parser.c`)，但文件仍保留以供参考或对比。

---

## 🛠️ 开发日志：遇到的问题与解决方案 (Challenges & Solutions)

在开发过程中，我们从最初依赖 `mpc` 库，逐步演进到完全手写核心组件，并进行了深度优化。

### 1. 移除 `mpc` 依赖 (Removing mpc Dependency)
*   **问题**: `mpc` 是一个强大的组合子解析库，但为了更深入理解，我们需要移除它。
*   **解决**: 实现了一个手写的**递归下降解析器** (`parser.c`)。
    *   **难点**: 处理字符串转义和 S-Expression 的嵌套结构。
    *   **实现**: 编写了 `Tokenizer` 进行词法分析，随后通过 `parse_sexpr` 和 `parse_atom` 递归构建抽象语法树 (AST)。

### 2. 内存管理优化 (Memory Optimization)
*   **问题**: 解释器运行中会创建大量临时的 `lval` 对象，频繁调用 `malloc` 和 `free` 导致性能开销大且容易产生碎片。
*   **解决**: 引入**对象池 (Object Pool)** (`pool.c`)。
    *   预先分配一大块内存，使用**空闲链表 (Free List)** 管理。
    *   `lval_alloc` 变为 O(1) 操作，极大地提升了分配速度。
    *   增加了内存日志功能 (`lval_pool_dump_log`)，可实时监控对象存活数量。

### 3. 栈溢出与尾调用优化 (Stack Overflow & TCO)
*   **问题**: 在递归计算（如长列表处理或递归函数）时，C 语言的调用栈容易溢出。
*   **解决**:
    *   **迭代求值**: 将 `lval_eval` 重构为 `while` 循环结构，不再依赖 C 函数递归。
    *   **环境压缩**: 实现了环境路径压缩 (Path Compression)，减少环境查找链的长度。
    *   **TCO**: 对尾调用进行了特殊处理，复用当前栈帧。

### 4. 单元素求值 Bug (Evaluation Bug)
*   **问题**: 输入 `(exit)` 无法退出，REPL 仅显示 `<builtin>`。
*   **原因**: 为了优化 `(5)` -> `5` 的情况，代码中加入了 `if (count == 1) return take(0)`，但这错误地把无参数的函数调用也直接返回了函数本身，而没有执行它。
*   **解决**: 修正优化逻辑，仅当单元素**不是函数** (`LVAL_FUN`) 时才直接返回。

### 5. 字符串转义 (String Escaping)
*   **问题**: 移除 `mpc` 后，丢失了 `mpcf_escape` 等辅助函数，导致字符串打印和读取异常。
*   **解决**: 在 `lval.c` 中手写了 `lval_str_escape` 和 `lval_str_unescape`，支持常见的转义字符 (`\n`, `\t`, `\"`, `\\` 等)。

## ⚠️ 当前局限性 (Limitations)

1.  **错误报告**: 目前的解析器 (`parser.c`) 在遇到语法错误时，报错信息较为简略（例如 "Unexpected token"），不如 `mpc` 提供的详细。
2.  **垃圾回收 (GC)**: 目前使用引用计数 (Reference Counting) 和手动管理 (`lval_del`)。虽然有内存池，但对于循环引用的结构无法自动回收。
3.  **类型系统**: 类型检查是在运行时动态进行的，对于复杂的类型错误，只有在执行到那一行时才会发现。

## 🚀 未来工作 (Future Work)

*   **实现标记-清除 (Mark-and-Sweep) GC**: 彻底解决循环引用导致的内存泄漏问题。
*   **增强 Parser**: 提供更友好的语法错误提示，支持行号定位。
*   **宏系统 (Macros)**: 引入宏，允许用户在不求值参数的情况下操作代码结构，从而在语言层面扩展语法（如实现 `defun` 等语法糖）。
*   **标准库扩充**: 增加更多实用的列表处理和数学函数。
