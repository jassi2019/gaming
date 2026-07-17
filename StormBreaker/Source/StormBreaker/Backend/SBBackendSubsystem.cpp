// Copyright Island Of Death Games. All Rights Reserved.

#include "Backend/SBBackendSubsystem.h"
#include "StormBreaker.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "Misc/Guid.h"
#include "HAL/PlatformFileManager.h"

void USBBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    AuthState = ESBAuthState::NotAuthenticated;

    // Try loading local save
    LoadFromLocal();

    UE_LOG(LogSBBackend, Log, TEXT("Backend subsystem initialized."));
}

// ============================================================================
// Auth
// ============================================================================

void USBBackendSubsystem::Login(ESBAuthProvider Provider)
{
    SetAuthState(ESBAuthState::Authenticating);

    switch (Provider)
    {
    case ESBAuthProvider::Guest:
        if (Profile.UID.IsEmpty())
        {
            CreateGuestProfile();
        }
        SetAuthState(ESBAuthState::Authenticated);
        OnProfileLoaded.Broadcast(Profile);
        break;

    case ESBAuthProvider::EOS:
    case ESBAuthProvider::GooglePlay:
    case ESBAuthProvider::AppleGameCenter:
        // Platform-specific auth handled via Online Subsystem
        // On success callback → load cloud data → set Authenticated
        // For now, fall back to guest
        if (Profile.UID.IsEmpty()) CreateGuestProfile();
        Profile.AuthProvider = Provider;
        SetAuthState(ESBAuthState::Authenticated);
        OnProfileLoaded.Broadcast(Profile);
        break;
    }

    UE_LOG(LogSBBackend, Log, TEXT("Login via %d. UID: %s"), (int32)Provider, *Profile.UID);
}

void USBBackendSubsystem::Logout()
{
    SaveToLocal();
    SetAuthState(ESBAuthState::NotAuthenticated);
    UE_LOG(LogSBBackend, Log, TEXT("Logged out."));
}

void USBBackendSubsystem::CreateGuestProfile()
{
    Profile.UID = FGuid::NewGuid().ToString();
    Profile.Username = FString::Printf(TEXT("Player_%s"), *Profile.UID.Left(6));
    Profile.Level = 1;
    Profile.XP = 0;
    Profile.XPToNextLevel = GetXPForLevel(2);
    Profile.Rank = ESBRankTier::Bronze;
    Profile.RankPoints = 0;
    Profile.Coins = 1000;
    Profile.Gems = 100;
    Profile.AuthProvider = ESBAuthProvider::Guest;

    CloudData.Profile = Profile;
}

void USBBackendSubsystem::SetUsername(const FString& NewName)
{
    Profile.Username = NewName;
    CloudData.Profile.Username = NewName;
    SaveToLocal();
}

// ============================================================================
// XP / Leveling
// ============================================================================

void USBBackendSubsystem::AddXP(int32 Amount)
{
    Profile.XP += Amount;

    while (Profile.XP >= Profile.XPToNextLevel)
    {
        Profile.XP -= Profile.XPToNextLevel;
        Profile.Level++;
        Profile.XPToNextLevel = GetXPForLevel(Profile.Level + 1);

        UE_LOG(LogSBBackend, Log, TEXT("Level up! Now level %d"), Profile.Level);
    }

    CloudData.Profile = Profile;
}

int64 USBBackendSubsystem::GetXPForLevel(int32 Level) const
{
    // Exponential curve: 100 * Level^1.5
    return static_cast<int64>(100.0 * FMath::Pow(static_cast<float>(Level), 1.5f));
}

// ============================================================================
// Match Results
// ============================================================================

