- [UE4动画](#ue4动画)
  - [基本资源介绍](#基本资源介绍)
    - [骨架(Skeleton)](#骨架skeleton)
      - [骨架的理解](#骨架的理解)
        - [骨骼和关节](#骨骼和关节)
      - [骨架数据](#骨架数据)
    - [骨骼模型(SkeletalMesh)](#骨骼模型skeletalmesh)
    - [动画序列(Animation Sequence)](#动画序列animation-sequence)
    - [混合空间(Blend Space)](#混合空间blend-space)
    - [AimOffset](#aimoffset)
    - [AnimationMotage](#animationmotage)
  - [UE4角色动画原理简介](#ue4角色动画原理简介)
  - [动画系统更新过程简述](#动画系统更新过程简述)
  - [动画原理剖析](#动画原理剖析)

# UE4动画
## 基本资源介绍
![](https://pic1.zhimg.com/80/v2-1678a1044ca085eef049680d5fe40c68_1440w.webp)
### 骨架(Skeleton)
#### 骨架的理解
* 在骨骼动画中，不是把Mesh直接放到世界坐标系中，Mesh只是作为Skin使用的，真正决定模型在世界坐标的位置和朝向的是骨骼。
* 在渲染静态模型中，由于模型的顶点都是定义在模型参考系中的，所以各顶点只要经过模型坐标系到世界坐标系的变换后就可以渲染。
* 而对于骨骼动画，我们设置模型的位置和朝向实际上是设置根骨骼的位置和朝向，然后根据骨骼层次结构中父子骨骼之间的变换关系计算出各个骨骼之间的位置和旋转，然后根据骨骼对Mesh顶点的绑定计算出顶点在世界坐标系中的坐标，从而对顶点进行渲染。
##### 骨骼和关节

骨骼只是一个形象的说法，实际上骨骼可以理解为一个坐标空间，关节可以理解为骨骼坐标空间的原点。关节的位置由他在父骨骼坐标空间中的位置描述。关节既决定了骨骼空间的位置，又是骨骼空间的旋转和缩放中心。
> ![](https://img-blog.csdnimg.cn/20201015191854926.png)
> 上图中有三块骨骼，分别是上臂，前臂和两个手指。Clavicle（锁骨）是一个关节，他是上臂的原点，同时肘关节(Elbow Joint)是前臂的原点，腕关节(Wrist)是手指骨骼的原点。

骨骼是坐标空间，骨骼层次就是嵌套的坐标空间。关节只是描述骨骼的位置即骨骼自己的坐标空间原点在其父空间中的位置，绕关节旋转是指骨骼坐标空间(包括所有子空间)自身的旋转。
几个疑问
* 骨骼的长度问题？
  由于骨骼是坐标空间，没有所谓的长度和宽度的限制，我们看到的长度一方面是蒙皮后的效果，另一方面是子骨骼的原点(也就是关节)的位置往往决定了视觉上父骨骼的长度(比如upper arm线段的长度实际是由elbow joint决定的)
* 手指的长度如何确定的?
  在例子中手指尾部有一个端点，实际上是没有的，是方便演示画上去的，我们看到的手指部分的长度是由手指部分的顶点和蒙皮决定的。
* 为什么要将骨骼组织成层次结构呢？
  答案是为了方便做动画，如果只有一块骨骼，那那动起来是很简单的，动画每一帧直接指定他的位置即可。如果是n块的话，组织成一个层次结构，就可以通过父骨骼控制子骨骼的运动，牵一发而动全身，改变某骨骼时并不需要设置其下骨骼的位置，子骨骼的位置会通过计算自动得到。
#### 骨架数据
Skeleton是这个动画系统的基础，其主要作用是记录了骨架以下信息
* 骨骼层级信息
* 参考姿势信息
* 骨骼名称
* 插槽(Socket)信息
* 曲线(Curve)信息
* 动画通知(Animation Notify)信息
* 插槽数据(插槽名称，所属骨骼，Transform等)
* 虚拟骨骼信息
骨骼名称Index映射表
其他骨骼设置信息：包括位移重定向(Translation Retarget)设置，LOD设置等等

### 骨骼模型(SkeletalMesh)
骨骼模型就是在骨骼基础之上的模型，通俗来讲就是绑定骨骼后的网格体，也就是我们在游戏中所看到的的肉身，主要包含：
* 模型的顶点、线、面信息
* 顶点的骨骼蒙皮权重
* 模型的材质信息
* 模型LOD信息
* Morph Target信息
* Physics Asset设置信息
* 布料系统相关设置

### 动画序列(Animation Sequence)
动画序列是用来记录骨骼运动状态的资源，也是让动画动起来的关键之一，主要包含以下
* 动画关键帧信息
* 包含运动相关的骨骼信息
* 每帧骨骼的Transform信息：包含了骨骼的旋转，位移和缩放
* 动画通知信息：记录了触发通知的类型和时间
* 动画曲线信息：记录了随时间轴变化的曲线信息
* 其他基础信息：包括了叠加动画设置、跟骨骼位移设置等信息
### 混合空间(Blend Space)
其主要作用是根据输入的float值从若干临近的动画中采样并输出一个融合后的Pose
$$ PoseC=Lerp(PoseA,PoseB,float)$$

### AimOffset
AimOffset是BlendSpace的一种特殊形式，区别在于内部的所有采样点都是叠加动画，主要作用是在游戏过程中叠加角色瞄准和看的方向
$$ D(a)=PoseA -PoseBone$$
$$ PoseC=PoseB+D(a)$$
![](https://pic4.zhimg.com/80/v2-55a118685cc36068be8936a99e46e63b_720w.webp)
### AnimationMotage
AnimationMotage是对Animation Sequence等资源的扩充，可以方便的使用蓝图、代码控制动画资源的播放，并且可以方便的实现根骨骼动画和动画播放结束后的回调。

## UE4角色动画原理简介
**UE4的角色动画是采用了目前最流行的骨骼动画系统**
骨骼动画系统分为两步
* 计算骨骼Pose：每一根骨骼可以看做一个点，而我们的Pose就是所有骨骼Transform(位移+旋转+缩放)的集合，一般来说，这个Pose是基于骨骼参考姿势(Reference Pose)的变换矩阵。
* 蒙皮：骨骼计算好后我们还是无法看到一个角色的模型，这时候需要将美术制作好的网格体的顶点按照骨骼Pose进行变化。这里的变化的依据是顶点的蒙皮权重和参考姿势的信息(一个顶点可能受到多个骨骼的影响)

## 动画系统更新过程简述
* UE4承担动画职责的是SkeletalMeshComponent。在SkeletalMeshComponent创建时，会创建一个AnimationInstance,这就是主要负责计算最终Pose的对象，而我们制作的AnimationBlueprint也是基于UAnimationInstance这个类的。
* 在SkeletalMeshComponent进行Tick时，会调用TickAnimation方法，然后调用AnimationInstance的UpdateAnimation()方法，此方法会调用所有动画蓝图中连接的节点的Update_AnyThread()方法，用来更新所有节点的状态。
* 然后根据设置的不同会从Tick函数或者Work线程中调用SkeletalMeshComponent的RefreshBoneTransforms()方法，此方法会调用动画蓝图所有节点的Evaluate_AnyThread()方法。
* Evaluate的含义就是根据特定的条件计算出动画所有骨骼的Transforms信息，随后输出一个Pose给到渲染线程并存在本地Component上。

## 动画原理剖析

![](https://img-blog.csdnimg.cn/20201015191848213.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIzMDMwODQz,size_16,color_FFFFFF,t_70)


[UE4 动画系统 源码及原理剖析](https://blog.csdn.net/qq_23030843/article/details/109103433)
[UE4动画系统基础](https://zhuanlan.zhihu.com/p/62401630)
[UE4 动画系统优秀文摘](https://zhuanlan.zhihu.com/p/413608091)