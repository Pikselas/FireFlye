#pragma once
#include <wrl.h>
#include <mfapi.h>
#include <mfidl.h>
#include <windows.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mftransform.h>

#include <filesystem>

#include "RipeGrain/RepulsiveEngine/ResourceUtils.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#define THROW_ON_FAIL(hresult_action , error) if(auto hr_code = hresult_action;FAILED(hr_code)){  char * pcharBuff = nullptr;FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&pcharBuff), 0, nullptr); std::string message = pcharBuff; LocalFree(pcharBuff); throw std::runtime_error(std::string(error) + ":" + message);}

class VideoFrame
{
	friend class VideoReader;
private:
	LONGLONG timestamp = 0;
private:
	UINT32 width = 0, height = 0;
	Microsoft::WRL::ComPtr<IMFSample> sample;
public:
	LONGLONG GetTimestamp() const
	{
		return timestamp;
	}
	Image GetImage() const
	{
		Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
		THROW_ON_FAIL(sample->ConvertToContiguousBuffer(&buffer),"Failed to convert sample to contiguous buffer");
		BYTE* rgb24Data = nullptr;
		DWORD bufferLength = 0;
		THROW_ON_FAIL(buffer->Lock(&rgb24Data, nullptr, &bufferLength) , "Failed to lock buffer");

		// usually 4 bytes of padding are added to the end of each row
		// to align the data to a 4-byte boundary
		// the byte buffer size (actual size in bytes) is buffer_width (len of buffer packed with RGB as collection) * [R + G + B] = width * 3
		// (width * 3) + 3 => +3 ensures it is rounded to closest multiple of 4
		// 3 => ...0011 ~3 => ...1100
		// & ~3 => clear the last 2 bits
		// clearing the last 2 bits ensures that the stride is a multiple of 4 bytes
		// By first adding 3, we ensure that the value exceeds the current multiple of 4 if there’s any remainder.
		// Then, clearing the last two bits ensures the number "snaps" back to the nearest lower multiple of 4.
		// i.e 
		// width is 5 
		// 5 * 3 = 15 => ...1111
		// 15 + 3 = 18 => ...10010
		// 18 & ~3 = 16 => ...10010 & ...11100 = ...10000 [16 % 4 = 0]
		int stride = ((width * 3 + 3) & ~3);
		
		Image img(width, height);
		for (int j = 0; j < height; ++j)
		{
			// Source row (from bottom to top)
			int srcRow = height - 1 - j;
			BYTE* src = rgb24Data + srcRow * stride;

			// Destination row (top to bottom)
			for (int i = 0; i < width; ++i)
			{
				int idx = i * 3; // RGB24: 3 bytes per pixel
				img.SetPixel(i, j, ColorType{
					.b = src[idx],
					.g = src[idx + 1],
					.r = src[idx + 2]
					});
			}
		}
		buffer->Unlock();

		return img;
	}
};

class VideoReader : public ResourceEngineAccessor 
{
private:
	inline static GDIPlusManager manager;
private:
	class MediaFoundationManager
	{
	public:
		MediaFoundationManager()
		{
			THROW_ON_FAIL(MFStartup(MF_VERSION) , "Failed to initialize Media Foundation");
		}
		~MediaFoundationManager()
		{
			MFShutdown();
		};
	};
private:
	inline static MediaFoundationManager mfManager;
private:
	UINT32 width = 0, height = 0;
	UINT64 duration = 0;
	UINT64 current_timestamp = 0;
private:
	Microsoft::WRL::ComPtr<IMFMediaType> pMediaType;
	Microsoft::WRL::ComPtr<IMFMediaType> nv12_mediaType;
	Microsoft::WRL::ComPtr<IMFMediaType> rgb24_mediaType;
	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	Microsoft::WRL::ComPtr<IMFTransform> frameProcessor;
private:
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mf_dxgi_manager;
public:
	VideoReader() = default;
	VideoReader(std::filesystem::path path)
	{
		Open(path);
	}
private:
	void setup_media_reader()
	{
		THROW_ON_FAIL(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pMediaType), "Failed to get current media type");

		UINT64 frameSize = 0;
		THROW_ON_FAIL(pMediaType->GetUINT64(MF_MT_FRAME_SIZE, &frameSize), "Failed get frame size");
		height = static_cast<UINT32>(frameSize & 0xFFFFFFFF);
		width = static_cast<UINT32>((frameSize >> 32) & 0xFFFFFFFF);

