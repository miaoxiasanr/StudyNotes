## 移动组件的意义
移动组件看起来是一个和移动相关的组件，但其涉及到状态机，同步解决方案，物理模块，不同移动状态的细节处理，动画以及其他组件(Actor)之间的调用关系等内容。

## 移动实现的基本原理
###移动组件与玩家角色
角色的移动本质上就是合理的改变坐标位置，在UE里面角色移动的本质就是修改某个特定组件的坐标位置。例如在一个Character里
![](https://pic2.zhimg.com/80/v2-23bcb4bfb6e5e094519aa1aef840e5a9_720w.webp)
通常将CapsuleComponent组件作为自己的根组件，而Character的坐标实际上就是其RootComponent的坐标，Mesh等其他组件会跟随胶囊体移动，移动组件在初始化的时候会把胶囊体设置为移动基础组件UpdateComponent，随后的操作都是在计算UpdateComponent的位置。

### 移动组件子类
![](https://pic2.zhimg.com/80/v2-fc2ec78726823ebb61f7fa03e96cc335_720w.webp)
#### UMovementComponent
作为移动组件的基类实现了基本的移动接口SafeMovementUpdatedComponent(),可以调用UpdateComponent组件的接口函数来更新位置信息
~~~C++
bool UMovementComponent::MoveUpdatedComponentImpl( const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit, ETeleportType Teleport)
{
	if (UpdatedComponent)
	{
		const FVector NewDelta = ConstrainDirectionToPlane(Delta);
		return UpdatedComponent->MoveComponent(NewDelta, NewRotation, bSweep, OutHit, MoveComponentFlags, Teleport);
	}

	return false;
}
~~~
UpdateComponent本质上是一个SceneComponent，SceneComponent提供了基本的位置信息——ComponentToWorld(世界位置),同时还提供了该本自身以及子组件的位置接口InternalSetWorldLocationAndRotation()。
~~~c++
bool USceneComponent::InternalSetWorldLocationAndRotation(FVector NewLocation, const FQuat& RotationQuat, bool bNoPhysics, ETeleportType Teleport)
~~~
而PrimativeComponent又继承于USceneComponent，增加了渲染以及物理方面的信息。我们常见的Mesh组件以及胶囊体组件都是继承自UP
rimitiveComponent，因为想要实现一个真实的移动效果。
#### UNavMovementComponent
该组件更多的是提供给AI寻路的能力，同时包括基本的移动状态，比如是否能游泳，是否能飞行。

#### UPawnMovementComponent
UPawnMovementComponent提供了AddInputVector(),可以实现接收玩家的输入并根据输入值修改Pawn的位置。
#### UCharacterMovementComponent
里面非常精确的处理了各种常见的移动状态细节，实现了比较流畅的同步解决方案。各种位置矫正，平滑处理才达到了目前的移动效果。
#### UProjectileMovementComponent
用来模拟弓箭，子弹等抛物线的运动状态。


在一个普通的三维世界里，最简单的移动就是直接修改角色的坐标。所以我们的角色只要有一个包含坐标信息的组件，就可以通过基本的移动组件完成移动。但是随着游戏世界的复杂程度加深，移动也变得复杂起来，
> 玩家的脚下有地面才能行走，那就需要不停的检测地面碰撞信息(FFindFloorResult),FBasedMovementInfo);
> 玩家想进入水中游泳，那就需要检测水的体积(GetPhysicsVolume(),Overlap事件，同样需要物理)
> 水中的速度与效果与陆地上差别很大，那就把两个状态分开写(PhySwimming,PhyWalking)
> 移动的时候动画动作得匹配上，那就在更新位置的时候更新动画(TickCharacterPose)
> 移动的时候碰到障碍物怎么办，被其他玩家推怎么处理(MoveAlongFloor里有相关处理)
> 游戏内容太少，增加一些寻路NPC，有需要设置导航网格(涉及到FNavAgentPropertics)
> 联机(模拟移动同步FRepMovement,客户端修正ClientUpdatePositionAfterServerUpdate)




## 各个移动状态细节处理
### Walking
行走模式可以说是所有移动模式的基础，也是各个移动模式里面最为复杂的一个。为了模拟出真正世界的移动效果，玩家的脚下必须要有一个可以支撑不会掉落的物理对象(类似地面)，在移动组件里面，这个地面的通过成员变量FFindFloorResult CurrentFloor来记录。在游戏的一开始的时候，移动组件就会通过根据当前配置默认的MovementMode，如果是Walking，就会通过FindFloor操作来找到当前的地面。
#### FindFloor流程
FindFloor本质上就是通过胶囊体Sweep检测来找到脚下的地面，所以地面必须要有物理属性，而且通道类型要设置与玩家的Pawn有Block类型。
Sweep检测用的是胶囊体而不是射线检测，方便斜面处理，计算可站立半径等(HitResult里面的Normal与ImpartNormal在胶囊体Sweep检测时不一定相同，如图所示)
![](https://pic2.zhimg.com/80/v2-650101da3aacc97379e1b87ebf82d20d_720w.webp)
#### Walking的位移计算
~~~c++
void UCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(CharPhysWalking);

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
	
	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.Z = 0.f;

		// Apply acceleration
		if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}
		
		ApplyRootMotionToVelocity(timeTick);
		devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if( IsFalling() )
		{
			// Root motion could have put us into Falling.
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime+timeTick, Iterations-1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
			else if ( IsSwimming() ) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta/timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}	
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

~~~

为了表现的平滑流畅，UE4把一个Tick的移动分成了N段处理(每段时间不能超过MaxSimulationTimeStep).在处理每段时，首先把当前的位置信息，地面信息记录下来，在TickComponent的时候根据玩家的按键时长，计算出当前的加速度。随后在CalcVelocity()根据加速度计算速度，还会考虑地面摩擦，是否在水中等情况。
算出速度之后，调用函数MoveAlongFloor()改变当前对象的坐标位置。在真正调用移动接口SafeMoveUpdatedComponent()前还会简单处理一种特殊情况——玩家沿着斜面走。
> 在正常Walking状态下，玩家只会前后左右移动，不会有Z方向的移动速度

如果遇到斜坡怎么办？如果这个斜坡可以行走，就会调用ComputeGroundMovementDelta()函数去根据当前的水平速度计算出一个新的水平与斜面速度，这样就可以简单模拟一个沿着斜面行走的效果，而且一般来说上坡的时候玩家的水平速度应该减小，通过设置bMaintainHorizontalGroundVelocity为false可以自行处理这种情况

如果遇到障碍怎么处理？根据我们平时游戏经验，遇到障碍肯定移动失败，还可能沿着墙面滑动一点。
UE里面也是这么处理的，在角色移动的过程中(SafeMoveUpdatedComponent),会有一个碰撞检测流程。由于UPrimitiveComponent组件才有物理数据，所以这个操作是在函数UprimitiveComponent::MoveComponentImpl里面处理的
~~~c++
bool UMovementComponent::SafeMoveUpdatedComponent(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult& OutHit, ETeleportType Teleport)
{
	if (UpdatedComponent == NULL)
	{
		OutHit.Reset(1.f);
		return false;
	}

	bool bMoveResult = false;

	// Scope for move flags
	{
		// Conditionally ignore blocking overlaps (based on CVar)
		const EMoveComponentFlags IncludeBlockingOverlapsWithoutEvents = (MOVECOMP_NeverIgnoreBlockingOverlaps | MOVECOMP_DisableBlockingOverlapDispatch);
		TGuardValue<EMoveComponentFlags> ScopedFlagRestore(MoveComponentFlags, MovementComponentCVars::MoveIgnoreFirstBlockingOverlap ? MoveComponentFlags : (MoveComponentFlags | IncludeBlockingOverlapsWithoutEvents));
		bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, &OutHit, Teleport);
	}

	// Handle initial penetrations
	if (OutHit.bStartPenetrating && UpdatedComponent)
	{
		const FVector RequestedAdjustment = GetPenetrationAdjustment(OutHit);
		if (ResolvePenetration(RequestedAdjustment, OutHit, NewRotation))
		{
			// Retry original move
			bMoveResult = MoveUpdatedComponent(Delta, NewRotation, bSweep, &OutHit, Teleport);
		}
	}

	return bMoveResult;
}
~~~

