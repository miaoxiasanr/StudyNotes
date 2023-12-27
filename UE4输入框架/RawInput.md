# RawInput
## 基本概念
1. Action Mapping
   这个动作映射是指瞬间动作完成后所有执行的事件，比如按下键盘空格键就执行玩家角色跳跃函数，松开空格键就执行玩家角色停止跳跃函数。
2. Axis Mapping
   Axis Mapping是指连续动作完成后所要执行的时间，比如按下键盘W键玩家角色向前移动，按住键盘S键玩家角色向后移动。
~~~c++
void ATemplateProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATemplateProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATemplateProjectCharacter::MoveRight);
}
~~~

## FWindowApplication
FWindowApplication主要工作是:将操作系统的事件转化成UE事件，构造UE识别的参数类型，并派发给对应处理对象。
* 处理流程
![](https://pic4.zhimg.com/80/v2-28e78353f6a946f5dd856efcf28f4407_720w.webp)
~~~c++
//FWindowsApplication
// ProcessMessage的主要作用是根据msg类型 决定处理方式或者调用DeferMessage();
int32 FWindowsApplication::ProcessMessage( HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam )
{
    .....
        //如果想要自己在UE里面处理操作系统传过来的时间之前，先处理，可以自己传递一个MessageHandler
		// give others a chance to handle messages
		for (IWindowsMessageHandler* Handler : MessageHandlers)
		{
			int32 HandlerResult = 0;
			if (Handler->ProcessMessage(hwnd, msg, wParam, lParam, HandlerResult))
			{
				if (!bMessageExternallyHandled)
				{
					bMessageExternallyHandled = true;
					ExternalMessageHandlerResult = HandlerResult;
				}
			}
		}
    ....
}
~~~

~~~c++
void FWindowsApplication::DeferMessage( TSharedPtr<FWindowsWindow>& NativeWindow, HWND InHWnd, uint32 InMessage, WPARAM InWParam, LPARAM InLParam, int32 MouseX, int32 MouseY, uint32 RawInputFlags )
{
    //只有同时满足GPumpingMessagesOutsideOfMainLoop和bAllowedToDeferMessageProcessing时，才会添加到队列延迟处理。否则立刻调用ProcessDeferedMessage处理事件。
	if( GPumpingMessagesOutsideOfMainLoop && bAllowedToDeferMessageProcessing )
	{
		DeferredMessages.Add( FDeferredWindowsMessage( NativeWindow, InHWnd, InMessage, InWParam, InLParam, MouseX, MouseY, RawInputFlags ) );
	}
	else
	{
		// When not deferring messages, process them immediately
		ProcessDeferredMessage( FDeferredWindowsMessage( NativeWindow, InHWnd, InMessage, InWParam, InLParam, MouseX, MouseY, RawInputFlags ) );
	}
}
~~~
~~~c++
int32 FWindowsApplication::ProcessDeferredMessage( const FDeferredWindowsMessage& DeferredMessage )
{
    //如果想要禁用用户输入，可以传递一个新的MessageHandler，并将其ShouldProcessUserInputMessages函数返回false；
		// This effectively disables a window without actually disabling it natively with the OS.
		// This allows us to continue receiving messages for it.
		if ( !MessageHandler->ShouldProcessUserInputMessages( CurrentNativeEventWindowPtr ) && IsInputMessage( msg ) )
		{
			if (IsKeyboardInputMessage(msg))
			{
				// Force an update since we may have just consumed a modifier key state change
				UpdateAllModifierKeyStates();
			}
			return 0;	// consume input messages
		}

        //当非字符按键（如 shift）时，会触发 OnKeyDown；
        //当输入字符类型（如 A）时，会调用两次，先触发 OnKeyDown ，再触发 OnKeyChar；
}
~~~

## PlayerController
1. PlayerController的主要作用：
   1. InputComponent栈的管理
   2. 转发给PlayerInput完成输入事件响应，按键查询
   3. SetupInputComponent

### 管理InputComponent栈
1. Actor在EnableInput时创建InputComponent，并注册到PlayerController的CurrentInputStack。而DisableInput时将其从PlayerController的CurrentInputStack中移除。
~~~c++
void AActor::EnableInput(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		// If it doesn't exist create it and bind delegates
		if (!InputComponent)
		{
			InputComponent = NewObject<UInputComponent>(this, UInputSettings::GetDefaultInputComponentClass());
			InputComponent->RegisterComponent();
			InputComponent->bBlockInput = bBlockInput;
			InputComponent->Priority = InputPriority;

			UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent);
		}
		else
		{
			// Make sure we only have one instance of the InputComponent on the stack
			PlayerController->PopInputComponent(InputComponent);
		}

		PlayerController->PushInputComponent(InputComponent);
	}
}
~~~

