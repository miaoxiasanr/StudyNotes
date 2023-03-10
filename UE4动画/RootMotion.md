- [RootMotion](#rootmotion)
  - [RootMotion概述](#rootmotion概述)
  - [RootMotion单机流程和原理](#rootmotion单机流程和原理)
    - [动画数据初始化](#动画数据初始化)
      - [对于动画蓝图里面的动画数据](#对于动画蓝图里面的动画数据)
      - [移动组件PerformMovement](#移动组件performmovement)
        - [Prepare RootMotion阶段](#prepare-rootmotion阶段)
  - [RootMotion的同步](#rootmotion的同步)
    - [Simulated客户端同步](#simulated客户端同步)
      - [动画Montage初始化](#动画montage初始化)
      - [移动组件SimulatedTick](#移动组件simulatedtick)
    - [Autonomous客户端的同步](#autonomous客户端的同步)
      - [Montage初始化：](#montage初始化)
  - [总结](#总结)

# RootMotion

## RootMotion概述
**RootMotion，跟骨骼位移，属于移动组与动画系统相结合的一个部分，表示角色的整体移动(包括物理)是由动画来驱动的。**
一般来说，在大部分游戏的应用里面，玩家的移动与动画是分开的，移动系统只负责处理玩家的位置和旋转，动画系统只做对应的动画表现，只要移动的速度合适就可以与动画做到完美的匹配，也就是说动画播放的位置(即Mesh的位置)是由角色移动来驱动的(UE4里面，动画是胶囊体的位置数据来驱动的)。例如：如果胶囊体在向前移动，系统就会知道在角色上播放一个跑步或行走的动画，让角色看起来是在靠自己的力量移动。



<iframe 
    height=550
    width=1080
    src="https://vdn6.vzuu.com/SD/e7b489d6-ec61-11ea-acfd-5ab503a75443.mp4?pkey=AAUuVVsOY3kQPj2pl8FFpGB6B0PnrAvrNlYps4FxqxwyX_EQ0G4mB_T0sLLJttuXmh9SSqSTrAVwbMwAOgE4RSgP&c=avc.0.0&f=mp4&pu=078babd7&bu=078babd7&expiration=1672982941&v=ks6"     
    frameborder=0 
    allowfullscreen>>
</iframe>
除了普通的移动之外，还有一些复杂的移动是很难模拟的，比如官方给的例子：一个举着锤子的人向前挥舞，一开始速度比较慢，中间挥舞时由于角色控制不住速度会很快，到最后锤子落地时，速度又会变得很慢，角色会踉跄的走两步。

然而，当玩家发起一次特殊攻击，在这种攻击下，模型已预先设定好向前冲的动作。如果所有的角色动画都是基于玩家胶囊体的，这样的动画会导致角色迈出胶囊体，从而在事实上失去碰撞，并且一旦动画播放结束，玩家就会滑回其碰撞位置。
![](https://cfvod.kaltura.com/p/2501632/sp/250163200/thumbnail/entry_id/0_dzpr8nxm/version/100002/width/638/height/358)

为了解决这样的问题，所以引入了根运动(RootMotion)
**RootMotion的实现:其基本思路就是移动组件每帧不断的去读取动画数据里面的移动Transform。然后应用到实际的胶囊体上面。**
> 一些相关概念
> * USkeletalMeshComponent:对应一个包含骨骼动画的Mesh模型，可以以组件的形式Attach到一个玩家身上
> * UAnimInstance:动画实例，其实就是C++版本的动画蓝图，每个SKeletalMeshComponent组件都会有一个UAnimInstance Class类型的配置项一级一个UAnimInstance类型的指针，
>  * FAnimInstanceProxy:动画代理实例，0属于多线程优化动画系统的核心对象。他负责分担动画蓝图的更新工作，将部分动画蓝图的任务分配到其他线程去做
> * FAnimMontageInstance:Montage实例，播放时会在动画实例UAnimInstance里面存储一个FAnimMontageInstance数组记录该实例的所有Montage
> * FRootMotionMovementParams:用于积累计算RootMotion的自定义结构体，可以临时存储当前帧的RootMotionTransform数据，FAnimMontageInstance保存一个临时缓存RootMotion数据的成员变量
> * FRootMotionExtractionStep：由于一个蒙太奇动画里面可能有多个动画序列组成，所以在提取RootMotion数据的时候需要记录当前所在的片段以及具体位置，FRootMotionExtractionStep就是将这些数据封装并整合成一个结构体。
> * FSlotAnimationTrack:组成当前Montage插槽片段的集合，如下图所示有两个FSlotAnimationTrack
> ![](https://pic3.zhimg.com/80/v2-432cfe5c950e3a8f088efb6dde8468d6_1440w.webp)
> * FAnimTrack:一个FAnimTrack表示一个动画轨道，每个轨道可以由一个或多个AnimSequence片段构成，下图由三个动画序列构成
> ![](https://pic4.zhimg.com/80/v2-e641973c34bd2671bcaf8f597b0af343_1440w.webp)
> * FRepRootMotionMontage:Character身上用于同步RootMotion相关信息的属性，包括了UAnimMontage，执行位置，旋转，速度等
> * FSimualtedRootMotionReplicatedMove:在Character上以数组的形式存储，记录最近一段时间每帧的RootMotion信息，用于服务器到Simulated客户端的数据同步
> * FRootMotionSourse:广义上的RootMotion，本质上没有具体的动画数据，通过模拟力产生每帧的Transform信息，比如说玩家受到攻击产生位移，如果是使用普通的受力属于物理影响，同步效果差。在移送组件里集成FRootMotionSourse就可以使用类似RootMotion的方式非常流畅的处理角色的移动，同时还兼顾到网络同步。
> FRootMotionSourseGroup:包含了一组RootMotionSourse的结构体，同一时刻可能有多个不同的力(或者说是RootMotionSourse)作用于玩家，移动组件可以根据权重优先级等混合出一个合理的移动位置。
> FRootMotionServerToLocalIDMapping:作用于同步匹配客户端和服务器上面FRootMotionSourseGroup里面不同的RootMotionSourse

## RootMotion单机流程和原理
### 动画数据初始化
#### 对于动画蓝图里面的动画数据
1. 绑定动画蓝图的Character进入场景时就已经开始各种动画数据相关的初始化(UAnimInstance::InitializeAnimation)，随后通过UpdateAnimation不断的更新动画蓝图里面的逻辑。同时还会把一部分逻辑交给FAnimInstanceProxy处理。
####对于非动画蓝图里面的Montage数据
1. 一般是玩家手动触发Montage的播放，通过USkeletalMeshComponent找到对应的AnimInstance并执行UAnimInstance::Montage_Play;
2. 创建一个FAnimMontageInstance并进行相关的初始化，开始真正的播放蒙太奇
3. 判断蒙太奇是否带有RootMontage，是的话将会赋值给RootMotionMontageInstance，用于后续的ACharacter::IsPlayingNetworkedRootMotionMontage判断；
4. Montage初始化之后就会在后续每帧执行的A尼玛Instance::UpdateAnimation里面参与计算了，在通常情况下，只要我们的动画更新方式不选择EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered,Montage都是会参与更新的，更新逻辑在UAnimInstance::UpdateMontage里面
#### 移动组件PerformMovement
##### Prepare RootMotion阶段
1. 移动组件在TickComponent的PerformMovement里，先判断是否处于RootMotion的状态。满足下面两个条件就会先做一些RootMotion相关数据的处理和清理
> * CurrentRootMotion里面是否有ActiveRootMotionSourse
> * 通过当前角色身上的Mesh组件函数USkeletalMeshComponent::IsPlayerRootMotion判断其是否处于RootMotion状态。
2. 再次进行判断来确保是否要进行更新动画，如果上面的条件再次成立，就开始准备更新动画执行TickChatacterPose(强制执行一次)，在这里会调用UAnimInstance::UpdateMontage更新动画，并在MontageInstance->Advance(DeltaSeconds,RootMotionParams,bUsingBlendedRootMotion)将当前Montage位移信息更新到AnimInstance的成员变量ExtractedRootmotion里面
3. 假如我们将Montage里面的信息提取到ExtractedRootmotion里面，但是动画蓝图里面的AnimSequence、BlendSpace等动画数据怎么一起参与计算？关键在于多线程优化的核心类FAnimInstanceProxy，默认情况下，动画蓝图里面的大部分更新逻辑都被放到了FAnimInstanceProxy里面，无论是否为多线程，我们都需要通过FAnimInstanceProsy::TickAssetPlayerInstances去处理相关逻辑，并在这里根据权重去将所有的动画资源的RootMotion数据提取到ExtractedRootMotion里面。
> * 如果RootMotionMode为RootMotionFromEverything，那么我们的主线程Tick的时候就会立刻去更新TickAssetPlayerInstances，这样是为了能及时获取到每一帧的RootMotion信息，如果是RootMotionMode其他模式，那么TickAssetPlayerInstances就会被放到其他线程里面执行。
> * 如果RootMotionMode为RootMotionFromEverything，在Proxy更新完成后，饿哦们需要及时地根据权重将参与计算的资源数据提取到ExtractedRootMotion里面。
#####应用阶段
1. 在TickPose后，如果角色处于IsPlayingRootMotion状态就会执行ConsumeRootMotion消耗掉AnimInstance在前面阶段产生的ExtractedRootMotion,也就是将前面得到的ExtractedRootMotion数据复制到新的变量RootMotion并清空ExtractedRootMotion。
2. 得到的新的RootMotion数据会先根据ACharacter的AnimRootMotionTranslationScale进行缩放调整，同时把其相关数据拷贝到移动组件的成员变量RootMotionParams里面。
3. 对RootMotionParans进行局部到世界的坐标转换。
4. 执行移动模拟，也就是把前面得到的Transform应用到移动组件里面，这里会先根据当前的RootMotion的DeltaTransform以及DeltaTime算出一个速度AnimRootMotionVelocity进行模拟(这里并不更新Rotation)
5. 模拟结束，读取RootMotionParams的Transform来更新Rotation
6. 清除移动组件的成员RootMotionParams里面的数据。

## RootMotion的同步
在目前的引擎中，RootMotion只支持Montage的同步，基于Montage的同步流程。同步分为Simulated客户端以及Autonomous客户端两种情况
### Simulated客户端同步
#### 动画Montage初始化
1. 服务器本地先触发执行MontagePlay并赋值给RootMotionMontageInstance
2. Simulated客户端在服务端触发MontagePlay后。通过属性回调随后触发MontagePlay
#### 移动组件SimulatedTick
1. 移动组件执行Tick，UCharacterMovementComponent::SimulatedTick(float Deltatime)
2. 如果当前玩家的Mesh对应的AnimScriptINstance->RootMotionMode为RootMotionFromMontageOnly(也就是说其他三种ERootMotionMode不支持网络同步)，触发RootMotion在Simulated客户端的同步操作。
![](https://pic2.zhimg.com/80/v2-e2752755fb43956234e13d17e323ecf1_720w.webp)
3. TickCharacterPose,从动画当前位置里面解析出DeltaTransform，用AnimInstance上的ExtractedRootMotion提取出来。

4. 根据CharacterOwner->GetAnimRootMotioonTranslationScale()设置RootMotion的Scale并提取到移动组件的RootMotionParams里面。
5. 调用SimulateRootMotion转换到世界坐标，计算RootMotion的速度并开始调用StartNewPhusics进行模拟，与单机版本不同，其模拟流程都是在函数SimulateRootMotion里面处理
> * 由于Simualte客户端通过网络同步，可能因为网络波动而卡顿，所以该函数只会先更新胶囊体的位置(bEnableScopedMovementUpdates为True即可)，而不更新Mesh的位置，Mesh需要通过本地的Tick去平滑。
6. 模拟完毕之后，如果Rotation不为默认值FQuat::Identity就会通过MoveUpdatedComponent修改，清除临时提取的RootMotionParams的相关数据。
> Character身上有一个FRepRootMotionMontage RepRootMotion记录了每帧服务器的RootMotion的运行信息，包括当前帧的坐标，旋转，Montage，速度，执行的位置等。
> ~~~c++
> /** Replicated Root Motion montage */
> UPROPERTY(ReplicatedUsing=OnRep_RootMotion)
>   struct FRepRootMotionMontage RepRootMotion;
> ~~~
> 这个属性是同步的(而且只会在播放RootMotion的时候同步)，在服务器播放RootMotion的时候每帧会通过ACharacter::PreReplication处理这些数据并发给Simulated客户端，数组TArray < FSimulatedRootMotionReplicatedMove> RootMotionRepMoves 储存了服务器发来的所有RootMotion的数据，前面提到的RepRootMotion在客户端的回调函数里会被提取数据并添加到RootMotionRepMoves数组里面。
7. 客户端模拟移动后会根据服务器的RootMotion信息开始校验，执行函数ACharacter::SimualtedRootMotionPosFixup,这里会先判断客户端是否要用这个RootMotionRepMoves数据里面的数据(条件：小于0.5秒，在同一个Section，非循环Montage，服务器落后于客户端的位置最接近客户端的那个)
8. 如果寻找到满足条件的数据RepRootMotion，就会先通过ACharacter::RestoreReplicatedMove按照服务器传递的数据修改本地角色的坐标与旋转。随后按照当前服务器的位置以及客户端的位置执行RootMotion的回滚，这里只是为了得到一个回滚的效果。
9. 最后根据DeltaPosition，PlayRate算出一个DeltaTime，再根据回滚结果LocalRootMotionTransform进行一次本地模拟SimulateRootMotion.
10.  模拟之后调用SmoothCorrection进行平滑处理，前面提到SimulateRootMotion只会更新胶囊体位置而不是更新Mesh位置，就是为了在这里进行平滑，
> * 平滑的逻辑大概是客户端记录了一个ClientDate数据去记录当前的Mesh偏移以及服务器的时间戳，在随后的每帧的Tick里面，不断的更新Offset偏移，让其逐渐为0，当偏移为0时Mesh就和胶囊体完全重合完成了平滑。


### Autonomous客户端的同步
#### Montage初始化：
1. 在客户端本地先执行Montage播放,通过RPC通知服务器播放。
2. 服务器通过RPC触发MontagePlay并赋值给RootMotionMontageInstance
   > 这里也可以先在服务器播放，通过属性回调触达Autonomous客户端进行播放
####移动组件ReplicateMoveToServer
1. 类似于上面的Simulated客户端执行TickComponent，不过这里每帧触发的函数不是SimulatedTick而是ReplicateMoveToServer,随后执行PerformMovement里面提取RootMotion的信息。
2. 根据DeltaTime与DeltaTransform计算速度，调用StartNewPhysics进行RootMotion模拟。
3. 更新Rotation，清除临时提取的RootMotionParams的相关数据，将本次移动的数据存放到FSaveMove_Character里面，记录这次移动并储存到SavedMoves数组里面(用于回滚等)。
4. 执行CallServerMove，将本地计算的数据发送到服务器，调用RPC函数ServerMove。
**服务器处理：**
1. 服务器根据客户端信息执行MoveAutonomous重新模拟计算，并根据结果判断客户端移动是否合法。
2. 随后，在服务器执行UNetDriver::ServerReplicateActors的同步时，发送ACK(ClientAckGoodMove)或者Adjust(SendClientAdjustment).如果这时候服务器正在播放Montage且发现客户端数据有问题，就会执行ClientAdjustRootMotionSourcePosition进行纠正。否则就会执行正常的纠错流程。
3. 客户端收到ClientAdjustRootMotionSourcePosition信息，首先会根据服务器传递的坐标等信息先更新移动组件的数据(执行ClientAdjustPosition)，随后根据服务器传递的MontagePosition来直接设置本地Montage动画的Position。

## 总结
1. RootMotion本质上走的还是移动组件的处理流程，只不过其移动数据是从动画里面提取的，而且很明显的可以看出，RootMotion只支持Montage的同步，其他的模式根本不会走这套流程(IsPlayingNetworkedRootMotionMontage),其他模式也不会从动画蓝图Proxy里面提取相关的数据(FAnimInstanceProxy::TickAssetPlayerInstances).
2. 之所以引进RootMotion的原因是动画系统(或者说是动画状态机)的同步时复杂而困难的，目前UE通用的同步方法时客户单和服务器各自维护一个状态机以及几个同步的属性值，然后通过这些属性的判断来同步动画，这里的动画状态机并没有同步。一旦动画系统复杂起来，各个状态之间的切换和转变会变得很复杂。
3. 因为RootMotion本质上是为了提高表现效果才使用的方案，单机模拟效果尚可，对于还需要预测的网络同步就更困难了，除非是常规的线性运动的RootMotion，其他的不规则的运动几乎无法预测，而如果不预测，我们就很难应对网络抖动，一旦网络一卡整个表现不再流畅。所以当网络环境不好的情况下，RootMotion的表现实非常差的。
[官方文档——RootMotion](https://docs.unrealengine.com/4.27/zh-CN/AnimatingObjects/SkeletalMeshAnimation/RootMotion/)
[《Exploring in UE4》RootMotion详解【原理分析】](https://zhuanlan.zhihu.com/p/74554876)