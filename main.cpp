#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/GameReferences.h>
#include <SKSE/SafeWrite.h>
#include <SKSE/GameData.h>
#include <SKSE/GameMenus.h>

class BarterManager
{
public:
	static BarterManager* GetSingleton()
	{
		TESDataHandler* pDataHanler = TESDataHandler::GetSingleton();
		return (pDataHanler) ? *(BarterManager**)((UInt32)pDataHanler + 0xAAC) : nullptr;
	}

	//DEFINE_MEMBER_FN(GetBarterGoldInfo, Data::Info*, 0x00477B20, TESForm* form, UInt32 arg1, UInt32 formID);
	DEFINE_MEMBER_FN(GetMerchantGolds, UInt32, 0x0047AB00);
};


struct BaseExtraListEx
{
	UInt32	GetGoldCount()
	{
		UInt32	result = 0;
		BaseExtraList* pExtraList = reinterpret_cast<BaseExtraList*>(this);

		MenuManager* mm = MenuManager::GetSingleton();
		UIStringHolder* pHolder = UIStringHolder::GetSingleton();
		GFxMovieView* view = nullptr;
		if (mm->IsMenuOpen(pHolder->barterMenu))
		{
			view = mm->GetMovieView(pHolder->barterMenu);
		}
		if (view != nullptr)
		{
			GFxValue result(true);
			view->Invoke("_root.Menu_mc.isViewingVendorItems", &result, nullptr, 0);
			//_MESSAGE("[BARTER] isViewingVendorItems: %d", result.GetBool());
			if (!result.GetBool())
			{
				RefHandle refHandle = *(RefHandle*)0x01310630;
				pExtraList->GetOriginalReferenceHandle(refHandle);

				TESObjectREFR*	ref = nullptr;
				LookupREFRByHandle(refHandle, ref);

				if (ref != nullptr)
				{
					TESForm* baseForm = ref->baseForm;
					if (baseForm->Is(FormType::NPC) || baseForm->Is(FormType::Container))
					{
						InventoryChanges* (__cdecl* GetInventoryChanges)(TESObjectREFR* ref) = (InventoryChanges*(__cdecl*)(TESObjectREFR* actor))0x00476800;
						auto pChanges = GetInventoryChanges(ref);
						auto pGold = LookupFormByID(0xF);
						if (pChanges != nullptr && pGold != nullptr)
						{
							return pChanges->GetItemCount(pGold);
						}
					}
				}
			}
		}
		return static_cast<SInt32>(pExtraList->GetItemCount());
	}

	static void InitHook()
	{
		WriteRelCall(0x0043DB3A, &GetGoldCount);
		SafeWrite8(0x0043DB3F, 0x90);
	}
};



class BarterFix : public SKSEPlugin
{
public:
	BarterFix()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_3, SKSEMessagingInterface::Version_2, SKSETaskInterface::Version_2))
		{
			gLog << "ERROR: your skse version is too old." << std::endl;
			return false;
		}

		SetName(GetDllName());
		SetVersion(1);

		return true;
	}

	virtual bool OnLoad() override
	{
		SKSEPlugin::OnLoad();

		return true;
	}

	virtual void OnModLoaded() override
	{
		BaseExtraListEx::InitHook();
	}
} thePlugin;