~~~c++
void AActor::DisableInput(APlayerController* PlayerController)
{
	if (InputComponent)
	{
		if (PlayerController)
		{
			PlayerController->PopInputComponent(InputComponent);
		}
		else
		{
			for (FConstPlayerControllerIterator PCIt = GetWorld()->GetPlayerControllerIterator(); PCIt; ++PCIt)
			{
				if (APlayerController* PC = PCIt->Get())
				{
					PC->PopInputComponent(InputComponent);
				}
			}
		}
	}
}
~~~

2. 构建真正的栈:BuildInputStack,该函数在Tick里调用，栈构建完成之后直接传给PlayerInput。
>> 真正完整的栈在此处构建，其栈的顺序从栈低到栈顶依次为：Controlled->Level->PlayerController->CurrentInputStack,因此实际接收输入的顺序为Actor/UserWidget->Controller->Level->Pawn;
~~~c++
void APlayerController::BuildInputStack(TArray<UInputComponent*>& InputStack)
{
	// Controlled pawn gets last dibs on the input stack
	APawn* ControlledPawn = GetPawnOrSpectator();
	if (ControlledPawn)
	{
		if (ControlledPawn->InputEnabled())
		{
			// Get the explicit input component that is created upon Pawn possession. This one gets last dibs.
			if (ControlledPawn->InputComponent)
			{
				InputStack.Push(ControlledPawn->InputComponent);
			}

			// See if there is another InputComponent that was added to the Pawn's components array (possibly by script).
			for (UActorComponent* ActorComponent : ControlledPawn->GetComponents())
			{
				UInputComponent* PawnInputComponent = Cast<UInputComponent>(ActorComponent);
				if (PawnInputComponent && PawnInputComponent != ControlledPawn->InputComponent)
				{
					InputStack.Push(PawnInputComponent);
				}
			}
		}
	}

	// LevelScriptActors are put on the stack next
	for (ULevel* Level : GetWorld()->GetLevels())
	{
		ALevelScriptActor* ScriptActor = Level->GetLevelScriptActor();
		if (ScriptActor)
		{
			if (ScriptActor->InputEnabled() && ScriptActor->InputComponent)
			{
				InputStack.Push(ScriptActor->InputComponent);
			}
		}
	}

	if (InputEnabled())
	{
		InputStack.Push(InputComponent);
	}

	// Components pushed on to the stack get priority
	for (int32 Idx=0; Idx<CurrentInputStack.Num(); ++Idx)
	{
		UInputComponent* IC = CurrentInputStack[Idx].Get();
		if (IC)
		{
			InputStack.Push(IC);
		}
		else
		{
			CurrentInputStack.RemoveAt(Idx--);
		}
	}
}
~~~

3. Actor入栈操作：PushInputComponent
    将InputComponent加入CurrentInputStack,如果是重复加入该Component,会先将之前的移除，栈的顺序已Priority排列，即最低优先级在栈底，高优先级在栈顶。
~~~c++

