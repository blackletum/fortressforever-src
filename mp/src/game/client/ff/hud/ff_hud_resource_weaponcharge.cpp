#include "cbase.h"
#include "ff_hud_resource_weaponcharge.h"
#include "hud_macros.h"
#include "c_ff_player.h"
#include "ff_weapon_assaultcannon.h"
#include "ff_weapon_sniperrifle.h"
#include "ff_weapon_jumpgun.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"
#include <vgui/IVGui.h>

using namespace vgui;
using namespace FFQuantityHelper;

DECLARE_HUDELEMENT(CHudResourceSniperCharge);
DECLARE_HUDELEMENT(CHudResourceAssaultCannonCharge);
// Jump Gun is currently disabled in the normal loadout.
// DECLARE_HUDELEMENT(CHudResourceJumpgunCharge);

CHudResourceSniperCharge::CHudResourceSniperCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_SNIPERRIFLE,
		"#HudPanel_SniperCharge",
		true)
{
}

CHudResourceAssaultCannonCharge::CHudResourceAssaultCannonCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_ASSAULTCANNON,
		"#HudPanel_AssaultCannonCharge",
		true)
{
}

// CHudResourceJumpgunCharge::CHudResourceJumpgunCharge(const char* pElementName)
// 	: CHudResourceWeaponCharge(
// 		pElementName,
// 		FF_WEAPON_JUMPGUN,
// 		"#HudPanel_JumpgunCharge",
// 		true)
// {
// }

CHudResourceWeaponCharge::CHudResourceWeaponCharge(
	const char* pElementName,
	int iWeaponID,
	const char* pszTitle,
	bool bShowPanel)
	: CHudElement(pElementName), BaseClass(NULL, pElementName),
	m_iWeaponID(iWeaponID),
	m_pszTitle(pszTitle),
	m_bDefaultShowPanel(bShowPanel),
	m_showPanel(bShowPanel ? WHEN_ACTIVE : NEVER)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);
}

void CHudResourceWeaponCharge::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	const char* szLabel
		= m_iWeaponID == FF_WEAPON_ASSAULTCANNON
			? "#HudPanel_WindUp"
			: "#HudPanel_Charge";

	m_pCharge = AddItem("Charge");
	m_pCharge->SetLabel(szLabel);
	m_pCharge->SetIcon('f');
	m_pCharge->SetAmountFormat(FORMAT_PERCENTAGE);

	switch (m_iWeaponID)
	{
	case FF_WEAPON_SNIPERRIFLE:
		m_pCharge->SetAmountMax(FF_SNIPER_MAXCHARGE);
		m_pCharge->SetIcon('l');
		SetHeaderIconChar('x');
		break;

	case FF_WEAPON_ASSAULTCANNON:
		m_pCharge->SetAmountMax(FF_AC_WINDUPTIME);
		m_pCharge->SetIcon('b');
		SetHeaderIconChar('z');
		break;

	case FF_WEAPON_JUMPGUN:
		m_pCharge->SetAmountMax(JUMPGUN_CHARGEUPTIME);
		m_pCharge->SetIcon('j');
		break;
	}


	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_pszTitle,
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceWeaponCharge::VidInit()
{
	m_pCharge->SetAmount(0);


}

void CHudResourceWeaponCharge::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	const wchar_t* pszTitle
		= g_pVGuiLocalize->Find(m_pszTitle);

	if (pszTitle)
		SetHeaderText(pszTitle);

	if (m_iWeaponID != FF_WEAPON_ASSAULTCANNON)
		return;

	wchar_t* pszAnchored
		= g_pVGuiLocalize->Find("#HudPanel_Anchored");

	if (pszAnchored)
		SetText(pszAnchored);

	SetUseToggleText(true);
}

void CHudResourceWeaponCharge::AddPanelSpecificOptions(
	KeyValues* kvOptions)
{
	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_VisibilityNever");
	kvVisibility->SetString("1", "#HudPanel_VisibilityWhenActive");
	kvVisibility->SetString("2", "#HudPanel_VisibilityWhileEquipped");

	AddComboOption(
		kvOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvVisibility,
		m_bDefaultShowPanel ? WHEN_ACTIVE : NEVER,
		0);

	AddItemOption(
		kvOptions,
		"Charge",
		m_iWeaponID == FF_WEAPON_ASSAULTCANNON ? "#HudPanel_WindUp" : "#HudPanel_Charge",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_PERCENTAGE
			}
		});
}

