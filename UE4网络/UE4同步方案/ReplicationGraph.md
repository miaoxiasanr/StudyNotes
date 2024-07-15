# ReplicationGraph
## 总体思路
根据类型和状态对Actor进行节点划分，每次针对当前Connection只检查所在Grid内对象的信息来大大降低整个Replication的计算量，节省CPU时间。
![](../../Photo/ReplicationGraph.png)
结构图
![](./../../Photo/ReplicationGraph.jpg)
* UReplicationGraph:同步图表，NetDriver的网络分发路口
* GridSpatialization2D:按照2D空间划分的同步节点。
* PlayerState:与状态有关的节点，比如限制更新频率的对象
* AlwaysRelevant(All):与所有的链接都有相关性
* Tear Off:类似于OwnerRelevant,只同步给主客户端，不同步给模拟客户端。
* AlwaysRelevant(Connection):只与某一链接具有相关性，比如走进某一区域点亮一盏灯，只对这个链接有作用。
* AlwaysRelevant(Team):与队友相关的链接，比如复活次数等等。
* Static:静态同步对象(可破环物体)，该Actor创建后，坐标位置不会改变
* Dynamic:可以移动的同步对象，动态，该Actor会任意移动。
* Dormant:休眠对象。该Actor暂时不会动，但可以转变为动态。
* Streaming:和关卡有关的对象，只有关卡加载了才会进行同步。


## ReplicationGraphNode
### GridSpatialization2D
此节点是基于默认同步方案的距离裁剪的优化。对于优化复制压力，流量，客户端性能来说，裁剪是必不可少的，二默认方案中，全量遍历裁剪，存在遍历量过大的问题。

