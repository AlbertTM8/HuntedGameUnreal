// Fill out your copyright notice in the Description page of Project Settings.
#include "MultiplayerSessions.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineExternalUIInterface.h"
void PrintString(const FString& Str){
    if(GEngine){
	GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, Str);
}}
UMultiplayerSessions::UMultiplayerSessions(){
	PrintString("MSS Constructor");
	DestroyOnCreate = false;
	DestroyOnCreateSessionName = "";
	SessionInviteReceived = FOnSessionInviteReceivedDelegate::CreateUObject(this, &UMultiplayerSessions::OnSessionInviteReceived);
	SessionUserInviteAccepted = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UMultiplayerSessions::OnSessionUserInviteAccepted);
	ServerNameToFind = "";
	CurrentSessionName = FName("My Session");
}
void UMultiplayerSessions::Initialize(FSubsystemCollectionBase& Collection){
	PrintString("MSS Init");
	UE_LOG(LogTemp, Warning, TEXT("MSS Init"));
	OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem){
		FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString();
		PrintString(SubsystemName);
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if(SessionInterface.IsValid()){
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessions::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSessions::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSessions::OnFindSessionComplete);
			OnSessionInviteReceivedHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(SessionInviteReceived);
			OnSessionUserInviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAccepted);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessions::OnJoinSessionComplete);

		}
	}
	}
void UMultiplayerSessions::Deinitialize(){
 
}

void UMultiplayerSessions::HostServer(FString ServerName){
	PrintString("Host Server");
	if(ServerName.IsEmpty()){
		PrintString("Server Name is Empty");
		return;
	}

	FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(CurrentSessionName);
	if(ExistingSession){
			DestroyOnCreate = true;
			DestroyOnCreateSessionName = ServerName;
			PrintString("Destroying Session");
			SessionInterface->DestroySession(CurrentSessionName);
			return;
	}
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	bool Lan = false;
	if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"){
		Lan = true;
	}
	SessionSettings.bIsLANMatch = Lan;
	InviteSessionName = CurrentSessionName;
	SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionInterface->CreateSession(0, CurrentSessionName, SessionSettings);
}

void UMultiplayerSessions::JoinServer(FString ServerName){
	PrintString("Join Server");
	if(ServerNameToFind.IsEmpty()) return;
	if(ServerName.IsEmpty())	return;
	SessionSearchSettings = MakeShareable(new FOnlineSessionSearch());
	SessionSearchSettings->bIsLanQuery = false;
	if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL"){
		SessionSearchSettings->bIsLanQuery = true;
	}
	SessionSearchSettings->MaxSearchResults = 9999;
	SessionSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	ServerNameToFind = ServerName;
	SessionInterface->FindSessions(0, SessionSearchSettings.ToSharedRef());
}

void UMultiplayerSessions::OnCreateSessionComplete(FName SessionName, bool WasSuccessful){
	PrintString("OnCreateSessionComplete");
	if(WasSuccessful){
		GetWorld()->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
	}
}
void UMultiplayerSessions::OnDestroySessionComplete(FName SessionName, bool WasSuccessful){
	if(WasSuccessful){
		PrintString("Session Destroyed");
		if(DestroyOnCreate){
			DestroyOnCreate = false;
			HostServer(DestroyOnCreateSessionName);
		}
	}
}
void UMultiplayerSessions::OnFindSessionComplete(bool WasSuccessful){
	PrintString("OnFindSessionComplete");
	if(!WasSuccessful) return;
		PrintString("Session Found");
		TArray<FOnlineSessionSearchResult> Results = SessionSearchSettings->SearchResults;
		FOnlineSessionSearchResult* CorrectResult = 0;
		if(Results.Num() > 0){
			FString Msg = FString::Printf(TEXT("Found %d Sessions"), Results.Num());
			for(FOnlineSessionSearchResult Result : Results){
				if(Result.IsValid()){
					FString ServerName = "No-Name";
					Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

					if(ServerName.Equals(ServerNameToFind)){
						CorrectResult = &Result;
						break;
					}
				}
			}
			if(CorrectResult){
				PrintString("Joining Session");
				SessionInterface->JoinSession(0, CurrentSessionName, *CorrectResult);
			}
		}
}
void UMultiplayerSessions::GetAllFriends(){

	// if(IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface()){
	// 	FriendsPtr->ReadFriendsList(0, FString("FriendsListName"), FOnReadFriendsListComplete::CreateUObject(this, &UMultiplayerSessions::OnReadFriendsListCompleteDelegate));
	// }

}
//Read Friends List Delegate
void UMultiplayerSessions::OnReadFriendsListCompleteDelegate(int32 LocalUserNum, bool WasSuccessful, const FString& ListName, const FString& ErrorString){
	PrintString("OnReadFriendsListCompleteDelegate");
	if(WasSuccessful){
		PrintString("Friends List Read");
		if(IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface()){
			TArray<TSharedRef<FOnlineFriend>> FriendsList;
			if(FriendsPtr->GetFriendsList(0, ListName, FriendsList)){
				for(TSharedRef<FOnlineFriend> Friend : FriendsList){
					PrintString(Friend->GetDisplayName());
				}
			}	
		}
	}
}

void UMultiplayerSessions::InviteFriend(){
	if(IOnlineExternalUIPtr ExternalUIPtr = OnlineSubsystem->GetExternalUIInterface()){
		ExternalUIPtr->ShowInviteUI(0, FName(InviteSessionName));
	}
}

void UMultiplayerSessions::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FriendId, const FString& InviteText, const FOnlineSessionSearchResult& FriendSearchResult){
	PrintString("OnSessionInviteReceived");
}

void UMultiplayerSessions::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 LocalUserNum, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult){
	PrintString("OnSessionInviteAccepted");
	PrintString(FString::FromInt(bWasSuccessful));
	PrintString(FString::FromInt(LocalUserNum));
	PrintString(UserId->ToString());
	PrintString(InviteResult.Session.OwningUserName);
	bool JoinedSession;
	if(bWasSuccessful){
		if(IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface()){
			JoinedSession = SessionPtr->JoinSession(0, FName(InviteSessionName), InviteResult);
		}
	}
	PrintString("JoinedSession = " + FString::FromInt(JoinedSession));
}

void UMultiplayerSessions::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result){
	
	if(Result == EOnJoinSessionCompleteResult::Success){
		FString Address = "";
		bool Success = SessionInterface->GetResolvedConnectString(SessionName, Address);
		if (Success){
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if(PlayerController){
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

}