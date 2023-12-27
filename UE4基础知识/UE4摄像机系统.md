# UE摄像机系统

## 摄像机系统的主要功能
在游戏中，摄像机是玩家观察世界的眼睛。
摄像机的位置和朝向分别决定了玩家观察的位置和方向。而摄像机的视野、远近裁面等属性则决定可玩家观察的范围。UE用了POV(Point of View)来指代这些影响玩家观察游戏世界的属性。
> POV原来是一种电影拍摄的模式。指的是镜头相当于叙事主体的眼睛，所有被摄的内容看上去都是以主体人的眼睛所看到的内容。

## UE的摄像机系统框架
![](https://pic1.zhimg.com/80/v2-970c16458accff12462f5a5e6896f994_720w.webp)
从上面的类图可以看出，APlayerCameraManager是整个摄像机系统的核心，其他相关的类负责向APlayerCameraManager提供数据，或者从APlayerCameraManager获取数据。

APlayerCameraManager所管理的最重要的数据是FTViewTarget,他记录了摄像机的跟随对象Target以及摄像机的属性数据POV。

~~~c++
/** A ViewTarget is the primary actor the camera is associated with. */
USTRUCT(BlueprintType)
struct ENGINE_API FTViewTarget
{
	GENERATED_USTRUCT_BODY()
public:

	/** Target Actor used to compute POV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=TViewTarget)
	class AActor* Target;

	/** Computed point of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=TViewTarget)
	struct FMinimalViewInfo POV;

protected:
	/** PlayerState (used to follow same player through pawn transitions, etc., when spectating) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=TViewTarget)
	class APlayerState* PlayerState;

public:
	class APlayerState* GetPlayerState() const { return PlayerState; }
	
	void SetNewTarget(AActor* NewTarget);

	class APawn* GetTargetPawn() const;

	bool Equal(const FTViewTarget& OtherTarget) const;

	FTViewTarget()
		: Target(nullptr)
		, PlayerState(nullptr)
	{}

	/** Make sure ViewTarget is valid */
	void CheckViewTarget(APlayerController* OwningController);
};
~~~

UE的摄像机系统框架的主要流程：
1. 每个APlayerController对象(包括服务器上的)都会在PostInitializeComponents方法里创建APlayerCameraManager对象，并初始化ViewTarget。
2. UWorld在Tick时会调用APlayerController的UpdateCameraManager方法，最终驱动APlayerCameraManager在UpdateCamera方法里不断更新ViewTarget。
3. 更新完游戏逻辑后，在调用UGameViewportClient的Draw方法渲染画面时，会通过ULocalPlayer的CalcSceneView方法，最终从APlayerCameraManager中获取ViewTarget中的POV数据用于渲染。

## ViewTarget的管理
### 初始化ViewTarget
用于渲染画面的POV需要依赖Target才能计算，所以初始化VIewTarget的主要目的是确认初始化Target。
在创建APlayerCameraManager时，APlayerController会先调用APLayerCameraManager的SetViewTarget方法将Target设置为自己。
~~~c++
void APlayerCameraManager::InitializeFor(APlayerController* PC)
{
	FMinimalViewInfo DefaultFOVCache = GetCameraCachePOV();
	DefaultFOVCache.FOV = DefaultFOV;
	SetCameraCachePOV(DefaultFOVCache);
	PCOwner = PC;
	SetViewTarget(PC);
}
~~~

随后在APlayerController创建并在创建自己所控制的Pawn之后，则会在OnPossess方法中通过AutoManageActiveCameraTArget方法将Target设置为APlayerController所控制的Pawn。
> 正因如此，在Play之后，游戏默认展示的画面是角色身上所挂摄像机的渲染内容，而不是其他摄像机。

### 切换Target
APlayerCameraManager允许通过调用SetViewTarget方法来切换Target。如果需要在不同的Target之间平滑切换，只需要在传入新Target的同时传入BlendTime不为0的FViewTargetTransitionParams即可。
> 值得注意的是，如果设置了平滑切换，那么APlayerCameraManager并不会直接更新ViewTarget.Target.而是将其设置到PendingViewTarget.Target中，等待平滑切换结束后再将其设置为ViewTarget.Target.
~~~c++
void APlayerCameraManager::SetViewTarget(class AActor* NewTarget, struct FViewTargetTransitionParams TransitionParams)
{
		// if a transition time is specified, then set pending view target accordingly
		if (TransitionParams.BlendTime > 0)
		{
			// band-aid fix so that EndViewTarget() gets called properly in this case
			if (PendingViewTarget.Target == NULL)
			{
				PendingViewTarget.Target = ViewTarget.Target;
			}

			// use last frame's POV
			ViewTarget.POV = GetLastFrameCameraCachePOV();
			BlendParams = TransitionParams;
			BlendTimeToGo = TransitionParams.BlendTime;

			AssignViewTarget(NewTarget, PendingViewTarget, TransitionParams);
			PendingViewTarget.CheckViewTarget(PCOwner);

			if (bUseClientSideCameraUpdates && GetNetMode() != NM_Client)
			{
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(SwapPendingViewTargetWhenUsingClientSideCameraUpdatesTimerHandle, this, &ThisClass::SwapPendingViewTargetWhenUsingClientSideCameraUpdates, TransitionParams.BlendTime, false);
				}
			}
		}
}
~~~
虽然平滑切换结束前，新Target不会被设置到ViewTarget.Target，但逻辑上APlayerCameraManager认为Target已经切换完成了，此时通过APlayerCameraManager的GetViewTarget方法，将会直接返回PendingVIewTarget.Target.
~~~c++
AActor* APlayerCameraManager::GetViewTarget() const
{
	// to handle verification/caching behavior while preserving constness upstream
	APlayerCameraManager* const NonConstThis = const_cast<APlayerCameraManager*>(this);

	// if blending to another view target, return this one first
	if( PendingViewTarget.Target )
	{
		NonConstThis->PendingViewTarget.CheckViewTarget(NonConstThis->PCOwner);
		if( PendingViewTarget.Target )
		{
			return PendingViewTarget.Target;
		}
	}

	NonConstThis->ViewTarget.CheckViewTarget(NonConstThis->PCOwner);
	return ViewTarget.Target;
}
~~~
另外一个值得注意的是，每次更新Target时，都会调用FTViewTarget的CheckViewTarget()方法来记录PlayerState。这主要有两个目的
1. 校验Target是否合法，只允许时APawn,APlayerController或者是APlayerState.'
2. 确保玩家在切换不同控制的角色时(如观战其他玩家)，可以将Target切换到当前玩家控制的角色身上。

