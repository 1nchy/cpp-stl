# data-structure_cpp
a rough imitation of @stl

## 现状

目前主要实现了 stl 中的部分类型萃取和部分容器，后者包括数据结构本身以及测试类。

### 类型萃取

实现了 type_traits 头文件中部分功能。

### 容器

已实现的容器包括序列容器和关系容器：

- 序列容器

list  vector  deque

- 关系容器

红黑树（rb_tree）  哈希表（hash_table）  （以及基于上述俩数据结构实现的各种 map 和 set）

> (un)ordered_(multi)map/set

跳表（skip_list）

### 容器测试类

容器测试类包括序列容器测试类和关系容器测试类，注册对应函数后，即可进行控制台式的使用或自动随机测试。

该测试类要求容器**必须**包含 `size` `clear` 两个函数。对于**关系型容器**，在此基础上还要求容器暴露出 `mapped_type` `ireturn_type` 两种类型以及 `insert_status` `ext_iterator` 两个用于展开 `insert` 函数的仿函数类。前两者在我能想到的方案中，都是必要的，用于随机的生成操作以及保存成员函数指针用；后两者在其他方案中是不需要的，之后有空时改一下这里（解释这些类型及仿函数类时说明）。

> mapped_type : 在 map 型容器中为“键值对”中值的类型（不是指传给底层数据结构的pair类型），在 set 型容器中就是键/值的类型。
>
> ireturn_type : 在 unique 容器中为 pair\<iterator, bool\> 类型，在 multi 容器中就是 iterator 类型。
>
> insert_status : 由于 unique 容器和 multi 容器的 `insert` 函数返回类型不一，该仿函数用于获取 `insert` 操作是否成功（对于 u 容器，返回 pair 的第二个值；对于 m 容器，始终返回 true）
>
> ext_iterator : 存在原因同上，用于从返回值中获取迭代器值。
>
> 前面提到的不需要后两者的方案，指测试类（特指关系容器测试类）通过模板类的方法，主动的去获取 `insert` 返回值的头值或尾值。。

容器可以提供一个 `int C::*()const` 类型的成员函数指针，用于检测在操作执行后，容器结构是否被破坏（例如红黑树的黑高度不一致、跳表出现空子链等）。若一切正常应该返回**零**。

容器及容器迭代器都应该**重载输出运算符**。测试类中做的检测 `_HAS_OPERATOR(<<, out)` 有时不太准确，有空排查一下。

容器的键、值，都需要是 `int` 类型。因为这个类型方便生成随机值，如果用字符串等类型不太方便生成随机字符串等对象。

容器测试类将生成一个包含指定个数的操作的序列，该序列中包括单个添加、单个删除和其他操作（清空、查询等），暂不支持多个删除等操作。（主要是没想好怎么设计那个交互方式）

~~~c++
// 容器测试类举例
int main(void) {
    using _Container = asp::ordered_map<int, int>;
    // 序列容器应该使用 debug_seq_container 类
    asp::debug_asso_container<_Container> ad;
    // 手动注册对应函数
    ad._insert = &_Container::insert;
    ad._erase = &_Container::erase;
    ad._count = &_Container::count;
    ad._find = &_Container::find;

    // 容器结构完整性验证，可选的
    ad._check = &_Container::check;

    // ad._circle_count = 10000; // 自动测试初始化序列的操作次数（）

    ad.demo(); // 终端模式
    // ad.auto_test("a 1 d 1"); // 自动化测试（可自定义操作序列）
    return 0;
}
~~~

## 下次一定

- todolist

1. 关系容器自动测试类的 插入状态与迭代器 获取方式 改造

2. 多个元素删除的交互方式（删除有迭代器指定的范围内所有元素）