~~~c++
bool UPrimitiveComponent::MoveComponentImpl( const FVector& Delta, const FQuat& NewRotationQuat, bool bSweep, FHitResult* OutHit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
    ///...
    FComponentQueryParams Params(SCENE_QUERY_STAT(MoveComponent), Actor);
	FCollisionResponseParams ResponseParam;
	InitSweepCollisionParams(Params, ResponseParam);
	Params.bIgnoreTouches |= !(GetGenerateOverlapEvents() || bForceGatherOverlaps);
	Params.TraceTag = TraceTagName;
	bool const bHadBlockingHit = MyWorld->ComponentSweepMulti(Hits, this, TraceStart, TraceEnd, InitialRotationQuat, Params);
    ///...
}
~~~
在接收到SafeMoveUpdatedComponent()返回的HitResult后，会在下面的代码中处理碰撞障碍的情况
* 如果沿着Hit.Normal在Z方向上有值而且还可以行走，那说明这是一个可移动上去的斜面，随后让玩家沿着斜面移动
* 判断当前的碰撞体是否可以踩上去，如果可以的话就试着踩上去，如果过程中发现没有踩上去，也会调用SlideAlongSurface()沿着碰撞滑动

移动后还会立刻判断玩家是否进入水中，或者进入Falling状态，如果是的话立刻切换新的状态。
### Falling
Falling状态是处理Walking以外最常见的状态，只要玩家在空中(无论是挑起还是下落)，玩家都会处于Falling状态。
在处理时，首先计算玩家通过输入控制的水平速度，因为玩家在空中也可以受到玩家控制的影响。随后获取重力计算速度。重力的获取是通过Volume体积获取。
~~~c++
float UMovementComponent::GetGravityZ() const
{
	APhysicsVolume* PhysicsVolume = GetPhysicsVolume();
	return PhysicsVolume ? PhysicsVolume->GetGravityZ() : UPhysicsSettings::Get()->DefaultGravityZ;
}
APhysicsVolume* UMovementComponent::GetPhysicsVolume() const
{
	if (UpdatedComponent)
	{
		return UpdatedComponent->GetPhysicsVolume();
	}
	
	return GetWorld()->GetDefaultPhysicsVolume();
}
~~~
通过获取到的Gravity计算出当前新的FallSpeed。
~~~c++
FVector UCharacterMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	FVector Result = InitialVelocity;

	// Apply gravity.
	Result += Gravity * DeltaTime;