void USBBackendSubsystem::SubmitMatchResult(const FSBMatchResult& Result)
{
    CloudData.MatchHistory.Add(Result);

    // Cap history to 50 matches
    while (CloudData.MatchHistory.Num() > 50)
    {
        CloudData.MatchHistory.RemoveAt(0);
    }

    // Update stats
    FSBPlayerStats& S = Profile.Stats;
    S.MatchesPlayed++;
    S.TotalKills += Result.Kills;
    S.TotalDamage += Result.DamageDealt;
    S.TotalRevives += Result.Revives;
    S.TotalSurvivalMinutes += Result.SurvivalSeconds / 60.0f;

    if (Result.Placement == 1) S.Wins++;
    if (Result.Placement <= 10) S.Top10++;

    RecalculateStats();

    // Apply rewards
    AddXP(Result.XPEarned);
    AddCurrency(ESBCurrencyType::SB_Coins, Result.CoinsEarned);
    UpdateRankPoints(Result.RankPointsDelta);

    // Update battle pass
    AddBattlePassXP(Result.XPEarned / 2);

    CloudData.Profile = Profile;
    SaveToLocal();

    UE_LOG(LogSBBackend, Log, TEXT("Match result: #%d, %d kills, +%d XP, +%d RP"),
        Result.Placement, Result.Kills, Result.XPEarned, Result.RankPointsDelta);
}

void USBBackendSubsystem::RecalculateStats()
{
    FSBPlayerStats& S = Profile.Stats;
    int32 Deaths = FMath::Max(1, S.MatchesPlayed - S.Wins);
    S.KDRatio = static_cast<float>(S.TotalKills) / Deaths;
    S.WinRate = S.MatchesPlayed > 0 ? (static_cast<float>(S.Wins) / S.MatchesPlayed) * 100.0f : 0.0f;
    S.AvgDamagePerMatch = S.MatchesPlayed > 0 ? static_cast<float>(S.TotalDamage) / S.MatchesPlayed : 0.0f;
}

// ============================================================================
// Cloud Save / Local Save
// ============================================================================

void USBBackendSubsystem::SaveToCloud()
{
    // In production: HTTP POST to backend API
    // Fallback to local
    SaveToLocal();
    OnCloudSaveComplete.Broadcast();
    UE_LOG(LogSBBackend, Log, TEXT("Cloud save complete (local fallback)."));
}

void USBBackendSubsystem::LoadFromCloud()
{
    // In production: HTTP GET from backend API
    // Fallback to local
    LoadFromLocal();
    OnCloudLoadComplete.Broadcast();
}

void USBBackendSubsystem::SaveToLocal()
{
    CloudData.Profile = Profile;

    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(CloudData, JsonString);

    FString Path = GetLocalSavePath();
    FFileHelper::SaveStringToFile(JsonString, *Path);

    UE_LOG(LogSBBackend, Verbose, TEXT("Saved to %s"), *Path);
}

void USBBackendSubsystem::LoadFromLocal()
{
    FString Path = GetLocalSavePath();
    FString JsonString;

    if (FFileHelper::LoadFileToString(JsonString, *Path))
    {
        FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &CloudData);
        Profile = CloudData.Profile;
        UE_LOG(LogSBBackend, Log, TEXT("Loaded profile: %s (Level %d)"), *Profile.Username, Profile.Level);
    }
}

FString USBBackendSubsystem::GetLocalSavePath() const
{
    return FPaths::ProjectSavedDir() / TEXT("StormBreaker_SaveData.json");
}

// ============================================================================
// Battle Pass
// ============================================================================

void USBBackendSubsystem::AddBattlePassXP(int32 Amount)
{
    FSBBattlePassState& BP = CloudData.BattlePass;
    BP.TierXP += Amount;

    while (BP.TierXP >= BP.XPPerTier && BP.CurrentTier < BP.MaxTier)
    {
        BP.TierXP -= BP.XPPerTier;
        BP.CurrentTier++;
    }
}

void USBBackendSubsystem::ClaimBattlePassReward(int32 Tier)
{
    // Reward distribution handled by Blueprint or DataTable lookup
    UE_LOG(LogSBBackend, Log, TEXT("Claimed battle pass tier %d reward"), Tier);
}

