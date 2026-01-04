namespace MCM
{
	class Settings
	{
	public:
		class General
		{
		public:
			inline static REX::INI::Bool bActivateCONT{ "General", "bActivateCONT", false };
			inline static REX::INI::Bool bActivateDOOR{ "General", "bActivateDOOR", true };
		};

		static void Update()
		{
			Register();

			const auto ini = REX::INI::SettingStore::GetSingleton();
			ini->Init(
				"Data/MCM/Config/BakaNoLockpickActivate/settings.ini",
				"Data/MCM/Settings/BakaNoLockpickActivate.ini");
			ini->Load();
		}

	private:
		class EventHandler :
			public REX::Singleton<EventHandler>,
			public RE::BSTEventSink<RE::MenuOpenCloseEvent>
		{
		public:
			virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_sink) override
			{
				if (a_event.menuName == "PauseMenu" && !a_event.opening)
				{
					MCM::Settings::Update();
				}

				return RE::BSEventNotifyControl::kContinue;
			}
		};

		static void Register()
		{
			if (bRegistered)
			{
				return;
			}

			if (auto UI = RE::UI::GetSingleton())
			{
				UI->RegisterSink<RE::MenuOpenCloseEvent>(EventHandler::GetSingleton());
				bRegistered = true;
			}
		}

		inline static bool bRegistered{ false };
	};
}

namespace Hooks
{
	class hkActivateRef
	{
	private:
		static bool ActivateRef(
			RE::TESObjectREFR*  a_this,
			RE::TESObjectREFR*  a_actionRef,
			RE::TESBoundObject* a_objectToGet,
			std::int32_t        a_count,
			bool                a_defaultProcessingOnly,
			bool                a_fromScript,
			bool                a_looping)
		{
			if (!a_this)
			{
				return false;
			}

			if (!a_this->data.objectReference)
			{
				return false;
			}

			switch (a_this->data.objectReference->GetFormType())
			{
			case RE::ENUM_FORM_ID::kCONT:
				if (!MCM::Settings::General::bActivateCONT)
					return false;
				break;
			case RE::ENUM_FORM_ID::kDOOR:
				if (!MCM::Settings::General::bActivateDOOR)
					return false;
				break;
			default:
				break;
			}

			return _ActivateRef(a_this, a_actionRef, a_objectToGet, a_count, a_defaultProcessingOnly, a_fromScript, a_looping);
		}

		inline static REL::Hook _ActivateRef{ REL::ID(2249271), 0x21D, ActivateRef };
	};
}

namespace
{
	void MessageCallback(F4SE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case F4SE::MessagingInterface::kGameDataReady:
			if (static_cast<bool>(a_msg->data))
				MCM::Settings::Update();
			break;
		default:
			break;
		}
	}
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .trampoline = true, .trampolineSize = 16 });
	F4SE::GetMessagingInterface()->RegisterListener(MessageCallback);
	return true;
}
