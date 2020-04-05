/*
	    File: CMIO_DPA_Sample_Server_Assistant.h
	Abstract: Server which handles all the IPC between the various Sample DAL PlugIn instances.
	 Version: 1.2

*/

#if !defined(__CMIO_DPA_Sample_Server_Assistant_h__)
#define __CMIO_DPA_Sample_Server_Assistant_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
#include "CMIO_DPA_Sample_Server_MIGInterface.h"
#include "CMIO_DPA_Sample_Server_Common.h"
#include "CMIO_DPA_Sample_Shared.h"

// CA Public Utility Includes
#include "CAMutex.h"
#include "CACFDictionary.h"
#include "CACFObject.h"
#include "CACFString.h"

// Standard Library Includes
#include <map>
#include <set>

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	class Device;
	
    class Assistant: private CMIO::DPA::Sample::MIGInterface
	{
	// Construction/Destruction
	protected:
									Assistant();
								    ~Assistant();

		CAMutex						mStateMutex;				// Controls access to SampleAssistant's state
		CACFBundle					mPlugInBundle;				// The Sample.plugIn bundle used for locating resources for device names

	// Basic Operations
	public:
		mach_port_t					GetPortSet() { return mPortSet; }

	private:
		mach_port_t					mPortSet;

	// Client Connection / Disconnection
	public:
        void                        ClientDied(mach_port_t clientPort);

    // MIGInterface Implementation
    public:
        virtual kern_return_t		Connect(mach_port_t servicePort, pid_t client, mach_port_t* clientSendPort) override;
        virtual kern_return_t		Disconnect(mach_port_t client) override;
        virtual kern_return_t		GetDeviceStates(mach_port_t client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length) override;
        virtual kern_return_t		GetProperties(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, CMIOObjectPropertyAddress matchAddress, CMIO::PropertyAddress** addresses, mach_msg_type_number_t* length) override;
        virtual kern_return_t		GetPropertyState(mach_port_t client, UInt64 guid, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length) override;
        virtual kern_return_t		SetPropertyState(mach_port_t client, UInt64 guid, UInt32 sendChangedNotifications, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length) override;
        virtual kern_return_t		GetControls(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length) override;
        virtual kern_return_t		SetControl(mach_port_t client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue) override;
        virtual kern_return_t		ProcessRS422Command(mach_port_t client, UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount) override;
        virtual kern_return_t		StartStream(mach_port_t client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		StopStream(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		GetControlList(mach_port_t client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length) override;
        virtual kern_return_t		StartDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		StopDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		DeckPlay(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		DeckStop(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) override;
        virtual kern_return_t		DeckJog(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed) override;
        virtual kern_return_t		DeckCueTo(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, UInt32 playOnCue) override;

	protected:
		Device&						GetDeviceByGUID(UInt64 guid);

	// Messages
	protected:
		void						SendDeviceStatesChangedMessage(mach_port_t destination);

	// Device Mangagement
    protected:
		typedef std::set<Device*>					Devices;
		Devices						mDevices;

	// Client Management
	protected:
		struct StreamSpecifier
		{
			StreamSpecifier(Device& device, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) : mDevice(device), mScope(scope), mElement(element) {}

			Device&						mDevice;
			CMIOObjectPropertyScope		mScope;
			CMIOObjectPropertyElement	mElement;

			struct Less : public std::binary_function<StreamSpecifier, StreamSpecifier, bool>
			{
				bool	operator()(const StreamSpecifier& specifier1, const StreamSpecifier& specifier2) const {  bool answer = false; if (&specifier1.mDevice != &specifier2.mDevice) { answer = &specifier1.mDevice < &specifier2.mDevice; } else if (specifier1.mScope != specifier2.mScope) { answer = specifier1.mScope < specifier2.mScope; } else { answer = specifier1.mElement < specifier2.mElement; } return answer; }
			};

			class DeviceEqual
			{
			public:
						DeviceEqual(Device& device) : mDevice(device) {};
				bool	operator()(const StreamSpecifier& streamSpecifier) const { return &streamSpecifier.mDevice == &mDevice; }
			
			private:
						Device& mDevice;
			};
			
			
		};
		typedef std::set<StreamSpecifier, StreamSpecifier::Less> StreamSpecifiers;				 
		
		struct ClientInfo
		{
			ClientInfo(pid_t pid) : mPID(pid) {}
			pid_t					mPID;									// The process ID of the client
			StreamSpecifiers		mStreamSpecifiers;						// Set of streams client is streaming
		};
		typedef std::map<Client, ClientInfo*>		ClientInfoMap;			// Map Client to ClientInfo		
		typedef std::multimap<Client, mach_port_t>  ClientNotifiers;		// Clients Mach ports to message in response to various events

		ClientInfoMap				mClientInfoMap;
		ClientNotifiers				mDeviceStateNotifiers;					// Clients to notify when devices arrive / disappear
	};
}}}}
#endif
