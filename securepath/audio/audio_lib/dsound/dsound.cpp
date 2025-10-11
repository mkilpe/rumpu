#include "dsound.hpp"
#include <voice/audiolib/audio_device_modes.hpp>
#include <voice/audiolib/util.hpp>

#include <securepath/log/log.hpp>

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#include <Windows.h>
#include <mmsystem.h>
#include <DSound.h>

#ifdef _MSC_VER
#	pragma comment( lib, "Dsound.lib" )
#	pragma comment( lib, "dxguid.lib" )
#endif

namespace securepath {
namespace audio {
namespace {

using namespace std::literals;

struct dsound_device_info : audio_device_info {
public:
	dsound_device_info(LPGUID guid, LPCWSTR desc, bool play_device)
		: audio_device_info(play_device, securepath::to_string(desc), securepath::to_string(desc))
	{
		if(guid) {
			CopyMemory(&guid_, guid, sizeof(GUID));
		}
	}

	GUID guid_;
};

using enum_calltype = std::function<void (LPGUID, LPCWSTR)>;

BOOL CALLBACK enumerate_devices(LPGUID lpGuid, LPCWSTR lpszDesc, LPCWSTR lpszModule, LPVOID context) {
	(*static_cast<enum_calltype*>(context))(lpGuid, lpszDesc);
	return TRUE;
}

}

class dsound_play_device : public audio_play_device {
public:
	dsound_play_device(LPCGUID guid, device_config config)
		: config_(config)
		, started_()
		, pos_()
	{
		if(::DirectSoundCreate8(guid, &device_, 0) != DS_OK) {
			throw std::runtime_error("failed to create device");
		}
		if(device_->SetCooperativeLevel(::GetDesktopWindow(), DSSCL_NORMAL) != DS_OK) {
			throw std::runtime_error("failed to set cooperative level");
		}
		std::memset(&format_, 0, sizeof(format_));
		format_.wFormatTag = WAVE_FORMAT_PCM;
		format_.nChannels = config_.format.channels;
		format_.wBitsPerSample = config_.format.bits_per_sample;
		format_.nSamplesPerSec = config_.format.samples_per_second;
		format_.nBlockAlign = format_.nChannels * format_.wBitsPerSample / 8;
		format_.nAvgBytesPerSec = format_.nBlockAlign * format_.nSamplesPerSec;

		DSBUFFERDESC desc = {0};
		desc.dwSize = sizeof( DSBUFFERDESC );
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
		desc.dwBufferBytes = format_.nBlockAlign * config_.buffer_size;
		desc.lpwfxFormat = &format_;

		HRESULT res = device_->CreateSoundBuffer(&desc, &buffer_, 0);
		if( res != DS_OK ) {
			std::cerr << "Error code: " << std::hex << res << std::endl;
			throw std::runtime_error("failed to create play buffer");
		}
		buffer_size_in_bytes_ = desc.dwBufferBytes;

		//initialise to zeros
		void* p = 0;
		DWORD s = 0;
		HRESULT h = buffer_->Lock(0, buffer_size_in_bytes_, &p, &s, 0, 0, 0);
		if( h != DS_OK || s != buffer_size_in_bytes_ ) {
			throw std::runtime_error("failed to create play buffer");
		}
		std::memset( p, 0, s );
		buffer_->Unlock(p, s, 0, 0);
	}

	~dsound_play_device() {
		buffer_->Stop();
		buffer_->Release();
		device_->Release();
	}

	void start() {
		HRESULT h = buffer_->Play(0, 0, DSCBSTART_LOOPING);
		if( h != DS_OK ) {
			std::cout << "error: " << std::hex << h << std::endl;
			throw std::runtime_error("buffer play failed");
		}
		started_ = true;
	}

	void stop(stop_type s) {
		if(s == stop_type::drain) {
			DWORD play_pos = 0;
			DWORD write_pos = 0;
			HRESULT h;
			//is there a better way?
			while((h = buffer_->GetCurrentPosition(&play_pos, &write_pos)) == DS_OK && play_pos != write_pos) {
				std::this_thread::sleep_for(50ms);
			}
		}
		buffer_->Stop();
		started_ = false;
	}

	virtual std::size_t buffer_size() const {
		return config_.buffer_size;
	}

	virtual bool wait() {
		//todo: implement
		return true;
	}

	virtual int supported_modes() const {
		return audio_device_mode::notifications;
	}

	void set_notification(std::size_t samples) {
		//todo: implement
	}

	virtual void set_mode(mode const& m) {
		if(notification_mode const* p = dynamic_cast<notification_mode const*>(&m)) {
			set_notification(p->samples);
		}
	}
	
	virtual device_config config() const {
		return config_;
	}

