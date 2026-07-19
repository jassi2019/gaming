// Copyright Island Of Death Games. All Rights Reserved.

#include "UI/IslandOfDeathLoginWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScaleBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/ButtonSlot.h"
#include "Components/SizeBoxSlot.h"

UIslandOfDeathLoginWidget::UIslandOfDeathLoginWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Use Engine's default Roboto font
    UObject* RobotoObj = LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto"));
    const UObject* FontObj = RobotoObj;

    TitleFont = FSlateFontInfo(FontObj, 64);
    ButtonFont = FSlateFontInfo(FontObj, 22);
    SmallFont = FSlateFontInfo(FontObj, 14);
}

void UIslandOfDeathLoginWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildLayout();
}

void UIslandOfDeathLoginWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UIslandOfDeathLoginWidget::BuildLayout()
{
    if (!WidgetTree) return;

    // Root overlay: background art behind, content column on top
    RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
    WidgetTree->RootWidget = RootOverlay;

    // Layer 1: full-bleed background art
    UOverlaySlot* BgSlot = RootOverlay->AddChildToOverlay(BuildBackground());
    BgSlot->SetHorizontalAlignment(HAlign_Fill);
    BgSlot->SetVerticalAlignment(VAlign_Fill);

    // Layer 2: red-bordered frame
    UBorder* FrameBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FrameBorder"));
    FrameBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
    FrameBorder->SetPadding(FMargin(24.f));

    // Layer 3: main vertical content stack
    MainVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVerticalBox"));

    // Push content toward lower two-thirds (title starts mid-screen)
    USpacer* TopSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("TopSpacer"));
    TopSpacer->SetSize(FVector2D(1.f, 380.f));
    MainVerticalBox->AddChildToVerticalBox(TopSpacer);

    MainVerticalBox->AddChildToVerticalBox(BuildTitleBlock());
    MainVerticalBox->AddChildToVerticalBox(BuildTagline());

    // Helper: add spacing
    auto AddSpacer = [this](float Height)
    {
        USpacer* Sp = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
        Sp->SetSize(FVector2D(1.f, Height));
        MainVerticalBox->AddChildToVerticalBox(Sp);
    };

    AddSpacer(24.f);

    // Google button
    MainVerticalBox->AddChildToVerticalBox(
        BuildSocialButton(TEXT("GoogleButton"), GoogleIconTexture,
            FText::FromString(TEXT("CONTINUE WITH GOOGLE")),
            FLinearColor(0.94f, 0.94f, 0.92f, 1.f),
            FLinearColor(0.05f, 0.05f, 0.05f, 1.f),
            FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));

    AddSpacer(14.f);

    // Facebook button
    MainVerticalBox->AddChildToVerticalBox(
        BuildSocialButton(TEXT("FacebookButton"), FacebookIconTexture,
            FText::FromString(TEXT("CONTINUE WITH FACEBOOK")),
            FLinearColor(0.086f, 0.157f, 0.35f, 1.f),
            FLinearColor(1.f, 1.f, 1.f, 1.f),
            FLinearColor(0.29f, 0.45f, 0.85f, 1.f)));

    AddSpacer(14.f);

    // Apple button
    MainVerticalBox->AddChildToVerticalBox(
        BuildSocialButton(TEXT("AppleButton"), AppleIconTexture,
            FText::FromString(TEXT("CONTINUE WITH APPLE")),
            FLinearColor(0.02f, 0.02f, 0.02f, 1.f),
            FLinearColor(1.f, 1.f, 1.f, 1.f),
            FLinearColor(0.35f, 0.35f, 0.35f, 1.f)));

    AddSpacer(20.f);

    MainVerticalBox->AddChildToVerticalBox(BuildDivider());

    AddSpacer(20.f);

    MainVerticalBox->AddChildToVerticalBox(BuildGuestButton());

    AddSpacer(18.f);

    MainVerticalBox->AddChildToVerticalBox(BuildFooterNote());

    UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(MainVerticalBox);
    ContentSlot->SetHorizontalAlignment(HAlign_Fill);
    ContentSlot->SetVerticalAlignment(VAlign_Fill);
    ContentSlot->SetPadding(FMargin(28.f, 0.f, 28.f, 32.f));
}

