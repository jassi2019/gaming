// Copyright StormBreaker Games. All Rights Reserved.

#include "Backend/SBAnalyticsSubsystem.h"
#include "StormBreaker.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"

void USBAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSBBackend, Log, TEXT("Analytics subsystem initialized."));
}

void USBAnalyticsSubsystem::QueueEvent(const FString& Name, const TMap<FString, FString>& Props)
{
    FAnalyticsEvent Event;
    Event.EventName = Name;
    Event.Properties = Props;
    Event.Timestamp = FDateTime::UtcNow();
    EventQueue.Add(Event);

    // Auto-flush at 100 events
    if (EventQueue.Num() >= 100)
    {
        FlushEvents();
    }
}

void USBAnalyticsSubsystem::TrackEvent(const FString& EventName, const TMap<FString, FString>& Properties)
{
    QueueEvent(EventName, Properties);
}

void USBAnalyticsSubsystem::TrackMatchStart(const FString& MatchID, int32 PlayerCount, const FString& MapName)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("match_id"), MatchID);
    Props.Add(TEXT("player_count"), FString::FromInt(PlayerCount));
    Props.Add(TEXT("map"), MapName);
    QueueEvent(TEXT("match_start"), Props);
}

void USBAnalyticsSubsystem::TrackMatchEnd(const FString& MatchID, int32 Placement, int32 Kills, float SurvivalSeconds)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("match_id"), MatchID);
    Props.Add(TEXT("placement"), FString::FromInt(Placement));
    Props.Add(TEXT("kills"), FString::FromInt(Kills));
    Props.Add(TEXT("survival_s"), FString::SanitizeFloat(SurvivalSeconds));
    QueueEvent(TEXT("match_end"), Props);
}

void USBAnalyticsSubsystem::TrackKill(const FString& WeaponID, float Distance, bool bHeadshot)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("weapon"), WeaponID);
    Props.Add(TEXT("distance"), FString::SanitizeFloat(Distance));
    Props.Add(TEXT("headshot"), bHeadshot ? TEXT("1") : TEXT("0"));
    QueueEvent(TEXT("kill"), Props);
}

void USBAnalyticsSubsystem::TrackDeath(const FString& KillerWeaponID, float Distance)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("killer_weapon"), KillerWeaponID);
    Props.Add(TEXT("distance"), FString::SanitizeFloat(Distance));
    QueueEvent(TEXT("death"), Props);
}

void USBAnalyticsSubsystem::TrackPerformanceSnapshot(float FPS, float FrameTimeMs, int32 DrawCalls, int32 MemoryMB)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("fps"), FString::SanitizeFloat(FPS));
    Props.Add(TEXT("frame_ms"), FString::SanitizeFloat(FrameTimeMs));
    Props.Add(TEXT("draw_calls"), FString::FromInt(DrawCalls));
    Props.Add(TEXT("memory_mb"), FString::FromInt(MemoryMB));
    QueueEvent(TEXT("perf_snapshot"), Props);
}

void USBAnalyticsSubsystem::TrackDeviceInfo()
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("platform"), FPlatformProperties::PlatformName());
    Props.Add(TEXT("gpu"), FPlatformMisc::GetPrimaryGPUBrand());
    Props.Add(TEXT("cpu"), FPlatformMisc::GetCPUBrand());
    Props.Add(TEXT("cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
    Props.Add(TEXT("ram_gb"), FString::FromInt(
        static_cast<int32>(FPlatformMemory::GetConstants().TotalPhysical / (1024 * 1024 * 1024))));
    Props.Add(TEXT("os"), FPlatformMisc::GetOSVersion());
    QueueEvent(TEXT("device_info"), Props);
}

void USBAnalyticsSubsystem::ReportPlayer(const FString& ReportedUID, const FString& Reason)
{
    TMap<FString, FString> Props;
    Props.Add(TEXT("reported_uid"), ReportedUID);
    Props.Add(TEXT("reason"), Reason);
    QueueEvent(TEXT("player_report"), Props);

    UE_LOG(LogSBBackend, Log, TEXT("Player reported: %s — %s"), *ReportedUID, *Reason);
}

void USBAnalyticsSubsystem::FlushEvents()
{
    if (EventQueue.Num() == 0) return;

    // In production: HTTP POST batch to analytics endpoint
    // For now: log count and clear
    UE_LOG(LogSBBackend, Log, TEXT("Flushing %d analytics events."), EventQueue.Num());

    // Save to local file as fallback
    FString AnalyticsLogFile = FPaths::ProjectSavedDir() / TEXT("analytics_log.txt");
    FString BatchLog;
    for (const FAnalyticsEvent& Event : EventQueue)
    {
        BatchLog += FString::Printf(TEXT("[%s] %s\n"), *Event.Timestamp.ToString(), *Event.EventName);
    }
    FFileHelper::SaveStringToFile(BatchLog, *AnalyticsLogFile, FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

    EventQueue.Empty();
}
