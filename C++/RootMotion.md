- [RootMotion](#rootmotion)
  - [RootMotion概述](#rootmotion概述)

# RootMotion
## RootMotion概述
**RootMotion，跟骨骼位移，属于移动组与动画系统相结合的一个部分，表示角色的整体移动(包括物理)是由动画来驱动的。**
一般来说，在大部分游戏的应用里面，玩家的移动与动画是分开的，移动系统只负责处理玩家的位置和旋转，动画系统只做对应的动画表现，只要移动的速度合适就可以与动画做到完美的匹配，也就是说动画播放的位置(即Mesh的位置)是由角色移动来驱动的(UE4里面，动画是胶囊体的位置数据来驱动的)。例如：如果胶囊体在向前移动，系统就会知道在角色上播放一个跑步或行走的动画，让角色看起来是在靠自己的力量移动。

然而，当玩家发起一次特殊攻击，在这种攻击下，模型已预先设定好向前冲的动作。如果所有的角色动画都是基于玩家胶囊体的，这样的动画会导致角色迈出胶囊体，从而在事实上失去碰撞，一旦动画播放结束，玩家就会滑回其碰撞位置。

![]()







[官方文档——RootMotion](https://docs.unrealengine.com/4.27/zh-CN/AnimatingObjects/SkeletalMeshAnimation/RootMotion/)
[《Exploring in UE4》RootMotion详解【原理分析】](https://zhuanlan.zhihu.com/p/74554876)