UWidget* UIslandOfDeathLoginWidget::BuildBackground()
{
    BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundImage"));
    if (BackgroundArtTexture)
    {
        BackgroundImage->SetBrushFromTexture(BackgroundArtTexture);
    }
    BackgroundImage->SetBrush(FSlateBrush());
    return BackgroundImage;
}

UWidget* UIslandOfDeathLoginWidget::BuildTitleBlock()
{
    UVerticalBox* TitleBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TitleBox"));

    // "ISLAND"
    UTextBlock* IslandText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("IslandText"));
    IslandText->SetText(FText::FromString(TEXT("ISLAND")));
    IslandText->SetFont(TitleFont);
    IslandText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.96f, 0.94f, 1.f)));
    IslandText->SetJustification(ETextJustify::Center);

    // "OF"
    UTextBlock* OfText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OfText"));
    OfText->SetText(FText::FromString(TEXT("OF")));
    FSlateFontInfo OfFont = TitleFont;
    OfFont.Size = FMath::Max(18, TitleFont.Size / 3);
    OfText->SetFont(OfFont);
    OfText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)));
    OfText->SetJustification(ETextJustify::Center);

    // "DEATH" — blood red
    UTextBlock* DeathText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathText"));
    DeathText->SetText(FText::FromString(TEXT("DEATH")));
    FSlateFontInfo DeathFont = TitleFont;
    DeathFont.Size = TitleFont.Size + 8;
    DeathText->SetFont(DeathFont);
    DeathText->SetColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.05f, 0.05f, 1.f)));
    DeathText->SetJustification(ETextJustify::Center);

    TitleBox->AddChildToVerticalBox(IslandText);
    TitleBox->AddChildToVerticalBox(OfText);
    TitleBox->AddChildToVerticalBox(DeathText);

    // Skull icon below DEATH
    if (SkullShieldIconTexture)
    {
        UImage* SkullIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SkullIcon"));
        SkullIcon->SetBrushFromTexture(SkullShieldIconTexture);
        SkullIcon->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
        UVerticalBoxSlot* SkullSlot = TitleBox->AddChildToVerticalBox(SkullIcon);
        SkullSlot->SetHorizontalAlignment(HAlign_Center);
        SkullSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
    }

    return TitleBox;
}

UWidget* UIslandOfDeathLoginWidget::BuildTagline()
{
    UHorizontalBox* TaglineBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TaglineBox"));

    UTextBlock* Part1 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TaglinePart1"));
    Part1->SetText(FText::FromString(TEXT("Some Spirits ")));
    Part1->SetFont(SmallFont);
    Part1->SetColorAndOpacity(FSlateColor(FLinearColor::White));

    UTextBlock* Part2 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TaglinePart2"));
    Part2->SetText(FText::FromString(TEXT("Don't seek revenge... ")));
    Part2->SetFont(SmallFont);
    Part2->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.65f, 0.65f, 1.f)));

    UTextBlock* Part3 = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TaglinePart3"));
    Part3->SetText(FText::FromString(TEXT("they seek survival")));
    Part3->SetFont(SmallFont);
    Part3->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.1f, 0.1f, 1.f)));

    TaglineBox->AddChildToHorizontalBox(Part1);
    TaglineBox->AddChildToHorizontalBox(Part2);
    TaglineBox->AddChildToHorizontalBox(Part3);

    USizeBox* CenteredWrap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TaglineWrap"));
    CenteredWrap->AddChild(TaglineBox);

    return CenteredWrap;
}