	virtual std::size_t avail() const {
		DWORD play_pos;
		DWORD write_pos;
		HRESULT h = buffer_->GetCurrentPosition(&play_pos, &write_pos);
		if( h != DS_OK ) {
			LOG_TRACE("error: {}", ::GetLastError());
			throw std::runtime_error("buffer get current position failed");
		}
		int size = play_pos - pos_;
		if( size < 0 ) {
			size += buffer_size_in_bytes_;
		}
		return size;
	}

	std::size_t write_to_buffer(uint8_t const* buf, std::size_t size) {
		void* p1 = 0;
		void* p2 = 0;
		DWORD s1 = 0, s2 = 0;

		HRESULT h = buffer_->Lock(pos_, size, &p1, &s1, &p2, &s2, 0);

		if( h == DS_OK ) {
			if( s1 ) {
				std::memcpy(p1, buf, s1);
				if( s2 ) {
					std::memcpy(p2, buf+s1, s2);
				}
			}
			buffer_->Unlock(p1, s1, p2, s2);
			pos_ += s1+s2;
			pos_ %= buffer_size_in_bytes_;
		}
		return s1+s2;
	}

	virtual std::size_t write(audio_buffer& b) {
		std::size_t size = avail();
		if( pos_ == 0 && !started_ ) {
			size = std::min<int>(b.used_size(), buffer_size_in_bytes_);
		}
		std::size_t consumed = 0;
		if( size ) {
			consumed = write_to_buffer(b.begin<uint8_t>(), std::min<std::size_t>( size, b.used_size() ));
			b.consume(consumed);
		}
		return 8*consumed / config_.format.bits_per_sample;
	}
private:
	device_config config_;
	bool started_;
	std::size_t buffer_size_in_bytes_;
	DWORD pos_;

	GUID guid_;
	LPDIRECTSOUND8 device_;
	LPDIRECTSOUNDBUFFER buffer_;
	WAVEFORMATEX format_;
};

class dsound_capture_device : public audio_capture_device  {
public:
	dsound_capture_device(LPCGUID guid, device_config config)
		: config_(config)
		, pos_()
		, not_handle_( ::CreateEventW(NULL, FALSE, FALSE, NULL) )
	{
		HRESULT device_res = ::DirectSoundCaptureCreate8(guid, &device_, 0);
		if(device_res != DS_OK) {
			std::cerr << "Error code: " << std::hex << device_res << std::endl;
			throw std::runtime_error("failed to create device");
		}

		std::memset(&format_, 0, sizeof(format_));
		format_.wFormatTag = WAVE_FORMAT_PCM;
		format_.nChannels = config_.format.channels;
		format_.wBitsPerSample = config_.format.bits_per_sample;
		format_.nSamplesPerSec = config_.format.samples_per_second;
		format_.nBlockAlign = format_.nChannels * format_.wBitsPerSample / 8;
		format_.nAvgBytesPerSec = format_.nBlockAlign * format_.nSamplesPerSec;

		DSCBUFFERDESC desc = {0};
		desc.dwSize = sizeof(DSCBUFFERDESC);
		desc.dwFlags = DSCBCAPS_WAVEMAPPED;
		desc.dwBufferBytes = format_.nBlockAlign * config_.buffer_size;
		desc.lpwfxFormat = &format_;

		HRESULT res = device_->CreateCaptureBuffer(&desc, &buffer_, 0);
		if(res != DS_OK) {
			std::cerr << "Error code: " << std::hex << res << std::endl;
			throw std::runtime_error("failed to create capture buffer");
		}

		buffer_size_in_bytes_ = desc.dwBufferBytes;
	}

	~dsound_capture_device() {
		CloseHandle(not_handle_);
		buffer_->Stop();
		buffer_->Release();
		device_->Release();
	}

	void start() {
		buffer_->Start(DSCBSTART_LOOPING);
	}

	void stop(stop_type s) {
		if(s == stop_type::drain) {
			DWORD capture_pos = 0;
			DWORD read_pos = 0;
			HRESULT h;
			//is there a better way?
			while((h = buffer_->GetCurrentPosition(&capture_pos, &read_pos)) == DS_OK && capture_pos != read_pos) {
				std::this_thread::sleep_for(50ms);
			}
		}
		buffer_->Stop();
		SetEvent(not_handle_);
	}

	virtual std::size_t buffer_size() const {
		return config_.buffer_size;
	}
	
	virtual device_config config() const {
		return config_;
	}

	virtual std::size_t avail() const {
		DWORD capture_pos;
		DWORD read_pos;
		HRESULT h = buffer_->GetCurrentPosition(&capture_pos, &read_pos);
		if(h != DS_OK) {
			LOG_TRACE("error: {}", ::GetLastError());
			throw std::runtime_error("buffer get current position failed");
		}
		int size = read_pos - pos_;
		if(size < 0) {
			size += buffer_size_in_bytes_;
		}
		return size;
	}