void APlayerController::PushInputComponent(UInputComponent* InInputComponent)
{
	if (InInputComponent)
	{
		bool bPushed = false;
		CurrentInputStack.RemoveSingle(InInputComponent);
		for (int32 Index = CurrentInputStack.Num() - 1; Index >= 0; --Index)
		{
			UInputComponent* IC = CurrentInputStack[Index].Get();
			if (IC == nullptr)
			{
				CurrentInputStack.RemoveAt(Index);
			}
			else if (IC->Priority <= InInputComponent->Priority)
			{
				CurrentInputStack.Insert(InInputComponent, Index + 1);
				bPushed = true;
				break;
			}
		}
		if (!bPushed)
		{
			CurrentInputStack.Insert(InInputComponent, 0);
		}
	}
}
~~~
### 交由PlayerInput处理的中转站
1. 负责创建PlayerInput,最终所有操作都被转发到PlayerInput里进行。
~~~c++
void APlayerController::InitInputSystem()
{
	if (PlayerInput == NULL)
	{
		PlayerInput = NewObject<UPlayerInput>(this, UInputSettings::GetDefaultPlayerInputClass());
	}
}
~~~
2. 输入事件中转，接收来自GameViewportClient的事件，并交由PlayerInput处理，如InputKey,InputAxis等
#### InputKey
该函数由GameViewportClient调用，注意此Key不仅包括案件类型，也包括鼠标点击事件。
~~~c++
bool APlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
    //首先判断是否交由XR处理，处理过则不再传给PlayerInput。
    if (GEngine->XRSystem.IsValid())
	{
		auto XRInput = GEngine->XRSystem->GetXRInput();
		if (XRInput && XRInput->HandleInputKey(PlayerInput, Key, EventType, AmountDepressed, bGamepad))
		{
			return true;
		}
	}
    //交由PlayerInput处理
    bool bResult = false;
	if (PlayerInput)
	{
		bResult = PlayerInput->InputKey(Key, EventType, AmountDepressed, bGamepad);
		if (bEnableClickEvents && (ClickEventKeys.Contains(Key) || ClickEventKeys.Contains(EKeys::AnyKey)))
		{
            //PlayerInput处理后，会判断是否需要产生鼠标放在某个Actor/Component上时的事件，如需要，则会判断并通知给相应的Component按键和事件类型。
			FVector2D MousePosition;
			UGameViewportClient* ViewportClient = CastChecked<ULocalPlayer>(Player)->ViewportClient;
			if (ViewportClient && ViewportClient->GetMousePosition(MousePosition))
			{
				UPrimitiveComponent* ClickedPrimitive = NULL;
				if (bEnableMouseOverEvents)
				{
					ClickedPrimitive = CurrentClickablePrimitive.Get();
				}
				else
				{
					FHitResult HitResult;
					const bool bHit = GetHitResultAtScreenPosition(MousePosition, CurrentClickTraceChannel, true, HitResult);
					if (bHit)
					{
						ClickedPrimitive = HitResult.Component.Get();
					}
				}
            }
        }
    }
}
~~~
#### InputAxis
~~~c++
bool APlayerController::InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bool bResult = false;
	
	if (PlayerInput)
	{
		bResult = PlayerInput->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
	}

	return bResult;
}
~~~

3. Tick里交给PlayerInput处理Input栈
~~~c++
void APlayerController::ProcessPlayerInput(const float DeltaTime, const bool bGamePaused)
{
	static TArray<UInputComponent*> InputStack;

	// must be called non-recursively and on the game thread
	check(IsInGameThread() && !InputStack.Num());

	// process all input components in the stack, top down
	{
		SCOPE_CYCLE_COUNTER(STAT_PC_BuildInputStack);
		BuildInputStack(InputStack);
	}

	// process the desired components
	{
		SCOPE_CYCLE_COUNTER(STAT_PC_ProcessInputStack);
		PlayerInput->ProcessInputStack(InputStack, DeltaTime, bGamePaused);
	}

	InputStack.Reset();
}
~~~

### SetupInputComponent
//生成InputComponent，并绑定通过蓝图调用的事件。
~~~c++
void APlayerController::SetupInputComponent()
{
	// A subclass could create a different InputComponent class but still want the default bindings
	if (InputComponent == NULL)
	{
		InputComponent = NewObject<UInputComponent>(this, UInputSettings::GetDefaultInputComponentClass(), TEXT("PC_InputComponent0"));
		InputComponent->RegisterComponent();
	}

	if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
	{
		InputComponent->bBlockInput = bBlockInput;
		UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent);//绑定，
	}
}
~~~


## InputComponent
InputComponent完成行为和输入绑定的功能(将ActionName/AxisName/Key等与函数绑定)。
被拥有输入功能的类(如PlayerController,Actor,UserWidget)创建，创建完成之后交给Controller的输入栈整合，并在Tick里交给PlayerInput处理。


