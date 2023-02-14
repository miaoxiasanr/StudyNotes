- [UE4物理](#ue4物理)
  - [基础知识](#基础知识)
    - [碰撞查询](#碰撞查询)
      - [一些参数](#一些参数)
      - [碰撞查询](#碰撞查询-1)
      - [一些例子](#一些例子)
  - [深入了解](#深入了解)
    - [Mesh组件和物理](#mesh组件和物理)
    - [物理的创建时机](#物理的创建时机)
  - [引用](#引用)

# UE4物理
## 基础知识
1. 游戏中的物理系统
   游戏物理主要解决两个问题
   1. 碰撞查询(Query):比如我前面有一堵墙，我还可以走多远就会被撞到
   2. 物理模拟(Simulation):比如游戏死亡后身体如何倒下，载具碰到路上的石头如何表现
2. UE4物理是如何建立物理系统的
   目前版本还是依赖于第三方的PhysX，UE4自身提供PrimitiveComponent，拥有BodyInstance，上面记录了PhysX所需要的物理数据，会将信息传递给PhysX所创建的物理世界，然后由PhysX进行处理，并得到其返回的结果。
   ![](https://pic4.zhimg.com/80/v2-40d63faf7e38d36af1b9322c9c4623b3_720w.jpg)
3. 游戏中常见的带物理的物体
   1. 胶囊体一类(USphereComponent,UBocComponent,UCapsuleComponent)
   2. 静态网格物体StaticMesh
   3. 骨骼网格体SkeletalMesh
   4. LandScape地形
   5. PhysicsVolume(BrushComponent)
### 碰撞查询
1. 人物的移动距离是根据初中物理所学s=v*t得到的，v是当前人物的移动数据，而t是移动时长，在游戏时间t是很小的时间片，游戏中并不会有真正的连续移动，当t足够小时，每次移动量也很小，将人物SetActorLocation到这个小移动量表示的世界坐标里，就可以得到看似莲须的移动效果，本质是极小量的瞬移。在每次计算移动量时，都会进行碰撞查询，具体是用人的胶囊体向速度方向进行扫描，他返回的结果是一个比例，如果比例是1.0，那么就能够执行完整的瞬移，如果比例是0.5，就只能执行一半的瞬移长度，如果是0，就进行不瞬移；
#### 一些参数
![](https://pic2.zhimg.com/80/v2-f7a90a5c4fda6753c65b0fc8d52b9ebd_720w.jpg)
1. Collision Enable
   1. NoCollision:在物理引擎中此形体将不具有任何表示，不可用于空间查询(光线投射，Overlap)或模拟(刚体，约束)，此设置可提供最佳性能，尤其是对移动对象。
   2. QueryOnly:此形体可用于空间查询(LineTrace，Overlap)，不可用于模拟(刚体或约束)，对于角色运动和不需要物理模拟的对象，通过缩小物理模拟树中的数据来实现一些性能提升。
   3. PhysicsOnly:此形体可用于物理查询((刚体或约束)，不可用于空间查询(LineTrace，Overlap)，对于角色上不需要按骨骼进行检测的模拟次级对象运动，通过缩小物理模拟树中的数据来实现一些性能提升。
   4. Collision Enable:可用于空间查询和物理模拟
2. Collision Responses
   ![](https://docs.unrealengine.com/4.27/Images/InteractiveExperiences/Physics/Collision/Reference/iob.webp)
   * Ignore:无论一个物理形体的“Collision Responses"为何，此物理形体都将忽略交互
   * Overlap:如果已将另一物理形体设置为“Overlap"或"Block"，此物理形体的对象类型，将发生Overlap1事件
   * Block：如果将另一物理形体设置为"Block"此物理形体的对象类型，将发生Hit事件
3. UE4把查询后返回的Hit封装成了FHitResult
FHitResult的结构如下
bBlockHit:是否发生碰撞
bStartPenetrating:是否在检测开始就有渗透情况
Time:碰撞后实际移动距离除以检测移动距离
Diatance:碰撞后实际移动距离
Location:碰撞后最终位置
ImpactPoint:碰撞结束点
Normal:碰撞切面法向量
ImpactNormal:碰撞切面法向量(非胶囊体和球形检测与Normal不同)
TraceStart:检测开始位置
TraceEnd:检测结束位置
PenetrationDepth:渗透深度
第一种常见的胶囊体Sweep查询
![](https://pic4.zhimg.com/80/v2-8577ebbac0c2f51a0085eedb16bf741f_720w.jpg)
查询开始结束分别是TraceStart和TraceEnd两个位置，如果碰到了障碍，bBlockingHit就是true，胶囊体最终会停在Location的位置，他的移动距离就是Diatance，Time是一个0-1的值，表示实际移动距离比查询距离。
第二种是InitialOverlap，开始位置就检测到了Overlap
![](https://pic2.zhimg.com/80/v2-8d8508ccf881d5025187b9f18dd742f5_720w.jpg)
这时候bStartPenetrating是true，通过渗透深度计算可以获得PenetrationDepth，这个参数对于处理移动中穿透的情况非常重要
#### 碰撞查询
![](https://cdn2.unrealengine.com/blog/FilterTable-900x490-756106034.jpg)
需要明白两点
1. 只有两个对象互相Block时，才会真正的被阻挡，其他情况下不会
2. 只要有一方Ignore，就不会产生Overlap事件
#### 一些例子
![](https://cdn2.unrealengine.com/blog/ObjectExample-944x854-260481718.jpg)
首先Player向前移动，首先他将Overlap Sharp，因为Player是Pawn类型，Block WorldStatic ，而Sharp是WorldStatic，Overlap Pawn ，所以最后的结果是Overlap，当Player继续向前移动，会有Player Block Brick Wall ，因为 Player是Pawn，Block WorldStatic ,而Brick Wall 是WorldStatic ，Block Pawn，最后的结果是Block
## 深入了解
### Mesh组件和物理
![](https://pic1.zhimg.com/80/v2-b5184d4d38e6bb3834f4fbad1d1041c4_720w.jpg)
对于放在场景中的石块等StaticMesh，它的物理一般在建模软件里就应该被创建好，导入到UE时根据导入的数据创建物理信息，其本质就是在编辑器里给StaticMeshComponent构建一个UBodySetup，在游戏运行的时候创建运行时的基本物理数据UBodyInstance。
>UbodySetup和UBodyInstance
>* UbodySetup就是一个静态的物理数据，一般在游戏运行前就已经构建好了
>* UBodyInstance是一个在游戏时正在起作用的物理数据，运行时才真正出现，通过一个UbodySetup是可以创建出多个UBodyInstance的
对于骨骼网格体SkeletalMesh，由于数据比较多，他的物理数据储存在PhysicsAsset里面。在游戏运行的时候，SkeletalMeshComponent会读取物理资产里面的数据UBodySetup随后在通过UBodySetup给角色创建出对应的基本物理数据UBodyInstance。
### 物理的创建时机
1. UStaticMeshComponent的物理创建
    首先是UStaticMeshComponent。可以看到在场景里面加载Actor并注册UActorComponent的时候会对UPrimitiveComponent组件进行物理信息的创建。其实除了SkeletalMEshComponent以外，所有继承自UPRimitiveComponent的组件（第一部分提到的那5中都是)都会在注册后就创建物理数据(对于直接继承UActorComponent的组件，如移动组件就不会执行此操作)。因此除了SKeletalMeshComponent以外，其他继承自UPrimitiveComponent的组件物理创建的时机很明确，也就是UActorComponent被注册的时候创建物理(还有一种情况也需要更新物理，比如更换模型的时候)。
![](https://pic1.zhimg.com/80/v2-d7ef0f4d62d500dddef497eeb98cd218_720w.jpg)
2. USkeletalMeshComponent的物理创建
   USkeletalMeshComponent与其他物理组件不同，一般来说我们并不会在玩家一出生就创建出所有的物理骨骼，也不会让玩家的骨骼物理一直存在着。原因很简单，就是wield提升性能，对于一般的带物理的组件，我们只需要给他配置一个简单的碰撞体即可。这样的一个简单的物理组件在游戏运行时的开销是很小的，然而对于一个USkeletalMeshComponent，我们为了精确几乎需要给所有的骨骼都创建一个基本的物理单位，一旦我们的游戏想实现精确的打击，攻击不同位置的效果的时候，就必须用到骨骼的物理。常见的解决方案就是在需要的时候创建物理，在不需要的时候拿掉。
   



## 引用
[物理模块浅析[原理分析]](https://zhuanlan.zhihu.com/p/35686607)
[UE4的移动碰撞](https://zhuanlan.zhihu.com/p/33529865)