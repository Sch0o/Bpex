// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BpexHUD.h"

#include "Blueprint/UserWidget.h"
#include "Players/ShooterPlayerController.h"

void ABpexHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (MainHUDClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDClass);
		
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
	
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayerController()))
	{
		PC->TryInitMVVM();
	}
}