1. InputComponent拥有各种绑定的数组，并对其的维护(增删查改)。
~~~c++
class ENGINE_API UInputComponent
	: public UActorComponent
{
	GENERATED_UCLASS_BODY()

	/** The collection of key bindings. */
	TArray<FInputKeyBinding> KeyBindings;

	/** The collection of touch bindings. */
	TArray<FInputTouchBinding> TouchBindings;

	/** The collection of axis bindings. */
	TArray<FInputAxisBinding> AxisBindings;

	/** The collection of axis key bindings. */
	TArray<FInputAxisKeyBinding> AxisKeyBindings;

	/** The collection of vector axis bindings. */
	TArray<FInputVectorAxisBinding> VectorAxisBindings;

	/** The collection of gesture bindings. */
	TArray<FInputGestureBinding> GestureBindings;
}
~~~
添加绑定，除了添加到数组里保存下来，主要是中间的那个部分，目的是判断新ActionBinding是否能够Paired(即该ActionName下，同时拥有Pressed和Released)。做法是：去之前的ActionBindingArray里找，如果名字相等，且之前的是Released，新加的是Pressed，或者相反，则为Paired，或者之前同名的已经是Paired，则新的也是Paired。
~~~c++
FInputActionBinding& UInputComponent::AddActionBinding( FInputActionBinding InBinding )
{
	ActionBindings.Add(MakeShared<FInputActionBinding>(MoveTemp(InBinding)));
	FInputActionBinding& Binding = *ActionBindings.Last().Get();
	Binding.GenerateNewHandle();

	if (Binding.KeyEvent == IE_Pressed || Binding.KeyEvent == IE_Released)
	{
		const EInputEvent PairedEvent = (Binding.KeyEvent == IE_Pressed ? IE_Released : IE_Pressed);
		for (int32 BindingIndex = ActionBindings.Num() - 2; BindingIndex >= 0; --BindingIndex )
		{
			FInputActionBinding& ActionBinding = *ActionBindings[BindingIndex].Get();
			if (ActionBinding.ActionName == Binding.ActionName)
			{
				// If we find a matching event that is already paired we know this is paired so mark it off and we're done
				if (ActionBinding.bPaired)
				{
					Binding.bPaired = true;
					break;
				}
				// Otherwise if this is a pair to the new one mark them both as paired
				// Don't break as there could be two bound paired events
				else if (ActionBinding.KeyEvent == PairedEvent)
				{
					ActionBinding.bPaired = true;
					Binding.bPaired = true;
				}
			}
		}
	}

	for (FCachedKeyToActionInfo& CachedInfo : CachedKeyToActionInfo)
	{
		CachedInfo.KeyMapBuiltForIndex = 0;
	}

	return Binding;
}
~~~
除此之外，UE还提供了删除RemoveActionBinding(除了正常的从数组移除外，多了一个把与之对应Paired的绑定设为False)。

> InputComponent 还有两个变量：Priority ，即优先级，优先级高的在栈尾部，传递时会优先触发。bBlockInput，为 true 时会阻挡栈的后续事件触发。


## PlayerInput
作用：PlayerInput将InputSetting和EngineInputSetting里的<行为名，按键>,与InputComponent里的<行为名，委托>结合，完成最终调用。
### 创建
由PlayerController创建，并且可以更换，在InputSetting里设置即可
~~~c++
	if (PlayerInput == NULL)
	{
		PlayerInput = NewObject<UPlayerInput>(this, UInputSettings::GetDefaultPlayerInputClass());
	}
~~~

### FInputActionKeyMapping
FInputActionKeyMapping记录的是<ActionName,Key>.
~~~c++
//FInputActionKeyMapping
USTRUCT( BlueprintType )
struct FInputActionKeyMapping
{
	GENERATED_USTRUCT_BODY()