~~~
随后计算出位移
~~~c++
FVector Adjusted = 0.5f*(OldVelocity + Velocity) * timeTick; 
SafeMoveUpdatedComponent( Adjusted, PawnRotation, true, Hit);
~~~
计算完速度并移动玩家后，也一样要考虑移动碰撞问题
1. 正常落地
   如果玩家计算后发现碰撞到一个可站立的地形，直接调用ProcessLanded进行落地操作(这个判断是根据碰撞点的高度来的，可以筛选掉墙面)
2. 跳的时候遇到一个平台
   然后检测玩家的坐标与当前碰撞点是否在一个可接受的范围(IsWithinEdgeTolerance),是的话重新执行FindFloor重新检测一遍地面，检测到的话就执行落地流程。
3. 墙面等一些不可以踩上去的物体
   首先会执行HandleImpact给碰到的对象一个力，随后调整ComponentSlideVector计算滑动的位移，由于碰撞到障碍后，玩家的速度会有变化，这时候重新计算一下速度，再次调整玩家的位置和方向。如果玩家这时候有水平方向的位移，还会通过LimitAirControl来限制玩家的速度，

#### Jump
跳跃响应的基本流程
1. 绑定触发响应事件
~~~c++
void APrimalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Set up gameplay key bindings
    check(PlayerInputComponent);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}
void ACharacter::Jump()
{
    bPressedJump = true;
    JumpKeyHoldTime = 0.0f;
}

void ACharacter::StopJumping()
{
    bPressedJump = false;
    ResetJumpState();
}
~~~
2. 一旦按键响应立即设置bPressedJump为True，TickComponent的帧循环调用ACharacter::CheckJumpInput来立即检测到是否执行跳跃操作。
    * 执行CanJump()函数，处理蓝图里面的相关限制逻辑。如果蓝图不重写该函数，就会默认执行ACharacter::CanJumpInternal_Implementation(),这里面是控制玩家是否跳跃的依据，比如蹲伏状态不能跳跃，游泳状态不能跳跃。另外有一个JumpMaxHoldTime表示玩家按键超过这个值后不会触发跳跃。JumpMaxCount表示玩家可以执行跳跃的段数
    * 执行CharacterMovement->DoJump(bClientUpdateing)函数，执行跳跃操作，进行Falling，设置跳跃速度为JumpZVelocity,这个值不能小于0；
    * 判断Const bool bDidJump=CanJump&&CharacterMovement->DoJump(bClientUpdating)&&DoJump是否为真，做一些相关的操作
