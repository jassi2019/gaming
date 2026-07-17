// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBAnalyticsSubsystem.generated.h"

/**
 * Client-side analytics collection.
 * Tracks match events, performance metrics, device info, and crash data.
 * Events are batched and sent to backend periodically.
 * Works offline — queues events for later upload.
 */
UCLASS()
class STORMBREAKER_API USBAnalyticsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Event Tracking ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackEvent(const FString& EventName, const TMap<FString, FString>& Properties);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackMatchStart(const FString& MatchID, int32 PlayerCount, const FString& MapName);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackMatchEnd(const FString& MatchID, int32 Placement, int32 Kills, float SurvivalSeconds);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackKill(const FString& WeaponID, float Distance, bool bHeadshot);

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackDeath(const FString& KillerWeaponID, float Distance);

    // --- Performance ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackPerformanceSnapshot(float FPS, float FrameTimeMs, int32 DrawCalls, int32 MemoryMB);

    // --- Device ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void TrackDeviceInfo();

    // --- Report ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void ReportPlayer(const FString& ReportedUID, const FString& Reason);

    // --- Flush ---

    UFUNCTION(BlueprintCallable, Category = "IslandOfDeath|Analytics")
    void FlushEvents();

private:
    struct FAnalyticsEvent
    {
        FString EventName;
        TMap<FString, FString> Properties;
        FDateTime Timestamp;
    };

    TArray<FAnalyticsEvent> EventQueue;

    void QueueEvent(const FString& Name, const TMap<FString, FString>& Props);
};
