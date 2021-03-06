//--------------------------------------------------------------------------------
// Written by Justin Murphy
//--------------------------------------------------------------------------------
#include "pch.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Win32/AkPlatformFuncs.h>
#include <AkDefaultIOHookBlocking.h>

#include "./AudioSystemWwise.h"

#include "./Log.h"
//--------------------------------------------------------------------------------
#pragma comment(lib, "AkSoundEngineDLL.lib")
//--------------------------------------------------------------------------------
using namespace AK;
//--------------------------------------------------------------------------------
AudioSystemWwise* AudioSystemWwise::m_spAudioSystem = nullptr;
//--------------------------------------------------------------------------------
AudioSystemWwise::AudioSystemWwise() :
	m_fWorldScale(1.f),
	m_bInitialized(false) {
	if ( m_spAudioSystem == nullptr )
		m_spAudioSystem = this;
}
//--------------------------------------------------------------------------------
AudioSystemWwise::~AudioSystemWwise() {}
//--------------------------------------------------------------------------------
AudioSystemWwise* AudioSystemWwise::Get() {
	return m_spAudioSystem;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::Initialize() {
	// Initialize audio engine
	// Memory.
	AkMemSettings memSettings;
	memSettings.uMaxNumPools = 40;

	// Streaming.
	AkStreamMgrSettings stmSettings;
	// 	stmSettings.uMemorySize = 32 * 1024*1024; // 32 MB
	StreamMgr::GetDefaultSettings( stmSettings );
	
	AkDeviceSettings deviceSettings;
	StreamMgr::GetDefaultDeviceSettings( deviceSettings );

	AkInitSettings l_InitSettings;
	AkPlatformInitSettings l_platInitSetings;
	SoundEngine::GetDefaultInitSettings( l_InitSettings );
	SoundEngine::GetDefaultPlatformInitSettings( l_platInitSetings );

	// Setting pool sizes for this game.
	// These sizes are tuned for this game, every game should determine its own optimal values.
	//	l_InitSettings.uDefaultPoolSize           = 4 * 1024 * 1024;  // 4 MB
	//	l_InitSettings.uMaxNumPaths               = 16;
	//	l_InitSettings.uMaxNumTransitions         = 128;
	//	l_platInitSetings.uLEngineDefaultPoolSize    = 4 * 1024 * 1024;  // 4 MB
	//l_platInitSetings.hWnd = hwnd;

	AkMusicSettings musicInit;
	MusicEngine::GetDefaultInitSettings( musicInit );
	musicInit.fStreamingLookAheadRatio = 100;

	AKRESULT eResult = SOUNDENGINE_DLL::Init( &memSettings,
		&stmSettings,
		&deviceSettings,
		&l_InitSettings,
		&l_platInitSetings,
		&musicInit );

	if( eResult != AK_Success ) {
		// Then, we will run the game without sound
		SOUNDENGINE_DLL::Term();
		return false;
	}
	
	// This is a default base path. The game programmer should override this when the game is initialized.
	SOUNDENGINE_DLL::SetBasePath( L"../Resources/soundbanks/" );
	//SOUNDENGINE_DLL::SetLangSpecificDirName( L"English(US)/" );

	// register 64 dummy game objects for one-shot positioning. See PostEvent(ID, location) function for details
	for ( int i = 64; i < 128; ++i ) {
		char name[256];
		sprintf_s( name, 256, "Dummy #%i", i-63 );
		SoundEngine::RegisterGameObj( i, name );
	}

	m_bInitialized = true;
	Log("AudioSystemWwise has been Initialized");
	return true;
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::Shutdown() {
	SoundEngine::UnregisterAllGameObj();
	// Terminate audio engine
	SOUNDENGINE_DLL::Term();
	Log("AudioSystemWwise has been Shutdown");
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::Update() {
	//AkListenerPosition listenerPosition;
//	AkSoundPosition soundPosition;

	//// update listener positions and orientations
	//for( unsigned int i = 0; i < m_RegisteredListeners.size(); ++i) {
	//	const XMFLOAT3 * pos = m_RegisteredListeners[i]->GetPosition();
	//	
	//	listenerPosition.Position.X = pos->x;
	//	listenerPosition.Position.Y = pos->y;
	//	listenerPosition.Position.Z = pos->z;
	//
	//	const XMFLOAT3 * zAxis = m_RegisteredListeners[i]->GetZAxis();
	//	listenerPosition.OrientationFront.X = zAxis->x;
	//	listenerPosition.OrientationFront.Y = zAxis->y;
	//	listenerPosition.OrientationFront.Z = zAxis->z;
	//
	//	const XMFLOAT3 * yAxis = m_RegisteredListeners[i]->GetYAxis();
	//	listenerPosition.OrientationTop.X = yAxis->x;
	//	listenerPosition.OrientationTop.Y = yAxis->y;
	//	listenerPosition.OrientationTop.Z = yAxis->z;

	//	SoundEngine::SetListenerPosition(listenerPosition,i);
	//}
	//
	//// set entities positions and orientations
	//for ( UINT i = 0; i < m_RegisteredEntities.size(); ++i) {
	//	const XMFLOAT3 * pos = m_RegisteredEntities[i]->GetPosition();
	//	soundPosition.Position.X = pos->x;
	//	soundPosition.Position.Y = pos->y;
	//	soundPosition.Position.Z = pos->z;
	//
	//	 XMFLOAT3  zAxis = *m_RegisteredEntities[i]->GetZAxis();

	//	 DirectX::XMStoreFloat3(&zAxis, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&zAxis)));
	//	soundPosition.Orientation.X = zAxis.x;
	//	soundPosition.Orientation.Y = zAxis.y;
	//	soundPosition.Orientation.Z = zAxis.z;
	//	
	//	SoundEngine::SetPosition( (AkGameObjectID) m_RegisteredEntities[i], soundPosition );
	//}
	// render the audio
	SOUNDENGINE_DLL::Tick();
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::RegisterEntity(const IEntity* entity, const char* name) {
	if ( nullptr == name ) {
		static int entityNum = 0;
		std::stringstream ssNum;
		ssNum << ++entityNum;
		std::string strTemp = "entity" + ssNum.str();

		if(SoundEngine::RegisterGameObj( (AkGameObjectID)entity, strTemp.c_str() ) != AK_Success) {
			Log("*Audio Error*\n Registering : \"", strTemp.c_str(), "\"\n\n");
			return false;
		}
	}
	else {
		if(SoundEngine::RegisterGameObj( (AkGameObjectID)entity, name ) != AK_Success) {
			Log("*Audio Error*\n Registering audio entity", "\n\nContact the audio programmer and replicate error situation.");
			return false;
		}
	}

	m_RegisteredEntities.push_back( entity );
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::UnRegisterEntity(const IEntity* entity) {
	for ( unsigned int i = 0; i < m_RegisteredEntities.size(); ++i ) {
		if ( m_RegisteredEntities[i] == entity ) {
			m_RegisteredEntities.erase( m_RegisteredEntities.begin() + i );
			if( SoundEngine::UnregisterGameObj( (AkGameObjectID) entity ) != AK_Success) {
				Log("*Audio Error*\n Unegistering audio entity", "\n\nContact the audio programmer and replicate error situation.");
				return false;
			}
			break;
		}
	}
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::RegisterListener(const IEntity* entity, const char* name) {
	if ( nullptr == name ) {
		std::stringstream ssNum;
		ssNum << (m_RegisteredListeners.size()+1);
		std::string strTemp = "listener" + ssNum.str();

		if(SoundEngine::RegisterGameObj( (AkGameObjectID)entity, strTemp.c_str() ) != AK_Success) {
			Log("*Audio Error*\n Registering: \"", strTemp, "\"\n\nContact the audio programmer and replicate error situation.");
			return false;
		}
	}
	else {
		if(SoundEngine::RegisterGameObj( (AkGameObjectID)entity, name ) != AK_Success) {
			Log("*Audio Error*\n Registering audio entity", L"\n\nContact the audio programmer and replicate error situation.");
			return false;
		}
	}
	
	// ensure to update listeners scaling factor to the worlds
	AK::SoundEngine::SetListenerScalingFactor(static_cast<AkUInt32>(m_RegisteredListeners.size()),m_fWorldScale);
	m_RegisteredListeners.push_back( entity );
	m_RegisteredEntities.push_back( entity );
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::UnRegisterListener(const IEntity* entity) {
	for ( unsigned int i = 0; i < m_RegisteredListeners.size(); ++i ) {
		if ( m_RegisteredListeners[i] == entity ) {
			m_RegisteredListeners.erase( m_RegisteredListeners.begin() + i );

			if( SoundEngine::UnregisterGameObj( (AkGameObjectID) entity ) != AK_Success) {
				Log("*Audio Error*\n Unegistering audio entity", "\n\nContact the audio programmer and replicate error situation.");
				return false;
			}
			break;
		}
	}
	return UnRegisterEntity(entity);
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::PostEvent(AudioEvent eventId,const IEntity* eventCaller) {
#ifdef _DEBUG
	bool found = false;
	for each( auto entity in m_RegisteredEntities ) {

		if (entity == eventCaller) {
			found = true;
			break;
		}
	}
	if (!found) {
		Log(L"*Audio Error* PostEvent failed Entity not registered.");
		return false;
	}
#endif

	if(SoundEngine::PostEvent( static_cast<AkUniqueID>(eventId), (AkGameObjectID)eventCaller ) == AK_INVALID_PLAYING_ID) {
		Log(L"*Audio Error* PostEvent failed EventID might be incorrect.\n", "Make sure you're loading the soundbanks before attempting to play a sound.");
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::PostEvent(AudioEvent eventId,const XMFLOAT3& pos)
{
	// cycle through the dummy game objects
	// number of dummies needs to be big enough to avoid setting a different position
	// on the same object twice in the same game frame (64 is probably overkill in this game)
	static int iGameObj = 63;
	if (127 < ++iGameObj)
		iGameObj = 64;

	AkSoundPosition soundPosition;
	soundPosition.Orientation.X = 0;
	soundPosition.Orientation.Y = 0;
	soundPosition.Orientation.Z = 1.0f;
	
	soundPosition.Position.X = pos.x;
	soundPosition.Position.Y = pos.y;
	soundPosition.Position.Z = pos.z;

	SoundEngine::SetPosition( iGameObj, soundPosition );
	if(SoundEngine::PostEvent(static_cast<AkUniqueID>(eventId), iGameObj ) == AK_INVALID_PLAYING_ID) {
		Log(L"*Audio Error* PostEvent failed EventID might be incorrect.\n", L"Make sure you're loading the soundbanks before attempting to play a sound.");
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::PostEvent(AudioEvent eventId, unsigned int listenerId) {
	if( (m_RegisteredListeners.size() - listenerId) < 0) {
		Log(L"*Audio Error* The listener is not registered.", L"You cannot play an event on the listener if there is no listener.");
		return false;
	}

	if(SoundEngine::PostEvent(static_cast<AkUniqueID>(eventId), (AkGameObjectID)m_RegisteredListeners[listenerId]) == AK_INVALID_PLAYING_ID) {
		Log(L"*Audio Error* PostEvent failed EventID might be incorrect.", "Make sure you're loading the soundbanks before attempting to play a sound.");
	}
	return true;
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetMaterialID(const IEntity* pEntity, AkUniqueID akMaterialSwitchDefine, const unsigned int nMaterialID) {}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetRTCPValue(const char* szRTCPName, const AkRtpcValue akRtcpVal) {
	if(SoundEngine::SetRTPCValue( szRTCPName, akRtcpVal) != AK_Success) {
		Log("*Audio Error*\n RTCP Value fail on: \"", szRTCPName, L"\"\n\nRTCP name might be incorrect.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetRTCPValue(const char* szRTCPName, const AkRtpcValue akRtcpVal, const IEntity* pEntity) {
	if(SoundEngine::SetRTPCValue( szRTCPName, akRtcpVal, (AkGameObjectID)pEntity) != AK_Success) {
		Log("*Audio Error*\n RTCP Value fail on: \"", szRTCPName, L"\"\n\nRTCP name might be incorrect or the associated entity might not be registered.\n\nNOTE: Make sure you're loading the soundbanks before attempting to play a sound.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetRTCPValue(const AkRtpcID akRtcpID, const AkRtpcValue akRtcpVal) {
	if(SoundEngine::SetRTPCValue(akRtcpID, akRtcpVal) != AK_Success) {
		Log("*Audio Error*\n RTCP Value fail on: \"", akRtcpID, L"\"\n\nRTCP ID might be incorrect.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetRTCPValue(const AkRtpcID akRtcpID, const AkRtpcValue akRtcpVal, const IEntity* pEntity) {
	if(SoundEngine::SetRTPCValue(akRtcpID, akRtcpVal, (AkGameObjectID)pEntity) ) {
		Log("*Audio Error*\n RTCP Value fail on: \"", akRtcpID, L"\"\n\nRTCP ID might be incorrect or the associated entity might not be registered.\n\nNOTE: Make sure you're loading the soundbanks before attempting to play a sound.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetState(const char* szStateGroupName, const char* szStateName) {
	if(AK::SoundEngine::SetState(szStateGroupName, szStateName) != AK_Success) {
		Log("*Audio Error*\n Setting State Failure.\nState Group Name: \"", 
			szStateGroupName,
			L"\"\nState Name: ", 
			szStateName, 
			"\n\nState Group or State Name might not exist.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetState(const AkStateGroupID akStateGroupID, const AkStateID akStateID) {
	if(AK::SoundEngine::SetState(akStateGroupID, akStateID ) != AK_Success) {
		Log("*Audio Error*\n Setting State Failure.\nState Group ID: \"", 
			akStateGroupID,
			L"\"\nState ID: \"", 
			akStateID,
			"\"\n\nState Group or State Name might not exist.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetSwitch(const AkSwitchGroupID akSwitchGroupID, const AkSwitchStateID akStateID, const IEntity* pEntity) {
	if (AK::SoundEngine::SetSwitch(akSwitchGroupID, akStateID, (AkGameObjectID)pEntity) != AK_Success) {
		Log("*Audio Error*\n Setting Switch Failure.\nState Group ID: \"", 
			akSwitchGroupID,
			"\"\nState ID: \"", 
			akStateID, 
			"\"\nSwitch Group or Switch Name might not exist.");
	}
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::PostTrigger(const char* szTriggerName) {
	AK::SoundEngine::PostTrigger(szTriggerName, AK_INVALID_GAME_OBJECT );
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::PostTrigger(const AkTriggerID akTriggerID) {
	AK::SoundEngine::PostTrigger(akTriggerID, AK_INVALID_GAME_OBJECT );
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::PostTrigger(const char* szTriggerName, const IEntity* pEntity) {
	AK::SoundEngine::PostTrigger(szTriggerName, (AkGameObjectID)pEntity );
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::PostTrigger(const AkTriggerID akTriggerID, const IEntity* pEntity) {
	AK::SoundEngine::PostTrigger(akTriggerID, (AkGameObjectID)pEntity );
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetBasePath(const wchar_t* strPath) {
	SOUNDENGINE_DLL::SetBasePath( strPath );
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::LoadSoundBank(const wchar_t* szBankName) {
	AkBankID bankID;
	if ( SoundEngine::LoadBank( szBankName , AK_DEFAULT_POOL_ID, bankID ) == AK_Success )
		m_RegisteredSoundBanks[szBankName] = bankID;
	else {
		Log(L"*Audio Error*\n Loading sound bank.\nTry checking your soundbank names.");
		return false;
	}

	Log("Loaded SoundBank: ", szBankName);
	return true;
}
//--------------------------------------------------------------------------------
bool AudioSystemWwise::UnloadSoundBank(const wchar_t* szBankName) {
	if (m_RegisteredSoundBanks.find(szBankName) != m_RegisteredSoundBanks.end() ) {

		if (SoundEngine::UnloadBank(m_RegisteredSoundBanks[szBankName], nullptr, nullptr) != AK_Success) {
			Log(L"*Audio Error*\n Unloading sound bank.\nTry checking your soundbank names.");
			return false;
		}
		else {
			m_RegisteredSoundBanks.erase(szBankName);
		}
	}
	else {
		Log(L"*Audio Error*\n Unloading sound bank.\nSoundbank not in loaded banks.");
		return false;
	}
		
	return true;
}
//--------------------------------------------------------------------------------
void AudioSystemWwise::SetWorldScale(float fScale) {
	m_fWorldScale = fScale;
	for(unsigned int i = 0; i < m_RegisteredListeners.size(); ++i) {
		AK::SoundEngine::SetListenerScalingFactor(i,m_fWorldScale);
	}
}
//--------------------------------------------------------------------------------
#ifdef _DEBUG
void AudioSystemWwise::SendMessage(const char* message) {
	Monitor::PostString( message, Monitor::ErrorLevel_Message );
}
#endif
//--------------------------------------------------------------------------------