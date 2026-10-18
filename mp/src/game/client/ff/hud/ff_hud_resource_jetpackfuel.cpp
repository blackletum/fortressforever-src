#include "cbase.h"

#include "ff_hud_resource_jetpackfuel.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceJetpackFuel);

CHudResourceJetpackFuel::CHudResourceJetpackFuel(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceJetpackFuel"),
	m_wsFull(L"Full"),
	m_wsRegenerating(L"Regenerating...")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderText(L"Jetpack");
	SetHeaderIconChar('h');
	SetUseToggleText(true);
	SetText(m_wsRegenerating);

	m_qiJetpackFuel = nullptr;
}

CHudResourceJetpackFuel::~CHudResourceJetpackFuel()
{
}

void CHudResourceJetpackFuel::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_JetpackFuel");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_InUse");
	if (localized)
		m_wsInUse = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Recovering");
	if (localized)
		m_wsRecovering = localized;
}

void CHudResourceJetpackFuel::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddBooleanOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		true,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_ShowText",
		GetQuantityVisibilityOptions(),
		false,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideWhenFull",
		"#HudPanel_Option_HideWhenFull",
		false,
		0);

	AddItemOption(
		kvPanelSpecificOptions,
		"Fuel",
		"#FF_ITEM_FUEL",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});
}

KeyValues* CHudResourceJetpackFuel::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideWhenFull", false);
	kvPanelSpecificValues->SetBool("HideText", false);

	kvPanelSpecificValues->SetInt("ValueFormat", FORMAT_OVER_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourceJetpackFuel::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	FFQuantityPanel::ApplyStyleData(
		kvStyleData,
		kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	int iHideWhenFull = GetInt("HideWhenFull", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideWhenFull != -1)
		m_bHideWhenFull = iHideWhenFull == 1;

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iHideText
		= GetInt(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountFormat> valueFormat
		= GetAmountFormat(
			"ValueFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowPanel != -1)
	{
		m_bShowPanel = (iShowPanel == 1);

		if (m_bShowPanel)
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}

	if (iHideText != -1)
	{
		if (iHideText == 1)
			SetUseToggleText(false);
		else
			SetUseToggleText(true);
	}

	if (valueFormat.has_value())
		m_qiJetpackFuel->SetAmountFormat(
			valueFormat.value());

	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
		{
			SetHiddenBits(0);
		}
		else
		{
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
		}
	}
}

void CHudResourceJetpackFuel::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiJetpackFuel = AddItem("HudResourceJetpackFuel");
	m_qiJetpackFuel->SetLabel("#FF_ITEM_FUEL");
	m_qiJetpackFuel->SetIcon('m');
	m_qiJetpackFuel->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiJetpackFuel->SetAmountMax(JETPACK_MAXFUEL);

	AddPanelToHudOptions(
		"JetpackFuel",
		"#HudPanel_JetpackFuel",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceJetpackFuel::VidInit()
{
	m_qiJetpackFuel->SetAmount(0);
	SetText(m_wsRegenerating);
}

bool CHudResourceJetpackFuel::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (!engine->IsInGame())
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer)
		return false;

	if (m_bHideWhenFull && pPlayer->m_iJetpackFuel >= JETPACK_MAXFUEL)
		return false;

	return pPlayer->GetClassSlot() == CLASS_PYRO
		&& !FF_IsPlayerSpec(pPlayer)
		&& FF_HasPlayerPickedClass(pPlayer)
		&& pPlayer->m_bCanUseJetpack;
}

void CHudResourceJetpackFuel::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		m_qiJetpackFuel->SetAmount(pPlayer->m_iJetpackFuel);

		if (pPlayer->IsJetpacking())
			SetText(m_wsInUse);
		else if (pPlayer->m_iJetpackFuel >= JETPACK_MAXFUEL)
			SetText(m_wsFull);
		else if (gpGlobals->curtime < pPlayer->m_flJetpackCooldown)
			SetText(m_wsRecovering);
		else
			SetText(m_wsRegenerating);
	}

	BaseClass::Paint();
}
