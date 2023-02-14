- [Slate编程](#slate编程)
  - [新建一个S类组件](#新建一个s类组件)
  - [链式编程优缺点](#链式编程优缺点)
  - [创建Slate子物体的几种方式。](#创建slate子物体的几种方式)
  - [Slate编写](#slate编写)
    - [空类中必须要有如下的代码](#空类中必须要有如下的代码)
    - [宏讲解SLATE\_BEGIN\_ARGS(){}SLATE\_END\_ARGS()](#宏讲解slate_begin_argsslate_end_args)

# Slate编程
## 新建一个S类组件
~~~c++
TSharedRef<SButton>MyButton=SNew(SButton);//返回共享引用
//or
TSharedPtr<SButton> MyButton;
SAssignNew(MyButton,SButton);//返回共享指针
//如果在链式编程中直接获取值，直接赋值，在链式编程中想获取值就用SAssignNew
~~~

## 链式编程优缺点

* 优点：
  1. 效率比UMG要高，因为UMG封装的就是Slate。
* 缺点：
  1. 不能断点调试，断点无法命中链式内部。
  2. 编写界面制作麻烦且不易维护。


## 创建Slate子物体的几种方式。
~~~c++
TSharedRef<SDockTab> FCloudBoyModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FCloudBoyModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("CloudBoy.cpp"))
		);
    //第一种方法
    TSharedRef<SMainSlate>MyMainSlate;
    MyMainSlate=SNew(SMainSlate);
    return SNew(SDockTap)
        [
            MyMainSLate->AsShared()
        ];
    //第二种方法
    TSharedPtr<SMainSlate>MyMainSlate;
    SAssignNew(MyMainSlate,SMainSlate);
    return SNew(SDockTap)
        [
            MyMainSLate->AsShared()
        ];
    //第三种方法
    return SNew(SDockTap)
        [
            SNew(SMainSlate)
        ];
    //第四种方法
    TSharedPtr<SMainSlate>MyMainSlate;
    return SNew(SDockTap)
        [
            SAssignNew(MyMainSlate,SMainSlate)
        ];
}
~~~

## Slate编写
### 空类中必须要有如下的代码
~~~c++
class SMainSlate:public SCompoundwidget
{
public:
    //SLATE_BEGIN_ARGS+SLATE_END_ARGS 实际上就是一个结构体，内部写的东西相当于写在了一个结构体里面
    SLATE_BEGIN_ARGS(SMainSlate)
    {

    }
    SLATE_END_ARGS()

    //外部执行SNew或者SAssignNew时会调用Construct()
    void Construct(const FArgunments& InArgs);
}
~~~
### 宏讲解SLATE_BEGIN_ARGS(){}SLATE_END_ARGS()