UWidget* UIslandOfDeathLoginWidget::BuildSocialButton(const FString& ButtonName, UTexture2D* Icon,
    const FText& Label, const FLinearColor& BgColor, const FLinearColor& TextColor, const FLinearColor& BorderColor)
{
    UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), FName(*ButtonName));

    FButtonStyle Style = Btn->GetStyle();
    FSlateBrush NormalBrush;
    NormalBrush.TintColor = FSlateColor(BgColor);
    Style.Normal = NormalBrush;
    Style.Hovered = NormalBrush;
    Style.Pressed = NormalBrush;
    Btn->SetStyle(Style);

    USizeBox* Sizer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    Sizer->SetHeightOverride(58.f);

    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

    if (Icon)
    {
        UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        IconImage->SetBrushFromTexture(Icon);
        IconImage->SetDesiredSizeOverride(FVector2D(28.f, 28.f));
        UHorizontalBoxSlot* IconSlot = Row->AddChildToHorizontalBox(IconImage);
        IconSlot->SetPadding(FMargin(16.f, 0.f, 12.f, 0.f));
        IconSlot->SetVerticalAlignment(VAlign_Center);
    }

    UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    LabelText->SetText(Label);
    LabelText->SetFont(ButtonFont);
    LabelText->SetColorAndOpacity(FSlateColor(TextColor));
    UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText);
    LabelSlot->SetVerticalAlignment(VAlign_Center);
    LabelSlot->SetHorizontalAlignment(HAlign_Center);
    LabelSlot->SetPadding(FMargin(0.f, 0.f, 40.f, 0.f));

    Sizer->AddChild(Row);
    Btn->AddChild(Sizer);

    // Bind click + keep reference
    if (ButtonName == TEXT("GoogleButton"))
    {
        GoogleButton = Btn;
        Btn->OnClicked.AddDynamic(this, &UIslandOfDeathLoginWidget::HandleGoogleClicked);
    }
    else if (ButtonName == TEXT("FacebookButton"))
    {
        FacebookButton = Btn;
        Btn->OnClicked.AddDynamic(this, &UIslandOfDeathLoginWidget::HandleFacebookClicked);
    }
    else if (ButtonName == TEXT("AppleButton"))
    {
        AppleButton = Btn;
        Btn->OnClicked.AddDynamic(this, &UIslandOfDeathLoginWidget::HandleAppleClicked);
    }

    return Btn;
}

UWidget* UIslandOfDeathLoginWidget::BuildDivider()
{
    UHorizontalBox* DividerBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("DividerBox"));

    auto MakeLine = [this]() -> UWidget*
    {
        UBorder* Line = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        Line->SetBrushColor(FLinearColor(0.6f, 0.1f, 0.1f, 1.f));
        Line->SetPadding(FMargin(0.f));
        USizeBox* LineSizer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        LineSizer->SetHeightOverride(1.f);
        LineSizer->AddChild(Line);
        return LineSizer;
    };

    UHorizontalBoxSlot* LeftLineSlot = DividerBox->AddChildToHorizontalBox(MakeLine());
    LeftLineSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LeftLineSlot->SetVerticalAlignment(VAlign_Center);
    LeftLineSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));

    UTextBlock* OrText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OrText"));
    OrText->SetText(FText::FromString(TEXT("OR")));
    OrText->SetFont(SmallFont);
    OrText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));
    DividerBox->AddChildToHorizontalBox(OrText);

    UHorizontalBoxSlot* RightLineSlot = DividerBox->AddChildToHorizontalBox(MakeLine());
    RightLineSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    RightLineSlot->SetVerticalAlignment(VAlign_Center);
    RightLineSlot->SetPadding(FMargin(12.f, 0.f, 0.f, 0.f));

    return DividerBox;
}

