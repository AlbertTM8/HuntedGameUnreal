// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
//#include "Online/Sessions.h"
#include "OnlineSessionClient.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Delegates/IDelegateInstance.h"
#include "MultiplayerSessions.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERCOURSE_API UMultiplayerSessions : public UGameInstanceSubsystem
{
	GENERATED_BODY()
		public:
		UMultiplayerSessions();
		void Initialize(FSubsystemCollectionBase& Collection) override;
    	void Deinitialize() override;
		UFUNCTION(BlueprintCallable)
		void HostServer(FString ServerName);
		UFUNCTION(BlueprintCallable)
		void JoinServer(FString ServerName);
		UFUNCTION(BlueprintCallable)
		void GetAllFriends();
		UFUNCTION(BlueprintCallable)
		void InviteFriend();

		//Delegate Functions
		void OnCreateSessionComplete(FName SessionName, bool WasSuccessful);
		void OnDestroySessionComplete(FName SessionName, bool WasSuccessful);
		void OnFindSessionComplete(bool WasSuccessful);
		void OnReadFriendsListCompleteDelegate(int32 LocalUserNum, bool WasSuccessful, const FString& ListName, const FString& ErrorString);
		void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FriendId, const FString& InviteText, const FOnlineSessionSearchResult& FriendSearchResult);
		void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalUserNum, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);
		void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
		IOnlineSessionPtr SessionInterface;
		bool DestroyOnCreate;
		FString DestroyOnCreateSessionName;
		FName InviteSessionName;
		TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;
		IOnlineSubsystem* OnlineSubsystem;
		FOnSessionInviteReceivedDelegate SessionInviteReceived; 
		FOnSessionUserInviteAcceptedDelegate SessionUserInviteAccepted;
		FDelegateHandle OnSessionInviteReceivedHandle;
    	FDelegateHandle OnSessionUserInviteAcceptedHandle;
		FString ServerNameToFind;
		FName CurrentSessionName;
};

