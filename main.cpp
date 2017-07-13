#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/GameReferences.h>
#include <SKSE/SafeWrite.h>
#include <SKSE/GameData.h>

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
#ifdef _DEBUG
		BaseExtraList* pExtraList = reinterpret_cast<BaseExtraList*>(this);
		_MESSAGE("[BARTER] COINS: %d", pExtraList->GetItemCount()); //Signed 16 bits, so when above 2 << 15,it will be negative.we use 32 bits to store this value instead.
#endif
		TESObjectREFR* ref = nullptr;
		void(__cdecl* LookUpRefByHandle)(void*, TESObjectREFR*&) = (void(__cdecl*)(void*, TESObjectREFR*&))0x004A9180;//LookUpRefByHandle
		LookUpRefByHandle((void*)0x01B3E518, ref); // 1B3E764 container, 1B3E518 Barter
		if (ref != nullptr)
		{
			InventoryChanges*(__cdecl* GetInventoryChanges)(TESObjectREFR* ref) = (InventoryChanges*(__cdecl*)(TESObjectREFR* actor))0x00476800;
			auto pChanges = GetInventoryChanges(ref);
			if (pChanges != nullptr)
			{
				TESForm* pGold = LookupFormByID(0xF);// No need to use DefaultObjectManager.
				if (pGold != nullptr)
				{
					UInt32 totalGolds = BarterManager::GetSingleton()->GetMerchantGolds(); // Didn't test it too many times,should work as expected.
					UInt32 merchantGolds = abs(pChanges->GetItemCount(pGold));
					SInt32 contaienrGolds = static_cast<SInt32>(totalGolds - merchantGolds);  
#ifdef _DEBUG
					_MESSAGE("[BARTER] merchant: %p  totalGolds: %d  merchantGolds: %d  containerGolds: %d", ref, totalGolds, merchantGolds, contaienrGolds);
#endif // _DEBUG
					return (contaienrGolds > 0) ? contaienrGolds : merchantGolds;//In game,BarterMenu uses container's gold first,then use merchant's gold to trade.
				}
			}
		}
		return -1;
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