#### 位置计算和复制
![](https://static-1251934385.file.myqcloud.com/2022/1023/ReplicationGraph_GridSpatialization_CellCalc.svg)
2D网格节点在XY平面上将世界划分成多个正方形格子，以上图为例，某个Actor其裁剪半径为R，则上图的虚线是它的影响范围。作这个圆的外接正方形，外接正方形所覆盖到的范围的格子，都会被添加到该Actor。从图中来看，圆形范围只占5个格子，但是其覆盖范围是9个格子。

![](https://static-1251934385.file.myqcloud.com/2022/1023/ReplicationGraph_Grid_RedDot_123.svg)

当进行复制时，是以客户端连接的ViewTarget为位置参考的。假设图中的红点为客户端A点的ViewTarget。图中黑点为客户端B控制的Pawn，虚线代表B点Pawn的裁剪范围。
图1中，显然红点和黑点不再同一格子内；但按照上文所诉，黑点范围是包含了红点所在格子的，因此通过ViewTarget坐标得到红点位置，遍历其持有的Actor列表，是含有黑点所代表的Pawn的，此时会再次判断两点之间距离，发现距离并不符合要求，因此还是会被裁剪掉，因此此时红点不能同步黑点所代表的Pawn。
图2中，红点往上移动，进入虚线正方形范围后，黑点所代表的Pawn依然不会被复制下来。虚线正方形表示的是黑点所代表的Pawn将出现在哪些格子里。
图3中，红点进入了虚线圆形，此时会复制了，虚线圆形才是黑点所代表的Pawn真正的有效范围。




## 过程

### ReplicationGraph的初始化
* UReplicationGraph::InitForNetDriver
  ~~~c++
  void UReplicationGraph::InitForNetDriver(UNetDriver* InNetDriver)
  {
    	NetDriver = InNetDriver;

    	InitGlobalActorClassSettings();
    	InitGlobalGraphNodes();
      //遍历所有的NetDriver->ClientConnections,调用AddClientConnection(ClientConnection)。
    	for (UNetConnection* ClientConnection : NetDriver->ClientConnections)
    	{
    		AddClientConnection(ClientConnection);
    	}
  }
  ~~~
   1. ->InitGlobalActorClassSettings
   ~~~c++
   void UReplicationGraph::InitGlobalActorClassSettings()
   {
	   // AInfo and APlayerControllers have no world location, so distance scaling should always be 0
	   FClassReplicationInfo NonSpatialClassInfo;
	   NonSpatialClassInfo.DistancePriorityScale = 0.f;
      //GlobalActorReplicationInfoMap中添加AInfo和APlayerController
	   GlobalActorReplicationInfoMap.SetClassInfo( AInfo::StaticClass(), NonSpatialClassInfo );
	   GlobalActorReplicationInfoMap.SetClassInfo( APlayerController::StaticClass(), NonSpatialClassInfo );
      //默认设置RPC_Multicast_OpenChannelForClass中Actor的RPC为True；
	   RPC_Multicast_OpenChannelForClass.Reset();
	   RPC_Multicast_OpenChannelForClass.Set(AActor::StaticClass(), true); // Open channels for multicast RPCs by default
	   RPC_Multicast_OpenChannelForClass.Set(AController::StaticClass(), false);
	   RPC_Multicast_OpenChannelForClass.Set(AServerStatReplicator::StaticClass(), false);	
   }
   ~~~
   //UBasicReplicationGraph:UReplicationGraph 官方写的一个继承类。
   ~~~c++
   void UBasicReplicationGraph::InitGlobalActorClassSettings()
   {
	   Super::InitGlobalActorClassSettings();
	   for (TObjectIterator<UClass> It; It; ++It)
	   {
         //
		   UClass* Class = *It;
		   AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		   if (!ActorCDO || !ActorCDO->GetIsReplicated())
		   {
			   continue;
		   }

		   // Skip SKEL and REINST classes.
		   if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		   {
			   continue;
		   }

		   FClassReplicationInfo ClassInfo;

		   // ClassInfo.ReplicationPeriodFrame最小是1，最大是NetDriver->NetServerMaxTickRate/ActorCDO->NetUpdateFrequency.
		   ClassInfo.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(ActorCDO->NetUpdateFrequency);
            //AlwaysRelevant的CullDistanceSquared统一设为0；
		   if (ActorCDO->bAlwaysRelevant || ActorCDO->bOnlyRelevantToOwner)
		   {
			   ClassInfo.SetCullDistanceSquared(0.f);
		   }
		   else
		   {
			   ClassInfo.SetCullDistanceSquared(ActorCDO->NetCullDistanceSquared);
		   }
		
		   GlobalActorReplicationInfoMap.SetClassInfo( Class, ClassInfo );
	   }
   }
   ~~~
   2. ->InitGlobalGraphNodes，逻辑都在UBasicReplicationGraph，用于继承类，自定义自己的需要的Node；
   ~~~c++
   void UReplicationGraph::InitGlobalGraphNodes()
    {
    	// TODO: We should come up with a basic/default implementation for people to use to model
    }
   ~~~
   ~~~c++
   void UBasicReplicationGraph::InitGlobalGraphNodes()
   {
    // -----------------------------------------------
    //	Spatial Actors
    // -----------------------------------------------

    GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
    GridNode->CellSize = 10000.f;
    GridNode->SpatialBias = FVector2D(-WORLD_MAX, -WORLD_MAX);

    AddGlobalGraphNode(GridNode);

    // -----------------------------------------------
    //	Always Relevant (to everyone) Actors
    // -----------------------------------------------
    AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
    AddGlobalGraphNode(AlwaysRelevantNode);
   }
   ~~~
* UReplicationGraph::AddClientConnection
  ~~~c++
  void UReplicationGraph::AddClientConnection(UNetConnection* NetConnection)
  {
     //.....
     //依据旧的Connection，创建新的UNetReplicaionGraphConnection.
  	   Connections.Add(CreateClientConnectionManagerInternal(NetConnection));
  }
  ~~~
  ~~~c++
  UNetReplicationGraphConnection* UReplicationGraph::CreateClientConnectionManagerInternal(UNetConnection* Connection)
  {
    	repCheckf(Connection->GetReplicationConnectionDriver() == nullptr, TEXT("Connection %s on NetDriver %s already has a ReplicationConnectionDriver %s"), *GetNameSafe(Connection), *GetNameSafe(Connection->Driver), *Connection->GetReplicationConnectionDriver()->GetName() );

    	// Create the object
    	UNetReplicationGraphConnection* NewConnectionManager = NewObject<UNetReplicationGraphConnection>(this, ReplicationConnectionManagerClass.Get());

    	//分配ConnectID
    	const int32 NewConnectionNum = Connections.Num() + PendingConnections.Num();
    	NewConnectionManager->ConnectionOrderNum = NewConnectionNum;
    	PRAGMA_DISABLE_DEPRECATION_WARNINGS
    	NewConnectionManager->ConnectionId = NewConnectionNum;
    	PRAGMA_ENABLE_DEPRECATION_WARNINGS

    	// ActorInfo里加入GlobalData的Info
    	NewConnectionManager->InitForGraph(this);
          void UNetReplicationGraphConnection::InitForGraph(UReplicationGraph* Graph)
          {
            	// The per-connection data needs to know about the global data map so that it can pull defaults from it when we initialize a new actor
            	TSharedPtr<FReplicationGraphGlobalData> Globals = Graph ? Graph->GetGraphGlobals() : nullptr;
            	if (Globals.IsValid())
            	{
            		ActorInfoMap.SetGlobalMap(Globals->GlobalActorReplicationInfoMap);
            	}
          }
    	// Associate NetConnection with it
    	NewConnectionManager->InitForConnection(Connection);

    	// Create Graph Nodes for this specific connection
    	InitConnectionGraphNodes(NewConnectionManager);

    	return NewConnectionManager;
  }
  ~~~
* UReplicationGraph::InitializeActorsInWorld
  ~~~c++
  void UReplicationGraph::InitializeActorsInWorld(UWorld* InWorld)
   {
    	check(GraphGlobals.IsValid());
    	checkf(GraphGlobals->World == InWorld, TEXT("UReplicationGraph::InitializeActorsInWorld world mismatch. %s vs %s"), *GetPathNameSafe(GraphGlobals->World), *GetPathNameSafe(InWorld));

    	if (InWorld)
    	{
    		if (InWorld->AreActorsInitialized())
    		{
    			InitializeForWorld(InWorld);
    		}
    		else
    		{
    			// World isn't initialized yet. This happens when launching into a map directly from command line
    			InWorld->OnActorsInitialized.AddLambda([&](const UWorld::FActorsInitializedParams& P)
    			{
    				this->InitializeForWorld(P.World);
    			});
    		}
    	}
   }
  ~~~
  ~~~c++
  void UReplicationGraph::InitializeForWorld(UWorld* World)
    {
      //清理数据
    	ActiveNetworkActors.Reset();
    	GlobalActorReplicationInfoMap.ResetActorMap();
      //调用所有Node的NotifyResetAllNetworkActors；
    	for (UReplicationGraphNode* Manager : GlobalGraphNodes)
    	{
    		Manager->NotifyResetAllNetworkActors();
    	}

    	for (UNetReplicationGraphConnection* RepGraphConnection : Connections)
    	{
    		RepGraphConnection->NotifyResetAllNetworkActors();
    	}
    	//遍历World里所有的Actor，调用AddNetworkActor(Actor);
    	if (World)
    	{
    		for (FActorIterator Iter(World); Iter; ++Iter)
    		{
    			AActor* Actor = *Iter;
    			if (Actor != nullptr && !Actor->IsPendingKill() && ULevel::IsNetActor(Actor))
    			{
    				AddNetworkActor(Actor);
    			}
    		}
    	}
    }
  ~~~
* UReplicationGraph::AddNetworkActor
  ~~~c++
  void UReplicationGraph::AddNetworkActor(AActor* Actor)
    {
    	QUICK_SCOPE_CYCLE_COUNTER(UReplicationGraph_AddNetworkActor);
      //Condition
    	if (IsActorValidForReplicationGather(Actor) == false)
    	{
    		return;
    	}

    	if (NetDriver && !NetDriver->ShouldReplicateActor(Actor))
    	{
    		return;
    	}

    	bool bWasAlreadyThere = false;
      //加入ActiveNetworkActors，
    	ActiveNetworkActors.Add(Actor, &bWasAlreadyThere);
    	if (bWasAlreadyThere)
    	{
    		// Guarding against double adds
    		return;
    	}

    	// Create global rep info	
    	FGlobalActorReplicationInfo& GlobalInfo = GlobalActorReplicationInfoMap.Get(Actor);
    	GlobalInfo.bWantsToBeDormant = Actor->NetDormancy > DORM_Awake;

    	RouteAddNetworkActorToNodes(FNewReplicatedActorInfo(Actor), GlobalInfo);
    }
  ~~~
* UReplicationGraph::RouteAddNetworkActorToNodes
  ~~~c++
  void UReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
   {
    	// The base implementation just routes to every global node. Subclasses will want a more direct routing function where possible.
    	for (UReplicationGraphNode* Node : GlobalGraphNodes)
    	{
    		Node->NotifyAddNetworkActor(ActorInfo);
    	}
   }
  ~~~
  ~~~c++
  void UBasicReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
    {
    	if (ActorInfo.Actor->bAlwaysRelevant)
    	{
    		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
    	}
    	else if (ActorInfo.Actor->bOnlyRelevantToOwner)
    	{
    		ActorsWithoutNetConnection.Add(ActorInfo.Actor);
    	}
    	else
    	{
    		// Note that UReplicationGraphNode_GridSpatialization2D has 3 methods for adding actor based on the mobility of the actor. Since AActor lacks this information, we will
    		// add all spatialized actors as dormant actors: meaning they will be treated as possibly dynamic (moving) when not dormant, and as static (not moving) when dormant.
    		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
    	}
    }
  ~~~
### 添加
   生成Actor的时候通过调用各个类型节点的AddNetworkActor,分发Actor的存储。
   ~~~c++
   void UNetDriver::AddNetworkActor(AActor* Actor)
    {
        if (!IsDormInitialStartupActor(Actor))
	    {
		    GetNetworkObjectList().FindOrAdd(Actor, this);
		    if (ReplicationDriver)
		    {
			    ReplicationDriver->AddNetworkActor(Actor);
		    }
	    }
    }
   ~~~

### 转发到ReplicationGraph中
   ~~~c++
   int32 UNetDriver::ServerReplicateActors(float DeltaSeconds)
   {  
      //....
      if (ReplicationDriver)
	   {
		   return ReplicationDriver->ServerReplicateActors(DeltaSeconds);
	   }
      //...
   }
   ~~~
### 总入口函数
   ~~~c++
   int32 UReplicationGraph::ServerReplicateActors(float DeltaSeconds)
   {
      // -------------------------------------------------------
	   //	PREPARE (Global)
      // 各个节点的前期同步准备工作(对于空间划分的Grid主要是刷新各个网格中的Actor列表)
	   // -------------------------------------------------------
		QUICK_SCOPE_CYCLE_COUNTER(NET_PrepareReplication);
      for (UReplicationGraphNode* Node : PrepareForReplicationNodes)
      {
         Node->PrepareForReplication();   
      }
	   
      //处理各个链接的客户端
      for (UNetReplicationGraphConnection* ConnectionManager: Connections)
	   {
         //创建FConnectionGatherActorListParameters结构。
         const FConnectionGatherActorListParameters Parameters(ConnectionViewers, *ConnectionManager, AllVisibleLevelNames, FrameNum, GatheredReplicationListsForConnection);
         {
            //搜集公共节点需要的Actor数据，例如GridSpatialization节点，AlwaysRelevant(All)节点
            for (UReplicationGraphNode* Node : GlobalGraphNodes)
            {
               Node->GatherActorListsForConnection(Parameters);
            }
            
            //相关联的节点中的Actor列表，例如AlwaysRelevant(Connection)节点。
            for (UReplicationGraphNode* Node : ConnectionManager->ConnectionGraphNodes)
            {
               Node->GatherActorListsForConnection(Parameters);
            }

            //对应原始数据的ServerReplicateActors_PrioritizeActors,对收集好的ActorList进行同步优先级排序
            ReplicateActorListsForConnections_Default(ConnectionManager, GatheredReplicationListsForConnection, ConnectionViewers);

            //对应原始流程的ServerReplicateActors_ProcessPrioritizedActors，同步每个通过当前连接同步要求的Actor。
            ReplicateActorListsForConnections_FastShared(ConnectionManager, GatheredReplicationListsForConnection, ConnectionViewers);
         }
      }
      //...
      for (int32 ActorIdx = 0; ActorIdx < PrioritizedReplicationList.Items.Num(); ++ActorIdx)
      {
         //真正Actor进行同步的地方。
         ReplicateSingleActor(Actor, ActorInfo, GlobalActorInfo, ConnectionActorInfoMap, *ConnectionManager, FrameNum);
      }
   }
   ~~~
   * ReplicateSingleActor入口函数，执行Actor内部的CallPreReplication,创建或者获取ActorChannel，执行ChannelReplicateActor,然后执行Actor内部的PostReplicateActor,最后同步所有依赖的ActorList。
   ~~~c++
   int64 UReplicationGraph::ReplicateSingleActor(AActor* Actor, FConnectionReplicationActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalActorInfo, FPerConnectionActorInfoMap& ConnectionActorInfoMap, UNetReplicationGraphConnection& ConnectionManager, const uint32 FrameNum)
   {
      //...
   }
   ~~~


[UE4 ReplicationGraph分析](https://ask.qcloudimg.com/draft/2636436/orgzymepnx.png)