// Fill out your copyright notice in the Description page of Project Settings.


#include "TARCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "DrawDebugHelpers.h"

// Sets default values
ATARCharacter::ATARCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void ATARCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATARCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotation Visualization
	const float DrawScale = 100.f;
	const float Thickness = 5.f;

	FVector LineStart = GetActorLocation();
	// offset to the right of the pawn
	LineStart += GetActorRightVector() * 100;
	// set line end in direction of actors forward
	FVector ActorDirection_LineEnd = LineStart + (GetActorForwardVector() * 100.f);
	// draw actor's direction
	DrawDebugDirectionalArrow(GetWorld(), LineStart, ActorDirection_LineEnd, DrawScale, FColor::Yellow, false, 0.0f, 0, Thickness);

	FVector ControllerDirection_LineEnd = LineStart + (GetControlRotation().Vector() * 100.f);
	// draw controller rotation that possessed this character
	DrawDebugDirectionalArrow(GetWorld(), LineStart, ControllerDirection_LineEnd, DrawScale, FColor::Green, false, 0.0f, 0, Thickness);
}

// Called to bind functionality to input
void ATARCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP = PC->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	// complex games may have more 
	Subsystem->AddMappingContext(DefaultInputMapping, 0);

	UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	// general input
	InputComp->BindAction(Input_Move, ETriggerEvent::Triggered, this, &ATARCharacter::Move);
	InputComp->BindAction(Input_MagicProjectile, ETriggerEvent::Triggered, this, &ATARCharacter::CastMagicProjectile);
	InputComp->BindAction(Input_Jump, ETriggerEvent::Triggered, this, &ATARCharacter::Jump);

	// m+k
	InputComp->BindAction(Input_LookMouse, ETriggerEvent::Triggered, this, &ATARCharacter::LookMouse);
	// gamepad
	InputComp->BindAction(Input_LookStick, ETriggerEvent::Triggered, this, &ATARCharacter::LookStick);
}

void ATARCharacter::Move(const FInputActionInstance& Instance)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	// X - Forward - Red
	// Y - Right - Green
	// Z - Up - Blue

	const FVector2D AxisValue = Instance.GetValue().Get<FVector2D>();
	AddMovementInput(ControlRot.Vector(), AxisValue.Y);

	const FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	AddMovementInput(RightVector, AxisValue.X);
}

void ATARCharacter::LookMouse(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void ATARCharacter::LookStick(const FInputActionValue& InputValue)
{
	FVector2D Value = InputValue.Get<FVector2D>();

	// track negative as conversion loses this
	const bool XNegative = Value.X < 0.f;
	const bool YNegative = Value.Y < 0.f;

	// sensitivity
	static const float LookYawRate = 100.f;
	static const float LookPitchRate = 50.f;

	// non-linear to make aiming easier
	Value = Value * Value;
	if (XNegative)
	{
		Value.X *= -1.f;
	}
	if (YNegative)
	{
		Value.Y *= -1.f;
	}

	AddControllerYawInput(Value.X * LookYawRate * GetWorld()->GetDeltaSeconds());
	AddControllerPitchInput(Value.Y * LookPitchRate * GetWorld()->GetDeltaSeconds());
}

void ATARCharacter::CastMagicProjectile()
{
	FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
	FTransform SpawnTM = FTransform(GetControlRotation(), HandLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(MagicProjectileClass, SpawnTM, SpawnParams);
}

void ATARCharacter::Jump()
{
	ACharacter::Jump();
}