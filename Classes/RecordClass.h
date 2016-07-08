#pragma once
#include "Tools.h"
class RecordClass
{
public:

	CameraHandle    m_hCamera;
	INT				cameraNum;
	tSdkCameraCapbility m_sCameraInfo;
	ULONG_PTR m_ulRAMPages;
	ULONG_PTR* m_aRAMPages;
	float m_curTime;
	float m_lastTime;
	int m_BufferIndex;
	int BufferSize;
	vector<VideoRAW*> m_Buffer;
public:
	RecordClass(void);
	~RecordClass(void);
	bool init(tSdkCameraDevInfo CameraInfo);
	void ThreadCallBack();
	VideoRAW* getBufferByIndex(int index);
	int getBuffIndex();
	void setExposureTime(float ExposureTime);
	void setGain(int Gain);
};