	/** Friendly name of action, e.g "jump" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	FName ActionName;

	/** true if one of the Shift keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	uint8 bShift:1;

	/** true if one of the Ctrl keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	uint8 bCtrl:1;

	/** true if one of the Alt keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	uint8 bAlt:1;

	/** true if one of the Cmd keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	uint8 bCmd:1;

	/** Key to bind it to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	FKey Key;
}

//FInputAxisKeyMapping
USTRUCT( BlueprintType )
struct FInputAxisKeyMapping
{
	GENERATED_USTRUCT_BODY()

	/** Friendly name of axis, e.g "MoveForward" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	FName AxisName;

	/** Multiplier to use for the mapping when accumulating the axis value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	float Scale;

	/** Key to bind it to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	FKey Key;
}
~~~
与之对应的是设置
![](https://pic4.zhimg.com/80/v2-8ed65d1c074f9aa89895d97004a5040b_720w.webp)

### ActionMapping/AxisMapping
属于FInputActionKeyMapping和FInputAxisKeyMapping类型数组，其添加方法有AddActionMapping(const FInputActionKeyMapping& KeyMapping)；
目前两个数组赋值的方法是直接从InputSetting里获取。
~~~c++
void UPlayerInput::ForceRebuildingKeyMaps(const bool bRestoreDefaults)
{
	if (bRestoreDefaults)
	{
		const UInputSettings* InputSettings = GetDefault<UInputSettings>();
		if (InputSettings)
		{
			AxisConfig = InputSettings->AxisConfig;
			AxisMappings = InputSettings->GetAxisMappings();
			ActionMappings = InputSettings->GetActionMappings();
		}
	}
}
~~~

### ActionKeyMap/AxisKeyMap
以ActionKeyMap为例
ActionKeyMap类型为TMap<FName,FActionKeyDetails>,FActionKeyDetails可以理解为FInputActionKeyMapping的数组，对应多个绑定的情况。

将我们自己在ProjectSetting/Input下添加的绑定(即ActionMappings)和引擎默认定义的按键绑定添加到Map里。
~~~c++
void UPlayerInput::ConditionalBuildKeyMappings_Internal() const
{
	if (ActionKeyMap.Num() == 0)
	{
		struct
		{
			void Build(const TArray<FInputActionKeyMapping>& Mappings, TMap<FName, FActionKeyDetails>& KeyMap)
			{
				for (const FInputActionKeyMapping& ActionMapping : Mappings)
				{
					TArray<FInputActionKeyMapping>& KeyMappings = KeyMap.FindOrAdd(ActionMapping.ActionName).Actions;
					KeyMappings.AddUnique(ActionMapping);
				}
			}
		} ActionMappingsUtility;

		ActionMappingsUtility.Build(ActionMappings, ActionKeyMap);
		ActionMappingsUtility.Build(EngineDefinedActionMappings, ActionKeyMap);

		KeyMapBuildIndex++;
	}

	if (AxisKeyMap.Num() == 0)
	{
		struct
		{
			void Build(const TArray<FInputAxisKeyMapping>& Mappings, TMap<FName, FAxisKeyDetails>& AxisMap)
			{
				for (const FInputAxisKeyMapping& AxisMapping : Mappings)
				{
					bool bAdd = true;
					FAxisKeyDetails& KeyDetails = AxisMap.FindOrAdd(AxisMapping.AxisName);
					for (const FInputAxisKeyMapping& KeyMapping : KeyDetails.KeyMappings)
					{
						if (KeyMapping.Key == AxisMapping.Key)
						{
							UE_LOG(LogInput, Error, TEXT("Duplicate mapping of key %s for axis %s"), *KeyMapping.Key.ToString(), *AxisMapping.AxisName.ToString());
							bAdd = false;
							break;
						}
					}
					if (bAdd)
					{
						KeyDetails.KeyMappings.Add(AxisMapping);
					}
				}
			}
		} AxisMappingsUtility;

		AxisMappingsUtility.Build(AxisMappings, AxisKeyMap);
		AxisMappingsUtility.Build(EngineDefinedAxisMappings, AxisKeyMap);

		// Apply the axis inversions
		for (int32 InvertedAxisIndex = 0; InvertedAxisIndex < InvertedAxis.Num(); ++InvertedAxisIndex)
		{
			FAxisKeyDetails* KeyDetails = AxisKeyMap.Find(InvertedAxis[InvertedAxisIndex]);
			if (KeyDetails)
			{
				KeyDetails->bInverted = true;
			}
		}
	}

	bKeyMapsBuilt = true;
}
~~~

### KeyStateMap
KeyStateMap类型是TMap<FKey,FKeyState>,关于FKeyState,主要记录了Key的Value，bDown，bConsumed的属性，其中EventCount记录了某种事件(bPressed,Doubclick等)的个数，可以用来检测某个事件是否发生。
~~~c++
TMap<FKey,FKeyState> KeyStateMap;

struct FKeyState
{
	/** This is the most recent raw value reported by the device.  For digital buttons, 0 or 1.  For analog buttons, 0->1.  For axes, -1->1. The X field is for non-vector keys */
	FVector RawValue;

