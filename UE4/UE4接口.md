# UE4 接口的原理和使用
## 为什么需要接口
* 官方例子
  在某些游戏功能可能被大量复杂而不同的类共享的情况下非常有用。例如某个游戏可能有这样一个系统，依靠该系统输入一个触发器体积可以激活陷阱、警告敌人或者玩家奖励点数。这可能通过针对陷阱、敌人和点数奖励执行"ReactToTrigger"函数来实现。然而陷阱可能派生自"AActor"，敌人可能派生专门的"APawn"或"ACharacter",点数奖励可能派生"UDateAsset"。所以这些类都需要共享功能，但他们没有除"UObject"之外的共同上级。在这种情况下，推荐使用接口。
*简单来说
    1. 接口提供了一组公共的方法，不同的对象中继承这些方法后可以有不同的实现。
    2. 任何使用接口的类都必须实现这类接口
    3. 实现解耦
    4. 解决多继承的问题。
## UE4接口的实现
ReactToTriggerInterface例子
~~~c++
UINTERFACE(Blueprintable)
class UReactToTriggerInterface : public UInterface
{
	GENERATED_BODY()
};
class IReactToTriggerInterface
{ 
	GENERATED_BODY()
	public:
	/** 对激活该对象的触发器体积做出响应。如果响应成功，返回“真（true）”。*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Trigger Reaction")
	bool ReactToTrigger() const;
};
~~~
可以看到，声明的接口部分包含了两个部分：UReactToTriggerInterface和IReactToTriggerInterface，前者需要继承UE4接口基类UInterface,而UInterface是继承与UObject的，被纳入到了UE4对象系统中。我们使用该接口时需按照C++继承语法来继承IReactToTriggerInterface.

然后是基类UInterface包含内容：
~~~c++
class COREUOBJECT_API UInterface : public UObject
{
    DECLARE_CLASS_INTRINSIC(UInterface, UObject, CLASS_Interface | CLASS_Abstract, TEXT("/Script/CoreUObject"))
};
class COREUOBJECT_API IInterface
{
protected:
    virtual ~IInterface() {}
public:
    typedef UInterface UClassType;
};
~~~

在UInterface里，UInterface作为所有接口的基类，其并无多少实质性内容，仅仅是声明了一下所有接口应具有的一些属性。

再回到官方例子中，在IReactToTriggerInterface，其中定义了用UFUNCTION标记的方法ReactToTrigger，该方法可在蓝图类中实现，目前接口还不能声明UPROPERTY,如果函数不是由蓝图类实现，那么我们应该把函数定义成虚函数，这样以后在使用IReactToTriggerInterface->ReactToTrigger()进行调用才会表现正确。再看UReactToTriggerInterface中，发现里面除了一个GENERATED_BODY(),其他什么都没有。

这里可能给人一种错觉，就是UReactToTriggerInterface其实什么都没有做，用处不大，恰恰相反，UReactToTriggerInterface完成了UE4接口的大部分工作。前面提到过，UE4的接口其实是由两部分组成:普通意义上的接口部分，以及UE4对象系统部分。IInterface主要负责接口功能部分，而UInterface负责UE4对象系统部分，因此我们才能在一个并不继承自UObject的类中使用UFUNCTION标记。
### UHT为两个类生成的GENERATED_BODY()内容、
在UReactToTriggerInterface的GENERATED_BODY()中；
~~~c++
 #define TemplateProject_Source_TemplateProject_TestInterface_h_13_GENERATED_BODY \
	PRAGMA_DISABLE_DEPRECATION_WARNINGS \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_GENERATED_UINTERFACE_BODY() \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_ENHANCED_CONSTRUCTORS \
private: \
	PRAGMA_ENABLE_DEPRECATION_WARNINGS


 #define TemplateProject_Source_TemplateProject_TestInterface_h_13_GENERATED_UINTERFACE_BODY() \
private: \
	static void StaticRegisterNativesUTestInterface(); \
	friend struct Z_Construct_UClass_UTestInterface_Statics; \
public: \
	DECLARE_CLASS(UTestInterface, UInterface, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Interface), CASTCLASS_None, TEXT("/Script/TemplateProject"), TEMPLATEPROJECT_API) \
	DECLARE_SERIALIZER(UTestInterface)


 #define TemplateProject_Source_TemplateProject_TestInterface_h_13_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	TEMPLATEPROJECT_API UTestInterface(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	TEMPLATEPROJECT_API UTestInterface(UTestInterface&&); \
	TEMPLATEPROJECT_API UTestInterface(const UTestInterface&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(TEMPLATEPROJECT_API, UTestInterface); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UTestInterface); \
	DEFINE_ABSTRACT_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UTestInterface)
~~~
在IReactToTriggerInterface的GENERATED_BODY()中；
~~~c++
 #define TemplateProject_Source_TemplateProject_TestInterface_h_21_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_SPARSE_DATA \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_RPC_WRAPPERS \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_CALLBACK_WRAPPERS \
	TemplateProject_Source_TemplateProject_TestInterface_h_13_INCLASS_IINTERFACE \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS
~~~
这就比较清楚了，为IReactToTriggerInterface生成的代码并不包含对象系统相关信息，而用UReactToTriggerInterface完成了UFUNCTION注册等一系列操作。
## UE4接口的使用
* 接口类
~~~c++
//TestInterface.h
UINTERFACE(MinimalAPI,Blueprintable)
class UTestInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TEMPLATEPROJECT_API ITestInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	//此接口可以在接口的.cpp文件中定义一个默认的实现。并且只能在C++中实现
	virtual bool CPPInterface();


	//此接口只能在蓝图中重写
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void InterfaceImplement();

	//此接口能在蓝图和C++中重写，可以在接口的.cpp文件中实现一个默认的接口函数。但是这里没有实现。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void InterfaceNative();

	//能在蓝图和C++中重写，可以在接口的.cpp文件中实现一个默认的接口函数。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void InterfaceNativeWithCPP();
	virtual void InterfaceNativeWithCPP_Implementation();//在接口中实现一个默认的接口函数。
};


//TestInterface.cpp
bool ITestInterface::CPPInterface()
{
	UE_LOG(LogTemp, Log, TEXT("ITestInterface::CPPInterface"));
	return true;
}

void ITestInterface::InterfaceNativeWithCPP_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ITestInterface::InterfaceNativeWithCPP_Implementation"));
}
~~~
* 使用接口的类
~~~c++
//InterfaceActor.h
UCLASS()
class TEMPLATEPROJECT_API AInterfaceActor : public AActor,public ITestInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInterfaceActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	//CPP实现接口函数
	virtual bool CPPInterface()override;

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
		void InterfaceNative();
	virtual void  InterfaceNative_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void InterfaceNativeWithCPP();
	virtual void InterfaceNativeWithCPP_Implementation();
};

//InterfaceActor.cpp
bool AInterfaceActor::CPPInterface()
{
	UE_LOG(LogTemp, Log, TEXT("DisplayName= %s ,AInterfaceActor::CPPInterface"),*UKismetSystemLibrary::GetDisplayName(this));
	return false;
}

void AInterfaceActor::InterfaceNative_Implementation()
{

	UE_LOG(LogTemp, Log, TEXT("DisplayName=%s,AInterfaceActor::InterfaceNative_Implementation"), *UKismetSystemLibrary::GetDisplayName(this));
}

void AInterfaceActor::InterfaceNativeWithCPP_Implementation()
{

	UE_LOG(LogTemp, Log, TEXT("DisplayName=%s,AInterfaceActor::InterfaceNativeWithCPP_Implementation"), *UKismetSystemLibrary::GetDisplayName(this));

}
~~~
* 如何在cpp里调用接口
~~~c++
void ATemplateProjectCharacter::CallInterface()
{
    //遍历World的Actor
	TActorIterator<AActor> actorIt = TActorIterator<AActor>(GetWorld(), AActor::StaticClass());
	for (actorIt; actorIt; ++actorIt)
	{
		if(!actorIt)continue;
		AActor* ActorItem = *actorIt;
		bool IsImplement = ActorItem->GetClass()->ImplementsInterface(UTestInterface::StaticClass());
		if (IsImplement)
		{
			ITestInterface* InterfaceImp = Cast<ITestInterface>(ActorItem);
			if (InterfaceImp)
			{
				InterfaceImp->CPPInterface();
				InterfaceImp->Execute_InterfaceImplement(ActorItem);
				InterfaceImp->Execute_InterfaceNative(ActorItem);
				InterfaceImp->Execute_InterfaceNativeWithCPP(ActorItem);
			}
		}
	}

}
~~~


1. 如何确定类是否实现了接口
~~~c++
//第一种方法
bool bIsImplemented = OriginalObject->GetClass()->ImplementsInterface(UTestInterface::StaticClass()); // 如果OriginalObject实现了UTestInterface，则bisimplemated将为true。

//第二种方法

bool bIsImplemented = OriginalObject->Implements<UTestInterface>(); // 如果OriginalObject实现了UTestInterface，bIsImplemented将为true。

//第三种方法
ITestInterface* ReactingObject = Cast<ITestInterface>(OriginalObject); // 如果OriginalObject实现了UTestInterface，则ReactingObject将为非空。
~~~
2. 如何调用接口函数
分两类情况
    1. 原始虚函数调用按照原生C++调用即可，如CPPInterface();
    2. UFUNCTION()修饰的接口函数则以 I接口名::Excute_函数名(Actor实例，函数参数调用)，如Execute_InterfaceImplement(ActorItem)
## 一些问题
1. UE4接口如何解决多继承的问题
   在官方例子中，
   ~~~c++
   class ATrap : public AActor, public IReactToTriggerInterface
   ~~~
   继承关系如图
   ![](https://pic3.zhimg.com/80/v2-0ff68079702ed0179dbea0bea6b13136_720w.webp)
   虽然是多继承，但是IReactToTriggerInterface并没有继承其他类，因此不会引发菱形继承的问题。



2. 蓝图三种调用方法的区别
![](https://img-blog.csdnimg.cn/img_convert/1ece243ea4e590294103cfdc295fcbea.png)



[UE4 Interface原理与使用](https://zhuanlan.zhihu.com/p/60851912)
[接口](https://docs.unrealengine.com/5.1/zh-CN/interfaces-in-unreal-engine/)
[【UE4 C++ 基础知识】＜9＞ Interface 接口](https://blog.csdn.net/ttod/article/details/127716410?spm=1001.2101.3001.6650.5&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-5-127716410-blog-119459043.235%5Ev29%5Epc_relevant_default_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-5-127716410-blog-119459043.235%5Ev29%5Epc_relevant_default_base3&utm_relevant_index=6)
[[UE4]C++定义接口Interface，C++/蓝图使用Interface](https://blog.csdn.net/ZFSR05255134/article/details/119459043)