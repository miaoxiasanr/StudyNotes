#UE4动画
##基本资源介绍
![](https://pic1.zhimg.com/80/v2-1678a1044ca085eef049680d5fe40c68_1440w.webp)
###骨架(Skeleton)
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

###骨骼模型(SkeletalMesh)
骨骼模型就是在骨骼基础之上的模型，通俗来讲就是绑定骨骼后的网格体，也就是我们在游戏中所看到的的肉身，主要包含：
* 模型的顶点、线、面信息
* 顶点的骨骼蒙皮权重
* 模型的材质信息
* 模型LOD信息
* Morph Target信息
* Physics Asset设置信息
* 布料系统相关设置

###动画序列(Animation Sequence)
动画序列是用来记录骨骼运动状态的资源，也是让动画动起来的关键之一，主要包含以下
* 动画关键帧信息
* 包含运动相关的骨骼信息
* 每帧骨骼的Transform信息：包含了骨骼的旋转，位移和缩放
* 动画通知信息：记录了触发通知的类型和时间
* 动画曲线信息：记录了随时间轴变化的曲线信息
* 其他基础信息：包括了叠加动画设置、跟骨骼位移设置等信息
###混合空间(Blend Space)
其主要作用是根据输入的float值从若干临近的动画中采样并输出一个融合后的Pose
$$ PoseC=Lerp(PoseA,PoseB,float)$$

###AimOffset
AimOffset是BlendSpace的一种特殊形式，区别在于内部的所有采样点都是叠加动画，主要作用是在游戏过程中叠加角色瞄准和看的方向

###AnimationMotage
AnimationMotage是对Animation Sequence等资源的扩充，可以方便的使用蓝图、代码控制动画资源的播放，并且可以方便的实现根骨骼动画和动画播放结束后的回调。

##动画原理剖析

![](https://img-blog.csdnimg.cn/20201015191848213.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIzMDMwODQz,size_16,color_FFFFFF,t_70)


[UE4 动画系统 源码及原理剖析](https://blog.csdn.net/qq_23030843/article/details/109103433)
[UE4动画系统基础](https://zhuanlan.zhihu.com/p/62401630)
[UE4 动画系统优秀文摘](https://zhuanlan.zhihu.com/p/413608091)