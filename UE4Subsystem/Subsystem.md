# Subsystem
## 为什么使用Subsystem???
### 在未出现Subsystem的黑暗时代
当我们需要一个全局的，生命周期是在整个游戏进行的过程中一直存在的单例，在UE4中往往是以下代码：
~~~c++
UCLASS()
class HELLO_API UMyScoreManager : public UObject
{
    GENERATED_BODY()
public:
// 一些公用的函数或者Property
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Score;

    UFUNCTION(BlueprintPure,DisplayName="MyScoreManager")
    static UMyScoreManager* Instance()
    {
        static UMyScoreManager* instance=nullptr;
        if (instance==nullptr)
        {
            instance=NewObject<UMyScoreManager>();
            instance->AddToRoot();
        }
        return instance;
        //return GetMutableDefault<UMyScoreManager>();
    }
    UFUNCTION(BlueprintCallable)
        void AddScore(float delta);
};
~~~
以上代码可能出现的问题：
1. AddToRoot()往往很多人会忘掉，导致刚刚生成的对象被GC掉，调用的时候会导致崩溃。
2. 用这种方式创建的单例会在Editor模式下继续存在，所以运行预览和停止预览之后并不会销毁，下一次预览的时候里面的数据可能还是上一次运行的数据。
    > 如果想解决这个问题，就需要手动加上Initialize()和Deinitialize()函数，手动调用，自己管理声明周期。
3. 或者可以是另外一种方法，直接继承UGameInstance，然后在UGameInstance的Init()和Shutdown()里面进行创建和销毁，这样写的会就会出现以下问题：
   1. 一个类里挤占着太多逻辑。可维护性差(多个模块：任务系统，积分系统，经济系统，对话系统等等)
   2. 不利于模块复用
   3. 手动划分Manager类也不够机智。
### 使用Subsystem的好处
1. 不需要自己管理生命周期，引擎帮你自动管理，而且保证和指定的类型生命周期一致。
2. 自动实例化，会在合适的时机被创建出对象，然后再合适的时机释放。
3. 官方提供的蓝图接口，能够很方便地在蓝图调用Subsystem
4. 与UObject类一样，可以定义UFUNCTION和UPROPERTY
5. 容易使用，只需继承需要的Subsystem类型就能正常使用，维护成本低；
6. 更模块化，而且可以迁移；