KeyValues* CHudResourceWeaponCharge::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("x", 320);
	kvPreset->SetInt("y", 255);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_TOP);
	kvPreset->SetInt("showPanel", 0);
	if (m_iWeaponID == FF_WEAPON_SNIPERRIFLE)
		kvPreset->SetInt("showPanelBorder", 0);

	kvPreset->SetInt("showHeaderText", 0);
	kvPreset->SetInt("showHeaderIcon", 0);
	kvPreset->SetInt("showText", 0);
	kvPreset->SetInt("barWidth", 60);
	kvPreset->SetInt("barHeight", 4);
	kvPreset->SetInt("barBorderWidth", 1);
	kvPreset->FindKey("Icon")->SetInt("show", 0);
	kvPreset->FindKey("Label")->SetInt("show", 0);
	kvPreset->FindKey("Amount")->SetInt("show", 0);
	kvPreset->FindKey("Bar")->SetInt("colorMode", COLOR_MODE_FADED);
	kvPreset->FindKey("Bar")->SetInt("alpha", 150);
	kvPreset->FindKey("BarBorder")->SetInt("show", 1);
	kvPreset->FindKey("BarBorder")->SetInt("alpha", 200);
	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
	{
		kvPreset->SetInt("x", 580);
		kvPreset->SetInt("y", 325);
		kvPreset->SetInt("alignH", ALIGN_RIGHT);
		kvPreset->SetInt("alignV", ALIGN_BOTTOM);
		kvPreset->SetInt("showPanel", 1);
		kvPreset->SetInt("showHeaderText", 1);
		kvPreset->SetInt("showText", 1);
		kvPreset->SetInt("barWidth", 80);
		kvPreset->SetInt("barHeight", 10);
		kvPreset->FindKey("Label")->SetInt("show", 1);
		kvPreset->FindKey("Amount")->SetInt("show", 1);
	}

	KeyValues* kvValues = new KeyValues("PanelSpecificValues");
	kvValues->SetBool("ShowPanel", m_bDefaultShowPanel);
	kvValues->SetInt("ValueFormat", FORMAT_PERCENTAGE);

	kvPreset->AddSubKey(kvValues);
	return kvPreset;
}

void CHudResourceWeaponCharge::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaults)
{
	BaseClass::ApplyStyleData(
		kvStyleData,
		kvDefaults);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaults->FindKey("PanelSpecificValues", true);

	std::optional<PanelVisibility> showPanel
		= GetPanelVisibility(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (showPanel.has_value())
		m_showPanel = showPanel.value();

	std::optional<AmountFormat> chargeFormat
		= GetAmountFormat(
			"ValueFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (chargeFormat.has_value())
	{
		m_pCharge->SetAmountFormat(
			chargeFormat.value());
	}
}

std::optional<CHudResourceWeaponCharge::PanelVisibility> CHudResourceWeaponCharge::GetPanelVisibility(
	const char* keyName,
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	int value
		= GetInt(
			keyName,
			kvStyleData,
			kvDefaultStyleData,
			-1);

	return value == -1
		? std::optional<PanelVisibility>()
		: static_cast<PanelVisibility>(value);
}

bool CHudResourceWeaponCharge::IsEquipped()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || FF_IsPlayerSpec(pPlayer) || !FF_HasPlayerPickedClass(pPlayer))
		return false;

	CFFWeaponBase* pWeapon = pPlayer->GetActiveFFWeapon();
	if (!pWeapon || pWeapon->GetWeaponID() != m_iWeaponID)
		return false;

	return true;
}

float CHudResourceWeaponCharge::GetCharge()
{
	CFFWeaponBase* pWeapon
		= C_FFPlayer::GetLocalFFPlayer()
		->GetActiveFFWeapon();

	float flCharge = 0;

	switch (m_iWeaponID)
	{
	case FF_WEAPON_SNIPERRIFLE:
	{
		CFFWeaponSniperRifle* pRifle
			= static_cast<CFFWeaponSniperRifle*>(pWeapon);

		// Charge starts at the rifle's minimum damage while the shot is held.
		if (!pRifle->IsInFire())
		{
			flCharge = 0.0f;
		}
		else
		{
			flCharge
				= clamp(
					gpGlobals->curtime - pRifle->GetFireStartTime(),
					1.0f,
					FF_SNIPER_MAXCHARGE);
		}
		break;
	}

	case FF_WEAPON_ASSAULTCANNON:
		flCharge = static_cast<CFFWeaponAssaultCannon*>(pWeapon)->m_flChargeTime;
		break;

	case FF_WEAPON_JUMPGUN:
		flCharge = static_cast<CFFWeaponJumpgun*>(pWeapon)->GetClampedCharge();
		break;
	}

	return clamp(
		flCharge,
		0.0f,
		m_pCharge->GetAmountMax());
}

void CHudResourceWeaponCharge::UpdateCharge()
{
	if (m_showPanel == NEVER)
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
		return;
	}

	bool bIsAssultCannon
		= m_iWeaponID == FF_WEAPON_ASSAULTCANNON;

	if (IsInPreviewMode())
	{
		SetHiddenBits(0);

		if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
			SetToggleTextVisible(true);

		m_pCharge->SetAmount(m_pCharge->GetAmountMax() * 0.6f);
		return;
	}

	if (!engine->IsInGame() || !IsEquipped())
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
		return;
	}

	const bool bAnchored
		= bIsAssultCannon
			&& C_FFPlayer::GetLocalFFPlayer()
				->m_bAssaultCannonAnchored;

	if (bIsAssultCannon)
		SetToggleTextVisible(bAnchored);

	const float flCharge = GetCharge();

	m_pCharge->SetAmount(flCharge);

	bool bHidden
		= m_showPanel == WHEN_ACTIVE
			&& flCharge <= 0.0f
			&& !bAnchored;

	SetHiddenBits(
		bHidden? HIDEHUD_ALWAYS : DEFAULT_HIDDEN_BITS);
}

bool CHudResourceWeaponCharge::ShouldDraw()
{
	UpdateCharge();

	return CHudElement::ShouldDraw();
}
