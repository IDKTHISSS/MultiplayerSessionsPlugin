// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"



void UMultiplayerSessionMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	FInputModeUIOnly InputModeData;

	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance){
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}	

	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMultiplayerSessionMenu::Initialize()
{
	if (!Super::Initialize()) {
		return false;
	}

	if (HostButton) {
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (JoinButton) {
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMultiplayerSessionMenu::OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(inLevel, inWorld);
}

void UMultiplayerSessionMenu::OnCreateSession(bool bWaeSuccessful)
{
	if (bWaeSuccessful) {
		MultiplayerSessionsSubsystem->StartSession();
	}
	else {
		HostButton->SetIsEnabled(true);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Red, FString(TEXT("Failed to create session!")));
		}
	}
}

void UMultiplayerSessionMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr) return;
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, FString(TEXT("Found Sessions")));
	}
	for (auto Result : SessionResults)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, FString(TEXT("Session")));
		}
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType) {
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0) {
		JoinButton->SetIsEnabled(true);
	}
}

void UMultiplayerSessionMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, FString(TEXT("OnJoinSession")));
	}
	if (Subsystem) {
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid()) {
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController) {
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success) {
		JoinButton->SetIsEnabled(true);
	}
}

void UMultiplayerSessionMenu::OnDestroySession(bool bWaeSuccessful)
{
}

void UMultiplayerSessionMenu::OnStartSession(bool bWasSuccessful)
{
	if (!bWasSuccessful) return;
	UWorld* World = GetWorld();
	if (World) {
		World->ServerTravel(PathToLobby);
	}
}

void UMultiplayerSessionMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (!MultiplayerSessionsSubsystem) return;
	MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);

	
}

void UMultiplayerSessionMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (!MultiplayerSessionsSubsystem) return;
	MultiplayerSessionsSubsystem->FindSessions(10000);
}

void UMultiplayerSessionMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;
	
	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}
