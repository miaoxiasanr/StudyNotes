- [UClass](#uclass)
    - [类说明符](#类说明符)
    - [元数据说明符](#元数据说明符)

# UClass
类声明的语法如下所示
~~~c++
UCLASS([Specifier,Specifier,...],[meta(key=value,key=value...)])
class classname:public ParentName
{
    GENERATED_BODY()
}
~~~
### 类说明符
* Abstract 
  抽象基类说明符是将类说明符声明为”抽象基类“，防止用户在编辑器中向世界添加此类的参与者，或在游戏中创建此类的实例。这对于那些本身没有意义的类很有用。
  例如，ATriggerBase基类是抽象的，而ATriggerBox子类不是抽象的，可以在世界中放置一个ATriggerBox类的实例并且他是有用的，而ATriggerBase的实例本身并不可用。
* AdvancedClassDisplay
  AdvancedClassDisplay类说明符强制类的所有属性只显示在“详细信息”面板的高级部分中，默认情况下隐藏在“视图”中，若要在单个属性上岗重写此项，请在该属性上使用SimpleDisplay说明符。
* AutoCollapseCategories=(Category1,Category2)/DontAutoCollapseCategories=(Category,Category)
  AutoCollapseCategories类说明符取消对父类AutoExpandCategories说明符的列出类别的影响
  DontAutoCollapseCategories否定对父类继承的列出累呗的AutoCollapseCategories说明符
* AutoExpandCategories=(Category1,Category2)
* Blueprintable/NotBlueprintable
  将此类公开为创建蓝图的可接受基类，默认是NotBlueprintable，除非继承，否则该说明符有子类继承
* BlueprintType
  将此类公开为可用于蓝图中的变量的类型。

* ClassGroup=GroupName
  表示在Actor浏览器中启用GroupView后，编辑器的Actor浏览器应该在指定的GroupName中包含此类以及此类的任何子类。
* CollapseCategories/DontCollapseCategories
  表示不应该将此类的属性分组到编辑器属性窗口中的类别中，这个说明符被传播给子类；但是子类可以使用DontCollapseCategories说明符覆盖它
* Config=ConfigName
  表示允许在配置文件(.ini)中存储数据，如果有使用config或globalconfig说明符声明的任何属性，则此说明符将导致这些属性储存在命名的配置文件中。此说明符被传播到所有子类，不能被否定，但是子类可以通过re-declaring配置说明并提供不同的ConfigName来更改文件。常见的ConfigName值是“Engine”，“Editer”,"Input","Game";
* Const
  该类中的所有属性和函数都是const的，并以const的形式导出。该说明符由子类继承
* ConversionRoot
  
* CustomConstructor
  阻止自动生成构造函数声明
* DefaultToInstanced
  这个类的所有实例都被认为是“实例化”的，实例化的类(组件)在构建时被复制，该说明符由子类继承。
* DependsOn=(ClassName1,ClassName2,...)
  列出的所有类将在该类之前编译。类必须在同一个(或上一个)包中指定一个类。
* Deprecated
  这个类不推荐使用，而且这个类的对象在序列化的时候不会被保存，该说明符由子类继承。
* Transient/NotTransient
  属于此类的对象将永远不会保留到磁盘中，这与某些非永久的自然类(比如播放器或窗口)结合使用非常有用，此说明符会被传播到子类，但可以由NotTransient说明符重写。
* MinimalAPI
  仅导致要导出的类的类型信息供其他模块使用，类可以被强制转换，但不能调用类的函数(内联方法除外)。还提高了编译时间，因为不需要在其他模块中访问所有函数的类导出所有内容。
### 元数据说明符
* BlueprintSpawnableComponent
  如果存在，组件类可以由蓝图生成
* BlueprintThreadSaft
  仅对蓝图函数库有效，此说明符将此类中的函数标记为在动画蓝图中的游戏线程上调用。
* ChildCannnotTick/ChildCanTick
  * ChildCannnotTick用于Actor和Component类。如果本基类不能勾选，基于此Actor或Component的蓝图生成的类的Tick将永远不会勾选(即使bCanBlueprintTickByDefault为True)
  * ChildCanTick用于Actor和Component类，如果基类中由声明，则会覆盖子类中的bCanEventTick标志，即使bCanBlueprintTickByDefault为false
* DeprecatedNode
  对于行为树节点，只是该类已弃用，并在编译时显示警告
* DeprecationMessage="Message Text"
  如果该类被弃用，则在尝试编译时使用他的蓝图时，此消息将被添加到警告文本中
* DisPlayName="BlueprintNodeName"
  蓝图中此节点的名称将替换为此处提供的值，而不是代码中生成的名称
* IsBlueprintBase="true/false"
  声明此类是(或不是)用于创建蓝图的可接受基类，类似于Blueprintable/NotBlueprintable
* KismetHideOverrides="Event1,Event2,..."
  不允许重写的蓝图事件列表
* ProhibitedInterfaces=”Interface1, Interface2, …”
  列出与类不兼容的接口


[UE4入门-常见的宏-UCLASS](https://blog.csdn.net/u012793104/article/details/78547655?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-78547655-blog-120902898.pc_relevant_3mothn_strategy_recovery&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-78547655-blog-120902898.pc_relevant_3mothn_strategy_recovery&utm_relevant_index=1)
[类说明符](https://docs.unrealengine.com/4.27/zh-CN/ProgrammingAndScripting/GameplayArchitecture/Classes/Specifiers/)