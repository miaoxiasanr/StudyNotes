- [USTRUCT](#ustruct)
  - [结构体说明符](#结构体说明符)
  - [元数据说明符](#元数据说明符)

# USTRUCT
## 结构体说明符
* Atomic 
  表面此结构应该永远序列化为单个对象，不会为该类创建自动生成代码
* BlueprintType
  将此结构最为可以在蓝图中被用于变量的类型公开
## 元数据说明符
* BlueprintSpawnableComponent
  如果存在，组件类可以由蓝图生成
* BlueprintThreadSaft
  仅对蓝图函数库有效，此说明符将此类中的函数标记为在动画蓝图中的游戏线程上调用。
* ChildCannnotTick/ChildCanTick
  * ChildCannnotTick用于Actor和Component类。如果本基类不能勾选，基于此Actor或Component的蓝图生成的类的Tick将永远不会勾选(即使bCanBlueprintTickByDefault为True)
  * ChildCanTick用于Actor和Component类，如果基类中由声明，则会覆盖子类中的bCanEventTick标志，即使bCanBlueprintTickByDefault为false
* DisPlayName="BlueprintNodeName"
  蓝图中此节点的名称将替换为此处提供的值，而不是代码中生成的名称