	/** The final "value" for this control, after any optional processing. */
	FVector Value;

	/** Global time of last up->down or down->up transition. */
	float LastUpDownTransitionTime;

	/** True if this key is "down", false otherwise. */
	uint8 bDown:1;

	/** Queued state information.  This data is updated or flushed once player input is processed. */
	uint8 bDownPrevious:1;

	/** True if this key has been "consumed" by an InputComponent and should be ignored for further components during this update. */
	uint8 bConsumed:1;

	/** Flag paired axes that have been sampled this tick. X = LSB, Z = MSB */
	uint8 PairSampledAxes : 3;

	/** How many samples contributed to RawValueAccumulator. Used for smoothing operations, e.g. mouse */
	uint8 SampleCountAccumulator;

	/** How many of each event type had been received when input was last processed. */
	TArray<uint32> EventCounts[IE_MAX];

	/** Used to accumulate events during the frame and flushed when processed. */
	TArray<uint32> EventAccumulator[IE_MAX];

	/** Used to accumulate input values during the frame and flushed after processing. */
	FVector RawValueAccumulator;
}
~~~

该Map在InputKey/InputAxis/InputTouch等输入时便会添加，即设备所有的按键输入状态都会被记录在KeyStateMap里。
~~~c++
bool UPlayerInput::InputKey(FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	// first event associated with this key, add it to the map
	FKeyState& KeyState = KeyStateMap.FindOrAdd(Key);
	UWorld* World = GetWorld();
	check(World);
}
~~~

### Input类型函数
当输入事件触发时，如键盘事件，由GameViewportClient传给PlayerController，再传给PlayerInput，以InputKey为例
~~~c++
bool UPlayerInput::InputKey(FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	// first event associated with this key, add it to the map
	FKeyState& KeyState = KeyStateMap.FindOrAdd(Key);
	UWorld* World = GetWorld();
	check(World);

	switch(Event)
	{
	case IE_Pressed:
	case IE_Repeat:
		KeyState.RawValueAccumulator.X = AmountDepressed;
		KeyState.EventAccumulator[Event].Add(++EventCount);
		if (KeyState.bDownPrevious == false)
		{
			// check for doubleclick
			// note, a tripleclick will currently count as a 2nd double click.
			const float WorldRealTimeSeconds = World->GetRealTimeSeconds();
			if ((WorldRealTimeSeconds - KeyState.LastUpDownTransitionTime) < GetDefault<UInputSettings>()->DoubleClickTime)
			{
				KeyState.EventAccumulator[IE_DoubleClick].Add(++EventCount);
			}

			// just went down
			KeyState.LastUpDownTransitionTime = WorldRealTimeSeconds;
		}
		break;
	case IE_Released:
		KeyState.RawValueAccumulator.X = 0.f;
		KeyState.EventAccumulator[IE_Released].Add(++EventCount);
		break;
	case IE_DoubleClick:
		KeyState.RawValueAccumulator.X = AmountDepressed;
		KeyState.EventAccumulator[IE_Pressed].Add(++EventCount);
		KeyState.EventAccumulator[IE_DoubleClick].Add(++EventCount);
		break;
	}
	KeyState.SampleCountAccumulator++;

#if !UE_BUILD_SHIPPING
	CurrentEvent		= Event;

	const FString Command = GetBind(Key);
	if(Command.Len())
	{
		return ExecInputCommands(World, *Command,*GLog);
	}
#endif

	if( Event == IE_Pressed )
	{
		return IsKeyHandledByAction( Key );
	}

	return true;
}
~~~
InputKey在往KeyStateMap添加或者找到该成员后，还更新了该成员的变量值，一个是相同事件类型的累加(EventAccumulator),一个是更新RawValueAccumulator的值，其他值都是1，Released类型为0，另外，在Pressed/Repeat(重复)类型时，会对是否双击进行检测，也就是双击可以由WindowApplication那边传过来，也可以在InputKey里自己识别到。双击事件的事件可以在UInputSetttings里进行设置。

### ProcessInputStack