	void set_notification(std::size_t samples) {
		std::size_t milliseconds = samples_to_length(config_.format, samples).count();
		if(buffer_length_ % milliseconds != 0) {
			throw std::logic_error("buffer length not dividable by notification period");
		}
		int notification_points = buffer_length_ / milliseconds;

		LPDIRECTSOUNDNOTIFY notify;
		if( buffer_->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&notify ) != DS_OK) {
			throw std::runtime_error("failed to query notification interface");
		}

		std::vector<DSBPOSITIONNOTIFY> pnot;

		if(!not_handle_) {
			LOG_TRACE("error: {}", ::GetLastError());
			throw std::runtime_error("failed to create notification handle");
		}

		for(int i = 0; i != notification_points; ++i) {
			DSBPOSITIONNOTIFY n = {0};
			n.dwOffset = (buffer_size_in_bytes_ / notification_points) * (i+1) - 1;
			n.hEventNotify = not_handle_;
			pnot.push_back(n);
		}

		if( notify->SetNotificationPositions(pnot.size(), pnot.data() ) != DS_OK) {
			throw std::runtime_error("failed to set notification");
		}

		notify->Release();
	}

	std::size_t capture_to_buffer(uint8_t* buf, std::size_t size) {
		void* p1 = 0;
		void* p2 = 0;
		DWORD s1 = 0, s2 = 0;
		HRESULT h = buffer_->Lock(pos_, size, &p1, &s1, &p2, &s2, 0);
		if( h == DS_OK ) {
			if( s1 ) {
				std::memcpy(buf, p1, s1);
				if( s2 ) {
					std::memcpy(buf+s1, p2, s2);
				}
			}
			buffer_->Unlock(p1, s1, p2, s2);
			pos_ += s1 + s2;
			pos_ %= buffer_size_in_bytes_;
		}
		return s1+s2;
	}

	virtual std::size_t read(audio_buffer& b) {
		std::size_t read_size = 0;
		std::size_t size = avail();
		if(size) {
			read_size = capture_to_buffer(b.free_begin<uint8_t>(), std::min<std::size_t>( size, b.free_size() ));
			b.conserve(read_size);
		}
		return 8*read_size/config_.format.bits_per_sample;
	}

	virtual int supported_modes() const {
		return audio_device_mode::notifications;
	}

	virtual void set_mode(mode const& m) {
		if(notification_mode const* p = dynamic_cast<notification_mode const*>(&m)) {
			set_notification(p->samples);
		}
	}

	bool wait() {
		::WaitForSingleObject(not_handle_, INFINITE);
		return true;
	}

private:
	device_config config_;
	std::size_t buffer_length_;
	std::size_t buffer_size_in_bytes_;

	DWORD pos_;
	HANDLE not_handle_;

	LPDIRECTSOUNDCAPTURE8 device_;
	LPDIRECTSOUNDCAPTUREBUFFER buffer_;
	WAVEFORMATEX format_;
};

std::string dsound_audio_interface::name() const {
	return "DirectSound";
}

adinfos dsound_audio_interface::enumerate_devices(audio_device_type type) const {
	adinfos devices;
	if( type & audio_device_t::play ) {
		enum_calltype callback = [&](LPGUID guid, LPCWSTR desc) {
			devices.push_back(std::make_shared<dsound_device_info>(guid, desc, true));
		};
		if(FAILED(::DirectSoundEnumerate(&audio::enumerate_devices, &callback))) {
			throw std::runtime_error("failed to enumerate audio devices");
		}
	}
	if( type & audio_device_t::capture ) {
		enum_calltype callback = [&](LPGUID guid, LPCWSTR desc) {
			devices.push_back(std::make_shared<dsound_device_info>(guid, desc, false));
		};
		if(FAILED(::DirectSoundCaptureEnumerate(&audio::enumerate_devices, &callback))) {
			throw std::runtime_error("failed to enumerate audio devices");
		}
	}
	return devices;
}

audio_play_device_ptr dsound_audio_interface::play_device(device_config const& config, audio_device_info_ptr info) const {
	dsound_device_info* p = info ? dynamic_cast<dsound_device_info*>(info.get()) : nullptr;
	if(info && (!p || !info->is_play_device())) {
		throw std::runtime_error("invalid audio play device info");
	}
	return audio_play_device_ptr(new dsound_play_device(p ? &p->guid_ : nullptr, config));
}

audio_capture_device_ptr dsound_audio_interface::capture_device(device_config const& config, audio_device_info_ptr info) const {
	dsound_device_info* p = info ? dynamic_cast<dsound_device_info*>(info.get()) : nullptr;
	if(info && (!p || info->is_play_device())) {
		throw std::runtime_error("invalid audio capture device info");
	}
	return audio_capture_device_ptr(new dsound_capture_device(p ?&p->guid_ : nullptr, config));
}

}
}
