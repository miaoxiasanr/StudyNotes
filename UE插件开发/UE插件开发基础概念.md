# 基础概念
## 什么是UE中的插件
* UE中的插件是通过uplugin进行组织的一些模块(Module)集合。可以在引擎中进行方便的启用，能够基于引擎中的已有功能进行开发，也可以包含其他插件中的模块进行扩展。
* 一般情况下，从功能上进行区分，最常用的模块类型可以分为以下几类：
    1. Runtime：引擎运行时，在Editor和打包后都会运行。
    2. Developer：在非Shipping阶段都会参与编译和运行，用于一些开发阶段的功能，在Shipping会自动剔除，避免测试功能进入发行包。
    3. Editor：在Target为Editor的目标程序加载的类型，如启动引擎的编辑器、Commandlet等，都会启动Editor类型的模块。通常情况下，Editor类型的模块用于扩展编辑器功能，如创建独立的视口、给某些资源添加功能按钮，以及配置面板等等；
* 单个插件可以包含多个不同类型的模块，只需要在插件描述文件中描述。
### 依赖层级关系
引擎和项目中的插件有一定的依赖层级关系，需要控制好依赖层级，避免出现依赖混乱
1. 引擎中的Module应该只依赖引擎内置的Module，不应该包含其他插件中的Module。
2. 引擎插件中的Module可以依赖引擎内置的Module和其他引擎内置插件中的Module，不应该包含工程中的Moudle。
3. 工程插件的Module可以包含引擎内置Module/引擎插件Module/其他工程插件的Module。
4. 工程中的Module则可以包含以上所有的Module的集合。
![](https://img.imzlp.com/imgs/zlp/picgo/2023/ob-20221226-1.webp)
* 插件必须依托于某个工程启动，工程不单指游戏工程，而是包含target.cs定义的代码单元。一个工程既可以包含插件，也可以包含自己的模块。工程的Target配置会影响插件中的Module加载。
![](https://img.imzlp.com/imgs/zlp/picgo/2023/ob-20221226-2.webp)
在sourse目录下，创建出两个target.cs文件，用于区分打包的Runtime和Editor，他们的区别和文件命名无关，和文件内容有关，以下为两者区别
![](https://img.imzlp.com/imgs/zlp/picgo/2023/ob-20221226-3.webp)
通过TargetType指定当前Target的类型
TargetType是在UBT中定义的枚举类型表示了UE项目中五种工程的Target类型
~~~c#
namespace UnrealBuildTool
{
	[Serializable]
	public enum TargetType
	{
		/// Cooked monolithic game executable (GameName.exe).  Also used for a game-agnostic engine executable (UE4Game.exe or RocketGame.exe)
		Game,

		/// Uncooked modular editor executable and DLLs (UE4Editor.exe, UE4Editor*.dll, GameName*.dll)
		Editor,

		/// Cooked monolithic game client executable (GameNameClient.exe, but no server code)
		Client,

		/// Cooked monolithic game server executable (GameNameServer.exe, but no client code)
		Server,

		/// Program (standalone program, e.g. ShaderCompileWorker.exe, can be modular or monolithic depending on the program)
		Program,
	}
}
~~~
在IDE中选择不同的BuildConfiguration时，就会使用不同的Target。
> Development Editor会使用Blank427Editor.Target.cs中定义的配置(TargetType.Editor)
> Development会使用Blank427.Target.cs中定义的配置(TargetType.Game)

在编译时，工程中的BuildConfiguration也会传递给插件，用于决定插件中的哪些Module参与编译。
在插件的build.cs中对当前的Target的信息进行传递，用于编译不同Target时行为不同的情况
~~~c#
public LearnPlugin(ReadOnlyTargetRules Target) : base(Target)
{
    if (Target.Type == TargetType.Editor)
    {
        // ...
    }
}
~~~
## 如何描述一个插件
### 插件描述文件uplugin
* 插件描述文件是命名以.uplugin结尾的文件
* 插件描述文件使用Json文件格式
~~~json
{
    "FileVersion" :3,
    "Version" :1,                               //版本号
    "VersionName" :"1.0",                       //版本名
    "FriendlyName" :"UObject Example Plugin",   //插件名
    "Description" :"An example of a plugin which declares its own UObject type.This can be used as a starting point when creating your own plugin.",
    "Category" :"Examples",                     //插件目录
    "CreatedBy" :"Epic Games, Inc.",            //作者
    "CreatedByURL" :"http://epicgames.com",
    "DocsURL" :"",
    "MarketplaceURL" :"",
    "SupportURL" :"",
    "EnabledByDefault" : true,
    "CanContainContent" : false,                //是否包含Content目录
    "IsBetaVersion" : false,
    "Installed" : false,
    "Modules" :                                 //插件包含的模块，数组形式
    [
        {
            "Name" :"UObjectPlugin",            //模块名
            "Type" :"Developer",                //模块类型,类型为EHostType
            "LoadingPhase" :"Default"           //模块加载的阶段，类型为ELoadingPhase
        }
    ]
}
~~~
~~~c++
namespace EHostType
{
    enum Type
    {
        Runtime,                    //运行时，任何情况下
        RuntimeNoCommandlet,
        RuntimeAndProgram,
        CookedOnly,
        Developer,                  //开发时使用的插件
        Editor,                     //编辑器类型插件
        EditorNoCommandlet,
        Program,                    //只有运行独立程序时的插件
        ServerOnly,
        ClientOnly,
        Max
    }
}
namespace ELoadingPhase
{
    enum Type
    {
        PostConfigInit,             //引擎完全加载前，配置文件加载后。适用于较底层的模块。
        PreEarlyLoadingScreen,      //在UObject加载前，用于补丁系统
        PreLoadingScreen,           //在引擎模块完全加载和加载页面之前
        PreDefault,                 //默认模块加载之前阶段
        Default,                    //默认加载阶段，在引擎初始化时，游戏模块加载之后
        PostDefault,                //默认加载阶段之后加载
        PostEngineInit,             //引擎初始化后
        None,                       //不自动加载模块
        Max
    };
}
~~~
通过该信息，能够清晰的知道插件中的模块数量、类型和启动时机。
* 在编译时，UBT也会读取插件的Type，决定模块是否参与编译
* 在引擎启动时，会根据模块定义的信息进行启动，如LoadingPhase配置，在不同的启动阶段加载插件。

## Plugin的目录结构
通常一个插件会包含以下几个目录和文件：
1. Content：可选，插件的uasset资源，类似于游戏的Content目录。需要.uplugin中的CanContainContent=true;
2. Resourse:插件依赖的其他资源，如插件内的图标Icon等
3. Config：可选，插件的配置文件，类似于工程的Config目录，用于储存一些ini的配置项
4. Sourse：插件中的代码目录
5. *.uplugin：当前插件的uplugin描述
   
需要重点关注的是sourse目录，它是实现代码插件的关键。通常Sourse下会创建对应Module的目录，用于储存不同Module的代码。
如uplugin中定义了两个Module：
~~~json
"Modules": 
[
	{
		"Name": "ResScanner",
		"Type": "Developer",
		"LoadingPhase": "Default"
	},
	{
		"Name": "ResScannerEditor",
		"Type": "Editor",
		"LoadingPhase": "Default"
	}
]
~~~
就在Sourse目录下创建里那个对应名字的目录
![](https://img.imzlp.com/imgs/zlp/picgo/2023/ob-20221226-5.webp)



## Module的C++定义
C++的插件通常会包含一个继承自IModuleInterface的类，用于注册到引擎中，当启动该模块时，作为模块的入口，并且在引擎关闭时，执行模块卸载，可以清理资源。
~~~c++
//.h
#pragma once  
  
 #include "CoreMinimal.h"  
 #include "Modules/ModuleManager.h"  

class FNewCreateModule : public IModuleInterface  
{  
public:  
  
   /** IModuleInterface implementation */  
   virtual void StartupModule() override;  
   virtual void ShutdownModule() override;  
  
};

//.cpp

 #include "NewCreateModule.h"  
  
 #define LOCTEXT_NAMESPACE "FNewCreateModule"  

void FNewCreateModule::StartupModule() {}  
  
void FNewCreateModule::ShutdownModule() {}  

 #undef LOCTEXT_NAMESPACE  
IMPLEMENT_MODULE(FNewCreateModule, NewCreate)
~~~

最关键的是最后的IMPLEMENT_MODULE宏，他负责把模块暴露给外部的。
UE编译的可执行程序，有两种链接模式。分别是Modular、Monolithic。
可以在target.cs中进行控制，不过默认情况下，Editor是Modular的，其他的Target类型(如Game、Program)是Monolithic的。
~~~c++
public TargetLinkType LinkType
{
	get
	{
		return (LinkTypePrivate != TargetLinkType.Default) ? LinkTypePrivate : ((Type == global::UnrealBuildTool.TargetType.Editor) ? TargetLinkType.Modular : TargetLinkType.Monolithic);
	}
	set
	{
		LinkTypePrivate = value;
	}
}
~~~
### UE的两种链接模式
#### modular模式
Modular模式，就是所谓的分片模式，每个模块都会编译成为独立的可执行2文件，优点是可以增量编译变动的模块，而不用编译整个工程。
~~~c++
#define IMPLEMENT_MODULE( ModuleImplClass, ModuleName ) \
	\
	/**/ \
	/* InitializeModule function, called by module manager after this module's DLL has been loaded */ \
	/**/ \
	/* @return	Returns an instance of this module */ \
	/**/ \
	extern "C" DLLEXPORT IModuleInterface* InitializeModule() \
	{ \
		return new ModuleImplClass(); \
	} \
	/* Forced reference to this function is added by the linker to check that each module uses IMPLEMENT_MODULE */ \
	extern "C" void IMPLEMENT_MODULE_##ModuleName() { } \
	PER_MODULE_BOILERPLATE \
	PER_MODULE_BOILERPLATE_ANYLINK(ModuleImplClass, ModuleName)


// for not-monolithic
extern "C" DLLEXPORT IModuleInterface* InitializeModule()
{
	return new FNewCreateModule();
}
extern "C" void IMPLEMENT_MODULE_NewCreate() { }
~~~
#### Monolithic模式
Monolithic模式，就是工程中所有的C++代码，都被静态编译链接在同一个可执行程序之内(添加的动态链接不算，额外的静态链接库也会编译到可执行程序之内)。
UE中，默认打包时就是Monolithic模式，引擎和工程内的代码多会编译到同一个文件，如
* WindowsNoEditor：WindowsNoEditor/GameName/Binaries/Win64/GameName.exe
* Android：lib/arm64-v8a/libUE4.so
* IOS：Payload/GameName.app/GameName

这种方式优点是没有额外的PE文件查找和加载开销，所有操作都是在当前的进程空间内执行。
模块1的定义也对应有些区别，在Not-Monolithic模式下，为了编译为独立的可执行程序，会创建标记了DLLEXPORT的InitializeMoudle函数。但是Monolithic模式不需要，他是用的是静态符号。
~~~c++
// If we're linking monolithically we assume all modules are linked in with the main binary.  
#define IMPLEMENT_MODULE( ModuleImplClass, ModuleName ) \  
	/** Global registrant object for this module when linked statically */ \  
	static FStaticallyLinkedModuleRegistrant< ModuleImplClass > ModuleRegistrant##ModuleName( TEXT(#ModuleName) ); \  
   /* Forced reference to this function is added by the linker to check that each module uses IMPLEMENT_MODULE */ \  
   extern "C" void IMPLEMENT_MODULE_##ModuleName() { } \  
   PER_MODULE_BOILERPLATE_ANYLINK(ModuleImplClass, ModuleName)


static FStaticallyLinkedModuleRegistrant< FNewCreateModule > ModuleRegistrantNewCreateModule( TEXT("NewCreateModule") );
extern "C" void IMPLEMENT_MODULE_NewCreate() { }
~~~
实际上是定义了一个static的对象，它实际上是利用了C++的一个特性：具有static的对象的初始化会在main函数的第一个语句之前执行。
所以当引擎启动时。所有的模块都会构造一个static对象，并进行初始化。




[UE 插件与工具开发：基础概念](https://imzlp.com/posts/75405/)