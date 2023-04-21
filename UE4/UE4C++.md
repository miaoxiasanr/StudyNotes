- [UE4](#ue4)
  - [编译工具](#编译工具)
  - [UE4反射(可以选择加入)](#ue4反射可以选择加入)
    - [标记](#标记)
  - [游戏开发中的3C](#游戏开发中的3c)
    - [Character](#character)
    - [Control](#control)
    - [Camera](#camera)
  - [软引用和强引用](#软引用和强引用)
    - [软引用](#软引用)
    - [强引用](#强引用)
  - [同步加载和异步加载](#同步加载和异步加载)
    - [同步加载](#同步加载)
    - [异步加载](#异步加载)
  - [Actor的生命周期](#actor的生命周期)
    - [从磁盘加载](#从磁盘加载)
    - [PlayerInEditor](#playerineditor)
    - [SpawnActor](#spawnactor)
    - [SpawnActorDeferred(延迟生成)](#spawnactordeferred延迟生成)
    - [生命周期的结束](#生命周期的结束)
  - [垃圾回收](#垃圾回收)
  - [委托](#委托)
    - [单播委托](#单播委托)
    - [动态委托](#动态委托)
    - [动态多播委托](#动态多播委托)
    - [委托应用场景](#委托应用场景)
  - [其他](#其他)
  - [引用](#引用)

# UE4
## 编译工具
UBT(UnrealBuiltTool，c#):UE4的自定义工具，各个模块的编译并处理各模块之间的依赖关系。我们编写的Target.cs,Build.cs都是为这个工具服务的。
UHT(UnrealHeaderTool，c++):C++代码解析生成工具，负责处理引擎反射系统编译所需要的信息，是支持UObject系统的自定义解析的代码生成工具。
代码编译分两个阶段进行：
   1. UBT调用UHT，他将解析C++头文件中引擎相关类元数据，并生成自定义代码，已实现诸多UObject相关的功能
   2. UBT调用普通C++\编译器，以便对结果进行编译

完整编译流程:UBT搜集目录中的.cs文件，然后UBT调用UHT分析需要分析的.h,.cpp文件(典型根据文件是否含有#include"FileName.generated.h"，是否含有UCLASS(),UPROPERTY()等宏)生成generated.h和gen.cpp文件，生成的问件路径为Intermediate->build-Win64-UE4-inc,最后UBT调用MSBuild，将.h.cpp和generated.f,gen.cpp结合到一起然后编译

## UE4反射(可以选择加入)
具体作用：对于一个类，我们可以会得这个类的所有属性和方法，而对于一个类对象，我们可以调用他所拥有的方法和属性
> UE4使用反射可以实现序列化。editor的details panel。垃圾回收，网络复制，蓝图/c++通信和相互调用等功能。
###反射系统实现原理
UObject是反射系统的核心。每一个继承UObject且支持反射系统的类型都有一个相应的UClass或者它的子类，UClass中包含了该类的信息。UObject与UClass也组成了UE4对象系统的基石。
### 标记
为了标记一个含有反射类型的文件，需要添加一个特殊的include，这时UHT知道需要处理这个文件
> include"FileName.generated.h"
可以使用UENUM(),UCLASS(),UFUNCTION(),UPROPERTY()来标记不同的类型和成员变量，标记也可以包含额外的描述关键字。
也可以声明成非反射类型的属性，这些属性对反射系统是非可见性的(因此储存一个引用UObject的裸指针是非常危险的，因为垃圾回收系统看不到这个引用，而UE4也提供了FReferenceCollector来手动添加对UObject的引用)

## 游戏开发中的3C
### Character
* 移动
  * 看起来很简单，其实有很多细节
  * 角色状态
  * 与场景交互
  * 需要接入物理系统来实现更加真实的效果
  * 状态机实现
* 战斗
* 属性
* 技能
* 天赋
* 等等
### Control
* 不同输入设备
  * 手柄 鼠标 模拟装置
* 不同的输入
* 相机的拉近和拉远
* 瞄准辅助
  * 从眼睛->脑->手->输入设备->机器反馈，整体有100到200的延迟，所以需要辅助瞄准
* 反馈
  * 震动
* 组合键&按键序列
### Camera
* 相机基本属性
  * FOV(Field of view)
  * POV(Point of view)
* 相机绑定
  * 相机贴墙处理
  * 相机拉近，平滑处理
* 恐惧的时候，瞳孔会缩小，聚焦一个点，看的东西比较小
* 相机效果
  * 相机抖动
  * 相机后处理
* 多相机管理
  * 第一视角和第三视角切换
  * 双人游戏相机分屏处理
* 主观感受
  * 比如加速->加速度，动态模糊，拉近相机
  * 相机惯性
## 软引用和强引用
### 软引用
即对象A的通过间接机制（例如字符串形式的对象路径）来引用对象B
* 软引用可以减少加载负担，可以缩短程序启动时间
* 软引用不会主动加载到内存中，在需要时加载，用完释放。
例子
1. FSoftOjectPath
   * FSoftOjectPath是一个简单的结构体，其中有一个字符串包含资源的完整名称
   * FSoftOjectPath.SolveObject()可以检查其引用的资源是否已经载入在内存中，若载入则返回资源引用对象，否则返回空
   * FSoftOjectPath.Reset() 重置软引用为空
   * AllowedClasses meta标签可以筛选资源类型
~~~c++
// .h 文件
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObject", meta = (AllowedClasses = "SkeletalMesh, StaticMesh" ))
	FSoftObjectPath SoftObjectPath1;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObject", meta = (AllowedClasses = "Texture2D"))
	FSoftObjectPath SoftObjectPath2;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObject", meta = (AllowedClasses = "Blueprint Class"))
	FSoftObjectPath SoftObjectPath3;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObject", meta = (AllowedClasses = "Drone")) //自定义类型 不推荐
	FSoftObjectPath SoftObjectPath4;


// .cpp文件
void ADrone::BeginPlay()
{
	Super::BeginPlay();

	if (SoftObjectPath1.IsValid()){ /* 处理*/ }
	if (SoftObjectPath2.IsNull()){ /* 处理*/ }
	if (SoftObjectPath3.IsAsset()){ /* 处理*/ }
	FString SoftObjectPath4_AssetName = SoftObjectPath4.GetAssetName();			
	FString SoftObjectPath3_AssetPath = SoftObjectPath3.GetAssetPathString();
}
~~~ 
2. FSoftClassPath
   * FSoftClassPath继承自FSoftObjectPath,用于储存一个类型的软引用
   * MetaClass meta标签可以筛选类类型
~~~c++
// .h 文件
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectClass")
	FSoftClassPath SoftClassPath;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectClass", meta = ( MetaClass= "Pawn"))
	FSoftClassPath SoftClassPath_Pawn;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectClass", meta = (MetaClass = "Drone"))
	FSoftClassPath SoftClassPath_Drone;

~~~
3. TSoftObjectPtr<T>
   * TSoftObjectPtr基本上包含了FSoftObjectPath的TWeakObjectPtr
   * 可用于在异步加载完成触发回调函数时，获取资源对应的对象指针
   * TSoftObjectPtr.IsPending() 方法可检查资源是否已准备好可供访问
   * TSoftObjectPtr.Get() 如果被引用资源存在于内存中，将返回这个资源
~~~c++
// .h 文件
// 目前使用4.26，不加 <T> 编译不通过
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectPtr")
	TSoftObjectPtr<UObject> SoftObjectPtr1;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectPtr")
	TSoftObjectPtr<UObject> SoftObjectPtr2;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftObjectPtr")
	TSoftObjectPtr<UTexture2D> SoftObjectPtr_Texture2D;

// .cpp 文件 BeginPlay() 函数内 
SoftObjectPtr2 = TSoftObjectPtr<AActor>(SoftObjectPath3); //可用 FSoftObjectPath 参数初始化

//此处资源未加载，因而判断为false
if (SoftObjectPtr_Texture2D.IsPending()) 
{
	//获取资源
	UTexture2D* MyTexture = SoftObjectPtr_Texture2D.Get();
}

//转换成 FSoftObjectPath
FSoftObjectPath AActorSoftPath1 = SoftObjectPtr_Texture2D.ToSoftObjectPath();
~~~
4. TSoftClassPtr<T>
   * 获取类的软引用，转成 UClass*
~~~c++
// .h
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftClassPtr")
	TSoftClassPtr<AActor> SoftClassPtr_Actor;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftClassPtr")
	TSoftClassPtr<ADrone> SoftClassPtr_Drone;

UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SoftClassPtr")
	TSoftClassPtr<UUserWidget> SoftClassPtr_UserWidget;


// .cpp 

if (SoftClassPtr_Actor.IsPending())
{
	UClass* MyActor = SoftClassPtr_Actor.Get();
}
~~~
### 强引用
即对象A引用对象B，并导致对象B在对象A加载是加载
* 强引用过多会导致运行时很多暂时用不到的资源也被加载到内存中
* 大量资源会导致进程阻塞，致使程序启动时间过长
* 用处不大的资源也在内存中，会占用内存
例子
1. 直接属性引用
   引用资源的最简单的方法就是创建指针，并通过UPROPERTY宏公开，这样允许设计人员通过蓝图继承对原型指定特定资源，或通过放在环境中的实例来指定该资源
~~~c++
UPROPERTY(VisiableAnywhere)
    UStaticMeshComponent*body;
UPROPERTY(VisiableAnywhere)
    class UPhysicsThrusterComponent*upThrusterComp;
~~~
2. TSubclass与UClass
~~~C++
UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="class")
    UClass *classRef;

UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="class")
    TSubclass<AActor> ActorClassRef;//TSubclass具有类型检查的功能

ClassRef=ActorClassRef.Get();
~~~
3. 构造是引用
* ConstructorHeapers::FObjectFinder<T> 一般用来加载非蓝图资源
* ConstructorHeapers::FClassFinder<T> 一般用来加载蓝图class
* FObjectFinder和FClassFinder构造函数都是调用LoadObject();
* 只能在类的构造函数中使用，否则会Crash
* 变量名必须是static类型，也可以使用auto 
~~~c++
// 构造函数
// FObjectFinder 方法一
auto paddleMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Demo_Drone/SM/paddle.paddle'"));
if (paddleMesh.Object != nullptr)
{
	paddle1->SetStaticMesh(paddleMesh.Object);
}

//FObjectFinder 方法二
static ConstructorHelpers::FObjectFinder<UStaticMesh> paddleMesh(TEXT("StaticMesh'/Game/Demo_Drone/SM/paddle.paddle'"));
if (paddleMesh.Succeeded())
{
	paddle1->SetStaticMesh(paddleMesh.Object);
}

// FClassFinder
static ConstructorHelpers::FClassFinder<AActor> BPClassFinder(TEXT("Blueprint'/Game/CPPFunction/Load/BP_MyActor'"));
if (BPClassFinder.Succeeded()) //或者使用 BPClassFinder.Class != nullptr 判断
{
	UClass* MyActorClass = BPClassFinder.Class.Get();
	TSubclassOf<AActor>BP_MyActorClass = BPClassFinder.Class;
	UE_LOG(LogTemp, Warning, TEXT("class name:%s"),*BP_MyActorClass->GetName());
}
~~~

## 同步加载和异步加载
对于资源操作，UE4有同步加载和异步加载两种方式。同步加载是阻塞操作，比如LoadObject函数，会阻塞主线程，如果加载一个较大资源，或者外部依赖较多的资源，会造成游戏明显卡顿。异步加载会使用另一个专用的异步加载线程来加载资源，或者依然在主线程做加载，只是使用异步编程模式，在tick中分散加载一些资源。异步加载不会阻塞主线程，常用的LevelStreaming和SeamlessTravel，以及EventDrivenLoader都使用了此方式，来营造流畅的游戏体验和编辑器使用体验。
### 同步加载
同步加载阻塞主进程
1. LoadObject
   用来加载资源对象
~~~c++
UMaterial *M_cube=LoadObject<UMaterial>(nullptr,TEXT("Material'/Game/Geometry/Meshes/CubeMaterial.CubeMaterial'"));
if(M_cube)
{
    UE_LOG(LogTemp, Warning, TEXT("Material name:%s"), *M_Cube->GetName());
}
~~~
2. LoadClass
   一般用来加载蓝图类，UClass*
   蓝图类的路径末尾加上_C;
~~~c++
UClass* pClass = LoadClass<AActor>(nullptr, TEXT("Blueprint'/Game/CPPFunction/Load/BP_LoadActor.BP_LoadActor_C'"));
if (pClass)
{
	UE_LOG(LogTemp, Warning, TEXT("pClass name:%s"), *pClass->GetName());
}

TSubclassOf<AActor> BPClass = LoadClass<AActor>(nullptr, TEXT("Blueprint'/Game/CPPFunction/Load/BP_MyActor'"));
if (BPClass)
{
	UE_LOG(LogTemp, Warning, TEXT("BPClass name:%s"), *BPClass->GetName());
}
~~~
3. FStreamableManager::LoadSynchronous
FStreamableManager::内部调用RequestSyncLoad
参数返回一个FStreamableHandle类型的指针
可加载非蓝图资源类
~~~c++
//配合FSoftObjectPath使用(1)
FSoftObjectPath SoftObjectPaths_Mesh1 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh.StaticMesh'"));
UStaticMesh* StaticMesh1 = UAssetManager::GetStreamableManager().LoadSynchronous<UStaticMesh>(SoftObjectPaths_Mesh1);
if (StaticMesh1)
{
	UE_LOG(LogTemp, Warning, TEXT("Mesh1 name:%s"), *StaticMesh1->GetName());
}
//配合FSoftObjectPath使用(2)
FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
FSoftObjectPath SoftObjectPaths_Mesh2 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh_Soft.StaticMesh_Soft'"));
UStaticMesh* StaticMesh2 = streamableManager.LoadSynchronous<UStaticMesh>(SoftObjectPaths_Mesh2);
if (StaticMesh2)
{
	UE_LOG(LogTemp, Warning, TEXT("Mesh2 name:%s"), *StaticMesh2->GetName());
}
//配合TSoftObjectPtr<T>使用
FSoftObjectPath SoftObjectPaths_Mesh3 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh_Soft2.StaticMesh_Soft2'"));
TSoftObjectPtr<UStaticMesh> SoftObjectPtr_Mesh = TSoftObjectPtr<UStaticMesh>(SoftObjectPaths_Mesh3);
UStaticMesh* StaticMesh3 = streamableManager.LoadSynchronous(SoftObjectPtr_Mesh);
if (StaticMesh3)
{
	UE_LOG(LogTemp, Warning, TEXT("Mesh3 name:%s"), *StaticMesh3->GetName());
}
~~~
加载蓝图类UClass*
~~~c++
FSoftObjectPath SoftObjectPaths_Actor1 = FSoftObjectPath(TEXT("Blueprint'/Game/CPPFunction/Load/BP_MyActor.BP_MyActor_C'"));
UClass* BPClass1 = UAssetManager::GetStreamableManager().LoadSynchronous<UClass>(SoftObjectPaths_Actor1);
if (BPClass1)
{
	UE_LOG(LogTemp, Warning, TEXT("BPClass1 name:%s"), *BPClass1->GetName());
}
~~~
4. FStreamableManager::RequestSyncLoad
* 配合FStreamableMAnger、FSoftObjectPath使用
* 返回一个FStreamableHandle类型的指针
* TSharedPtr通过GetLoadedAsset()获取单个资源
* TSharedPtr通过GetLoadedAssets()获取多个资源
~~~c++
//获取单个资源
FSoftObjectPath SoftObjectPaths_Mesh1 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh.StaticMesh'"));
TSharedPtr<FStreamableHandle> Handle1  = UAssetManager::GetStreamableManager().RequestSyncLoad(SoftObjectPaths_Mesh1);	
if (Handle1.IsValid())
{
	UStaticMesh* StaticMesh1 = Cast<UStaticMesh>(Handle1->GetLoadedAsset());
	UE_LOG(LogTemp, Warning, TEXT("Mesh1 name:%s"), *StaticMesh1->GetName());
}
//获取多个资源
FSoftObjectPath SoftObjectPaths_Mesh1 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh.StaticMesh'"));
FSoftObjectPath SoftObjectPaths_Mesh2 = FSoftObjectPath(TEXT("StaticMesh'/Game/CPPFunction/Load/StaticMesh_Soft.StaticMesh_Soft'"));
TArray<FSoftObjectPath> SoftObjectPaths;
SoftObjectPaths.Add(SoftObjectPaths_Mesh1);
SoftObjectPaths.Add(SoftObjectPaths_Mesh2);
TSharedPtr<FStreamableHandle> Handle3 = streamableManager.RequestSyncLoad(SoftObjectPaths);	
{
	TArray<UObject*> Objects;
	Handle3->GetLoadedAssets(Objects);
	for (UObject* item : Objects)
	{
		UStaticMesh* StaticMesh3 = Cast<UStaticMesh>(item);
		UE_LOG(LogTemp, Warning, TEXT("GetLoadedAssets(), item name:%s"), *StaticMesh3->GetName());
	}
}
~~~
### 异步加载
* 为了避免阻塞主线程，可以使用异步加载的方式加载资源
* 异步加载完成后，可以设置回调函数
1. FSreatmableManager::RequestAsyncLoad
单文件加载
~~~c++
UPROPERTY(EditAnywhere, Category = "SoftObjectPath", meta = (AllowedClasses = "StaticMesh"))
	FSoftObjectPath SingeleObject;

UFUNCTION()
	void OnSingleAssetLoadFinshed();
// 函数内部分语句
FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
FStreamableDelegate streamableDelegate = FStreamableDelegate::CreateUObject(this, &ALoadActor::OnSingleAssetLoadFinshed);
streamableManager.RequestAsyncLoad(SingeleObject, streamableDelegate);

// 要回调的函数
void ALoadActor::OnSingleAssetLoadFinshed()
{
	FSoftObjectPtr SoftObjPtr = FSoftObjectPtr(SingeleObject);
	UStaticMesh* mesh = Cast<UStaticMesh>(SoftObjPtr.Get());
	if (mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("mesh name:%s"), *mesh->GetName());
	}
}
~~~
多文件加载
* FSoftObjectPtr是一个结构体，是一种指向UObject的弱指针，无法在蓝图中使用
* TSoftObjectPtr是一个模板类，是通用FSoftObjectPtr的模板话包装器
FSoftObjectPtr
~~~c++
UPROPERTY(EditAnywhere, Category="SoftObjectPath", meta = (AllowedClasses = "StaticMesh"))
	TArray<FSoftObjectPath> ObjectList1;

UFUNCTION()
	void OnMultiAssetsLoadFinshed1();
// 函数内部分语句
FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
FStreamableDelegate streamableDelegate = FStreamableDelegate::CreateUObject(this, &ALoadActor::OnMultiAssetsLoadFinshed1);
streamableManager.RequestAsyncLoad(ObjectList1, streamableDelegate);

// 要回调的函数
void ALoadActor::OnMultiAssetsLoadFinshed1()
{
	for (auto AssetItem : ObjectList1)
	{
		FSoftObjectPtr SoftObjPtr = FSoftObjectPtr(AssetItem); //此处也可用 TSoftObjectPtr<T>
		UStaticMesh* mesh = Cast<UStaticMesh>(SoftObjPtr.Get());
		if (mesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("mesh name:%s"), *mesh->GetName());
		}
	}
}
~~~  
TSoftObjectPtr
~~~c++
UPROPERTY(EditAnywhere, Category = "SoftObjectPath")
	TArray<TSoftObjectPtr<UTexture2D>> ObjectList2;

UFUNCTION()
	void OnMultiAssetsLoadFinshed2();
// 函数内部分语句
FStreamableManager& streamableManager = UAssetManager::GetStreamableManager();
FStreamableDelegate streamableDelegate;
streamableDelegate.BindUObject(this, &ALoadActor::OnMultiAssetsLoadFinshed2);

TArray<FSoftObjectPath> SoftPathList;
for (int32 i=0; i<ObjectList2.Num(); i++)
{
	SoftPathList.Add(ObjectList2[i].ToSoftObjectPath());
}
streamableManager.RequestAsyncLoad(SoftPathList, streamableDelegate);

// 要回调的函数
void ALoadActor::OnMultiAssetsLoadFinshed2()
{
	for (auto AssetItem : ObjectList2)
	{
		UTexture2D* ItemTex = AssetItem.Get();
		if (ItemTex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Texture2D name:%s"), *ItemTex->GetName());
		}
	}
}

~~~
## Actor的生命周期
![](https://docs.unrealengine.com/4.27/Images/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Actors/ActorLifecycle/ActorLifeCycle1.png)
UE4创建Actor的方法主要有如下几种
1. 从磁盘加载
2. PlayerInEditor
3. SpawnActor(动态生成)
4. SpawnActorDeferred(延迟生成)
### 从磁盘加载
已位于关卡中的Actor使用此路径，如LoadMap发生时，或者AddToWorld(从子关卡或流关卡)被调用时，一般都是提前搭建好的场景中资源的加载方式。
1. 包/关卡中的Actor从磁盘中进行加载
2. PostLoad，在序列化Actor从磁盘加载完成后调用，在此处可执行自定义版本化和修复操作。PostLoad和PostActorCreates互斥
3. InitializeActorsForPlay
4. 为未初始化的Actor执行RouteActorInitialize（包含Stream Level的加载）
   * PreInitializeComponent
   * InitializeComponent
   * PostInitializeCompinents；Actor的组件初始化后调用
5. BeginPlay 关卡开始后调用
### PlayerInEditor
与磁盘加载十分相似，然而Actor却并非从磁盘中加载，而是从编辑器中复制而来，一般是在Debug是资源的加载方式
1. PostDuplicate被调用
2. InitalizeActorForPlay
3. 为未初始化的Actor执行RouteActorInitialize
   * PreInitializeComponent
   * InitializeComponent
   * PostInitializeCompinents；Actor的组件初始化后调用
4. BeginPlay 关卡开始后调用
### SpawnActor
生成Actor实例的路径，在实际工程中一般是通过SpawnActor等函数加载的资源
1. SpawnActor被调用
2. PostSpawnInitialize
3. PostActorCreated 创建后即被生成的Actor调用，构建函数类行为在此发生
4. ExecuteConstruction:
   * OnConstruction Actor的构建，蓝图Actor的组件在此处生成，蓝图变量在此处初始化
5. PostActorConstruction:
   * PreInitializeComponent
   * InitializeComponent
   * PostInitializeCompinents；Actor的组件初始化后调用

6. OnActorSpawned 在UWorld上广播
7. BeginPlay被调用
### SpawnActorDeferred(延迟生成)
将任意属性设置为"Expose On Spawn"即可延迟Actor的生成
1. SpawnActorDeferred 生成程序化Actor，在蓝图构建脚本之前进行额外设置
2. PostSpawnInitialize
3. PostActorCreated 创建后即被生成的Actor调用，构建函数类行为在此发生
4. FinishSpawningActor
5. ExecuteConstruction:
   * OnConstruction Actor的构建，蓝图Actor的组件在此处生成，蓝图变量在此处初始化
6. PostActorConstruction:
   * PreInitializeComponent
   * InitializeComponent
   * PostInitializeCompinents；Actor的组件初始化后调用

7. OnActorSpawned 在UWorld上广播
8. BeginPlay被调用
### 生命周期的结束
1. Destroy 游戏在Actor需要被移除时手动调用，但游戏进程仍在继续，Actor被标记为等待销毁并从关卡的Actor数组中移除
2. EndPlay 在数个地方进行调用，保证Actor的生命走向终点，调用EndPlay的全部情形：
   1. 对Destroy显式调用
   2. PlayInEditer终结
   3. 关卡过度（无缝地图或加载地图），包含Actor的流关卡被卸载
   4. Actor的生命周期已过
   5. 应用程序关闭(所有Actor被销毁)
   无论哪个情形出现，Actor都会被标记RF_PendingKill
3. OnDestroy
**垃圾回收过程**
4. BeginDestroy 对象可利用此机会释放内存并处理其他多线程资源，与游戏相关的大多数游戏性功能应在EndPlay处更早的被处理
5. IsReadyForFinishDestroy 垃圾回收过程调用次函数，已确定对象是否被永久解除分配。返回false，次函数即可延迟对象的时机销毁，知道下一个垃圾回收过程
6. FinishDestroy 最后对象被销毁，这是释放内部数据结构的另一个机会，
##ActorComponent流程分析
UActorComponent::OnComponentCreated
UActorComponent::OnRegister
UActorComponent::InitializeComponent
UActorComponent::BeginPlay
UActorComponent::TickComponent
UActorComponent::EndPlay
UActorComponent::UninitializeComponent
UActorComponent::OnUnregister
UActorComponent::OnComponentDestroyed

参考
[ActorComponent流程分析](https://zhuanlan.zhihu.com/p/74084967)
## 垃圾回收
1. 目标
   被UPROPERTY宏修饰或在AddReferencedObject函数被手动调用添加引用的UObject*成员，才能被GC识别或追踪(USTRUCT宏修饰的结构体对象和普通对象一样不会被GC管理)
   当UObject对象没有直接或间接被根节点对象引用或被设置成PendingKill状态，就会被GC标记位垃圾，并被GC回收
2. 回收方式——标记-清扫垃圾回收方式
   1. 第一阶段：定时从一个根集合触发，遍历所有对象，遍历完成后就能标记出可达对象和不可达对象(一帧内完成)
   2. 第二阶段：渐进式的清理这些不可达对象，因为不可达对象将永远不能被访问到，所以会分帧清理他么安，避免了一下子清理很多UObject对象，比如map卸载时，发生明显的卡顿
3. GC处理的时间
   1. 主动引发：执行操作后手动调用GC，而且方式有多种，游戏中可以调用ForceGarbageCollection来让World下次tick时进行垃圾回收。也可以直接调用CollectGarbage进行垃圾回收，引擎中大部分情况都用这种方式主动引发
   2. 自动引发：游戏中大部分垃圾回收都是由UE4自动引发的，普通情况下不需要手动调用GC，当World进行tick时，会调用UEngine::ConditionalCollectGarbage()函数，函数中进行了一些判断，当满足GC条件时，才会执行GC
4. 防止GC的方法：调用AddToRoot函数将UObject对象加到根节点上，调用RemoveFromRoot去除标记会被GC
## 委托
### 单播委托
单播委托指只能绑定一个函数指针的委托，也就是当执行委托时只能触发一个唯一绑定的函数
单播委托可以绑定一个无返回值或有返回值的函数
~~~c++
//无返回值函数委托声明
DECLARE_DELEGATE(DelegateName);//无参
DECLARE_DELEGATE_OneParam(DelegateName,Param1Type);//1个参数
DECLARE_DELEGATE_XXXPAram(DelagateNAme,Param1Typr,...);//多个参数
//有返回值函数委托声明
DECLARE_DELEGATE_RetVal(RetValType,DelegateName);//有返回值无参
DECLARE_DELEGATE_RetVal_OneParam(RetVal,DelagateName,Param1Type);//有返回值有参
DECLARE_DELEGATE_RetVal_XXXParam(RetVal,DelagateName,Param1Type,...);//有返回值有参
~~~
声明委托
~~~c++
DECLARE_DELEGATE(FTestDelegate);
.......
FTestDelegate TestDelegate;
~~~
绑定委托
多种绑定方式
|函数|描述|
|-|-|
|BindSP|绑定SharedPtr指定对象的函数，即纯C++类的函数。因为纯C++类一般会使用TSharedPtr用来内存管理|
|BindThreadSafeSP|绑定线程安全的共享成员函数委托|
|BindRaw|纯C++类变量，可以用其绑定委托|
|BindUFunction|如果成员函数有UFUNCTION宏修饰，可用于绑定委托|
|BindUObject|绑定继承UObject类的成员函数委托|
|BindStatic|绑定静态(全局)函数委托，BindStatic(&MyClass::StaticFunc)|
|BindLambda|绑定一个Lambda函数|
|BindWeakLambda|绑定一个弱引用Lambda函数|
|UnBind|取消绑定委托|
~~~c++
TestDelegate.BindUObject(this,&UMyObiectClass::XXXFunc)
~~~
委托调用
|函数|说明|
|-|-|
|IsBound|检测是否绑定一个委托|
|Execute|不检查绑定而执行委托|
|ExecuteIfBound|绑定一个委托时执行调用|
###多播委托
多播委托可以绑定多个函数，但不能有返回值
事件是特殊类型的多播委托，它在访问Broadcast(),IsBound(),Clear()函数时受限
~~~c++
DECLARE_MULTICAST_DELEGATE(DelegateName);//无参
DECLARE_MULTICAST_DELEGATE_XXXParam(DelegateName,Param1Type,...);//多参
~~~
绑定委托
|函数|描述|
|-|-|
|Add()|	绑定类型参见单播委托，不同之处就是可以绑定多个函数|
|AddStatic()||
|AddRaw()||
|AddSP()||
|AddUObject()||
|Remove()||
|RemoveAll()||
多播执行
Broadcast调用后，会执行所有绑定的委托，但是委托的执行顺序尚未定义。
|函数|描述|
|-|-|
|Broadcast()|执行所有绑定对的委托|
### 动态委托
动态委托可以序列化，其函数可以按命名查找，但执行速度比常规委托慢。
~~~c++
DECLARE_DYNAMIC_DELEGATE(DelegateName);
DECLARE_DYNAMIC_DELEGATE_XXXParam(DelegateName,Param1Type,...);
~~~
绑定委托
|函数|描述|
|-|-|
|BindDynamic(UserObject,FunctionName)||
|AddDynamic(UserObject,FunctionName)||
|RemoveDynamic(UserObject,FunctionName)||
执行委托
|函数|描述|
|-|-|
|IsBound()||
|Execute()||
|ExecuteIfBound|绑定一个委托时执行调用|
### 动态多播委托
动态多播委托可以暴露给蓝图使用，并且声明宏中不仅包含变量类型，也包含变量名。如果暴露给蓝图，需要给委托变量的UPROPERTY宏添加BlueprintAssignable标识符
~~~c++
DECLARE_DYNAMIC_MULTICAST_DELEGATE(DelegateName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ONEPARAM(DelegateName, Param1Type, Param1Name);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_XXXPARAM(DelegateName, Param1Type, Param1Name,...);
~~~
声明委托
~~~c++
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ONEPARAM(FDelegateTest, class AActor*, MyActor);

UPROPERTY(BlueprintAssignable)
FDelegateTest TestDelegate;
~~~

使用委托
~~~c++
TestDelegate.AddDynamic(UEObject,FUnctionName);
~~~
### 委托应用场景
1. 单播委托： 当我们只需要在C++中绑定和调用，且只有一个函数需要绑定委托时，可以使用单播委托。
当然，这个委托的绑定和调用可以通过二次函数调用暴露给蓝图。就是绑定和调用的函数再包裹一层，但一般没有必要。
一般常用的就是函数回调时，通过绑定到单播委托进行回调。

2. 多播委托： 单播和多播最大不同就是多播可以绑定多个函数，且不能有返回值，其它功能与单播委托一样。

3. 动态委托： 顾名思义，就是允许动态绑定，它可以序列化，也就是说可以在蓝图中使用，
4. 动态多播委托： 这个更容易理解，它就是蓝图中的事件调度器（EventDispatcher），可以用其在C++和蓝图中绑定和调用委托。

## 其他
1. UE4函数，事件，宏的区别
   1. 执行引脚
    调用函数的触发事件只能有一个执行引脚，而宏在调用的时候可以有很多执行引脚进入或输出，例如Flip Flop，A输出和B输出交替。
   2. 返回值
   事件没有返回值，函数和宏都有返回值
   3. 延迟节点
   自定义事件可以添加一个系统自带的延迟节点，来延迟事件触发后的执行逻辑，而函数和宏不能添加这个延迟节点，函数的调用会立即执行并等待结果，而事件的调用只是触发。当我们在调用函数的时候，系统一定会等函数执行结束返回结果收，才会有后续动作，而事件的调用，只是触发曾事件的开始，系统就继续往下执行了。
   4. 允许访问范围
   函数和事件都可以跨蓝图类访问，而宏只允许当前定义宏的蓝图类访问。
2. UE4函数，事件的区别
   1. Delay/TimeLine这些需要等待时间的节点，事件中可以调用，函数不能调用
   2. 函数可以有返回值，事件没有返回值。
        >1. 函数调用会立刻执行等待结果，事件调用只是触发
        >2. 函数执行在同一线程，事件执行在不同线程
    3. 函数可以有局部变量，事件没有局部变量
    4. 没有返回值的函数，再被子类Override时会变成事件
3. Blueprintimplementable和BlueprintNativeEvent之间有什么区别？
   BlueprintImplementableEvent和BlueprintNativeEvent两者都是函数标记，用于修饰函数的
    BlueprintImplementableEvent:在C++可以声明函数(不能定义，蓝图重写)，在C++里调用该函数，蓝图内重写该函数
    BlueprintNativeEvent：在C++可以声明和定义函数，在C++里调用该函数，蓝图重写实现该函数（如果蓝图重写，则调的是蓝图的实现，为重写，则默认为C++的实现）
4. 如何解决子弹穿墙问题？
   如果采用的是射线检测的话，LineTraceByChannel函数只会返回首个命中对象，但为了防止真出现子弹穿墙问题，可以让LineTraceByChannel函数只要命中对象就停止射线检测
   如果子弹是一个单独的物体的话，在射击时，只要将墙和子弹的属性都设置为Block，便会阻止子弹继续前进。
5. UE4会对USTRUCT的内存自动垃圾回收吗？
   不会，只有使用UPROPERTY宏标记的USTRUCT才能计入垃圾回收
6. Blueprinttable和BlueprintType的区别？
    Blueprinttable：将使用宏标志的类公开为创建蓝图的可接受基类(如：Object类，Actor类)，默认为 NotBlueprinttable，即不可以创建基类
    BlueprintType：将使用该宏标志的类公开为蓝图中变量的类型（如：int）与之对应的有NotBlueprintType,即不可以在蓝图中创建该类型的变量。
7. UE4中的模块系统
   1. Deleloper主要是跨平台工具，Merge和一些底层的工具
   2. Editor主要是编辑器相关代码
   3. Programs主要是独立于引擎，但大多数又依赖于引擎的工具，例如UBT
   4. ThirdParty第三方库或插件
   5. RunTimes是主要的GamePlay等等和游戏相关的代码
8. 在客户端是否可以获取到AIController
   不可以，AiController只存在于服务器上，通过服务器控制Pawn
9. 宏和修饰符
    [UCLASS](https://blog.csdn.net/u012793104/article/details/78547655?spm=1001.2014.3001.5501)
    [UPROPERTY](https://blog.csdn.net/u012793104/article/details/78480085?spm=1001.2014.3001.5501)
    [USTRUCT](https://blog.csdn.net/u012793104/article/details/78594119)
    [UFUNCTION](https://blog.csdn.net/u012793104/article/details/78487893)
## 引用
[资源的引用](https://www.cnblogs.com/shiroe/p/14691199.html)
[资源的同步加载与异步加载](https://www.cnblogs.com/shiroe/p/14710066.html)
[UE4Actor生命周期](https://zhuanlan.zhihu.com/p/308217207)
[各种Delegate委托的区别和应用](https://blog.csdn.net/github_38111866/article/details/104668701)