		THROW_ON_FAIL(MFCreateMediaType(&nv12_mediaType), "Failed to create NV12 media type");
		nv12_mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		nv12_mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
		MFSetAttributeSize(nv12_mediaType.Get(), MF_MT_FRAME_SIZE, width, height);
		THROW_ON_FAIL(sourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, nv12_mediaType.Get()), "Failed to set nv12 media type");

		THROW_ON_FAIL(MFCreateMediaType(&rgb24_mediaType), "Failed to create rgb24 media type");
		rgb24_mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		rgb24_mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
		MFSetAttributeSize(rgb24_mediaType.Get(), MF_MT_FRAME_SIZE, width, height);

		THROW_ON_FAIL(CoCreateInstance(
			CLSID_VideoProcessorMFT,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&frameProcessor)
		), "Failed to create frame processor");


		THROW_ON_FAIL(frameProcessor->SetInputType(0, nv12_mediaType.Get(), 0), "Failed to set frame processor input type");
		THROW_ON_FAIL(frameProcessor->SetOutputType(0, rgb24_mediaType.Get(), 0), "Failed to set frame processor output type");

		Microsoft::WRL::ComPtr<IMFMediaSource> mediaSource;
		THROW_ON_FAIL(sourceReader->GetServiceForStream(MF_SOURCE_READER_MEDIASOURCE, GUID_NULL, IID_PPV_ARGS(&mediaSource)), "Failed to get media source from source reader");

		Microsoft::WRL::ComPtr<IMFPresentationDescriptor> presentationDescriptor;
		THROW_ON_FAIL(mediaSource->CreatePresentationDescriptor(&presentationDescriptor), "Failed to create presentation descriptor");
		THROW_ON_FAIL(presentationDescriptor->GetUINT64(MF_PD_DURATION, &duration), "Failed to get source duration");
	}
public:
	void Open(std::filesystem::path path)
	{
		THROW_ON_FAIL(MFCreateSourceReaderFromURL(path.wstring().c_str(), nullptr, &sourceReader), "Failed to create Source Reader");
		setup_media_reader();
	}
	/*void Open(std::filesystem::path path, ResourceEngine& gpu_resource_engine)
	{
		UINT resetToken = 0;
		THROW_ON_FAIL(MFCreateDXGIDeviceManager(&resetToken, &mf_dxgi_manager), "Failed to create DXGI device manager");
		THROW_ON_FAIL(GetDevice(gpu_resource_engine)->QueryInterface(IID_PPV_ARGS(&dxgiDevice)), "Failed to query DXGI device");
		THROW_ON_FAIL(mf_dxgi_manager->ResetDevice(dxgiDevice.Get(), resetToken), "Failed to reset DXGI device manager");

		Microsoft::WRL::ComPtr<IMFAttributes> attributes;
		THROW_ON_FAIL(MFCreateAttributes(&attributes, 1), "Failed to create attributes");
		THROW_ON_FAIL(attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, mf_dxgi_manager.Get()), "Failed to set D3D manager");
		THROW_ON_FAIL(MFCreateSourceReaderFromURL(path.wstring().c_str(), attributes.Get(), &sourceReader), "Failed to create Source Reader");
		setup_media_reader();
	}*/
	std::optional<VideoFrame> GetNextFrame()
	{
		DWORD streamIndex = 0;
		DWORD flags = 0;
		LONGLONG timestamp = 0;
		VideoFrame frame;

		Microsoft::WRL::ComPtr<IMFSample> nv12_sample;

		frame.width = width;
		frame.height = height;

		THROW_ON_FAIL(sourceReader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,
			&streamIndex,
			&flags,
			&timestamp,
			&nv12_sample
		) , "Failed to read sample");

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			return std::nullopt;
		}

		THROW_ON_FAIL(frameProcessor->ProcessInput(0, nv12_sample.Get(), 0) , "Failed to process input");

		Microsoft::WRL::ComPtr<IMFSample> rgb_sample;
		THROW_ON_FAIL(MFCreateSample(&rgb_sample) , "Failed to create RGB sample");

		Microsoft::WRL::ComPtr<IMFMediaBuffer> outputBuffer;
		MFT_OUTPUT_STREAM_INFO streamInfo = {};
		THROW_ON_FAIL(frameProcessor->GetOutputStreamInfo(0, &streamInfo) , "Failed to get output stream info");

		// Allocate a buffer of the required size
		DWORD requiredBufferSize = max(streamInfo.cbSize, width * height * 3); // Ensure it's large enough for RGB24
		THROW_ON_FAIL(MFCreateMemoryBuffer(requiredBufferSize, &outputBuffer) , "Failed to create memory buffer for output sample");
		THROW_ON_FAIL(rgb_sample->AddBuffer(outputBuffer.Get()) , "Failed to add buffer to output sample");

		MFT_OUTPUT_DATA_BUFFER outputDataBuffer = { 0 };
		outputDataBuffer.dwStreamID = 0;
		outputDataBuffer.pSample = rgb_sample.Get();
		DWORD outputStatus = 0;
		THROW_ON_FAIL(frameProcessor->ProcessOutput(0,1,&outputDataBuffer,&outputStatus) , "Failed to process output");

		frame.sample = rgb_sample;
		frame.timestamp = timestamp;
		current_timestamp = timestamp;
		return frame;
	};
	void SeekTo(unsigned long long dur)
	{
		if (dur < 0 || dur > duration)
		{
			throw std::runtime_error("Invalid duration");
		}
		PROPVARIANT var;
		PropVariantInit(&var);
		var.hVal.QuadPart = dur;
		var.vt = VT_I8;
		THROW_ON_FAIL(sourceReader->SetCurrentPosition(GUID_NULL, var) , "Failed to seek");
		PropVariantClear(&var);
		current_timestamp = dur;
	}
	unsigned long long GetCurrentPosition()
	{
		return current_timestamp;
	}
	unsigned long long GetDuration() const
	{
		return duration;
	}
};

//GDIPlusManager VideoReader::manager;
//VideoReader::MediaFoundationManager VideoReader::mfManager;