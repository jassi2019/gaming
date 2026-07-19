// Copyright Island Of Death Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "IslandOfDeathLoginWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginButtonClicked);

/**
 * Island of Death — Login Screen Widget.
 * Full BGMI-style login with background art, social login buttons, and guest play.
 * Constructed entirely in C++ — no UMG designer needed.
 */
UCLASS()
class STORMBREAKER_API UIslandOfDeathLoginWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UIslandOfDeathLoginWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // --- Exposed Textures (set in Blueprint or C++) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* BackgroundArtTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* GoogleIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* FacebookIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* AppleIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* GuestIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    UTexture2D* SkullShieldIconTexture;

    // --- Fonts ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    FSlateFontInfo TitleFont;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    FSlateFontInfo ButtonFont;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IslandOfDeath|Login")
    FSlateFontInfo SmallFont;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Login")
    FOnLoginButtonClicked OnGoogleLoginClicked;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Login")
    FOnLoginButtonClicked OnFacebookLoginClicked;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Login")
    FOnLoginButtonClicked OnAppleLoginClicked;

    UPROPERTY(BlueprintAssignable, Category = "IslandOfDeath|Login")
    FOnLoginButtonClicked OnGuestLoginClicked;

protected:
    // --- Widget References ---
    UPROPERTY()
    UOverlay* RootOverlay;

    UPROPERTY()
    UVerticalBox* MainVerticalBox;

    UPROPERTY()
    UImage* BackgroundImage;

    UPROPERTY()
    UButton* GoogleButton;

    UPROPERTY()
    UButton* FacebookButton;

    UPROPERTY()
    UButton* AppleButton;

    UPROPERTY()
    UButton* GuestButton;

    // --- Build Functions ---
    void BuildLayout();
    UWidget* BuildBackground();
    UWidget* BuildTitleBlock();
    UWidget* BuildTagline();
    UWidget* BuildSocialButton(const FString& ButtonName, UTexture2D* Icon, const FText& Label,
        const FLinearColor& BgColor, const FLinearColor& TextColor, const FLinearColor& BorderColor);
    UWidget* BuildDivider();
    UWidget* BuildGuestButton();
    UWidget* BuildFooterNote();

    // --- Click Handlers ---
    UFUNCTION()
    void HandleGoogleClicked();

    UFUNCTION()
    void HandleFacebookClicked();

    UFUNCTION()
    void HandleAppleClicked();

    UFUNCTION()
    void HandleGuestClicked();
};
