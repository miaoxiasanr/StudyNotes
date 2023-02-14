- [UE4GamePlay](#ue4gameplay)
	- [Object](#object)
	- [Actor](#actor)
	- [UComponent(继承自UObject)](#ucomponent继承自uobject)
		- [SceneComponent](#scenecomponent)
		- [一些思考](#一些思考)
	- [ULevel](#ulevel)
		- [一些问题](#一些问题)
	- [World](#world)
		- [一些问题](#一些问题-1)
	- [WorldContext](#worldcontext)
	- [GameInstance](#gameinstance)
	- [Engine](#engine)
	- [GamePlayStatics](#gameplaystatics)
	- [Pawn](#pawn)
		- [派生子类](#派生子类)
		- [一些问题](#一些问题-2)
	- [AController](#acontroller)
		- [MVC](#mvc)
		- [AController](#acontroller-1)
		- [派生子类](#派生子类-1)
	- [APlayerState](#aplayerstate)
	- [GameMode](#gamemode)
	- [GameState](#gamestate)

# UE4GamePlay
![](https://pic1.zhimg.com/80/v2-c0cd2e5121f63c37615f78476e2a425c_720w.jpg)
## Object
![](https://pic3.zhimg.com/80/v2-750c05a282e8784c3af5815a481d549e_720w.jpg)
1. 依靠着UObject提供的元数据、反射、GC垃圾回收、序列化和反序列化、网络同步RPC，编辑器可见性、Class Default Object等特性，UE构建一个UObject的世界
2. sizeof(UObject)=56;
## Actor
![](https://pic3.zhimg.com/80/v2-9348f6bdadbe382a505aff9be7d5d99e_720w.jpg)
1. 功能
   1. 继承UObject的特性
   2. replication（网络复制）
   3. Spawn
   4. Tick
   5. 相互嵌套，拥有父子关系
   6. 处理Input事件的能力（转发到内部的UInputComponent里处理）
2. 一些思考
   1. 为何Actor不像Object一样自带Transform？
   在UE看来，Actor不止是会在3D世界中显示出来，也有一些不会再3D世界中显示出来的“不可见对象”，例如AAInfor的派生类（AWorldSetting,AGameState,APlayerState等），AHUD等代表着这个世界的规则，信息，状态。第二个原因是节省内存，保存一个没有用处的矩阵是一个不明智的选择，UE给出的解决方案是把Tranform当成一个可装卸的额外能力，就是把Transform封装成SceneComponent当做RootComponent提供位置信息，获取Actor的Location实际上都是在获取SceneComponent的Location。
   ~~~c++
   //GetActorLocation
   	FORCEINLINE FVector GetActorLocation() const
	{
		return TemplateGetActorLocation(RootComponent);
	}
   	template<class T>
	static FORCEINLINE const FTransform& TemplateGetActorTransform(const T* RootComponent)
	{
		return (RootComponent != nullptr) ? RootComponent->GetComponentTransform() : FTransform::Identity;
	}
    //SetActorLocation
    bool AActor::K2_SetActorLocation(FVector NewLocation, bool bSweep, FHitResult& SweepHitResult, bool bTeleport)
    {
	    return SetActorLocation(NewLocation, bSweep, (bSweep ? &SweepHitResult : nullptr), TeleportFlagToEnum(bTeleport));
    }
    bool AActor::SetActorLocation(const FVector& NewLocation, bool bSweep, FHitResult* OutSweepHitResult, ETeleportType Teleport)
    {
	    if (RootComponent)
	    {
		    const FVector Delta = NewLocation - GetActorLocation();
		    return RootComponent->MoveComponent(Delta, GetActorQuat(), bSweep, OutSweepHitResult, MOVECOMP_NoFlags, Teleport);
	    }
	    else if (OutSweepHitResult)
	    {
		    *OutSweepHitResult = FHitResult();
	    }

	    return false;
    }
   ~~~
3. 
## UComponent(继承自UObject)
![](https://pic4.zhimg.com/80/v2-825217f7dc7b7f3ce2068433b037dfb7_720w.jpg)
TSet<UActorComponent*> OwnedComponents保存着这个Actor所拥有的所有Component，一般其中会有一个SceneComponent作为RootComponent
~~~c++
	/**
	 * All ActorComponents owned by this Actor. Stored as a Set as actors may have a large number of components
	 * @see GetComponents()
	 */
	TSet<UActorComponent*> OwnedComponents;
~~~
TArray<UActorComponent*> InstanceComponents则会保存着实例化的Components，但OwnedComponents总是最全的
~~~c++
	/** Array of ActorComponents that have been added by the user on a per-instance basis. */
	UPROPERTY(Instanced)
	TArray<UActorComponent*> InstanceComponents;
~~~
### SceneComponent
SceneComponent的两大能力
1. Transform
2. SceneComponent的互相嵌套
![](https://pic1.zhimg.com/80/v2-91234c7d5bc32dd04c7221ac9dcc56d0_720w.jpg)
### 一些思考
1. 为何ActorComponent不能相互嵌套？SceneComponent可以相互嵌套？
   我认为SceneComponent能够相互嵌套是UE为我们提供一种位置的相对关系，各个SceneComponent在3D世界里的Transform一一对应起来
   而ActorComponent是为了编写功能单一的Component，例如UMovementComponent，而嵌套的话会把一个Component整个其他Component的功能而变得很复杂，恰恰和UE的理念有冲突
2. Actor的父子关系
   Actor之间的父子关系是通过Component确定的，同一般的Parent：AddChild操作不同，UE是通过Child:AttachToActor或Child:AttachToComponent来创建父子关系的，由于一个Actor是可以带多个SceneComponent的，所以在创建父子关系时，需要指定Transform的锚点(SceneComponent的挂点(Socket))
    ~~~c++
    void AActor::AttachToActor(AActor* ParentActor, const FAttachmentTransformRules& AttachmentRules, FName SocketName)
    {
	    if (RootComponent && ParentActor)
	    {
		    USceneComponent* ParentDefaultAttachComponent = ParentActor->GetDefaultAttachComponent();
		    if (ParentDefaultAttachComponent)
		    {
			    RootComponent->AttachToComponent(ParentDefaultAttachComponent, AttachmentRules, SocketName);
		    }
	    }
    }
    void AActor::AttachToComponent(USceneComponent* Parent, const FAttachmentTransformRules& AttachmentRules, FName SocketName)
    {
	    if (RootComponent && Parent)
	    {
		    RootComponent->AttachToComponent(Parent, AttachmentRules, SocketName);
	    }
    }
    ~~~

## ULevel
![](https://pic3.zhimg.com/v2-bca44e1f846c37b12f08bc0a6659b4ae_r.jpg)
Actor的容器，同时也划分了World，一方面支持了Level的动态加载，另一方面也允许了团队的协作，同时编辑不同的Level
1. ALevelScriptActor:Level的管理者，允许我们在关卡里编写脚本，可以对关卡里的所有Actor通过名字获取
2. AWorldSetting：只和Level有关
### 一些问题
1. 为何AWorldSetting要放在Actors[0]的位置？而ALevelScriptActor却不用？
~~~c++
void ULevel::SortActorList()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_Level_SortActorList);
	if (Actors.Num() == 0)
	{
		// No need to sort an empty list
		return;
	}
	LLM_REALLOC_SCOPE(Actors.GetData());

	TArray<AActor*> NewActors;
	TArray<AActor*> NewNetActors;
	NewActors.Reserve(Actors.Num());
	NewNetActors.Reserve(Actors.Num());

	if (WorldSettings)
	{
		// The WorldSettings tries to stay at index 0
		NewActors.Add(WorldSettings);

		if (OwningWorld != nullptr)
		{
			OwningWorld->AddNetworkActor(WorldSettings);
		}
	}

	// Add non-net actors to the NewActors immediately, cache off the net actors to Append after
	for (AActor* Actor : Actors)
	{
		if (Actor != nullptr && Actor != WorldSettings && !Actor->IsPendingKill())
		{
			if (IsNetActor(Actor))
			{
				NewNetActors.Add(Actor);
				if (OwningWorld != nullptr)
				{
					OwningWorld->AddNetworkActor(Actor);
				}
			}
			else
			{
				NewActors.Add(Actor);
			}
		}
	}

	NewActors.Append(MoveTemp(NewNetActors));

	// Replace with sorted list.
	Actors = MoveTemp(NewActors);
}

~~~
通过代码可知，Actors的排序依据是吧“非网络”的Actor放在前面，而把“网络复制”Actor“放在后面，AWorldSetting不需要网络复制，可以一直放在前面，而放在第一个的原因是可以和其他"非网络"Actor做一个再度区分，在需要的时候加速判断。ALevelScriptActor是允许携带复制变量的，所以排在后面
## World
![](https://pic2.zhimg.com/80/v2-41963e6f39bcefb2d799d31bec703759_720w.png)
一个World里有多个Level，这些Level是在什么位置，是在一开始就加载进来，还是Streaming运行时加载。UE里面每个World支持一个PersistentLevel和多个其他Level。
Persistent的意思是一开始就加载进World，streaming是后续动态加载的意思。Levels里面保存只有所有的当前已经加载的Level，StreamingLevels保存着整个World的Levels配置列表。PersistentLevel和CurrentLevel只是一个快速引用，编辑时可以指向任何Level，运行时只能指向PersistentLevel。
### 一些问题
1. 为什么要有主PersistentLevel？
   1. 需要有个默认值
   2. WorldSetting的归属，World的Setting是以PersistentLevel为主的



## WorldContext
![](https://pic4.zhimg.com/80/v2-5ded56be67f4082ee7f77a0c3fa0960f_720w.png)
由于world不只有一种类型，所以需要有一个Object来管理和跟踪这些world，这个工具就是WorldContext
~~~c++
namespace EWorldType
{
	enum Type
	{
		None,		// An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels
		Game,		// The game world
		Editor,		// A world being edited in the editor
		PIE,		// A Play In Editor world
		Preview,	// A preview world for an editor tool
		Inactive	// An editor world that was loaded but not currently being edited in the level editor
	};
}
~~~
FWorldContext保存这个ThisCurrentWorld来指向当前的World，而当需要从一个World切换到另一个World的时候，FWorldContext就用来保存着切换过程信息和目标World上下文信息

## GameInstance
![](https://pic4.zhimg.com/80/v2-36b45a7b36ac77d978719bc6fe8db17b_720w.png)
GameInstance里会保存着当前的WorldContext和其他整个游戏的信息
## Engine
![](https://pic1.zhimg.com/80/v2-94d1f4e3750b6f4fd09d02b20bc980b0_720w.png)
## GamePlayStatics
~~~c++
UCLASS ()
class UGameplayStatics : public UBlueprintFunctionLibrary 
~~~
我们在蓝图里见到的GetPlayerController、SpawnActor等都是来自于这个库的接口，相当于C++的静态类，只为蓝图暴露提供一些静态方法。

## Pawn
![](https://pic2.zhimg.com/80/v2-12b8b0034f3068f8d7c2739cb1f654a5_720w.png)
定义了三个基本的模板方法接口
1. 可被Controller控制
2. PhysicsCollision表示
3. MovementInput的基本响应接口

### 派生子类
1. DefaultPawn
   默认Pawn
   默认带了一个DefaultPawnMovementComponent、CollisionComponent和StaticMeshComponent
2. SpectatorPawn
   观战玩家
   提供基本的USpectatorPawnMovement(不带重力移动)，并且关闭了StaticMesh的显示，碰撞也设置到了"Spectator"通道
3. Character
   人行Pawn
   默认带CharacterMovementComponent，CapsuleComponent,SkeltalMeshComponent

### 一些问题
1. 为什么Actor也能接受Input事件
   EnableInput接口是在Actor上的，同时InputComponent也是在Actor里面的；
   输入事件的种类是有很多的。例如：按键，遥杆，触摸和陀螺仪，不能确定和分类哪些Actor的子类该接受什么样的输入事件，
   同时有事因为Actor是由Component组件化组装而成的，UE不可能为了输入事件就改变Component的组织方式，还不如在Actor的基类里提供InputComponent的集成，保证了灵活性。
## AController
### MVC
![](https://pic2.zhimg.com/80/v2-6bfd369c2f163d0fd730bb7db8c5a7f9_720w.jpg)
程序=数据+算法+显示  对于游戏
* 显示 指的是游戏的UI，是屏幕上显示的3DUI，或是手柄上的输入和震动，也可是VR头像上的镜片和定位，是与玩家直接交互的载体
* 数据 指的是Mesh，Material，Actor，Level等各种元素组织起来的内存数据表示
* 算法 可以是各种渲染算法，物理模拟，AI寻路，
### AController
![](https://pic2.zhimg.com/80/v2-4151952d1f2ab74fcc78d7c3bd215e0d_720w.jpg)
一个Controller能够灵活的Possess/UnPossess一个Pawn，但一次只能possess一个Pawn，但是我们可以灵活的切换Pawn
### 派生子类
* APlayerController
![](https://pic1.zhimg.com/80/v2-88131e55febd8886e0f3c87b92c526e8_720w.png)
  Input输入事件
  Uplayer的关联
  HUD的显示
  Level的切换
* AAlController
* ![](https://pic3.zhimg.com/80/v2-a0c2148ff8331da1b70ab4157e19f1c2_720w.png)
  增加了一些AI需要1的组件
  Navigation
  行为树，黑板
  Task任务
## APlayerState
![](https://pic1.zhimg.com/80/v2-ba203b15c1e9356d5aa7fe6bf2fd556c_720w.jpg)
~~~c++
 /** PlayerState containing replicated information about the player using this controller (only exists for players, not NPCs). */
    UPROPERTY(replicatedUsing=OnRep_PlayerState, BlueprintReadOnly, Category="Controller")
    class APlayerState* PlayerState;
~~~
PlayerState也理所当然的生成在level中，更Pawn和Controller是平级的关系，Controller里也只不过保存了一个指针引用，注释里说PlayerState只为Players存在，不为NPC生成，指的是PlayerState是跟UPlayer对应的，有多少个玩家就会有多少个PlayerState，而那些AL控制的NPC因为不是真正的玩家。
玩家在切换关卡时，AplayerState也会被释放掉，所有的PlayerState实际上表达的是当前关卡的玩家得分等数据。所以跨关卡的统计数据不该放到PlayerState，应该放在外面的GameInstance，然后SaveGame保存起来；

## GameMode
![](https://pic2.zhimg.com/80/v2-df7c0a12f2b806dbb60e84a95ae9758d_720w.png)
功能
    1. Class登记，GameMode里登记了游戏里基本需要的类型信息，在需要的时候通过UClass的反射自动Spawn出相应的对象来添加进关卡里
    2. 游戏进度，一个游戏支不支持暂停，怎么重启等涉及游戏内的操作也是gameMode的工作之一
    3. Level的切换(world的切换)，Gamemode也决定了刚进入到一场游戏是否应该播放开场动画等
    4. 多人游戏的步调统一：在多人游戏的时候，尝尝需要等所有玩家都连上之后，载入地图完毕后才能一起开始逻辑
## GameState
![](https://pic2.zhimg.com/80/v2-336fd667fb674bf176fa198741eec129_720w.png)