void USBBackendSubsystem::UpdateMissionProgress(const FName& MissionID, int32 Delta)
{
    auto UpdateMissionArray = [&](TArray<FSBMission>& Missions)
    {
        for (FSBMission& M : Missions)
        {
            if (M.MissionID == MissionID && !M.bCompleted)
            {
                M.CurrentCount = FMath::Min(M.CurrentCount + Delta, M.TargetCount);
                if (M.IsComplete())
                {
                    M.bCompleted = true;
                }
                return;
            }
        }
    };

    UpdateMissionArray(CloudData.BattlePass.DailyMissions);
    UpdateMissionArray(CloudData.BattlePass.WeeklyMissions);
    UpdateMissionArray(CloudData.BattlePass.SeasonalMissions);
}

void USBBackendSubsystem::ClaimMission(const FName& MissionID)
{
    auto ClaimFromArray = [&](TArray<FSBMission>& Missions) -> bool
    {
        for (FSBMission& M : Missions)
        {
            if (M.MissionID == MissionID && M.bCompleted && !M.bClaimed)
            {
                M.bClaimed = true;
                AddBattlePassXP(M.XPReward);
                return true;
            }
        }
        return false;
    };

    ClaimFromArray(CloudData.BattlePass.DailyMissions) ||
    ClaimFromArray(CloudData.BattlePass.WeeklyMissions) ||
    ClaimFromArray(CloudData.BattlePass.SeasonalMissions);
}

// ============================================================================
// Store
// ============================================================================

bool USBBackendSubsystem::PurchaseItem(const FSBStoreItem& Item)
{
    int32* Balance = nullptr;
    if (Item.Currency == ESBCurrencyType::SB_Coins)
        Balance = &Profile.Coins;
    else
        Balance = &Profile.Gems;

    if (!Balance || *Balance < Item.Price)
    {
        UE_LOG(LogSBBackend, Warning, TEXT("Insufficient funds for %s"), *Item.DisplayName.ToString());
        return false;
    }

    *Balance -= Item.Price;
    CloudData.OwnedItems.AddUnique(Item.ItemID);
    CloudData.Profile = Profile;
    SaveToLocal();

    UE_LOG(LogSBBackend, Log, TEXT("Purchased %s for %d"), *Item.DisplayName.ToString(), Item.Price);
    return true;
}

bool USBBackendSubsystem::OwnsItem(const FName& ItemID) const
{
    return CloudData.OwnedItems.Contains(ItemID);
}

void USBBackendSubsystem::AddCurrency(ESBCurrencyType Type, int32 Amount)
{
    if (Type == ESBCurrencyType::SB_Coins)
        Profile.Coins += Amount;
    else
        Profile.Gems += Amount;

    CloudData.Profile = Profile;
}

// ============================================================================
// Ranking
// ============================================================================

void USBBackendSubsystem::UpdateRankPoints(int32 Delta)
{
    Profile.RankPoints = FMath::Max(0, Profile.RankPoints + Delta);
    Profile.Rank = CalculateRankTier(Profile.RankPoints);
    CloudData.Profile = Profile;
}

ESBRankTier USBBackendSubsystem::CalculateRankTier(int32 Points) const
{
    if (Points >= 5000) return ESBRankTier::Conqueror;
    if (Points >= 4200) return ESBRankTier::AceDominator;
    if (Points >= 3600) return ESBRankTier::Ace;
    if (Points >= 3000) return ESBRankTier::Crown;
    if (Points >= 2400) return ESBRankTier::Diamond;
    if (Points >= 1800) return ESBRankTier::Platinum;
    if (Points >= 1200) return ESBRankTier::Gold;
    if (Points >= 600)  return ESBRankTier::Silver;
    return ESBRankTier::Bronze;
}

// ============================================================================
// Internal
// ============================================================================

void USBBackendSubsystem::SetAuthState(ESBAuthState NewState)
{
    AuthState = NewState;
    OnAuthStateChanged.Broadcast(NewState);
}
