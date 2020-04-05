/*
	    File: CMIO_DPA_Sample_ClientExtras.h
	Abstract: Extra utility functions used by clients of the SampleAssistant.
	 Version: 1.2

*/

#if !defined(__CMIO_DPA_Sample_ClientExtras_h__)
#define __CMIO_DPA_Sample_ClientExtras_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// MIG Client Interface
#include "CMIODPASampleClient.h"

// Internal Includes
#include "CMIO_DPA_Sample_Shared.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"

// System Includes
#include <mach/mach_port.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

namespace CMIO { namespace DPA { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Port
	//	Wraps a port used to talk to the Assistant.  When destroyed, it will automatically disconnect from the Assistant and deallocate the Mach port it was using to send it messages.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Port
	{
	// Construction/Destruction
	public:
					explicit Port(mach_port_t port = MACH_PORT_NULL) : mPort(MACH_PORT_NULL) { Reset(port); }
					~Port() { Reset(); }
		void		Reset(mach_port_t port = MACH_PORT_NULL) { if (MACH_PORT_NULL != mPort) { CMIODPASampleDisconnect(mPort); mach_port_deallocate(mach_task_self(), mPort); } mPort = port; }
	
	private:
		mach_port_t	mPort;
		Port&		operator=(Port& rhs);	// Don't allow copying
		
	// Value Access
	public:
		operator	mach_port_t() const { return mPort; }
		mach_port_t	Get() const { return mPort; }													
	};
	
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AutoFreeUnboundedArray
	//	A helper class for dealing with MIG unbounded arrays.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	template <typename T> class AutoFreeUnboundedArray
	{
	// Construction/Destruction
	public:
									AutoFreeUnboundedArray(mach_msg_type_number_t length = 0) : mLength(length), mStorage(0) { if (0 != mLength) { kern_return_t err =  vm_allocate(mach_task_self(), &mStorage, GetSize(), true); ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::AutoFreeUnboundedArray allocation failed"); }}
									~AutoFreeUnboundedArray() { if (0 != mStorage) vm_deallocate(mach_task_self(), mStorage, GetSize()); }
	private:
		mach_msg_type_number_t		mLength;
		vm_address_t				mStorage;
		
	// Value Access
	public:
		T&							operator[](size_t index) { return reinterpret_cast<T*>(mStorage)[index]; }
		mach_msg_type_number_t&		GetLength() { return mLength; }
		void						CorrectLength() { mLength /= sizeof(T); }	// Correct length from returned data in a CMIOSampleUnboundedByteArray (which reports byte length instead of element length)
		size_t						GetSize() { return mLength * sizeof(T); }
		T**							GetAddress() { return reinterpret_cast<T**>(&mStorage); }
		Byte**						GetByteAddress() { return reinterpret_cast<Byte**>(&mStorage); }
		void						SetAddress(vm_address_t storage) { mStorage = storage; }
	};

    mach_port_t			GetPort(const char* assistantServiceName);
	void				Disconnect(mach_port_t port);
	void				GetDeviceStates(mach_port_t port, mach_port_t messagePort, AutoFreeUnboundedArray<DeviceState>& deviceStates);
	void				GetProperties(mach_port_t port, UInt64 guid, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, AutoFreeUnboundedArray<PropertyAddress>& addresses);

	#pragma mark -
	#pragma mark Controls at the Device Level
	CFArrayRef			CopyControlList(mach_port_t clientSendPort, UInt64 guid);
	void				GetControls(mach_port_t port, UInt64 guid, mach_port_t messagePort, UInt64 time, AutoFreeUnboundedArray<ControlChanges>& controlChanges);
	
	#pragma mark -
	#pragma mark Properties
	void				GetProperties(mach_port_t clientSendPort, UInt64 guid, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, AutoFreeUnboundedArray<PropertyAddress>& addresses);

	#pragma mark -
	#pragma mark Device
	void				GetStreamConfiguration(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, AutoFreeUnboundedArray<UInt32>& configuration);
	bool				GetDeviceIsRunningSomewhere(mach_port_t port, UInt64 guid);
	pid_t				GetHogMode(mach_port_t port, UInt64 guid);
	pid_t				GetDeviceMaster(mach_port_t port, UInt64 guid);
	void				SetDeviceMaster(mach_port_t port, UInt64 guid, pid_t pid, bool sendChangedNotifications = true);
	bool				GetExcludeNonDALAccess(mach_port_t port, UInt64 guid);
	void				SetExcludeNonDALAccess(mach_port_t port, UInt64 guid, Boolean excludeNonDALAccess, bool sendChangedNotifications = true);
	bool				GetClientSyncDiscontinuity(mach_port_t port, UInt64 guid);
	void				SetClientSyncDiscontinuity(mach_port_t port, UInt64 guid, Boolean forceDiscontinuity, bool sendChangedNotifications = true);
	void				SetControl(mach_port_t port, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue);
	void				ProcessRS422Command(mach_port_t clientSendPort, UInt64 guid, CMIODeviceRS422Command& ioRS422Command);

	#pragma mark All Streams (Scopes & Elements are in regard to the owning Device)
	void				StartStream(mach_port_t port, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				StopStream(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);

	void				GetFormatDescriptions(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, AutoFreeUnboundedArray<FrameFormat>& frameFormats);
	FrameType			GetFormatDescription(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				SetFormatDescription(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, FrameType frameType);
	void				GetFrameRates(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, FrameType frameType, AutoFreeUnboundedArray<Float64>& frameRates);
	Float64				GetFrameRate(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				SetFrameRate(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 frameRate);

	#pragma mark Streams with Decks (Scopes & Elements are in regard to the owning Device)
	void				StartDeckThreads(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				StopDeckThreads(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	
	CMIOStreamDeck	GetDeck(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	Float64				GetDeckTimecode(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	SInt32				GetDeckCueing(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	
	void				DeckPlay(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				DeckStop(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	void				DeckJog(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed);
	void				DeckCueTo(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, bool playOnCue);

	#pragma mark Input Streams (Scopes & Elements are in regard to the owning Device)
	UInt32				GetNoDataTimeout(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element);
	void				SetNoDataTimeout(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element, UInt32 noDataTimeout, bool sendChangedNotifications = true);
	UInt32				GetDeviceSyncTimeout(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element);
	void				SetDeviceSyncTimeout(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element, UInt32 deviceSyncTimeout, bool sendChangedNotifications = true);

	#pragma mark Output Streams (Scopes & Elements are in regard to the owning Device)
	bool				GetEndOfData(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element);
	void				SetEndOfData(mach_port_t port, UInt64 guid, CMIOObjectPropertyElement element, UInt32 endOfData, bool sendChangedNotifications = true);

						// This is going way
	void				SetDeckStatusControl(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element, UInt32 statusControl, bool sendChangedNotifications = true);
}}}
#endif
