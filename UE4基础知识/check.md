# Assert诊断方式:check()、verify()和ensure()

## check()
check系列类似于std里面的assset，在release版本里check会被定义为空宏，如果碰见false会立刻触发崩溃。
|Macro|Parameters|Behavior|
|-|-|-|
|check或checkSlow|Expression|若Expression为false，停止执行|
|checkf或checkfSlow|Expression、FormattedText|若Expression为false，则停止执行并将FormattedText输出到日志|
|checkCode|Code|在运行以此的do-while循环结构中执行code；主要准备用于另一个check所需的信息|
|checkNoEntry|无|若此行被hit，则停止行行，类似于check(false),主要用于不可到达的代码路径|
|checkNoReentry|无|若此行被hit超过一次，则停止执行|
|checkNoRecursion|无|若此行被hit超过一次而且未离开作用域，则停止执行|
|unimplemented|无|若此行被hit。则停止执行，类似于check(false),但主要用于应被覆盖而不会被调用的虚函数|
## verify()
若某个函数执行操作，然后返回bool来说明该操作是否成功，则应使用verify而非check来确保操作成功(发布版本中verify将忽略返回值)
|Macro|Parameters|Behavior|
|-|-|-|
|verify或verifySlow|Expression|若Expression为false，停止执行|
|verify或verifyfSlow|Expression、FormattedText|若Expression为false，则停止执行并将FormattedText输出到日志|
## ensure()
若Ensure宏的表达式计算得到的值为false，引擎将通知崩溃报告器，但仍将继续运行(仅报告一次)
|Macro|Parameters|Behavior|
|-|-|-|
|ensure|Expression|Expression首次为false时通知崩溃报告器|
|ensureMsgf|Expression、FOrmattedText|Expression首次为false时通知崩溃报告器并将FoemattedText输出到日志|
|ensureAlways|Expression|Expression为false时通知奔溃报告器|
|ensureAlwaysMsgf|Expression、FOrmattedText|Expression首次为false时通知崩溃报告器并将FoemattedText输出到日志|
## 示例
~~~c++
// 决不可使用空JumpTarget调用此函数。若发生此情况，须停止程序。
void AMyActor::CalculateJumpVelocity(AActor* JumpTarget, FVector& JumpVelocity)
{
    check(JumpTarget != nullptr);
    //（计算在JumpTarget上着陆所需的速度。现在可确定JumpTarget为非空。）
}
~~~
~~~c++
// 这将设置Mesh的值，并预计为非空值。若之后Mesh的值为空，则停止程序。
// 使用Verify而非Check，因为表达式存在副作用（设置网格体）。
verify((Mesh = GetRenderMesh()) != nullptr);
~~~
~~~c++
// 这行代码捕获了在产品发布版本中可能出现的小错误。
// 此错误较小，无需停止执行便可解决。
// 虽然该bug已修复，但开发者仍然希望了解之前是否曾经出现过此bug。
void AMyActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // 确保bWasInitialized为true，然后再继续。若为false，则在日志中记录该bug尚未修复。
    if (ensureMsgf(bWasInitialized, TEXT("%s ran Tick() with bWasInitialized == false"), *GetActorLabel()))
    {
        //（执行一些需要已正确初始化AMyActor的操作。)
    }
}
~~~
## 编译flags
|宏|作用|
|-|-|
|DO_GUARD_SLOW|编译checkSlow、checkfSlow和verifySLow|
|DO_CHECK|编译check族和verify族函数|
|DO_ENSURE|编译ensure族函数|
其中
* Debug模式
  Debug模式下所有的断言默认开启
  ~~~c++
    #define DO_GUARD_SLOW 1 
    #define DO_CHECK 1
    #define DO_ENSURE 1
  ~~~

* Development模式
  Development下，SLOW的被禁用。
* shipping和Test模式
  Test模式下和SHIPPING Without editor下差不多，在shipping模式下分两种，如果是带editor的情况下说明在调试打包，此时
  ~~~c++
    #define DO_GUARD_SLOW 0
    #define DO_CHECK 1
    #define DO_ENSURE 1
  ~~~
  如果没有editor则DO_CHECK和DO_ENSURE的值与USE_CHECKS_IN_SHIPPING和USE_ENSURES_IN_SHIPPING有关
  ~~~c++
    #define DO_GUARD_SLOW 0
    #define DO_CHECK USE_CHECKS_IN_SHIPPING
    #define DO_ENSURE USE_ENSURES_IN_SHIPPING
  ~~~
  USE_CHECKS_IN_SHIPPING和USE_ENSURES_IN_SHIPPING这两个值都有默认值，默认情况下是禁用的。
  ~~~c++
    #define USE_CHECKS_IN_SHIPPING 0
    #define USE_ENSURESIN_SHIPPING USE_CHECKS_IN_SHIPPING
  ~~~
  也就是说在发行版本下面，默认check,verify,ensure都是禁用的。
* 