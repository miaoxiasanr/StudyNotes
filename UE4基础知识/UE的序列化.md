#UE的序列化
## 序列化基础概念
> 序列化（serialization）在计算机科学的数据处理中，是指将数据结构或对象状态转换成可取用格式（例如存成文件，存于缓冲，或经由网络中发送），以留待后续在相同或另一台计算机环境中，能恢复原先状态的过程。依照序列化格式重新获取字节的结果时，可以利用它来产生与原始对象相同语义的副本。对于许多对象，像是使用大量引用的复杂对象，这种序列化重建的过程并不容易。面向对象中的对象序列化，并不概括之前原始对象所关系的函数。这种过程也称为对象编组（marshalling）。从一系列字节提取数据结构的反向操作，是反序列化（也称为解编组、deserialization、unmarshalling）。

简而言之。就是把一堆对象和数据，像袋子一样可以装起来保存，可以取出来还原，就是最普遍意义的序列化，但是在具体开发中，其实袋子的种类，装东西的时候有没有其他的规则，袋子的大小都是不同的。

基于以上，需要把程序中使用传任何对象，任意数据都转化成一个通用的数据格式，这个通用的数据格式需要满足以下条件
1. 可以存储咱各个平台的文件中，如win，linux，安卓
2. 可以加载到内存，数据库
3. 可以在网络中发送，且比较容易管理
4. 可以通过这个通用数据，反序列化之前存储的对象和数据

满足上述条件，其实是由很多的通用数据格式，比如说Json,字节流，比特流，XML。
UE中也确实有基于Json。字节流，比特流的序列化实现。本文主要侧重于字节流的序列化。

## 字节流
### 什么是字节流？
字节流可以理解为字节数组，字节是8位，uint以为是八位，并且在任意操作系统和编译器下都是8位，字节流的另一种表示方式就是TArray< uint8 >ByteBuffer.
### 如何把数据转换成字节流呢？
重点就是4个字:内存拷贝。
例如int，把int占用的内存空间全部拷贝到我们的字节流即可。拷贝任何一个数据进内存，只需要把拷贝数据的内存起点、它所占用的大小就行。
例如UE的内存拷贝函数:
~~~c++
static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
{
   return FPlatformMemory::Memcpy(Dest,Src,Count);
}
~~~
需要三个参数，目的位置的起点、源数据的起点，拷贝的大小、字节为单位。
手动序列化
~~~c++
int intnum=8;
TArray<uint8>ByteBUffer;
int pos=0;
//序列化
int size=sizeof(intnum);
FMemory::memcpy(&ByteBuffer.GetDate()+pos,&intnum,size);
pos+=size;

//反序列化
pos=0;
int numseed=0;
FMemory::Memcpy(&numseed,ByteBuffer.GetDate()+pos,size);
~~~
手动序列化会有很多问题，比如
1. 我们只是手动序列化了一个int，就手动写了很多，需要一个管理类是帮我们管理
2. TArray< uint8 >ByteBuffer是直接声明，他需要被更好的管理。
3. UE自定义类型很多。

## 访问者模式
UE4的序列化使用了访问者模式(Vistor Pattem),将序列化的存档接口抽象话，其中FArchive为访问者，其他UObject实现了void Serialize(FArchive& Ar),接口的类为被访问者。


## UObeject的序列化
* UE的变量分为UPROPERTY()修饰的变量和普通C++变量两种，普通的C++变量在Runtime的时候进行读写，UPROPERTY宏修饰的变量可以在Editor中进行各类操作。
* UE中的序列化有两种:TaggedPropertySerializer(TPS)和UnversionedPropertySerializer(UPS).
### TPS
#### 序列化过程
* TPS方式下，没有被UPROPERTY标记的成员变量不参与序列化。TPS首先找到UClass中的持有FProperty属性的变量，这个FProperty属性保存着这个变量的名字、类型、类中的位置、meta修饰符数据等等的数据情报。根据变量的FProperty属性，TPS会为其创建一个FPROPERTYTag的数据。
> FPropertyTag里面包含了GUID数据的结构信息。

#### 反序列化过程
反序列化的过程就是和上面的相反
* 首先是从Asset文件(.uasset)中取出数据和FPropertyTag数据
* 然后根据FPropertyTag中保存的数据(GUID和名字等数据)在UClass中检索，找到了相应的位置数据之后按照顺序将每一个变量的数据展开到内存中的数据对象的适当位置。

