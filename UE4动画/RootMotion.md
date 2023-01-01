#RootMotion

##RootMotion概述
**RootMotion，跟骨骼位移，属于移动组与动画系统相结合的一个部分，表示角色的整体移动(包括物理)是由动画来驱动的。**
一般来说，在大部分游戏的应用里面，玩家的移动与动画是分开的，移动系统只负责处理玩家的位置和旋转，动画系统只做对应的动画表现，只要移动的速度合适就可以与动画做到完美的匹配，也就是说动画播放的位置(即Mesh的位置)是由角色移动来驱动的(UE4里面，动画是胶囊体的位置数据来驱动的)。例如：如果胶囊体在向前移动，系统就会知道在角色上播放一个跑步或行走的动画，让角色看起来是在靠自己的力量移动。



<iframe 
    height=550
    width=1080
    src="https://vdn.vzuu.com/SD/e7b489d6-ec61-11ea-acfd-5ab503a75443.mp4?disable_local_cache=1&bu=078babd7&c=avc.0.0&f=mp4&expiration=1672595470&auth_key=1672595470-0-0-b6430047f4edd5502a5841cf762c4f7e&v=ali&pu=078babd7)"     
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



[官方文档——RootMotion](https://docs.unrealengine.com/4.27/zh-CN/AnimatingObjects/SkeletalMeshAnimation/RootMotion/)
[《Exploring in UE4》RootMotion详解【原理分析】](https://zhuanlan.zhihu.com/p/74554876)