UWidget* UIslandOfDeathLoginWidget::BuildGuestButton()
{
    GuestButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuestButton"));

    FButtonStyle Style = GuestButton->GetStyle();
    FSlateBrush NormalBrush;
    NormalBrush.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.3f));
    Style.Normal = NormalBrush;
    Style.Hovered = NormalBrush;
    Style.Pressed = NormalBrush;
    GuestButton->SetStyle(Style);
    GuestButton->OnClicked.AddDynamic(this, &UIslandOfDeathLoginWidget::HandleGuestClicked);

    USizeBox* Sizer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    Sizer->SetHeightOverride(66.f);

    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

    if (GuestIconTexture)
    {
        UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        Icon->SetBrushFromTexture(GuestIconTexture);
        Icon->SetDesiredSizeOverride(FVector2D(32.f, 32.f));
        UHorizontalBoxSlot* IconSlot = Row->AddChildToHorizontalBox(Icon);
        IconSlot->SetPadding(FMargin(16.f, 0.f, 12.f, 0.f));
        IconSlot->SetVerticalAlignment(VAlign_Center);
    }

    UVerticalBox* TextStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

    UTextBlock* PlayAsGuestText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    PlayAsGuestText->SetText(FText::FromString(TEXT("PLAY AS GUEST")));
    PlayAsGuestText->SetFont(ButtonFont);
    PlayAsGuestText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.1f, 0.1f, 1.f)));

    UTextBlock* SubText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    SubText->SetText(FText::FromString(TEXT("Jump into the island as a guest")));
    SubText->SetFont(SmallFont);
    SubText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)));

    TextStack->AddChildToVerticalBox(PlayAsGuestText);
    TextStack->AddChildToVerticalBox(SubText);

    UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(TextStack);
    TextSlot->SetVerticalAlignment(VAlign_Center);
    TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    // Chevron arrow ">"
    UTextBlock* Chevron = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Chevron->SetText(FText::FromString(TEXT(">")));
    Chevron->SetFont(ButtonFont);
    Chevron->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.1f, 0.1f, 1.f)));
    UHorizontalBoxSlot* ChevronSlot = Row->AddChildToHorizontalBox(Chevron);
    ChevronSlot->SetVerticalAlignment(VAlign_Center);
    ChevronSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));

    Sizer->AddChild(Row);
    GuestButton->AddChild(Sizer);

    return GuestButton;
}

UWidget* UIslandOfDeathLoginWidget::BuildFooterNote()
{
    UHorizontalBox* FooterBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FooterBox"));

    if (SkullShieldIconTexture)
    {
        UImage* ShieldIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        ShieldIcon->SetBrushFromTexture(SkullShieldIconTexture);
        ShieldIcon->SetDesiredSizeOverride(FVector2D(20.f, 20.f));
        UHorizontalBoxSlot* IconSlot = FooterBox->AddChildToHorizontalBox(ShieldIcon);
        IconSlot->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
        IconSlot->SetVerticalAlignment(VAlign_Center);
    }

    UTextBlock* FooterText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FooterText"));
    FooterText->SetText(FText::FromString(TEXT("Your progress as a guest will be\nstored on this device only.")));
    FooterText->SetFont(SmallFont);
    FooterText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));
    FooterText->SetJustification(ETextJustify::Left);
    FooterBox->AddChildToHorizontalBox(FooterText);

    USizeBox* CenteredWrap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    CenteredWrap->AddChild(FooterBox);

    return CenteredWrap;
}

// --- Click Handlers ---

void UIslandOfDeathLoginWidget::HandleGoogleClicked()
{
    OnGoogleLoginClicked.Broadcast();
}

void UIslandOfDeathLoginWidget::HandleFacebookClicked()
{
    OnFacebookLoginClicked.Broadcast();
}

void UIslandOfDeathLoginWidget::HandleAppleClicked()
{
    OnAppleLoginClicked.Broadcast();
}

void UIslandOfDeathLoginWidget::HandleGuestClicked()
{
    OnGuestLoginClicked.Broadcast();
}
