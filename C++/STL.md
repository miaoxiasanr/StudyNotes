- [STL](#stl)
  - [基本知识](#基本知识)
  - [STL六大组件](#stl六大组件)
  - [STL容器](#stl容器)
    - [vector](#vector)
    - [list](#list)
    - [deque](#deque)
    - [stack](#stack)
    - [queue](#queue)
    - [string](#string)
    - [map/multimap](#mapmultimap)
    - [set/multiset](#setmultiset)
    - [unordered\_set/unordered\_map](#unordered_setunordered_map)
  - [各种容器的特点和适用情况](#各种容器的特点和适用情况)
  - [常见面试题](#常见面试题)
  - [引用](#引用)

# STL
## 基本知识
STL(Standard temptlate library) 标椎模板库 里面含有大量的模板类和模板函数，是C++提供的一个基础模板的集合。
## STL六大组件
* 容器(Container):STL容器为各种数据结构，如vector,stack,queue,mao,set等用来存放数据，从实现角度来看，STL容器是一种class Template   容器在内存中的空间不一定是连续的；
* 算法(Algorithm)：STL的算法多数定义在<algorithm>头文件中，其中包括了各种常用的算法，如sort,find,copy,reverse等，STL算法是一种function template
* 迭代器(Iterator)：STL迭代器扮演了容器和算法之间的胶合剂，泛化的指针，
* 仿函数(Functor):行为类似函数，可作为算法的某种策略，
* 适配器(Adaptor)：一种用来修饰容器或仿函数或迭代器接口的东西
* 空间配置器(Allocator)：负责空间的配置和管理，


> 需要注意的是 迭代器end()指向的是容器的随后一个元素的下一个位置
> ~~~C++
> container<T> c;
> .....
> container::iterator it=c.begin();
> for(;it!=c.end();++it)
> ....
> ~~~

## STL容器
### vector
![](https://img-blog.csdn.net/20180908193955105?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L1dpemFyZHRvSA==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
**底层原理**：动态数组，当空间不够装上数据时，会自动申请另一片更大的空间(1.5倍或2倍)，然后把原来的数据拷贝到新的内存空间，接着释放原来的那片空间，当释放或删除数据时，其储存空间不释放，仅仅是清空了里面的数据，
**优点**
支持随机访问，查找效率高，时间复杂度是O(1);
**缺点**
当向其头部后中部插入或删除元素时，为了保持原本的相对次序，插入或删除点之后的所有元素都必须移动，所以插入效率低,时间复杂度是O(n)；
**定义方式**
~~~c++
vector<int>v;//定义一个vector，其中的元素类型为int类型
vector<int>v[n];//定义n个vector数组，
vector<int>v(len);//定义一个长度为len的vector数组，且被初始化为0
vector<int>v(len ,x);//定义一个长度为len的vector数组，且被初始化为x
vector<int>v1(v);//用v给v1复制
vector<int>v2(v1.begin(),v1.end()+3);//将v1中0-2三个元素赋值给v2
~~~
**vector的常用内置函数**
~~~c++
vector<int>v={1,2,3};//初始化vector为1，2，3     v{1,2,3}
vector<int>::begin() it=v.begin();//定义vector的迭代器，指向begin()
v.push_back(4);//尾部插入   v{1,2,3,4}
v.pop_back();//删除vector的最后一个元素   v{1,2,3}
lower_bound(v.begin(),v.end(),2);;//返回第一个大于或等于2的元素的迭代器v.begin()+1,若不存在返回v.end()
upper_bound(v.begin(),v.end(),2);//返回第一个大于2的元素的迭代器v.begin()+2,若不存在则返回v.end()
v.size();//返回vector中元素的个数
v.empty();//返回vector是否为空，若为空则返回true，否则返回false
v.front();//返回vector的第一个元素
v.back();//返回vector中的最后一个元素
v.begin();//返回vector第一个元素的迭代器
v.end();//返回vector最后一个元素后一个位置的迭代器
v.clear();//清空vector;
v.erase(v.begin());//删除迭代器it所指的元素，返回vector中下一个有效的迭代器
v.erase(v.begin(),v.begin()+2);//删除区间[v.begin(),v.begin()+2)的所有元素
v.insert(v.begin(),1);//在迭代器it所指的位置前插入元素1，返回插入元素的迭代器
reverse(v.begin(),v.end());//翻转vector
find(v.begin(),v.end(),1);//查找元素
sort(v.begin(),v.end());//排序，默认升序

~~~

### list
![](https://imgconvert.csdnimg.cn/aHR0cHM6Ly9pbWFnZXMyMDE1LmNuYmxvZ3MuY29tL2Jsb2cvNzc5MzY4LzIwMTYxMC83NzkzNjgtMjAxNjEwMjQxNDI2NTU4NTktMTUyMjI3NzQ5Ny5wbmc?x-oss-process=image/format,png)
**底层原理**
list的底层是一个双向链表，以节点为单位存放数据，节点的地址在内存中不一定连续，每次插入或删除一个元素，就配置或释放一个元素空间
**优点**
内存不连续，动态操作，可在任意位置插入或删除且效率高
**缺点**
不支持随机访问
> 由于list不支持随机访问，故
> ~~~c++
> list<int>::iterator it=list.begin();
> it++;//正确
> it=it+1;//错误

**list的常用内置函数**
~~~c++
list l；
l.push_back(elem);//在尾部插入一个数据
l.pop_back();//在尾部删除一个元素
l.push_front(elem);//在头部插入一个数据
l.pop_front();//删除头部数据
l.size();//返回容器中实际数据的个数
l.sort();//排序。升序
l.unique();//移除相同的连续元素
l.back();//去尾部迭代器
l.erase(iterator);//删除一个元素，参数是迭代器，返回的是删除迭代器的下一个位置
l.erase(l.begin(),l.begin()++);//删除区间元素，参数是迭代器，
l.remove(elem);//移除值为elem的元素
~~~

### deque
![](https://img-blog.csdn.net/20150826111941696?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
**底层原理**
是一个双向开口的连续线性空间（实际是分段连续），在头部和尾部进行元素的插入和删除都有理性的时间复杂度
**优点**
支持随机访问，查找效率高
**缺点**
不适合中间插入删除操作，占用内存多

**deque内置函数**
~~~c++
deq.push_back(elem);//尾部插入
deq.pop_back();//尾部删除
deq.push_front(elem);//头部插入
deq.pop_front();//头部删除
deq.size();//返回元素大小
deq.empty();//返回是否为空
deq.front();//返回第一个元素
deq.back();//返回最后一个元素
deq.begin();//返回第一个元素的迭代器
deq.end();//但会最后一个元素的下一个元素的迭代器
deq.clear();//清空deque
deq.erase(deq.begin());//删除头部元素，返回下一个元素的迭代器
deq.erase(deq.begin(),deq,begin()+2);//删除区间函数
deq.insert(deq.begin(),elem);//插入元素，在迭代器前插入元素elem，返回插入元素的迭代器
~~~
### stack
后进先出的数据结构，允许新增元素，移除元素，取得栈顶元素，但是除了最顶端以外，没有任何方法可以存取其他元素，stack不允许有遍历行为

**stack定义**
~~~c++
stack<int>stk;//定义一个skack，其中元素的类型为int
stack<int>stk[n];//定义n个stack；
~~~

**stack内置函数**
~~~c++
stack<int>stk;
stk.push(k);//在stack中插入元素k；
stk.pop();//移除栈顶元素
stk.top();//返回栈顶元素
stk.size();//元素个数
stk.empty();//返回栈是否为空，

~~~
### queue
**底层原理**
对元素采用先进先出的管理策略
**queue内置函数**
~~~c++
que.push(x);//在queue的队尾插入元素x
que.pop();//出队queue的队头元素
que.front();//返回queue的队头元素
que.back();//返回queue的队尾元素
que.size();//返回stack中元素的个数
que.empty();//返回stack是否为空，若为空则返回true否则返回false
~~~


### string
**与vector<char的异同**
同：在数据结构，内存管理等方面是相同的；
异：vector<char是单纯的一个“char元素的容器"
    string不仅是一个容器，还扩展了一些针对字符串的操作，例如输入输出流，还重载了+，+=运算符

**string的定义方式**
~~~C++
string str;//定义一个空的字符串
string str(5,'a');//使用5个字符'a'初始化
string str("abc");//使用字符串初始化
~~~
**string内置函数**
~~~C++
string str;
str.push_back(elem);//尾部插入字符
str.pop_back();//删除尾部字符
str.length();//返回string的字符个数
str.size();//返回str字符个数
str.empty();//返回string是否为空
str.substr(index);//返回string从下标为index到末尾的子串*
str.substr(index,len);//返回string下标为index，长度为len的子串
str.insert(index,len,elem);//在下标为index的字符前插入len个字符elem
str.insert(index,elem);//在下标index的字符前插入字符串elem
str.erase(index,len);//删除从位置index开始的len个字符
str.find(elem);//返回字符elem在string中第一次出现的位置，返回index，若没有则返回-1
str.find(elem,index);//返回从位置index开始字符elem在string第一次出现的位置
str.rfind(elem);//反向查找
str.rfind(elem,index);//反向查找
stoi(str);//返回str的整数形式
to_string(value);//返回value的字符串形式
~~~

### map/multimap
![红黑树](https://img-blog.csdn.net/20180622213441557?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2RhYWlrdWFpY2h1YW4=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
**底层原理**
红黑树，将任何类型的元素映射到另一任意类型的元素上，会根据键值自动排序，map不允许有两个相同的键值，multimap则允许键值重复
**优点**
使用平衡二叉树实现，便于元素查找，且能把一个值映射为另一个值
**缺点**
每次插入值的时候，都需要调整红黑树，效率有一定影响
**map/multimap内置函数**
~~~c++
map<string,int>mp;
mp[key]=value;//将key映射到value
mp[key]++;//将key所映射的值++；若没有该键值，则加入该键值且默认是为0
mp.insert(make_pair(key,value));//插入元素
mp.insert({key,value});//插入元素
mp.size();//返回map中的元素个数
mp.empty();//返回map是否为空
mp.clear();//清空map
mp.erase(key);//清除元素
mp[key];//返回key映射的值
mp.begin();//返回第一个元素的迭代器
mp.end();//返回最后一个元素后一个位置的迭代器
mp.find(key);//返回第一个key的迭代器，若不存在则返回mp.end();
mp.find(key,value);//返回元素{key,value}的迭代器，若不存在则返回mp.end();
mp.count(key);//返回第一个键值为key的元素数量，因为map不能重复所以count返回值为0或1
mp.count({key,value});//返回元素{key，value}的数量
mp.lower_bound(key);//返回第一个键值大于等于key值的元素的迭代器
mp.upper_bound(key);//返回第一个大于key值的元素的迭代器
~~~

### set/multiset
**底层原理**
set的元素即是key值也是value值，底层实现是红黑树，set里面的元素数有序且不重复的，multiset里面的元素是有序且可以重复的
**优点**
使用平衡二叉树，便于元素查找，自动排序
**缺点**
每次插入值时，都需要调整红黑树，效率有影响

**set/multiset内置函数**
~~~C++
set<int>st;
st.insert(elem);//插入元素
st.size();//返回set中元素的个数
st.empty();//返回set是否为空，
st.erase(elem);//清除元素elem
st.begin();//返回第一个元素的迭代器
st.end();//返回最后一个元素后一个位置的迭代器
st.clear();//清空set
st.find(elem);//返回元素elem的迭代器，没有则返回st.end();
st.count(elem);//返回elem的个数，由于不可重复，所以返回值为0或1
st.lower_bound(elem);//返回第一个键值大于等于elem值的元素的迭代器
st.upper_bound(elem);//返回第一个大于elem值的元素的迭代器
~~~


### unordered_set/unordered_map
**底层原理**
底层实现是哈希表结构，拥有快速检索的功能，和map/set的最大区别是无序的，增删查改的时间复杂度是O(1),而map/set的增删查改的时间复杂度是O(logn),但是不支持lower_bound()/upper_bound()函数
![](https://imgconvert.csdnimg.cn/aHR0cHM6Ly9pbWFnZXMyMDE4LmNuYmxvZ3MuY29tL2Jsb2cvMTI3Mjk3OC8yMDE4MDYvMTI3Mjk3OC0yMDE4MDYxMDE5Mjk1MzEwOS01NzQwNTg2MS5wbmc?x-oss-process=image/format,png)

## 各种容器的特点和适用情况
* array:固定大小的数组，支持快速随机访问，不能添加或删除元素
* vector:可变大小的数组，支持快速随机访问，在尾部之外插入或删除元素会很慢
* deque:双端队列，支持快速随机访问，在头尾部插入删除速度快
* queue:插入只可以在尾部进行，删除，检索和修改只允许从头部进行
* stack:后进先出。检索和修改只能在最后插入的元素上进行
* list:双向链表，只支持双向顺序访问，在list任何位置插入或删除速度很快
* forward_list:单向链表，只支持单向顺序访问。在forward_list任何位置插入或删除速度很快
* string:与vector相似的容器，专门储存字符，随机访问快，在稳步位置插入或删除速度很快

## 常见面试题
1. vector中reserve和resize的区别？
   reserve是直接扩充到以确定的大小，可以减少多次开辟、释放空间的问题，提高效率，减少多次拷贝数据的问题
   resize可以改变有效空间的大小，也有改变默认值的功能，capacity()打大小也随之改变
2. vector中size和capacity的区别
   size表示当前vector中有多少个元素(finish-start),而capacity函数则表示他已经分配的内存中可以容纳多少元素
3. vector中的元素类型可以是引用吗？
   vector的底层实现要求连续的对象排列，引用并非对象，没有实际地址，因此vector的元素类型不能是引用
4. vector中迭代器失效的原因？
   但插入一个元素到vector中，由于引起了内存重新分配，所以指向原内存的迭代器全部失效
   当删除容器中一个元素后，该迭代器所指向的元素已经被删除，那么造成迭代器也会失效
5. vector扩容为什么以1.5倍增长
   理想分配方案是在第N次分配时能重用之前N-1次释放的内存，如果按照1.5进行分配，则有1,1.5,3.4.5...当需要分配4.5时，前面已经分配5.5，可以直接利用，把旧数据move过去。但选择两倍的增长时，则会有1,2,4,8,16,32....每次需要申请的空间都会大于前面释放的内存，无法重用。
6. 什么情况下用vector，什么情况用list，什么情况下用deque?
   * vector可以随机存储元素，但在非尾部插入或删除数据时效率低，适合对象简单，对象数量变化不大，随机访问频繁。
   * list不支持随机存储，适用于对象大，对象数量变化频繁，插入和删除频繁，
   * deque适合首尾两端进行杀入和删除操作
7. push_back和emplace_back的区别？
   push_back需要先构造临时对象，再将这个对象拷贝到容器的末尾
   emplace_back直接在容器的末尾构造对象，节省了临时对象的内存空间申请以及拷贝构造函数的复制操作   
   push_back只能接受该类型的对象
   emplace_back还能接受该类型的构造函数的参数
~~~c++
class student
{
public:
   int age;
   student(int age_)
   {

   }
}

int main()
{
   vector<student>temp;
   temp.push_back(student(24));
   temp.emplace_back(24);
}
~~~


## 引用
[算法竞赛C++ STL详解](https://www.acwing.com/blog/content/10558/)
[STL详解及常见面试题](https://blog.csdn.net/daaikuaichuan/article/details/80717222)
