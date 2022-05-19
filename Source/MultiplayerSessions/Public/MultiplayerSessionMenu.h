// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("Testing")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld) override;

	UFUNCTION()
	void OnCreateSession(bool bWaeSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionsResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWaeSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("Testing")};
	FString PathToLobby{TEXT("")};
};