3. 在一次PerformMovement结束后，就会执行ClearJumpInput,设置bProcessdJump为false。但是不会清除JumpCurrentCount这样继续处理多段跳
4. 玩家松开按键p时也会设置bPressedJump为false。清空相关状态。如果玩家仍在空中，那也不会清除JumpCurrentCount,一旦bPressedJump为false，就不会处理任何跳跃操作了；
5. 如果玩家在空中进入ACharacter::CheckJumpInput，如果JumpCurrentCount小于JumpMaxCount，玩家就可以继续执行跳跃操作了。
### Swimming
游泳状态表现上来看是一个右移动惯性(松手后不会立刻停止)，受重力影响小(在水中慢慢下落或不动)，移动速度比平时慢(表现水有阻力)的状态。而玩家是否在水中的默认检测逻辑比较简单，就是判断当前的UpdateComponent所在的Volume是否为WaterVolume。
CharacterMovement组件里面有浮力大小配置为Buoyancy，根据玩家潜入水中的程度(ImmersionDepth返回0-1)计算最终的浮力。随后要计算速度，需要获取Volume里面的摩擦力Friction，然后传入CalcVelocity里面，这体现出说中移动速度变慢的效果。随后在Z方向通过计算浮力大小计算该方向的速度，随着玩家潜水的程度，会发现玩家在Z方向的速度越来越小，一旦全身浸入了水中，在Z轴方向的重力加速度就会完全忽略。

处理移动中检测到碰撞障碍的情况？
如果可以踩上去(StepUp())就调整玩家位置踩上去，如果踩不上去就给障碍一个力，然后顺着障碍表面滑动一点距离。

水中移动的惯性表现处理？
计算速度时有两个传入的参数与Walking不同，一个是Friction表示摩擦力，另一个是BrakingDeceleration表示刹车的反向速度，在加速度为0的时候(表示玩家的输入已经被清空)，水中的传入的摩擦力要远比地面摩擦力。
### Flying
首先根据前面输入计算Acceleration.然后根据摩擦力开始计算当前的速度，速度计算后调用SafeMoveUpdatedComponent进行移动。如果碰到障碍，就看能不能踩上去，不能的话处理碰撞，沿着障碍表面滑动。
## 移动同步解决方案
* 前面关于移动逻辑的细节处理都是在PerformMovement里面实现的，我们可以把函数PerformMovement当成一个完整的移动处理流程。
* 这个流程无论是在客户端还是在服务器都必须要执行。
* 在网络游戏中，为了让所有的游戏玩家体验一个几乎相同的世界，需要保证一个具有绝对权威的服务器，这个服务器可以修正客户端的不正常移动行为，保证各个客户端的一致性。
* 相关同步的操作都是基于UCharacterMovement组件实现的。
* 移动组件的同步全都是基于RPC不可靠传输的，
### 服务器角色正常的移动流程
对于DedicateServer，它的本地没有控制的角色，对移动的处理分为两种情况
1. 该角色在客户端是模拟角色(Simulate)，移动完全是有服务器同步过去，如各类AI角色。这类移动一般是服务器上行为树主动触发的
2. 该角色咋客户端是拥有自治(Autonomous)权利的Character，如玩家控制的主角，这类移动一般是客户端接收玩家输入数据本地模拟后，在通过RPC发给服务器进行模拟的。
~~~c++
void UCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    	if (CharacterOwner->GetLocalRole() == ROLE_Authority && (!bCheatFlying || bIsSimulatingPhysics) && !CharacterOwner->CheckStillInWorld())
	{
		return;
	}

	// We don't update if simulating physics (eg ragdolls).
	if (bIsSimulatingPhysics)
	{
		// Update camera to ensure client gets updates even when physics move him far away from point where simulation started
		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy && IsNetMode(NM_Client))
		{
			MarkForClientCameraUpdate();
		}

		ClearAccumulatedForces();
		return;
	}

	AvoidanceLockTimer -= DeltaTime;

	if (CharacterOwner->GetLocalRole() > ROLE_SimulatedProxy)
	{
		SCOPE_CYCLE_COUNTER(STAT_CharacterMovementNonSimulated);

		// If we are a client we might have received an update from the server.
		const bool bIsClient = (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy && IsNetMode(NM_Client));
		if (bIsClient)
		{
			FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
			if (ClientData && ClientData->bUpdatePosition)
			{
				ClientUpdatePositionAfterServerUpdate();
			}
		}

		// Allow root motion to move characters that have no controller.
		if (CharacterOwner->IsLocallyControlled() || (!CharacterOwner->Controller && bRunPhysicsWithNoController) || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
		{
			ControlledCharacterMove(InputVector, DeltaTime);
		}
		else if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			// Server ticking for remote client.
			// Between net updates from the client we need to update position if based on another object,
			// otherwise the object will move on intermediate frames and we won't follow it.
			MaybeUpdateBasedMovement(DeltaTime);
			MaybeSaveBaseLocation();

			// Smooth on listen server for local view of remote clients. We may receive updates at a rate different than our own tick rate.
			if (CharacterMovementCVars::NetEnableListenServerSmoothing && !bNetworkSmoothingComplete && IsNetMode(NM_ListenServer))
			{
				SmoothClientPosition(DeltaTime);
			}
		}
	}
	else if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (bShrinkProxyCapsule)
		{
			AdjustProxyCapsuleSize();
		}
		SimulatedTick(DeltaTime);
	}
}
~~~

