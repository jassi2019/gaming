// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Backend/SBBackendTypes.h"
#include "SBBackendSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthStateChanged, ESBAuthState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileLoaded, const FSBPlayerProfile&, Profile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloudSaveComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloudLoadComplete);

/**
 * Central backend subsystem — manages auth, profile, cloud save, rankings, battle pass, store.
 * Uses local JSON save as fallback when offline.
 * Provider-agnostic: swap REST API endpoints without touching game code.
 */
UCLASS()
class STORMBREAKER_API USBBackendSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Auth ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void Login(ESBAuthProvider Provider);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void Logout();

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    ESBAuthState GetAuthState() const { return AuthState; }

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    bool IsAuthenticated() const { return AuthState == ESBAuthState::Authenticated; }

    // --- Profile ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    const FSBPlayerProfile& GetProfile() const { return Profile; }

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void SetUsername(const FString& NewName);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void AddXP(int32 Amount);

    // --- Match Results ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void SubmitMatchResult(const FSBMatchResult& Result);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    const TArray<FSBMatchResult>& GetMatchHistory() const { return CloudData.MatchHistory; }

    // --- Cloud Save ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void SaveToCloud();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void LoadFromCloud();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void SaveToLocal();

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void LoadFromLocal();

    // --- Battle Pass ---

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    const FSBBattlePassState& GetBattlePass() const { return CloudData.BattlePass; }

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void AddBattlePassXP(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void ClaimBattlePassReward(int32 Tier);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void UpdateMissionProgress(const FName& MissionID, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void ClaimMission(const FName& MissionID);

    // --- Store ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    bool PurchaseItem(const FSBStoreItem& Item);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    bool OwnsItem(const FName& ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void AddCurrency(ESBCurrencyType Type, int32 Amount);

    // --- Ranking ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Backend")
    void UpdateRankPoints(int32 Delta);

    UFUNCTION(BlueprintPure, Category = "IslandOfDeath|Backend")
    ESBRankTier CalculateRankTier(int32 Points) const;

    // --- Delegates ---

    UPROPERTY(BlueprintAssignable) FOnAuthStateChanged OnAuthStateChanged;
    UPROPERTY(BlueprintAssignable) FOnProfileLoaded OnProfileLoaded;
    UPROPERTY(BlueprintAssignable) FOnCloudSaveComplete OnCloudSaveComplete;
    UPROPERTY(BlueprintAssignable) FOnCloudLoadComplete OnCloudLoadComplete;

private:
    void SetAuthState(ESBAuthState NewState);
    void CreateGuestProfile();
    void RecalculateStats();
    int64 GetXPForLevel(int32 Level) const;
    FString GetLocalSavePath() const;

    ESBAuthState AuthState;
    FSBPlayerProfile Profile;
    FSBCloudSaveData CloudData;
};