> APawn再被APlayerController调用Possess方法控制时，会调用PossessBy方法将自身的PlayerState设置成Controller的PlayerState。因此不管玩家切换到那个控制的玩家。都可以通过PlayerState快速找到当前控制的APawn。

### 更新POV
APlayerCameraManager在UpdateCamera方法里完成POV的更新。主要流程如下图:
![](https://pic4.zhimg.com/80/v2-164db3cb74236928226630754689c173_720w.webp)

其中，UpdateVIewTarget方法是计算POV数据的主要方法，具体步骤如下
1. 首先判断Target是否为CameraActor，如果是则直接找到挂载的UCameraComponent，并调用UCameraComponent的GetCameraView方法。
> UCameraComponent里并没有渲染逻辑，其主要功能是为APlayerCameraManager提供POV数据。
~~~c++
	if (ACameraActor* CamActor = Cast<ACameraActor>(OutVT.Target))
	{
		// Viewing through a camera actor.
		CamActor->GetCameraComponent()->GetCameraView(DeltaTime, OutVT.POV);
	}
~~~
2. 如果Target不为CameraActor，那么允许通过修改CameraStyle的值执行自定义POV数据的计算逻辑；
> UE默认提供了以下五中，可以根据需要额外新增，并添加相应的POV数据计算逻辑；
~~~c++
		static const FName NAME_Fixed = FName(TEXT("Fixed"));
		static const FName NAME_ThirdPerson = FName(TEXT("ThirdPerson"));
		static const FName NAME_FreeCam = FName(TEXT("FreeCam"));
		static const FName NAME_FreeCam_Default = FName(TEXT("FreeCam_Default"));
		static const FName NAME_FirstPerson = FName(TEXT("FirstPerson"));
~~~

~~~c++
void APlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	......
		if (CameraStyle == NAME_Fixed)
		{

			OutVT.POV = OrigPOV;
			bDoNotApplyModifiers = true;
		}
		else if (CameraStyle == NAME_ThirdPerson || CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default)
		{
			// Simple third person view implementation
			FVector Loc = OutVT.Target->GetActorLocation();
			FRotator Rotator = OutVT.Target->GetActorRotation();

			if (OutVT.Target == PCOwner)
			{
				Loc = PCOwner->GetFocalLocation();
			}
			if( CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default )
			{
				Rotator = PCOwner->GetControlRotation();
			}

			FVector Pos = Loc + ViewTargetOffset + FRotationMatrix(Rotator).TransformVector(FreeCamOffset) - Rotator.Vector() * FreeCamDistance;
			FCollisionQueryParams BoxParams(SCENE_QUERY_STAT(FreeCam), false, this);
			BoxParams.AddIgnoredActor(OutVT.Target);
			FHitResult Result;

			GetWorld()->SweepSingleByChannel(Result, Loc, Pos, FQuat::Identity, ECC_Camera, FCollisionShape::MakeBox(FVector(12.f)), BoxParams);
			OutVT.POV.Location = !Result.bBlockingHit ? Pos : Result.Location;
			OutVT.POV.Rotation = Rotator;
			bDoNotApplyModifiers = true;
		}
		else if (CameraStyle == NAME_FirstPerson)
		{
			// Simple first person, view through viewtarget's 'eyes'
			OutVT.Target->GetActorEyesViewPoint(OutVT.POV.Location, OutVT.POV.Rotation);
	
			// don't apply modifiers when using this debug camera mode
			bDoNotApplyModifiers = true;
		}
		else
		{
			UpdateViewTargetInternal(OutVT, DeltaTime);
		}
	......
}
~~~
3. 如果CameraStyle为默认值NAME_NONE,或者没有与其对应的计算逻辑，将会调用默认的UpdateVIewTargetInternal方法。
~~~c++
void APlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (BlueprintUpdateCamera(OutVT.Target, OutLocation, OutRotation, OutFOV))
		{
			OutVT.POV.Location = OutLocation;
			OutVT.POV.Rotation = OutRotation;
			OutVT.POV.FOV = OutFOV;
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}
~~~
> BlueprintUpdateCamera可以在蓝图中自定义实现的接口，可以直接覆盖相机的表现。
4. 如果没有在蓝图中自定义实现BlueprintUpdateCamera方法，那么将会调用Target的CalcCamera方法；
> 默认情况下，将会遍历所有Attach上的UCameraComponent，然后找到第一个激活的UCameraComponent调用其GetCameraView方法获取POV数据，如果找不到符合条件的UCameraComponent,那么将会直接调用GetActorEyedViewPoint方法提供玩家观察位置的朝向；因此，如果AActor上有多个UCameraComponent，可以直接通过UCameraComponent激活不激活来实现切换摄像机的功能。
~~~c++
void AActor::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (bFindCameraComponentWhenViewTarget)
	{
		// Look for the first active camera component and use that for the view
		TInlineComponentArray<UCameraComponent*> Cameras;
		GetComponents<UCameraComponent>(/*out*/ Cameras);

		for (UCameraComponent* CameraComponent : Cameras)
		{
			if (CameraComponent->IsActive())
			{
				CameraComponent->GetCameraView(DeltaTime, OutResult);
				return;
			}
		}
	}

	GetActorEyesViewPoint(OutResult.Location, OutResult.Rotation);
}
~~~


5. 最后，判断是否需要调用ApplyCameraModifiers方法对POV数据进行修正，得到最终的POV数据。
~~~c++
void APlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	......
		if (!bDoNotApplyModifiers || bAlwaysApplyModifiers)
	{
		// Apply camera modifiers at the end (view shakes for example)
		ApplyCameraModifiers(DeltaTime, OutVT.POV);
	}
	.......
}
~~~

~~~c++
//APlayerCameraManager允许动态添加、删除UCameraModifier,且多个UCameraModifier可以同时生效。
void APlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	ClearCachedPPBlends();
	// Loop through each camera modifier
	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ++ModifierIdx)
	{
		// Apply camera modification and output into DesiredCameraOffset/DesiredCameraRotation
		if ((ModifierList[ModifierIdx] != NULL) && !ModifierList[ModifierIdx]->IsDisabled())
		{
			// If ModifyCamera returns true, exit loop
			// Allows high priority things to dictate if they are
			// the last modifier to be applied
			if (ModifierList[ModifierIdx]->ModifyCamera(DeltaTime, InOutPOV))
			{
				break;
			}
		}
	}
}
~~~
[UnrealEngine摄像机系统简析](https://zhuanlan.zhihu.com/p/605468663)



