# data-structure_cpp
a rough imitation of @stl

目前主要实现了 stl 中的部分类型萃取和部分容器，后者包括数据结构本身以及测试类。

已实现的容器包括序列容器和关系容器：

- 序列容器

list  vector  deque

- 关系容器

红黑树  哈希表  （以及基于上述俩数据结构实现的各种 map 和 set）

容器测试类包括序列容器测试类和关系容器测试类，注册对应函数后，即可进行控制台式的使用或自动随机测试。

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

    ad.demo(); // 终端模式
    // ad.auto_test("a 1 d 1"); // 自动化测试（可自定义操作序列）
    return 0;
}
~~~