// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SBBackendTypes.generated.h"

// ============================================================================
// Auth
// ============================================================================

UENUM(BlueprintType)
enum class ESBAuthProvider : uint8
{
    EOS,
    GooglePlay,
    AppleGameCenter,
    Guest
};

UENUM(BlueprintType)
enum class ESBAuthState : uint8
{
    NotAuthenticated,
    Authenticating,
    Authenticated,
    Failed
};

// ============================================================================
// Rank
// ============================================================================

UENUM(BlueprintType)
enum class ESBRankTier : uint8
{
    Bronze,
    Silver,
    Gold,
    Platinum,
    Diamond,
    Crown,
    Ace,
    AceDominator,
    Conqueror
};

// ============================================================================
// Currency
// ============================================================================

UENUM(BlueprintType)
enum class ESBCurrencyType : uint8
{
    SB_Coins,
    SB_Gems
};

UENUM(BlueprintType)
enum class ESBStoreItemType : uint8
{
    Skin,
    Character,
    Emote,
    WeaponFinish,
    Crate,
    BattlePass
};

// ============================================================================
// Player Profile
// ============================================================================

USTRUCT(BlueprintType)
struct FSBPlayerStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 MatchesPlayed = 0;
    UPROPERTY(BlueprintReadWrite) int32 Wins = 0;
    UPROPERTY(BlueprintReadWrite) int32 Top10 = 0;
    UPROPERTY(BlueprintReadWrite) int32 TotalKills = 0;
    UPROPERTY(BlueprintReadWrite) int32 TotalDamage = 0;
    UPROPERTY(BlueprintReadWrite) int32 TotalRevives = 0;
    UPROPERTY(BlueprintReadWrite) float TotalSurvivalMinutes = 0.0f;
    UPROPERTY(BlueprintReadWrite) float LongestKillDistance = 0.0f;
    UPROPERTY(BlueprintReadWrite) int32 HeadshotKills = 0;
    UPROPERTY(BlueprintReadWrite) float KDRatio = 0.0f;
    UPROPERTY(BlueprintReadWrite) float WinRate = 0.0f;
    UPROPERTY(BlueprintReadWrite) float AvgDamagePerMatch = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBPlayerProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString UID;
    UPROPERTY(BlueprintReadWrite) FString Username;
    UPROPERTY(BlueprintReadWrite) FString AvatarURL;
    UPROPERTY(BlueprintReadWrite) int32 Level = 1;
    UPROPERTY(BlueprintReadWrite) int64 XP = 0;
    UPROPERTY(BlueprintReadWrite) int64 XPToNextLevel = 100;
    UPROPERTY(BlueprintReadWrite) ESBRankTier Rank = ESBRankTier::Bronze;
    UPROPERTY(BlueprintReadWrite) int32 RankPoints = 0;
    UPROPERTY(BlueprintReadWrite) int32 Season = 1;
    UPROPERTY(BlueprintReadWrite) int32 Coins = 0;
    UPROPERTY(BlueprintReadWrite) int32 Gems = 0;
    UPROPERTY(BlueprintReadWrite) FSBPlayerStats Stats;
    UPROPERTY(BlueprintReadWrite) ESBAuthProvider AuthProvider = ESBAuthProvider::Guest;
};

// ============================================================================
// Match Result
// ============================================================================

USTRUCT(BlueprintType)
struct FSBMatchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString MatchID;
    UPROPERTY(BlueprintReadWrite) int32 Placement = 0;
    UPROPERTY(BlueprintReadWrite) int32 Kills = 0;
    UPROPERTY(BlueprintReadWrite) int32 Assists = 0;
    UPROPERTY(BlueprintReadWrite) int32 DamageDealt = 0;
    UPROPERTY(BlueprintReadWrite) float SurvivalSeconds = 0.0f;
    UPROPERTY(BlueprintReadWrite) int32 Revives = 0;
    UPROPERTY(BlueprintReadWrite) int32 XPEarned = 0;
    UPROPERTY(BlueprintReadWrite) int32 RankPointsDelta = 0;
    UPROPERTY(BlueprintReadWrite) int32 CoinsEarned = 0;
    UPROPERTY(BlueprintReadWrite) FDateTime Timestamp;
};

// ============================================================================
// Battle Pass
// ============================================================================

USTRUCT(BlueprintType)
struct FSBBattlePassReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Tier = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RewardItemID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RewardCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsPremium = false;
};

USTRUCT(BlueprintType)
struct FSBMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName MissionID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TargetCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CurrentCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 XPReward = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsDaily = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsWeekly = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsSeasonal = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCompleted = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bClaimed = false;

    bool IsComplete() const { return CurrentCount >= TargetCount; }
};

USTRUCT(BlueprintType)
struct FSBBattlePassState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 CurrentTier = 0;
    UPROPERTY(BlueprintReadWrite) int32 MaxTier = 100;
    UPROPERTY(BlueprintReadWrite) int64 TierXP = 0;
    UPROPERTY(BlueprintReadWrite) int64 XPPerTier = 500;
    UPROPERTY(BlueprintReadWrite) bool bHasPremium = false;
    UPROPERTY(BlueprintReadWrite) TArray<FSBMission> DailyMissions;
    UPROPERTY(BlueprintReadWrite) TArray<FSBMission> WeeklyMissions;
    UPROPERTY(BlueprintReadWrite) TArray<FSBMission> SeasonalMissions;
};

// ============================================================================
// Store Item
// ============================================================================

USTRUCT(BlueprintType)
struct FSBStoreItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ESBStoreItemType Type = ESBStoreItemType::Skin;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ESBCurrencyType Currency = ESBCurrencyType::SB_Coins;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Price = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsLimitedTime = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime ExpiresAt;
};

// ============================================================================
// Cloud Save
// ============================================================================

USTRUCT(BlueprintType)
struct FSBCloudSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FSBPlayerProfile Profile;
    UPROPERTY(BlueprintReadWrite) FSBBattlePassState BattlePass;
    UPROPERTY(BlueprintReadWrite) TArray<FName> OwnedItems;
    UPROPERTY(BlueprintReadWrite) TArray<FName> EquippedCosmetics;
    UPROPERTY(BlueprintReadWrite) TArray<FSBMatchResult> MatchHistory;
    UPROPERTY(BlueprintReadWrite) int32 SaveVersion = 1;
};

// ============================================================================
// Leaderboard
// ============================================================================

USTRUCT(BlueprintType)
struct FSBLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32 Rank = 0;
    UPROPERTY(BlueprintReadOnly) FString Username;
    UPROPERTY(BlueprintReadOnly) ESBRankTier Tier = ESBRankTier::Bronze;
    UPROPERTY(BlueprintReadOnly) int32 RankPoints = 0;
    UPROPERTY(BlueprintReadOnly) int32 Kills = 0;
    UPROPERTY(BlueprintReadOnly) float KDRatio = 0.0f;
    UPROPERTY(BlueprintReadOnly) float WinRate = 0.0f;
};
