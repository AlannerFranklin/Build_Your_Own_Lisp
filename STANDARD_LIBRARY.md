# Lispy Standard Library Documentation | Lispy 标准库文档

This document provides documentation for the Lispy standard library (`prelude.lspy`), including function descriptions, usage examples, and test cases.
本文档提供 Lispy 标准库 (`prelude.lspy`) 的说明，包含函数描述、使用示例和测试用例。

## Core Functions | 核心函数

### Functional Helpers | 函数式编程辅助工具

#### `fun {name args body}`
Defines a new function.
定义一个新函数。
- **Example**: `fun {add x y} {+ x y}`

#### `unpack {f l}`
Takes a function `f` and a list `l`, and applies the function to the list contents.
接受一个函数 `f` 和一个列表 `l`，将函数应用于列表内容。
- **Example**: `unpack + {1 2}` -> `3`

#### `pack {f & xs}`
Takes a function `f` and a variable number of arguments, packing them into a list before calling `f`.
接受一个函数 `f` 和可变数量的参数，在调用 `f` 之前将它们打包成一个列表。
- **Example**: `pack head 1 2 3` -> `1`

#### `curry {f l}` / `uncurry {f & xs}`
Aliases for `unpack` and `pack`.
`unpack` 和 `pack` 的别名。

#### `do {& l}`
Evaluates a list of expressions and returns the result of the last one.
评估一系列表达式，并返回最后一个表达式的结果。
- **Example**: `do (print "hello") (print "world")`

### Logical Functions | 逻辑函数

#### `not {x}`
Logical negation. Returns 1 if x is 0, else 0.
逻辑非。如果 x 为 0 返回 1，否则返回 0。
- **Example**: `not true` -> `0`

#### `or {x y}`
Logical OR.
逻辑或。
- **Example**: `or true false` -> `1`

#### `and {x y}`
Logical AND.
逻辑与。
- **Example**: `and true false` -> `0`

### List Functions | 列表操作函数

#### `fst {l}`, `snd {l}`, `trd {l}`
Returns the first, second, or third element of a list.
返回列表的第一、第二或第三个元素。
- **Example**: `fst {1 2 3}` -> `1`

#### `len {l}`
Returns the length of a list. Implemented using `foldl`.
返回列表长度。使用 `foldl` 实现。
- **Example**: `len {1 2 3 4}` -> `4`

#### `nth {n l}`
Returns the Nth element of a list (0-based).
返回列表的第 N 个元素（从 0 开始计数）。
- **Example**: `nth 1 {10 20 30}` -> `20`

#### `last {l}`
Returns the last element of a list.
返回列表的最后一个元素。
- **Example**: `last {1 2 3}` -> `3`

#### `map {f l}`
Applies function `f` to every element in list `l`.
将函数 `f` 应用于列表 `l` 中的每个元素。
- **Example**: `map (\ {x} {* x 2}) {1 2 3}` -> `{2 4 6}`

#### `filter {f l}`
Returns a new list containing only elements from `l` where `f` returns true.
返回一个新列表，仅包含 `l` 中使 `f` 返回 true 的元素。
- **Example**: `filter (\ {x} {> x 2}) {1 2 3 4}` -> `{3 4}`

#### `reverse {l}`
Reverses a list.
反转列表。
- **Example**: `reverse {1 2 3}` -> `{3 2 1}`

#### `foldl {f z l}`
Fold Left. Applies function `f` to accumulator `z` and each element of `l` from left to right.
左折叠。将函数 `f` 应用于累加器 `z` 和 `l` 的每个元素（从左到右）。
- **Example**: `foldl + 0 {1 2 3}` -> `6`

#### `elem {x l}`
Checks if element `x` is in list `l`. Implemented using `foldl`.
检查元素 `x` 是否在列表 `l` 中。使用 `foldl` 实现。
- **Example**: `elem 2 {1 2 3}` -> `1`

## Example Programs | 示例程序

### 1. Fibonacci Sequence | 斐波那契数列
Generating the first N Fibonacci numbers.
生成前 N 个斐波那契数。

```lisp
fun {fib n} {
  select
    { (== n 0) 0 }
    { (== n 1) 1 }
    { otherwise (+ (fib (- n 1)) (fib (- n 2))) }
}

print (map fib {0 1 2 3 4 5 6 7 8})
; Output: {0 1 1 2 3 5 8 13 21}
```

### 2. Custom Filter Logic | 自定义过滤逻辑
Filtering even numbers from a list.
过滤列表中的偶数。

```lisp
fun {is-even x} { == (% x 2) 0 }
print (filter is-even {1 2 3 4 5 6})
; Note: Need to implement % (modulo) operator or use subtraction logic
; 注意：需要实现 % (取模) 运算符或使用减法逻辑
```

## Test Cases | 测试用例

Run these in the Lispy REPL to verify functionality.
在 Lispy REPL 中运行这些代码以验证功能。

### Test `len`
```lisp
if (== (len {}) 0) {print "len test 1 passed"} {print "len test 1 failed"}
if (== (len {1}) 1) {print "len test 2 passed"} {print "len test 2 failed"}
if (== (len {1 2 3 4 5}) 5) {print "len test 3 passed"} {print "len test 3 failed"}
```

### Test `elem`
```lisp
if (== (elem 1 {1 2 3}) true) {print "elem test 1 passed"} {print "elem test 1 failed"}
if (== (elem 5 {1 2 3}) false) {print "elem test 2 passed"} {print "elem test 2 failed"}
if (== (elem 1 {}) false) {print "elem test 3 passed"} {print "elem test 3 failed"}
```

### Test `map`
```lisp
if (== (map (\ {x} {+ x 1}) {1 2 3}) {2 3 4}) {print "map test passed"} {print "map test failed"}
```

### Test `foldl`
```lisp
if (== (foldl + 0 {1 2 3}) 6) {print "foldl test passed"} {print "foldl test failed"}
```