#### TPS总结
* PropertyTag的存在是得数据得到了更好的互换性
* 需要给每一个变量添加一个PropertyTag
* 将数据在展开到内存的时候会有检索开销
  

###  UPS
#### 序列化过程

#### 反序列化过程
#### UPS总结


### FArchive
在UObject中声明了两个Serualize()方法。
~~~c++
/** Object.h

 * Handles reading, writing, and reference collecting using FArchive.
 * This implementation handles all FProperty serialization, but can be overridden for native variables.
 */
virtual void Serialize(FArchive& Ar);
virtual void Serialize(FStructuredArchive::FRecord Record);
~~~
其中Serialize(FArchive&Ar)的宏定义
~~~c++
// Obj.cpp
IMPLEMENT_FARCHIVE_SERIALIZER(UObject)

// ObjectMacros.h
 #define IMPLEMENT_FARCHIVE_SERIALIZER( TClass ) void TClass::Serialize(FArchive& Ar) { TClass::Serialize(FStructuredArchiveFromArchive(Ar).GetSlot().EnterRecord()); }
~~~
等价于
~~~c++
void UObject::Serialize(FArchive& Ar) 
{ UObject::Serialize(FStructuredArchiveFromArchive(Ar).GetSlot().EnterRecord()); 
}
~~~
由此可见UObject的第一个Serialize(FArchive&Ar)方法实际上接受一个FArchive类型的输入，FStructuredArchiveFormArcive就是将传入的FArchive包了一层并做了一些处理，最终返回了FStructuredArchive的成员Record，并以此为参数传入UObject的第二个Serialize()方法。


### UObject基础
通过NewObject<>()方法实例化一个UObject时需要传入一个参数Outer来指定这个UObject属于哪一个UPackage，如果不传则创建一个临时的Package，每个UObject都存在于一个UPackage中，UObeject的保存和加载也是基于UPackage。
~~~c++
template< class T >
FUNCTION_NON_NULL_RETURN_START
	T* NewObject(UObject* Outer = (UObject*)GetTransientPackage())
FUNCTION_NON_NULL_RETURN_END
{
	// Name is always None for this case
	FObjectInitializer::AssertIfInConstructor(Outer, TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSubobject<> instead."));

	FStaticConstructObjectParameters Params(T::StaticClass());
	Params.Outer = Outer;
	return static_cast<T*>(StaticConstructObject_Internal(Params));
}
~~~


### UPackage结构
![](https://pic1.zhimg.com/80/v2-67dc42c1f35fbf0e7509fe7f6d6f9988_720w.webp)

* File Summary 文件头信息
* NameTable 包中对象的名字表
* Import Table 存放被该包中对象引用的其他包中的对象信息(路径名和类型)
* Export Table 该包中的对象信息(路径名和类型)
* Export Objects 所有Export Table中对象的实际数据

### UObject的保存
对一个Object调用序列化函数后大致过程如下
1. 通过GetClass函数获取当前的类信息，通过GetOuter函数获取Outer。这个Outer实际上指定了当前UObject会被当做哪一个对象的子对象进行序列化。
2. 判断当前等待序列化的对象的类UClass的信息是否被载入。
3. 如果UClass没有被载入，预载入当前类的信息
4. 预载入当前类的默认对象CDO的信息
5. 载入名字
6. 载入Outer
7. 载入当前对象的类信息，保存于ObjClass对象中
8. 载入对象的所有脚本成员变量信息。这一步必须在类信息加载后，否则无法根据类信息获得有哪些脚本成员变量需要加载。
9. 对应函数为SerializeScriptProperties,序列化为类中定义的对象属性
10. 调用FArchive.MarkScriptSerializationStart,标志脚本序列化数据开始
11. 调用SerializeTaggedProperties，序列化对象属性，并且加入Tag
12. 调用FArchive.MarkScriptSerializationEnd,标志脚本序列化数据结束。
### UObject的加载

[Unreal Engine序列化个人笔记](https://zhuanlan.zhihu.com/p/616109129)
[UE 序列化介绍及源码解析](https://zhuanlan.zhihu.com/p/617464719)
[UE4中的Serialization](https://stonelzp.github.io/ue4-serialization/)