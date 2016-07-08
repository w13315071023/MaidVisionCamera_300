#include "MovieVideoLayer.h"
#include "DataMager.h"
#include "SerialMager.h"
#include "RecordClass.h"

MovieVideoLayer::MovieVideoLayer()
{
	
}
MovieVideoLayer::~MovieVideoLayer()
{
	FreeUserPhysicalPages(GetCurrentProcess(), &m_ulRAMPages, m_aRAMPages);
	HeapFree(GetProcessHeap(), 0, m_aRAMPages);
	av_free(m_pFrameImageRGB);
}
MovieVideoLayer* MovieVideoLayer::create(int Direction)
{
    MovieVideoLayer *pRet = new MovieVideoLayer();
    if (pRet && pRet->init(Direction))
    {
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}
bool MovieVideoLayer::init(int Direction)
{
	if (!CCLayer::init())
    {
        return false;
    }
	printf("初始化MovieVideoLayer\n");
	m_Camera = NULL;
	m_ShowIndex = 0;
	m_TransIndex = 0;
	m_IsRecord = false;
	m_Width = 640;
	m_Height = 480;

	if (Ext_cameraNum == 0)
	{
		return true;
	}
	if (!AWESetLockPagesPrivilege(GetCurrentProcess(), TRUE))
	{
		printf("权限获取失败");
	}
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);

	m_ulRAMPages = 921600 * 1000 / sinf.dwPageSize;
	m_aRAMPages = (ULONG_PTR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_ulRAMPages * 4);
	AllocateUserPhysicalPages(GetCurrentProcess(), &m_ulRAMPages, m_aRAMPages);
	if (Direction == FRONT)
	{
		m_Camera = m_Camera1;
		MapUserPhysicalPages(Ext_AEWwindow, m_ulRAMPages, m_aRAMPages);
		for (size_t i = 0; i < 900; i++)
		{
			unsigned char* pVideoRGB24 = &Ext_AEWwindow[m_Camera->BufferSize*i];
			m_VideoList.push_back(pVideoRGB24);
		}
	}
	else
	{
		m_Camera = m_Camera2;
	}
	
	
	m_pFrameImageRGB = (unsigned char*)av_malloc(m_Camera->BufferSize);
	this->setRotationY(180);
	CCSprite* beijing2 = CCSprite::create("VideoUI/beijing2.png");
	beijing2->setPosition(ccp(VISIBLEW/2,VISIBLEH/2));
	this->addChild(beijing2);
	m_pSprite->setPosition(ccp(VISIBLEW / 2, VISIBLEH / 2));
	this->addChild(m_pSprite);

    return true;
}
void MovieVideoLayer::update(float dt)
{
	m_ShowIndex = (m_Camera->getBuffIndex() - 1<0) ? 999 : m_Camera->getBuffIndex() - 1;
	m_FrameImageHead = m_Camera->getBufferByIndex(m_ShowIndex)->FrameHead;
	int index = CameraImageProcess(m_Camera->m_hCamera,
		m_Camera->getBufferByIndex(m_ShowIndex)->FrameData,
		m_pFrameImageRGB,
		&m_FrameImageHead);
	if (index)
	{
		return;
	}
	m_pTexture->updateVideoWithData(
		m_pFrameImageRGB,
		kTexture2DPixelFormat_RGB888,
		640,
		480,
		CCSize(630, 470));
	m_pSprite->initWithTexture(m_pTexture);
}
void MovieVideoLayer::Record(bool isRecord)
{
	m_IsRecord = isRecord;
	m_IsPlayOver = isRecord;
}
void MovieVideoLayer::TransData()
{
	if (m_TransIndex < 20 || Ext_IsRecordBegin == false)
	{
		return;
	}
	if (m_TransIndex == Ext_VideoSize * 10)
	{
		m_TransIndex = 0;
		return;
	}
	int curIndex = 0;
	curIndex = (m_Camera->getBuffIndex() - 900 + m_TransIndex) % 1000;
	curIndex = curIndex < 0 ? 999 + curIndex : curIndex;

	m_FrameImageHead = m_Camera->getBufferByIndex(curIndex)->FrameHead;
	CameraImageProcess(m_Camera->m_hCamera,
		m_Camera->getBufferByIndex(curIndex)->FrameData,
		m_pFrameImageRGB,
		&m_FrameImageHead);
	swap(m_pFrameImageRGB, m_VideoList[m_TransIndex]);
	m_TransIndex++;
}
void MovieVideoLayer::RecordOk()
{
	int curIndex = 0;
	this->Record(false);
	for (size_t i = m_TransIndex; i < Ext_VideoSize * 10; i++)
	{
		if (i == m_TransIndex+20)
		{
			m_TransIndex = i;
			break;
		}
		curIndex = (m_Camera->getBuffIndex() - Ext_VideoSize * 10 + i) % 1000;
		curIndex = curIndex < 0 ? 999 + curIndex : curIndex;
		m_FrameImageHead = m_Camera->getBufferByIndex(curIndex)->FrameHead;
		CameraImageProcess(m_Camera->m_hCamera,
			m_Camera->getBufferByIndex(curIndex)->FrameData,
			m_pFrameImageRGB,
			&m_FrameImageHead);
		swap(m_pFrameImageRGB, m_VideoList[i]);
	}
	m_VideoIter = m_VideoList.begin();
}