## Subsystem
Subsystem是一套可以定义自动实例化和释放的类的框架。
![](https://img2020.cnblogs.com/blog/1472587/202107/1472587-20210719153141813-975261415.png)
~~~c++
UCLASS(Abstract, Within = GameInstance)
class ENGINE_API UGameInstanceSubsystem : public USubsystem
{
}
~~~
两个重要的宏标记：
* Abstract：标记指明UGameInstanceSubsystem是抽象基类，是不能被创建出来的；
* Within = GameInstance：Within这个标记是其对象的Outer必须是某个类型，另外Within的标记是会被继承到子类的。
  
### 引擎支持的五类Subsystem及其生命周期
* UEngineSubsystem
    ~~~c++
    class UMyEngineSubsystem : public UEngineSubsystem { ... };
    ~~~
    * EngineSubsystem的模块加载时，Subsysten将会在模块的Startup()函数返回后执行Initialize().Subsystem将会在模块的Shutdown()函数返回后执行Deinitialize();
    * 继承的子系统会通过GEngine进行访问
    ~~~c++
    UMyEngineSubSysten MySubsystem=GEngine->GetEngineSubsystem();
    ~~~
* UEditorSubsystem
    ~~~c++
    class UMyEditorSubsystem : public UEditorSubsystem { ... };
    ~~~
    * UEditorSubsystem的模块加载时，Subsysten将会在模块的Startup()函数返回后执行Initialize().Subsystem将会在模块的Shutdown()函数返回后执行Deinitialize();
    * 继承的子系统会通过GEditor进行访问
    ~~~c++
    UMyEditorSubsystem MySubsystem=GEditor->GetEditorSubsystem();
    ~~~
    > 纯编辑器的子系统不能访问常规蓝图，只能访问编辑器工具和Blutility类；
* UGameInstanceSubsystem
  ~~~c++
  class UMyGameSubsystem : public UGameInstanceSubsystem { ... };
  ~~~
  * 在GameInstance进行初始化时，会创建所有的GameInstanceSubsystem。在Shutdown()函数中会执行Deinitialize()。

  * 这能通过UGameInstance进行访问：
    ~~~c++
    UGameInstance* GameInstance = ...;
    UMyGameSubsystem* MySubsystem = GameInstance->GetSubsystem();
    ~~~
* UWorldSubsystem 
* ULocalPlayerSubsystem
   ~~~c++
  class UMyPlayerSubsystem  : public ULocalPlayerSubsystem  { ... };
  ~~~
  * 在LocalPlayer进行初始化时，会创建所有的LocalPlayerSubsystem。在PlayerRemoved()函数中会执行Deinitialize()。

  * 这能通过ULocalPlayer进行访问：
    ~~~c++
    ULocalPlayer* LocalPlayer = ...;
    UMyPlayerSubsystem * MySubsystem = LocalPlayer->GetSubsystem();;
    ~~~

### SubSysten对象的创建销毁流程
#### 深刻理解5类outer对象的生命周期
> 三种模式
> 1. 编辑器模式：启动编辑器之后。
> 2. Runtime模式：你游戏打包之后的运行状态。
> 3. PIE模式：在编辑器里点击Play后的运行状态。

1. UEngine* GEngine：代表引擎，数量1.Editor或Runtime模式下唯一，从进程启动开始创建，进程退出时销毁。
2. UEditorEngine*GEditer：代表编辑器，数量1。顾名思义只在编辑器下存在且唯一，重编辑器启动时开始创建，编辑器退出时销毁。
3. UGameInstance*GameInstance：代表一场游戏，数量1.从游戏的启动时开始创建，游戏推出时销毁(这里的一场游戏指的是RUntime或PIE模式的运行都算)。
4. UWorld*World：代表一个世界，数量可能>1;World和GameMode是关联的，可以包含多个Level，默认情况下OpenLevel常常会切换World(EWorldType有多个类型：Game，Editor，PIE，EditorPreview，GamePreview)。
5. ULocalPlayer*LocalPlayer：代表本地玩家，数量可能>1.UE支持分屏多人玩家，但往往只有一个，LocalPlayer虽然往往跟PlayerController一起访问，但是其生命周期其实是跟GameInstance一起的。

#### 理解SubSystem对象反射创建销毁流程
##### 创建
这5类Outer对象自己创建的时候，都会调用一下FSubsystemCollectionBase::Initialize(UObject* NewOuter)，把自己作为outer传进去；
~~~c++
void FSubsystemCollectionBase::Initialize(UObject* NewOuter)
{
    	if (BaseType->IsChildOf(UDynamicSubsystem::StaticClass()))//判断是否为DynamicSubsystem
		{
			for (const TPair<FName, TArray<TSubclassOf<UDynamicSubsystem>>>& SubsystemClasses : DynamicSystemModuleMap)
			{
				for (const TSubclassOf<UDynamicSubsystem>& SubsystemClass : SubsystemClasses.Value)
				{
					if (SubsystemClass->IsChildOf(BaseType))
					{
						AddAndInitializeSubsystem(SubsystemClass);
					}
				}
			}
		}
		else
		{
			TArray<UClass*> SubsystemClasses;
			GetDerivedClasses(BaseType, SubsystemClasses, true);

			for (UClass* SubsystemClass : SubsystemClasses)
			{
				AddAndInitializeSubsystem(SubsystemClass);
			}
		}
}

USubsystem* FSubsystemCollectionBase::AddAndInitializeSubsystem(UClass* SubsystemClass)
{
        //CDO调用ShouldCreateSubsystem判断是否需要创建
		const USubsystem* CDO = SubsystemClass->GetDefaultObject<USubsystem>();
		if (CDO->ShouldCreateSubsystem(Outer))
		{
			USubsystem* Subsystem = NewObject<USubsystem>(Outer, SubsystemClass);//创建对象
			SubsystemMap.Add(SubsystemClass,Subsystem);//保存到Map里
			Subsystem->InternalOwningSubsystem = this;//保存父指针
			Subsystem->Initialize(*this);//Initialize
			return Subsystem;
		}
}
~~~
##### 销毁
Outer对象要被销毁时，其内部就添加了SubsystemCollection.Deinitialize();把Map里面的数据清除。
~~~c++
void FSubsystemCollectionBase::Deinitialize()
{
    //...省略一些清除代码
    for (auto Iter = SubsystemMap.CreateIterator(); Iter; ++Iter)   //遍历Map
    {
        UClass* KeyClass = Iter.Key();
        USubsystem* Subsystem = Iter.Value();
        if (Subsystem->GetClass() == KeyClass)
        {
            Subsystem->Deinitialize();  //反初始化
            Subsystem->InternalOwningSubsystem = nullptr;
        }
    }
    SubsystemMap.Empty();
    Outer = nullptr;
}
~~~
### 如何理解UDynamicSubsystem
![](https://img2020.cnblogs.com/blog/1472587/202107/1472587-20210719153146506-239968989.png)
从一开始的类继承结构来看，UEngineSubsystem和UEditorSubsystem是继承于UDynamicSubsystem。设计目的是什么？

* DynamicSubsystem其为动态SubSystem，这里的动态指的是随着模块的加载释放来创建销毁。要理解这点，需要先理解UE4的模块机制。
#### UE4的模块机制
* 简单来说，一个uproject项目或者Uplugin插件可以包含多个Module模块，每个Module可以有个Build.cs，每个模块可以被编译成dll。模块之间可以相互引用。因此一个模块可能会有多个依赖的其他模块。假设需要依赖的模块叫做：DependencyModules。引擎的机制是依赖一个模块的时候就会自动的加载。

#### Plugins的SubSystem什么时候创建的呢？
* 其实是在第一次初始化的时候，用数目来判断是否是第一次创建，然后用FSubsystemModuleWatcher初始化。
* FSubsystemModuleWatcher初始化最重要的就是遍历当前的UDynamicSubSystem子类们，并按照模块划分存储进DynamicSystemModuleMap，这样之后就知道加载或释放某个模块的时候，应该去创建或销毁哪些SubSystem类型对象。
~~~c++
void FSubsystemCollectionBase::Initialize(UObject* NewOuter)
{
    if (SubsystemCollections.Num() == 0)//静态变量，用数目来判断是第一次创建
    {
        FSubsystemModuleWatcher::InitializeModuleWatcher();
    }
}
void FSubsystemModuleWatcher::InitializeModuleWatcher()
{
    // 获得所有UDynamicSubsystem的子类
    TArray<UClass*> SubsystemClasses;
    GetDerivedClasses(UDynamicSubsystem::StaticClass(), SubsystemClasses, true);

    for (UClass* SubsystemClass : SubsystemClasses) //遍历
    {
        if (!SubsystemClass->HasAllClassFlags(CLASS_Abstract))  //不为抽象类
        {
            UPackage* const ClassPackage = SubsystemClass->GetOuterUPackage();//获得所属于的包
            if (ClassPackage)
            {
                const FName ModuleName = FPackageName::GetShortFName(ClassPackage->GetFName());
                if (FModuleManager::Get().IsModuleLoaded(ModuleName))
                {
                    TArray<TSubclassOf<UDynamicSubsystem>>& ModuleSubsystemClasses = FSubsystemCollectionBase::DynamicSystemModuleMap.FindOrAdd(ModuleName);
                    ModuleSubsystemClasses.Add(SubsystemClass);//添加到DynamicSystemModuleMap
                }
            }
        }
    }
    //注册模块加载和释放事件
    ModulesChangedHandle = FModuleManager::Get().OnModulesChanged().AddStatic(&FSubsystemModuleWatcher::OnModulesChanged);
}
~~~
* 然后就是ONmodulesChanged事件的注册，这样在后面加载的模块就能够得到通知。
~~~c++
void FSubsystemModuleWatcher::OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange)
{
    switch (ReasonForChange)
    {
    case EModuleChangeReason::ModuleLoaded:
        AddClassesForModule(ModuleThatChanged);//创建一个模块的DynamicSubsystem类们
        break;

    case EModuleChangeReason::ModuleUnloaded:
        RemoveClassesForModule(ModuleThatChanged);//销毁一个模块的DynamicSubsystem类们
        break;
    }
}
~~~
**总而言之，UDynamicSubSystem提供的额外功能只有加载和卸载功能**


##### 一些思考
1. SubSystem如何被GC掉？
   在上述代码里并不会看到手动的调用Destroy，因为USubSystem对象是一个UObject对象。其依然是受GC、管理的。SubSystem对象和Outer对象之间隔了一个FSubsystemCollection结构，为了让F结构依然可以追溯到对象引用，因此FSubSystemCollectionBase继承于FGCObject，有以下代码
   ~~~c++
    void FSubsystemCollectionBase::AddReferencedObjects(FReferenceCollector& Collector)
    {
        Collector.AddReferencedObjects(SubsystemMap);
    }
   ~~~
   当FSubSystemCollectionBase::Deinitialize()里进行Subsystemmap.Empty()后，USubsystem对象就没有被引用了，在下一帧的GC的时候，就会被判定为PendingKill的对象，从而被Destroy。
   > 这里的妙用是直接利用了UObject对象之间引用所带来的生命周期绑定机制，来直接把USubsystem对象的生命周期和其Out对象关联起来。
2. 动态加载MyPlugin里的GameInstanceSubSystem可以正常工作吗？
   答案是可以，GameInstanceSubSystem需要创建的时机是游戏运行时，在MyPlugin这个模块的dll加载的时候，其自身带的反射代码里的全局静态变量会自动的在进程里注册进各种类型，这个时候就可以正常的时候使用MyPlugin里定义的非DynamicSubSystem了。
3. 为什么只有UEngineSubSystem和UEditorSubSystem才是UDynamicSubSystem?
   从生命周期来说，5类中只有UEngineSubSystem和UEditorSubSystem的生命周期是和游戏的进程绑定在一起的。
   > 游戏进程启动的时候创建SubSystem叫做默认创建，游戏启动一段时间后创建SubSystem叫做动态创建.
   
   而另外三个UGameInstanceSubSystem,UWorldSubSystem和ULocalPlayerSubSystem都是和一场游戏的生命周期绑定的。
4. SubSystem支持网络复制吗？
   不支持，因为UE里的网络复制是基于Actor的ActorChannel的，而SubSystem是普通的UObject对象。
   > 可以使用GameState和PlayerState等一些内建的GamePlay通信。
##参考文章
1. [《InsideUE4》GamePlay架构（十一）Subsystems](https://zhuanlan.zhihu.com/p/158717151)
2. [UE4中的Subsystem](https://www.cnblogs.com/yejianying/p/15029111.html#wiz-toc-0-95633076)