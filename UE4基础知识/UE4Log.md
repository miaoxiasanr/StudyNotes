# UE4Log
## UE_Log语法
下面是一个简单的日志消息示例：
~~~C++
UE_LOG(LogTemp,Log,TEXT("Hello"));
~~~
要利用日志语句上中获得输出，请确保在在测试更改之前编译项目。
UE_LOG是将日志消息输出到日志文件中的宏。它采用的是
第一个输出参数是日志记录类别的名称,
第二个参数是声明消息的输出等级。
~~~c++
//LogVerbosity.h
namespace ELogVerbosity
{
	enum Type : uint8
	{
		/** Not used */
		NoLogging		= 0,

		/** Always prints a fatal error to console (and log file) and crashes (even if logging is disabled) */
		Fatal,

		/** 
		 * Prints an error to console (and log file). 
		 * Commandlets and the editor collect and report errors. Error messages result in commandlet failure.
		 */
		Error,

		/** 
		 * Prints a warning to console (and log file).
		 * Commandlets and the editor collect and report warnings. Warnings can be treated as an error.
		 */
		Warning,

		/** Prints a message to console (and log file) */
		Display,

		/** Prints a message to a log file (does not print to console) */
		Log,

		/** 
		 * Prints a verbose message to a log file (if Verbose logging is enabled for the given category, 
		 * usually used for detailed logging) 
		 */
		Verbose,

		/** 
		 * Prints a verbose message to a log file (if VeryVerbose logging is enabled, 
		 * usually used for detailed logging that would otherwise spam output) 
		 */
		VeryVerbose,

		// Log masks and special Enum values

		All				= VeryVerbose,
    }
}
~~~
引擎中已经内置了许多这种类别，在CoreGlobes.h中定义。
~~~c++
//CoreGlobes.h
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogHAL, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogSerialization, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealMath, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealMatrix, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogContentComparisonCommandlet, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNetPackageMap, Warning, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNetSerialization, Warning, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogMemory, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogProfilingDebugging, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogOutputDevice, Log, All);

CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogSHA, Warning, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogStats, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogStreaming, Display, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogInit, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogExit, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogExec, Warning, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogScript, Warning, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLocalization, Error, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLongPackageNames, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogProcess, Log, All);
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLoad, Log, All);

// Temporary log category, generally you should not check things in that use this
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogTemp, Log, All);
~~~
## Log消息格式化实例
* 含FString的Log
~~~c++
UE_LOG(LogTemp, Warning, TEXT("The Actor's name is %s"), *YourActor->GetName());
~~~
* 含Bool的Log
~~~c++
UE_LOG(LogTemp, Warning, TEXT("The boolean value is %s"), ( bYourBool ? TEXT("true") : TEXT("false") ));
~~~
* 含Int的Log
~~~c++
UE_LOG(LogTemp, Warning, TEXT("The integer value is: %d"), YourInteger);
~~~
* 含Float的Log
~~~c++
UE_LOG(LogTemp, Warning, TEXT("The float value is: %f"), YourFloat);
~~~
* 含FVector的Log
~~~c++
UE_LOG(LogTemp, Warning, TEXT("The vector value is: %s"), *YourVector.ToString());
~~~

* 含FName的Log
~~~c++
UE_LOG(LogTemp,Warning,TEXT("MyCharacter's FName is %s"), *MyCharacter->GetFName().ToString());
//中文
FString str=UTF8_TO_TCHAR("中文");
UE_LOG(LogTemp,Warning,TEXT(" %s"), *str );
~~~
## 自定义Log的Category
自定义Log的步骤有三步：声明、定义、使用。
两种方法
*  如果你想定义一个"Public"的Category,并且在全局生效，不管是Static函数或者其他类都能使用，

    1. 头文件中声明
        ~~~c++
        DECLARE_LOG_CATEGORY_EXTERN(LogMyAwesomeGame, Log, All);
        ~~~
        * 第一个参数：我们新定义的LOG类别的名称
        * 第二个参数：这个种类的LOG定义了输出等级
        * 第三个参数：决定了编译阶段会被编译进去的LOG层级，这意味着在Runtime对Log的输出层级进行修改变为不可能，当然是指修改为未被编译进去的LOG层级。
        > 为什么会需要这个参数？原因就是对于我们不想输出的层级仍然需要进行比是否要输出该层级，索性直接将其从编译中移除更彻底。
    2. CPP文件中定义
        ~~~c++
        DEFINE_LOG_CATEGORY(LogMyAwesomeGame);
        ~~~
    3. 使用
        ~~~C++
        UE_LOG(LogMyAwesomeGame, Log, TEXT("Test Log Message"));
        ~~~

* 如果想定义一个只在CPP文件中使用的Category，不希望被其他类使用，可以定义一个Static的Category
~~~c++
DEFINE_LOG_CATEGORY_STATIC(LogMyAwesomeGame,Log,All);//参数同上
~~~

### 示例
~~~c++
// in A.h
DECLARE_LOG_CATEGORY_EXTERN(LogMyAwesomeGame, Log, All);

// in A.cpp
DEFINE_LOG_CATEGORY(LogMyAwesomeGame);
DEFINE_LOG_CATEGORY_STATIC(LogMyAwesomeGameStatic,Log,All);


UE_LOG(LogMyAwesomeGame, Log, TEXT("Test Log Message"));
UE_LOG(LogMyAwesomeGameStatic, Log, TEXT("Test Log Message"));

// in B.h
 #include"A.h"


// in B.cpp


UE_LOG(LogMyAwesomeGame, Log, TEXT("Test Log Message"));//正确
UE_LOG(LogMyAwesomeGameStatic, Log, TEXT("Test Log Message"));//Error,只在A.CPP有效

~~~

[UE4-正确使用LOG](https://stonelzp.github.io/how-to-use-log/)
[虚幻UE_LOG使用教程](https://zhuanlan.zhihu.com/p/463724067)