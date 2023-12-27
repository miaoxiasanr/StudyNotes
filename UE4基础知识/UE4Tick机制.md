# UE4Tick机制
为了管理时间，Unreal将游戏时间片分隔为Ticks，一个Tick是关卡是关卡中所有Actors更新的最小单位时间。一个Tick一般是10ms-100ms(CPU性能越好，游戏逻辑越简单，tick的时间越短)。
## Tick总流程
![](https://img2022.cnblogs.com/blog/78946/202207/78946-20220725120319459-2105233954.png)

一共有8个TickGroup
~~~c++
/** Determines which ticking group a tick function belongs to. */
UENUM(BlueprintType)
enum ETickingGroup
{
	/** Any item that needs to be executed before physics simulation starts. */
	TG_PrePhysics UMETA(DisplayName="Pre Physics"),

	/** Special tick group that starts physics simulation. */							
	TG_StartPhysics UMETA(Hidden, DisplayName="Start Physics"),

	/** Any item that can be run in parallel with our physics simulation work. */
	TG_DuringPhysics UMETA(DisplayName="During Physics"),

	/** Special tick group that ends physics simulation. */
	TG_EndPhysics UMETA(Hidden, DisplayName="End Physics"),

	/** Any item that needs rigid body and cloth simulation to be complete before being executed. */
	TG_PostPhysics UMETA(DisplayName="Post Physics"),

	/** Any item that needs the update work to be done before being ticked. */
	TG_PostUpdateWork UMETA(DisplayName="Post Update Work"),

	/** Catchall for anything demoted to the end. */
	TG_LastDemotable UMETA(Hidden, DisplayName = "Last Demotable"),

	/** Special tick group that is not actually a tick group. After every tick group this is repeatedly re-run until there are no more newly spawned items to run. */
	TG_NewlySpawned UMETA(Hidden, DisplayName="Newly Spawned"),

	TG_MAX,
};
~~~
各个TickGroup串行执行，保证有序，整个Tick流程代码
~~~c++
void UWorld::Tick( ELevelTick TickType, float DeltaSeconds )
{
    // TArray<FLevelCollection> LevelCollections成员变量  逐Level执行
    for (int32 i = 0; i < LevelCollections.Num(); ++i) 
    {
        SetupPhysicsTickFunctions(DeltaSeconds); 
        TickGroup = TG_PrePhysics; // reset this to the start tick group
        FTickTaskManagerInterface::Get().StartFrame(this, DeltaSeconds, TickType, LevelsToTick);

        RunTickGroup(TG_PrePhysics);
        RunTickGroup(TG_StartPhysics);
        RunTickGroup(TG_DuringPhysics, false); // 不阻塞
        RunTickGroup(TG_EndPhysics);
        RunTickGroup(TG_PostPhysics);

        // 对Timer进行Tick
        GetTimerManager().Tick(DeltaSeconds);  

        // 对Tickable对象进行Tick
        FTickableGameObject::TickObjects(this, TickType, bIsPaused, DeltaSeconds); 

        // 更新相机
        RunTickGroup(TG_PostUpdateWork);
        RunTickGroup(TG_LastDemotable);
        FTickTaskManagerInterface::Get().EndFrame();
    }
}
~~~
Tick步骤详解
|Tick步骤|说明|
|-|-|
|SetupPhysicsTickFunctions(DeltaSeconds)|1. 没有注册，则注册UWorld.StartPhysicsTickFunction到TG_StartPhysics组。注册注册UWorld.EndPhysicsTickFunction到TG_EndPhysics组(只会注册一次，后续执行对于的TickGroup时，调用相应的ExecuteTick()函数)。2.DeferredPhysResourceCleanup()//清理物理资源。 3.设置PhyScene->SetUpForFrame().如Gravity(重力),MaxPhysicsDeltaTime,MaxSubStepDeltaTime,MaxSubSteps,bSubStepping等|
|FTickTaskManagerInstance::Get().StartFrame(this,DeltaSeconds,LEVELTICK_ALL,LevelsToTick)| 1.调用TickTaskSequence.StartFrame(),重置FTickSequencer中的数据 2.设置当前帧的关卡数据到FTickTaskManager.LevelList数组中。 3.循环遍历FTickManager.LevelList数组，执行FTickTaskLevel.STartFrame(),返回为Enabled状态的FTickFunction的个数。4.循环遍历FTickTaskManager.LevelList数组，执行FTickTaskLevel.QueueAllTicks() |
|RunTickGroup(TG_PrePhysics)|FTickTaskManagerInterface::Get().RunTickGruoup(TG_PrePhysics,bBlockTillComplate=true)|










[UE4 Tick机制](https://www.cnblogs.com/kekec/p/14781454.html)