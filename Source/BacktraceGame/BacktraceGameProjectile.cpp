// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BacktraceGameProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "BacktraceWrapper.h"

#if PLATFORM_ANDROID
extern FString GFilePathBase;
#endif
#if PLATFORM_IOS
#import <Backtrace/Backtrace-Swift.h>
#endif

ABacktraceGameProjectile::ABacktraceGameProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ABacktraceGameProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

//void ABacktraceGameProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//	// Only add impulse and destroy projectile if we hit a physics
//	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
//	{
//		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
//
//		Destroy();
//	}
//}

void * volatile ptr;
void ABacktraceGameProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		TMap<FString, FString> BacktraceAttributes;

		BacktraceAttributes.Add("custom.attributeTPM1", "Hi Jason!");
		BacktraceAttributes.Add("custom.attributeTPM2", "Hi Vincent!");
		BacktraceAttributes.Add("custom.attributeTPM3", "Hi Drake!");
		UE_LOG(LogTemp, Error, TEXT("About to initialize Backtrace Client"));

		FString FileName = TEXT("/MyCustomFile.txt");
		FString FilePath = FPaths::ProjectSavedDir() + FileName;
		UE_LOG(LogTemp, Error, TEXT("File path %s"), *FilePath)

		bool success = FPaths::FileExists(FilePath);
		UE_LOG(LogTemp, Error, TEXT("Does file exist? %d"), success)

		#if PLATFORM_ANDROID
			FilePath = GFilePathBase + FString("/UE4Game/") + FApp::GetName() + TEXT("/") + FApp::GetName() + TEXT("/Saved") + FileName;
			UE_LOG(LogTemp, Error, TEXT("Android file path %s"), *FilePath)
		#endif

		TArray<FString> Attachments;
		Attachments.Add(FilePath);

		BacktraceIO::FInitializeBacktraceClient(BacktraceAttributes, Attachments);

		FString FileContent = TEXT("Some data\nSome other data\nLast line of data\n");
		FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_NoFail);

		#if PLATFORM_MAC
			NSException* myException = [NSException
				exceptionWithName : @"ForceCrash"
				reason : @"Force Crash on purpose"
				userInfo:nil];
			@throw myException;
		#elif PLATFORM_IOS || PLATFORM_TVOS
			BacktraceCredentials *credentials = [[BacktraceCredentials alloc]
								initWithEndpoint: [NSURL URLWithString: @"https://cd03.sp.backtrace.io:6098/"]
								token: @"459bd6c479f30dfd9043ca25e82822f9a8f46acd99d834faf8e9cc71df61c77a"];
			BacktraceClient.shared = [[BacktraceClient alloc] initWithCredentials: credentials error: nil];
			NSString* message = @"Unreal Engine iOS Test";
			NSArray* array = [NSArray arrayWithObjects: @"", nil];
			[[BacktraceClient shared] sendWithMessage: message attachmentPaths: array completion:^(BacktraceResult * _Nonnull result) {
			}];
			//@[][666];
		#else
			// try to kill default (works on Windows etc)
			memset(ptr, 0x42, 20 * 1000 * 1000);
		#endif
		
		
		Destroy();
	}
}
