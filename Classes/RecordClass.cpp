#include "RecordClass.h"
RecordClass::RecordClass(void)
{
	m_BufferIndex = 0;
	BufferSize = 0;

	if (!AWESetLockPagesPrivilege(GetCurrentProcess(), TRUE))
	{
		printf("权限获取失败");
	}
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);

	m_ulRAMPages = 921600 * 1000 / sinf.dwPageSize;
	m_aRAMPages = (ULONG_PTR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_ulRAMPages * 4);
	AllocateUserPhysicalPages(GetCurrentProcess(), &m_ulRAMPages, m_aRAMPages);
}


RecordClass::~RecordClass(void)
{
	FreeUserPhysicalPages(GetCurrentProcess(), &m_ulRAMPages, m_aRAMPages);
	HeapFree(GetProcessHeap(), 0, m_aRAMPages);
	m_Buffer.clear();
}
bool RecordClass::init(tSdkCameraDevInfo CameraInfo)
{
	printf("初始化Camera%s\n", CameraInfo.acFriendlyName);
	CameraInit(&CameraInfo, -1, -1, &m_hCamera);
	CameraGetCapability(m_hCamera, &m_sCameraInfo);
	CameraSetAeState(m_hCamera, false);
	tSdkImageResolution pImageResolution;
	CameraGetImageResolution(m_hCamera, &pImageResolution);
	BufferSize = pImageResolution.iWidth*pImageResolution.iHeight * 3;

	
	MapUserPhysicalPages(Ext_AEWwindow, m_ulRAMPages, m_aRAMPages);
	for (size_t i = 0; i < 1000; i++)
	{
		VideoRAW* pVideoRGB24 = new VideoRAW();
		pVideoRGB24->FrameData = &Ext_AEWwindow[i*BufferSize];
		m_Buffer.push_back(pVideoRGB24);
	}
	std::thread myThread(&RecordClass::ThreadCallBack, this);
	SetThreadPriority(myThread.native_handle(), THREAD_PRIORITY_HIGHEST);
	
	CameraPlay(m_hCamera);
	myThread.detach();

	return true;
}
void RecordClass::ThreadCallBack()
{
	BYTE* bydFrameBuffer;
	tSdkFrameHead FrameInfo;
	while (true)
	{
		if (Ext_IsThreadOn)
		{
			int index = CameraGetImageBuffer(m_hCamera, &FrameInfo, &bydFrameBuffer, 20);
			if (!index)
			{
				m_curTime = GetTime();
				printf("%p时间间隔%f\n", this, m_curTime - m_lastTime);
				m_lastTime = m_curTime;

				m_BufferIndex = (m_BufferIndex + 1) % 1000;
				//CameraImageProcess(m_hCamera, bydFrameBuffer, FrameBufferRGB24, &FrameInfo);
				memcpy(m_Buffer[m_BufferIndex]->FrameData, bydFrameBuffer, FrameInfo.uBytes);
				m_Buffer[m_BufferIndex]->FrameHead = FrameInfo;
				CameraReleaseImageBuffer(m_hCamera, bydFrameBuffer);
			}
		}
	}
}
VideoRAW* RecordClass::getBufferByIndex(int index)
{
	return m_Buffer[index % 1000];
}
int RecordClass::getBuffIndex()
{
	return m_BufferIndex;
}
void RecordClass::setExposureTime(float ExposureTime)
{
	CameraSetExposureTime(m_hCamera, ExposureTime);
	CameraSetOnceWB(m_hCamera);
}
void RecordClass::setGain(int Gain)
{
	CameraSetAnalogGain(m_hCamera,Gain);
}
