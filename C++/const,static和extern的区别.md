- [const的基本功能](#const的基本功能)
- [static的基本功能](#static的基本功能)
- [extern的基本功能](#extern的基本功能)
    - [引用](#引用)

# const的基本功能
1. 修饰变量
采用const修饰，功能是对变量声明为只读变量，以保护变量值不被修改
~~~c++
const int i =5;
~~~
**在定义变量的同时，必须初始化**
此外，const修饰变量还起到节约空间的目的，通常编译器并不给普通const只读变量分配空间，而是将他门保存到符号表中，无序读写内存，程序执行效率也会提高

2. 修饰数组
~~~c++
const int array[5]={1,2,3,4,5};
array[0]=array[0]+1;//错误
~~~
数组元素和变量类似，具有只读属性，不能被更改
3. 修饰指针
有两种情况
 1. **常量指针：** 用来限定指向空间的值不能改变  
   只能防止通过指针引用修改内存中的数据，不保护指针所指的对象
 2. **指针常量：** 用来限定指针不能改变
   指针常量必须在声明的时候对其初始化
~~~c++
int i=10,j=20,k=30;
const int *p1=&i;//常量指针
int const *p3=&i;//常量指针
int *const p2=&j;//指针常量
int *const p4;//错误。必须声明的时候赋值
*p1=40;//错误
p1=&k;//正确
*p2=50;//正确
p2=&k;//错误
~~~

3. 修饰函数参数
const修饰函数参数，对参数其限定作用，防止其在函数内部被修改，可以是普通变量，也可以是指针变量
~~~c++
void func1(const int i)
    i++;//报错
void func2(const int * i)
    (*i)++;//报错
~~~

4. 修饰函数返回值
如果以指针传递方式的函数返回值加const修饰，那么函数返回值的内容不能被修改，该返回值只能被赋给加const修饰的同类型指针
~~~c++
const int * func(int i)
{
    return &i;
}
int * j=func(20);//错误
const int  * j=func(20);//正确
~~~
如果函数返回值采用值传递方式，加const是没有任何价值的
~~~c++
const int func(int i)
    return i;
~~~

5. 修饰成员函数
任何不会修改数据成员的函数都应该声明成const类型
~~~c++
class Test
{
public:
    int num;
    int getnum()const;
    int pop();
}
Test::getnum()
{
    ++this->num;//编译错误，企图修改数据成员num
    this->pop();//编译错误，企图调用非const函数
    return num;
}
~~~

# static的基本功能
1. 修饰局部变量
static修饰的静态局部变量只执行初始化一次，而且延长了局部变量的生命周期，知道程序运行结束以后才释放；
~~~c++
void func()
{
    static int num_s=20;
    int num =20;
    cout<<num_s<<endl;
    cout<<num<<endl;
    num++；
}
int main()
{
    func();//num_s:20   num:20
    func();//num_s:21   num:20
    func();//num_s:22   num:20
}
~~~
通常在函数体内定义了一个局部变量，当程序运行到该语句时会分配栈内存，退出函数体时会回收内存,局部变量也会失效
而静态局部变量就解决了这个问题，静态局部变量保存在全局数据区，每次的是都会保持到下一次调用，直到下次赋新值
* 特点：
   * 该变量在全局数据区分配内存
   * 只初始化一次，之后的函数调用不在初始化
   * 一般在声明处初始化，没有显式的初始化就会被自动初始化为0
   * 始终驻留在全局数据区，直到程序运行结束
2. 修饰全局变量
static修饰全局变量的时候，这个全局变量只能在本文件下访问，不能再其他文件下访问，即便是extern外部声明也不可以
~~~c++
static int num_s;
void func()
{
    num_s++;
    cout<<num_s<<endl;
}
int main()
{
    num_s=20;
    cout<<nums_s<<endl;//20
    func();//21
}
~~~
* 特点
  * 该变量在全局区分配内存
  * 未经初始化的静态全局变量会被自动初始化为0
  * 静态全局变量在声明他的整个文件都是可见的，在文件之外不可见

3. 修饰函数
   在函数的返回值类型前加上static即被定义为静态函数，，只能在声明他的文件内可见
~~~c++
static void func()
{
    int n=10;
    cout<<n<<endl;
}
int main()
{
    func();
}
~~~

4. 修饰成员变量
~~~c++
class Test
{
public:
    Test(int A,int B, int C);
    void GetSum();

private:
    int a,b,c;
    static int sum;//声明静态成员变量
}
int Test::sum=0;//定义静态成员变量
Test(int A,int B,int C)
:a(A),b(B),c(C)
{
    sum=this->a+this->b+this->c;
}
void GetSum()
{
    cout<<sun<<endl;
}

int main()
{
    Test test1(2,3,4);
    test1.GetSum;//9
    Test test2(4,5,6);
    test2.GetSum;//15
    test1.GetSum;//15
}
~~~
* 特点
    * 无论这个类被定义了多少个，静态成员变量只有一份拷贝，有该类型的所有对象共享访问
    * 初始化的格式 <数据类型><类名>::<静态成员变量名>=<值>
    * 遵循public,private,protented访问规则
    * 两种访问格式：<类对象名>.<静态成员变量名>或<类类型名>::<静态数据变量名>
1. 修饰成员函数
普通成员函数fn(),实际上是this->fn();而静态成员函数不是与具体的对象相联系，不具有this指针
无法访问类对象的非静态成员变量，也无法访问非静态函数
~~~c++
class Test
{
public:
    Test(int A,int B, int C);
    static void GetSum();

private:
    int a,b,c;
    static int sum;//声明静态成员变量
}
int Test::sum=0;//定义静态成员变量
Test(int A,int B,int C)
:a(A),b(B),c(C)
{
    sum=this->a+this->b+this->c;//非静态函数可以访问静态成员变量
}
void GetSum()
{
    cout<<a<<endl;//错误，不能访问非静态成员变量
    cout<<sun<<endl;
}

int main()
{
    Test test1(2,3,4);
    test1.GetSum;//9
    Test test2(4,5,6);
    test2.GetSum;//15
    Test::GetSun();//15
}
~~~

# extern的基本功能
1. 基本解释
    extern可以置于变量或者函数前，以标示变量或函数的定义在被=别的文件中，提示编译器遇到此变量和函数时在其他模块中寻找其定义，跨文件访问

2. extern "c"
   在c++环境下使用C函数时，常常会出现编译器无法找到obj模块中的C函数定义，从而导致链接失败的问题

~~~c++
//Test1.h
extern int num;

//Test1.cpp
int num =20;
//Test2.cpp
include"Test1.h"
cout<<num<<endl;//20
~~~
或者
~~~c++
//Test1.h
extern int num=20;//这个时候相当于没有extern

//Test2.cpp
extern int num;
cout<<num<<endl;
~~~

### 引用
[C++关键词总结：const，static和extern](https://juejin.cn/post/6883056787042336775)
[const 修饰函数参数，返回值，函数体](https://blog.csdn.net/lz20120808/article/details/46662569)