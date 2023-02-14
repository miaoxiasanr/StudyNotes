- [UFunction](#ufunction)
  - [函数说明符](#函数说明符)
  - [元数据说明符](#元数据说明符)

# UFunction
## 函数说明符
* BlueprintAuthorOnly
  如果在具有网络权限的机器上运行(服务器。专用服务器或单人游戏)，此函数将仅从蓝图代码中执行
* BlueprintCosmetic
  次函数为修饰性的，无法在专用服务器上运行
* BlueprintCallable
  次函数可在蓝图或关卡蓝图表中执行
* BlueprintImplementableEvent
  此函数在蓝图中实现，在cpp中声明
* BlueprintNativeEvent
  此函数在CPP中的实现会被蓝图覆盖掉，但是可以再cpp中有默认实现。用于声明名称于主函数相同的附加函数，但是末尾添加了_Implementation,是写入代码的位置，如果未找到任何蓝图覆盖，则会调用_Implementation的方法
* BlueprintPure
  没有执行引脚
* CallInEditor
  可通过细节面板中的按钮在编辑器中的选定实例上调用此函数
* Category="TopCategory|SubCategory|Etc"
  指定函数的类别
* Client
  次函数仅在拥有客户端上执行，函数会在末尾添加_Implementation
* Server
  此函数在服务器上执行，函数会在末尾添加_Implementation
* NetMulticast
  此函数在服务器上调用，会在所有客户端上执行
* Reliable
  此函数将通过网络复制，并且一定会到达，即使出现网络错误，配合Client或Server配合使用才有效
* SealedEvent
  无法在子类中覆盖此函数。SealedEvent只能用于事件，对于非事件函数可以声明为static或final
* UnReliable
  此函数将通过网络复制，但是可能会因带宽限制或网络错误而失败，配合Client或Server配合使用才有效
* WithValidation
  用于声明名称和主函数相同的附加函数，但是末尾需要添加“_Validate",此函数使用相同的参数，但是会返回bool，已指示是否继续调用主函数
## 元数据说明符
* AdvancedDisplay="Param1,Param2,.."
  以逗号分隔的参数列表将显示为高级引脚(需要UI扩展)
* AdvancedDisplay=N
  用一个数字替代N，第N之后的所有参数将显示为高级引脚(需要UI扩展)
* BlueprintProtected
  此函数只能在蓝图中的拥有对象上调用，其无法在另一个实例上调用。
* CallableWithoutWOrldContext
  用于拥有一个WorldContext引脚的BlueprintCallable函数，说明函数可被调用，即使其类不是先GetWorld函数也同样如此。
* DevelopmentOnly
  函数只会在Development模式上运行，这适用于调试输出之类的功能。
* worldContext="Parameeter"
  由BlueprintCallable函数使用，说明那个参数运算决定正在发生的World