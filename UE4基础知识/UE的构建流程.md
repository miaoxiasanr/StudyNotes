# UE的构建流程
* UE通过UBT来构建项目(不管是VS里的Build也好，还是Editor的Compile也好，最终都会调用UBT)。UBT和UHT是UE工具链的基石。
* UBT 和 UHT 的工作职责
  * UBT
    1. 扫描模块和插件的解决方案目录
    2. 确定需要重建的所有模块
    3. 调用UHT来解析C++头文件
    4. 从 .Build.cs 和 .Target.cs 创建编译器和链接器选项
    5. 执行特定于平台的编译器（VisualStudio、LLVM）
  * UHT
    1. 解析所有包含 UClasses 的 C++ 头文件
    2. 为所有 Unreal 类和函数生成依赖代码
    3. 生成的文件存储在 Intermediates 目录中

* 在项目属性中可以看到构建命令使用的均是Engine/Build/BatchFiles目录下的bat
![](https://img.imzlp.com/imgs/zlp/blog/posts/6362/03_UE_VS_Project_Properties.webp)
~~~json
 # Build
 Engine\Build\BatchFiles\Build.bat
 # ReBuild
 Engine\Build\BatchFiles\Rebuild.bat
 # Clean
 Engine\Build\BatchFiles\Clean.bat
~~~
以Build.bat为例
~~~c++
@echo off
setlocal enabledelayedexpansion

REM The %~dp0 specifier resolves to the path to the directory where this .bat is located in.
REM We use this so that regardless of where the .bat file was executed from, we can change to
REM directory relative to where we know the .bat is stored.
pushd "%~dp0\..\..\Source"

REM %1 is the game name
REM %2 is the platform name
REM %3 is the configuration name

IF EXIST ..\..\Engine\Binaries\DotNET\UnrealBuildTool.exe (
        ..\..\Engine\Binaries\DotNET\UnrealBuildTool.exe %*
		popd

		REM Ignore exit codes of 2 ("ECompilationResult.UpToDate") from UBT; it's not a failure.
		if "!ERRORLEVEL!"=="2" (
			EXIT /B 0
		)
		 
		EXIT /B !ERRORLEVEL!
) ELSE (
	ECHO UnrealBuildTool.exe not found in ..\..\Engine\Binaries\DotNET\UnrealBuildTool.exe 
	popd
	EXIT /B 999
)
~~~
可以看到Build.bat将接收到的参数都转发给了UnrealBuildTool.exe;
~~~
..\..\Engine\Binaries\DotNET\UnrealBuildTool.exe %*
~~~

## UBT
通过UnrealBuildTool构建项目需要传递参数
1. GameName
2. PlatformName
3. ConfigurationName
4. ProjectPath
   
~~~
 # Example
UnrealBuildTool.exe ThridPerson420 Win64 Development "C:\Users\visionsmile\Documents\Unreal Projects\Examples\ThridPerson420\ThridPerson420.uproject" -WaitMutex -FromMsBuild
~~~
   
## UHT