### Autonomous角色
一个客户端的角色完全通过服务器同步过来的，他身上的移动组件也一样被同步过来的，所以游戏一开始客户端的角色与服务器的数据是完全相同的。对于Autonomous角色，大致的实现思路如下
> 客户端通过接收玩家的Input输入，开始进行本地的移动模拟流程，移动前首先创建一个移动预测数据结构FNetworkPredictionDate_Client_character，执行PerformMovement移动，随后保存当前的移动数据(速度，旋转，时间戳以及移动结束的位置等信息)到前面的FNetworkPredictionDate里面的SavedMoves列表里面，并通过RPC将当前的Move数据发送该数据到服务器。然后继续进行TickComponent操作，重复这个流程。

> 客户端在发送给服务器RPC消息的同时，本地还会不断的执行移动模拟。SaveMoved列表里面的数据也就会越来越多。
> * 如果这时候收到一个ClientAckGoodMove调用，那么表示服务器接受了对应时间戳的客户端移动，客户端就将这个时间戳之前的SavedMoves全部移除。
> * 如果客户端收到了ClientAdjustPosition调用，那么表示对应这个时间戳的移动有问题，客户端需要修改成服务器传过来的位置，并重新播放那些还没有被确认的SaveMoves列表里面的移动。


Simulated角色
Simulate角色在服务器上的移动是正常的PerformMovement流程，而在客户端上，该角色的移动分为两个步骤来处理(本质上是为了减少同步带来的开销)
1. 在收到服务器的同步数据时就直接进行设置
2. 在没有收到服务器消息的时候根据上一次服务器传过来的数据(包括速度和旋转等)在本地执行Simulate模拟，等着下一个同步数据到来，

~~~c++
void ACharacter::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );
    DOREPLIFETIME_CONDITION( ACharacter, RepRootMotion,COND_SimulatedOnlyNoReplay);
    DOREPLIFETIME_CONDITION( ACharacter, ReplicatedBasedMovement,   COND_SimulatedOnly );
    DOREPLIFETIME_CONDITION( ACharacter, ReplicatedServerLastTransformUpdateTimeStamp, COND_SimulatedOnlyNoReplay);

    DOREPLIFETIME_CONDITION( ACharacter, ReplicatedMovementMode,    COND_SimulatedOnly );
    DOREPLIFETIME_CONDITION( ACharacter, bIsCrouched,           COND_SimulatedOnly );

    // Change the condition of the replicated movement property to not replicate in replays since we handle this specifically via saving this out in external replay data
    DOREPLIFETIME_CHANGE_CONDITION(AActor,ReplicatedMovement,COND_SimulatedOrPhysicsNoReplay);


}
void APawn::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		UMovementComponent* const MoveComponent = GetMovementComponent();
		if ( MoveComponent )
		{
			MoveComponent->Velocity = NewVelocity;
		}
	}
}
void ACharacter::PostNetReceiveLocationAndRotation()
{
	if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation())
		{
			const FRepMovement& ConstRepMovement = GetReplicatedMovement();
			const FVector OldLocation = GetActorLocation();
			const FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(ConstRepMovement.Location, this);
			const FQuat OldRotation = GetActorQuat();

			CharacterMovement->bNetworkSmoothingComplete = false;
			CharacterMovement->bJustTeleported |= (OldLocation != NewLocation);
			CharacterMovement->SmoothCorrection(OldLocation, OldRotation, NewLocation, ConstRepMovement.Rotation.Quaternion());
			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
		CharacterMovement->bNetworkUpdateReceived = true;
	}
}

~~~
ReplicatedMovement记录了当前Character的位置旋转、速度等重要的移动数据，这个成员在SImulate或者开启物理模拟的客户端才执行。同时可以看到Character大部分的同步属性都是和移动同步有关，而且基本都是SimulateOnly,表示这些属性只有在模拟客户端时才会进行同步。除了ReplicatedMovement属性以外，ReplicatedMovementMode同步了当前的移动模式，ReplicatedBasedMovement同步了角色所在的Component的相关数据。。
#### 移动流程
![](https://pic3.zhimg.com/80/v2-6048fa8b09eee5c9eb7ffe35918f29be_720w.webp)

[移动组件详解](https://zhuanlan.zhihu.com/p/34257208)