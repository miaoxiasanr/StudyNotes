# UE4网络模块
## 基础知识
### UE4使用的是客户端—服务器模型
网络中的一台计算机作为服务器主持作为多人游戏会话，而其他玩家的计算机作为客户端连接到该服务器，然后服务器和连接的计算机分享游戏状态，并提供一种客户端之间的通信方法。(服务器处理gameplay，客户端向用户显示游戏)
![](https://docs.unrealengine.com/4.27/Images/InteractiveExperiences/Networking/Overview/NetworkPlayExample.webp)

### 网络模式和服务器类型
* 独立
  游戏作为服务器运行，不接受远程客户端连接，参与游戏的玩家必须是本地玩家，
* 客户端
  游戏作为网络游戏会话中与服务器连接的客户端运行，不会运行服务器逻辑
* 聆听服务器
  游戏作为网络多人游戏会话的服务器运行，其接受远程客户端的连接，且直接在服务器上拥有本地玩家
* 专属服务器
  游戏作为主持网络多人游戏会话的服务器运行，其接受远程客户端中的连接，但无本地玩家，为了高效运行，将废弃图形，音效，输入和面向其他玩家的功能


  >Actor，Pawn和角色的部分常用功能不会复制
  > * 骨架网格体和静态网格体
  > * 材质
  > * 动画蓝图
  > * 粒子系统
  > * 音效发射器
  > * 物理对象

### 网络角色和授权
* 授权(Authoritation)
    授权actor被认为是可控制actor的状态，可将信息复制到网络多人游戏会话中的其他机器
* 远程代理(Simulated Proxy)
  远程代理actor有另外一台机器上的授权actor完全控制，，是远程机器上的副本，将其接受授权actor中的复制信息
* 自主代理(Autonomous Proxy)
  自主代理actor能够执行本地部分任务，但会接收授权actor的矫正，自主代理通常为玩家直接控制的actor所保留，如pawn

**UE4使用的是默认模型是服务器是授权actor，意味着服务器对游戏状态具有权限，而信息固定从服务器复制给客户端**

### Role和RomoteRole
要确定谁对特定actor的主控权。需要判断Role属性是否为ROLE_Authority，如果是，说明，这个运行中的实例主管此actor，如果Role是Role_SimulatedProxy或Role_AutonomousProxy,则说明是表现端
* Role_SimulatedProxy
  标椎模拟路径，根据上次服务器发来的速率对移动进行推算，
* Role_AutonomousProxy
  通常用于PlayerController所拥有的actor，这说明actor会接受来自真人控制者的输入，所以我们推算时会有更多的信息，使用真人输入内容来补缺失的信息。
* 而对于不同的数值观察者，他们的Role和RemoteRole值可能发生对调
  * 在服务器中
  Role= =ROLE_Authority；
  RemoteRole= =ROLE_SimulatedProxy;

   *  在客户端中
  Role= =ROLE_SimulatedProxy;
  RemoteRole= =ROLE_Authority;

### 关于actor与其所属连接，对象的归属性问题
* **连接**
  * 所谓连接，就是客户端连接到服务器，在维持这个连接的条件下，我们才能真正的玩网络游戏。如果我们想要服务器吧某些特定的信息发送给特定的客户端，则需要找到这个连接。
  * 而这个连接的信息储存在PlayerController里面，而这个PlayerController不能是随便的PlayerController，一定是客户端第一次链接到服务器，服务器同步过来的那个PlayerController，因为只有这个PlayerController才包含相关的NetDriver，Connection以及session信息。
  * 对于任意一个actor(客户端上),他可以有连接，也可以无连接，一旦actor有连接，那他的role就是ROLE_AutonomousProxy;如果没有连接，他的Role就是ROLE_SimulatedProxy;

* Client_Owned Actor
  在PlayerController内产生的任何东西都被认为是玩家拥有的，服务器和客户端都同属于一个UNetConnection家族的actor
* Server_Owned Actor
  在PlayerController之外的服务器产生的任何东西都认为是服务器拥有的，只存在于server端的actor
* UnOwned actor
  我们认为persistent maps是Unowned Actor


### 三种方法得到PlayerController连接
1. 设置自己的owner为拥有连接的PlayerController或者自己owner的owner为拥有连接的PlayerController(也就是官方文档里说的最外层的owner是否为PlayerController且这个PlayerController拥有连接)
2. 这个actor必须是Pawn而且这个Pawn以及Possess了拥有连接的PlayerController，
3. 把这个actor的owner为拥有连接的pawn，和第一点的区别是：pawn与controller的绑定方式不是通过owner这个属性，而是这个pawn本身就拥有controller这个属性，所以pawn的owner可能为空


## RPC(远程过程调用)
### RPC基础知识
RPC是在本地调用但在其他机器上(不同于执行调用的机器)远程执行的函数
> * 主要作用
>   执行那些不可靠的暂时性/修饰性游戏事件，比如播放声音，生成粒子或产生其他临时效果，他们对actor的正常运行并不重要。

* Client
  在服务器上调用，但需要在客户端上执行的RPC
  ~~~c++
  UFUNCTION(Client)
  void ClientRPCFunction();
  ~~~

* Server
  在客户端上调用，在服务器上执行的RPC
  ~~~c++
  UFUNCTION(Server)
  void ServerRPCFunction();
  ~~~
* Multicast
  在服务器上调用，在服务器和当前连接的所有客户端上执行
  ~~~c++
  UFUNCTION(NetMulticast)
  void MulticastRPCFunction();
  ~~~
### 执行
从服务器调用的RPC
| Actor所有权 | 未复制 | NetMulticast | Server | CLient |
| ---------  | -------- | ------------ | ------ | -------|
| Client_Owned actor | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在actor所属的客户端上运行 |
| Server_Owned actor | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在服务器上运行 |
| UnOwned actor | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在服务器上运行 |


从客户端调用的RPC
| Actor所有权 | 未复制 | NetMulticast | Server | CLient |
| ---------  | -------- | ------------ | ------ | -------|
| Owned By self | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 在服务器上运行 | 在执行调用的客户端上运行 |
| Owned By Different Client | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃 | 在执行调用的客户端上运行 |
| Server_Owned actor |在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃 | 在执行调用的客户端上运行 |
| UnOwned actor | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃 | 在执行调用的客户端上运行 |

### 可靠性
默认情况下，RPC并不可靠，要确保在远程机器上执行RPC调用，可以指定可靠性
~~~c++
UFUNCTION(Client,Reliable)
void ClientRPCFunction();
~~~
* 不可靠RPC
  无法保证必会到达预定目的地，但其发送速度和频率高于可靠RPC，其最适用于GamePlay而言相对不重要或经常调用的函数，例如由于actor每帧都会移动，因此使用不可靠rpc复制该移动
* 可靠RPC
  保证到达预定目的地，并在成功接收之前一直保留在队列中，其适用于GamePlay很关键或者不经常调用的函数，例如碰撞事件，武器发射的开始和结束，或actor生成

### 关于RPC的几个问题
1. Owner是如何在RPC调用中生效的？
   在AActor::GetFunctionCallspace里面，每次调用RPC函数时，会调用该函数判断当前是不是在远端调用，是的话会通过网络发送RPC。GetFunctionCallspace里面会通过Owner找到Connection’信息
   
2. RPC和Actor同步谁先执行
   答案是不确定的，这个涉及到UE4网络消息的发送机制与发送时机，一般来说，RPC的数据会立刻塞到Sendbuffer里面，而Actor的同步要等到netDriver统一处理。所以RPC的消息是相对靠前的，不过由于存在丢包延迟的情况，这个结果在网络环境下不能确定；
3. MulticastRPC会发送给所有客户端吗？
   不一定，考虑到服务器的一个NPC，在地图的最北面，有两个客户端玩家。一个玩家A在这个NPC附近，另一个玩家在最南边看不到这个NPC(网络裁剪，服务器没有把这个actor同步到玩家B的客户端)，NPC发送多播RPC消息，则不会发送到玩家B上面
   UE4不考虑多播函数是否Reliable，只要不满足网络相关性都不会发送
   ![](https://pic3.zhimg.com/80/v2-6d0cb35ba7784dfd736dbf8db43a8326_720w.jpg)
4. RPC参数和返回值？
   * 参数
    RPC函数除了UObejct类型的指针以及conststring&的字符串外，其他类型的指针或引用都不能作为RPC的参数，对于UObject指针类型是通过在另一端通过GUID识别，至于其他类型，无法还原其地址，所以不允许传输其指针或引用；

   * 返回值
    一个RPC函数是不能有返回值的，因为本身的执行就是一次消息的传递。如果一个客户端执行一个ServerRPC，如果有返回值的话，那么岂不是服务器执行后还要再发送一个消息给客户端，如果还有返回值的话那岂不是无限循环？因此RPC不能有返回值。

### RPC时序和时效的问题
1. 同一个Channel的Reliable的RPC一定是保序的，且无论网络状况，一定能发送到对端
2. Unreliable的RPC可能是乱序，也可能会被丢弃掉
3. 只有NetMulticast且UnreliabledeRPC才会被延迟发送，其他RPC立即写入Connection的SendBuffer(对端实际收到的时间不能保证)
4. RPC与属性回调的时序不可保证，最经典的比如beginplay；

## 同步
### 属性同步
> 非休眠状态下的Actor的属性同步：在服务器属性值发生改变的情况下执行
> 休眠的Actor：不同步
> 回调函数执行条件：服务器同步过来的数值和客户端不相同

#### 属性同步的原理
服务器在创建同步通道的时候把给每一个actor对象创建一个属性变化表，里面记录一个当前默认的Actor属性值，之后，每次属性发生变化的时候，服务器就会判断新的值与当前属性变化表里面的值是否相同，如果不同就把数据同步给客户端并修改属性变化表里的数据，对于一个非休眠且保持连接的Actor，它的属性变化表一直存在的。
####结构体的属性同步
UE里面的UStruct类型的结构体在反射系统中对应的是USrriptStruct，它本身可以被标记为Replicated并且结构体内的数据默认都会被同步，而且如果里面还有子结构体的话也仍然会递归的进行同步。如果不想同步的话，需要在对应的属性标记Notreplicated，而且这个标记只对UStruct有效，对UClass无效。
有一点特别的是，Struct结构内的数据是不能标记Replicated的。如果你给Struct里面的属性标记replicated，UHT在编译的时候就会提醒你编译失败。
#### 属性回调
* 首先要知道，同步操作触发是由服务器决定的，所以不管客户端是什么值，服务器觉得该同步的时候就会把数据同步给客户端，而回调操作是客户端执行，客户端会判断当前的值是否相同来决定是否产生回调。
* 关于属性回调的几个问题
  1. 属性回调和RPC的区别?
    属性回调理论上一定会执行，而RPC函数有可能由于错过执行时机不再执行

### 组件同步
1. 静态组件
   一旦Actor被标记为同步，那么这个actor身上默认挂载的组件也会随着Actor一起同步给客户端。
   什么事默认挂载的组件呢？就是C++构造函数里面创建的默认组件或者蓝图里添加的组件，
2. 动态组件 
   对于动态组件，就是在游戏运行过程中，服务器创建或删除的组件，比如一个玩家走进一个洞穴时，给洞穴里面的火把添加一个粒子特效组件，然后同步到客户端上，当玩家离开时删除这个组件，客户端上也随之删除这个组件，我们必须在attach到actor上并设置他的Replicate属性为true；
   
### 变量同步
选择变量同步属性Replication为Replicated，有三个可选值
1. None 默认值，服务端不会同步给所有客户端
2. Replicated：在服务端修改，会同步给其他客户端
3. RepNotify：选择这个的时候，会有Function中产生一个变量Var对应的方法OnRep_Var,在服务器里修改Var的时候会回调服务器和客户端中该Actor的这个OnRep_Var方法；
~~~c++
UPROPERTY(ReplicatedUsing=OnRep_Fun)
	bool Var;
UFUNCTION()
	void OnRep_Fun();
~~~
同步规则:当且仅当变量在服务器发生改变时向客户端同步。

OnRep函数调用规则：更改变量的那个客户端或服务器不会调用OnRep函数，需要手动调用。其他客户端一旦变量改变（被同步）将自动调用OnRep函数。

### 注意事项
1. 尽可能少使用RPC或者复制蓝图函数，在合适的情况下改用RepNotify
2. Multicast函数会导致会话中各连接客户端的额外网络流量，需尤其少用
3. 若能保证非复制函数仅在服务器上运行，则服务器RPC中无需包含纯服务器逻辑
4. 将可靠RPC绑定到玩家输入是需谨慎，玩家可能会反复点击按钮，导致可靠RPC队列溢出
5. 若游戏频繁调用RPC或复制函数，如tick时，则将其设为不可靠
6. 检查Actor的网络角色可查看其是否为ROLE_Authority。此方法适用于过滤函数的执行，该函数同时在服务器和客户端激活；
7. 使用C++函数中的IsLocallyControlled函数或者蓝图中的IsLocally Controlled函数，可检查Pawn是否受本地控制，基于执行是否拥有客户端相关来过滤函数。


### 多人模式下的GamePlayer框架
* GameInstance 
  GameInstance在引擎会话的持续时间内一直存在，意味着引擎启动时创建，在引擎关闭后才会销毁或更换，服务器和客户端都存在着一个独立的GameInstance，这些实例彼此互不通信
* GameMode
  GameMode对象仅存在服务器上，他通常储存客户端不需要知道的信息
* GameState
  GameState存在于服务器和客户端，因此服务器可以在GameState上使用复制变量让所有客户端保持最新的游戏数据。与所有玩家和旁观者有关，而不是和任意一个特定玩家有关的信息最适合与GameState复制
* PlayerController
  每一个客户端的每一个玩家存在着一个PlayerController，他们在服务器和关联的客户端之间相互复制，但不会复制到其他客户端，因此在服务器上每个玩家都有PlayerController，但本地玩家只有本地玩家的PlayerController，客户端保持连接时存在PlayerController，PlayerController和Pawn相连，但不会像Pawn一样被销毁或重新产生，他们非常适合在客户端和服务器之间传递消息，而不必将信息复制到其他客户端
* PlayerState
  服务器和客户端上存在游戏相连的每个玩家的PlayerState，这个类可以用于所有客户端感兴趣的复制属性，而不仅仅是所属客户端，比如当前玩家的单前分数，与PlayerController一样，他们与pawn相连，但不会像pawn一样被销毁和重新产生
  当玩家偶尔因网络波动断线，因为这个连接不在了，所以Controller失效被释放了，服务器可以吧对应的该PlayerState先暂存起来，等玩家重新连接上了，可以利用该PlayerState重新挂上Controller。
* Pawn
  Pawn也存在与服务器和所有客户端上，可以包含复制变量和事件。但要注意的是：只要所属玩家和游戏相连，且游戏没有加载新关卡，则PlayerController和PlayerState就保持不变，Pawn却不同，如果pawn在游戏期间死亡，通常会被销毁或替换一个新的pawn，而PlayerController和PlayerState将继续存在，并在新pawn产生后和新pawn相连。






