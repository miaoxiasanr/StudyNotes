# UPROPERTY
~~~c++
	UPROPERTY(Specifier,Specifier,...,meta="key=value,key=value,...")
		ValueType ValueName;
~~~
## 属性标签
* AdvancedDisplay
  属性会被放置在其出现的任意面板的高级(下拉)部分中
* BlueprintAssignable
  只能与组播委托共用。公共函数在蓝图中指定
* BlueprintAuthorityOnly
  此属性必须为一个组播委托。在蓝图中，其只接受带BlueprintAuthorityOnly标签的事件
* BlueprintCallable
  仅用于组播委托，应公开属性在蓝图中调用
* BlueprintGetter=GetterFunctionName
  此属性制定一个自定义存取器函数，如果此属性不带有BlueprintSetter或BlueprintReadWrite标签，则其隐式BlueprintReadOnly
* BlueprintReadOnly
  此属性可由蓝图读取，但不能被修改，此说明符与BlueprintReadWrite说明符不兼容
* BlueprintReadWrite
  此属性可以从蓝图读取或写入属性，此说明符与BlueprintReadOnly不兼容
* Category="TopCategory|SubCategory|..."
  指定在蓝图编辑器工具中显示时的属性类别，使用|运算定义符嵌套
* Config
  此属性设为可配置，当前值可被传入与类相等的.ini文件，创建后将被加载，无法从默认属性中给定一个值，暗示为BlueprintReadOnly
* GlobalConfig
  工作原理和Config相似，不同点是无法在子类中进行覆盖，无法从默认属性中给定一个值，暗示为BlueprintReadOnly
* DuplicateTransient
  说明在任意类型的复制中(复制/粘贴，二进制复制等)，属性的值应该被设为类默认值
* EditAnywhere
  说明此属性可通过哦属性窗口在原型和实例中进行编辑，此说明符与所有可见说明符不兼容
* EditDefaultsOnly
  说明此属性可通过属性窗口进行编辑，但只能在类默认值上进行，此说明符与所有可见说明符不兼容
* EditFixedSize
  只适用于动态数组，这能防止用户通过编辑器窗口修改数组长度
* EditInstanceOnly
  说明此属性可通过属性窗口进行编辑，但只能在实例上进行编辑，不能再类默认值上进行设置，此说明符与所有可见说明符不兼容
* VisibleAnywhere
  说明此属性在所有窗口中可见，但无法被编辑，此说明符与“”Edit“说明符不兼容
* VisibleDefaultsOnly
  说明此属性只能在类默认值上可见，但无法被编辑，此说明符与“”Edit“说明符不兼容
* VIsibleInstanceOnly
  说明此属性只能在实例的属性窗口上可见(类默认值窗口不可见)，且无法被编辑，此说明符与“”Edit“说明符不兼容
* Replicated
  属性应随网络进行复制
* NotReplicated
  跳过复制，这只会应用到服务请求函数中的结构体成员和参数
* ReplicatedUsing=FunctionName
  ReplicatedUsing指定一个回调函数，属性通过网络更新时执行

* SkinSerialization
  此属性不会被序列化，但仍能导出一个文本格式(用于复制、粘贴操作)
* SerializeText
  属性会被序列化为文本
* TextExportTransient
  此属性不会被导出为一个文本格式(无法复制、粘贴操作)
* Transient
  属性为临时，意味着其无法被保存或加载，以此方法标记的属性将在加载时被0填充
* SimpleDisport
  出现在细节面板的可见或可编辑属性，无需打开高级部分即可见
## 元数据标签
* AllowAbstract="true/false"
  用于SubClass和SoftClass属性，说明抽象类属性是否应显示在类选取器上
* AllowedClassed="Class1,CLass2,..."
  用于FSoftObjectPAth属性，逗号分隔的列表，表明要显示在资源选取器中的资源类类型
* AllowPreserveRatio
  用于FVector属性，在细节面板显示此属性是应添加一个比率锁
* ClampMin="N"
  用于浮点和整数属性，指定可在属性中输入的最小值N
* ClimpMax="M"
  用于浮点和整数属性，指可在属性中输入的最大值M
* ContentDir
  由FDirectoryPath属性使用，说明将使用Content文件夹中的Slate风格目录选取器来选取路径。未调用则会显示Window下的文件选取
* DisplayName="PropertyName"
  此属性显示的命名，不显示代码生成的命名
* DisplayThumbnail="true"
  说明属性是一个资源属性，其应显示选中资源的缩略图