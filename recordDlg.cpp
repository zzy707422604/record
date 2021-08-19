
// recordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "record.h"
#include "recordDlg.h"
#include "afxdialogex.h"
#define BUFFER_SIZE (44100*16*2/8*100)    // 录制声音长度  
#define FRAGMENT_SIZE 1024              // 缓存区大小  
#define FRAGMENT_NUM 4                  // 缓存区个数  

static unsigned char buffer[BUFFER_SIZE] = { 0 };
static int buf_count = 0;
bool voiceCaptureControl;
double endtime;
unsigned char PSFrameBuffer[10 * 1024 * 1024]; //转换后的ps帧缓存区
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 函数定义   
void CALLBACK waveInProc(HWAVEIN hwi,//输入回调
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2);
void CALLBACK waveOutProc(HWAVEOUT hwo,//输出回调
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2);
// CAboutDlg dialog used for App About


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CrecordDlg dialog



CrecordDlg::CrecordDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CrecordDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CrecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CrecordDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CrecordDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CrecordDlg message handlers

BOOL CrecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	voiceCaptureControl = false;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CrecordDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CrecordDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CrecordDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
FILE *streampcm = NULL;
FILE *streamg711a = NULL;
FILE *streamps = NULL;
uint32_t ts = 0;
// 录音回调函数   
void CALLBACK waveInProc(HWAVEIN hwi,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;

	if ((WIM_DATA == uMsg) && voiceCaptureControl)
	{
		//int temp = pwh->dwBytesRecorded;
		if (streampcm == NULL)
			streampcm = fopen("test.pcm", "wb+");
		fwrite(pwh->lpData, pwh->dwBytesRecorded, 1, streampcm);//录制pcm声音

		//将pcm编码成g711a
		char *g711Buffer = new char[(unsigned int)pwh->dwBytesRecorded];
		int g711Len = (unsigned int)pwh->dwBytesRecorded / 2;
		Pcm2G711(pwh->lpData, (unsigned int)pwh->dwBytesRecorded, g711Buffer, g711Len, 0);

		if (streamg711a == NULL)
			streamg711a = fopen("test.G711a", "wb+");
		fwrite(g711Buffer, g711Len, 1, streamg711a);//录制g711a声音


		//添加ps头
		if (streamps == NULL)
		{
			streamps = fopen("test.ps", "wb+");
		}

		int ps_size = TransPSFrame(g711Buffer, g711Len, ts, PSFrameBuffer);
		fwrite(PSFrameBuffer, 1, ps_size, streamps);
		ts += 3600;
		delete g711Buffer;
		g711Buffer = NULL;
		memcpy(buffer + buf_count, pwh->lpData, pwh->dwBytesRecorded);//把这句存储改成send
		buf_count += pwh->dwBytesRecorded;
		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));//这句会清空pwh->lpData

		//清空缓存
	}
	else if (voiceCaptureControl==false)
	{
		fclose(streampcm);
		streampcm = NULL;
		fclose(streamg711a);
		streamg711a = NULL;
		fclose(streamg711a);
		streamg711a = NULL;
		fclose(streamps);
		streamps = NULL;
	}
}

// 放音回调函数   
void CALLBACK waveOutProc(HWAVEOUT hwo,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2)
{
	if (WOM_DONE == uMsg)
	{
		buf_count = 44100 * 16 * 2 / 8 * endtime;
	}
}

static void StartCaptureThread(LPVOID Arg)
{
	CrecordDlg *pArg = (CrecordDlg *)Arg;
	pArg->voiceCaptureOn();
}

void CrecordDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	voiceCaptureControl = !voiceCaptureControl;
	if (voiceCaptureControl == true)
	{
		_beginthread(StartCaptureThread, NULL, this);
		//voiceCaptureOn();
	}
}


void CrecordDlg::voiceCaptureOn()
{
	/* 录音 */

	clock_t start, end;
	// Device   
	int nReturn = waveInGetNumDevs();
	printf("输入设备数目：%d\n", nReturn);
	for (int i = 0; i<nReturn; i++)
	{
		WAVEINCAPS wic;
		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
		printf("#%d\t设备名：%s\n", i, wic.szPname);
	}

	// open   
	HWAVEIN hWaveIn;//设备句柄
	WAVEFORMATEX wavform;//波形音频流格式的数据结构，指针
	wavform.wFormatTag = WAVE_FORMAT_PCM;//声音格式
	wavform.nChannels = 2;//通道数量 1：单声道 2：立体声
	wavform.nSamplesPerSec = 44100;//采样率
	wavform.nAvgBytesPerSec = 44100 * 16 * 2 / 8;//平均数据传输率 
	wavform.nBlockAlign = 4;//以字节为单位设置块对齐。块对齐是指最小数据的原子大小。如果wFormatTag = WAVE_FORMAT_PCM，nBlockAlign 为(nChannels*wBitsPerSample)/8
	wavform.wBitsPerSample = 16;//根据wFormatTag的类型设置每个样本的位深（即每次采样样本的大小，以bit为单位）。如果wFormatTag = WAVE_FORMAT_PCM，此值应该设为8或16
	wavform.cbSize = 0;//额外信息的大小，以字节为单位

	waveInOpen(&hWaveIn, WAVE_MAPPER, &wavform, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION);//第二个值为打开设备的id，如果为WAVE_MAPPER则为自动寻找合适

	WAVEINCAPS wic;
	waveInGetDevCaps((UINT_PTR)hWaveIn, &wic, sizeof(WAVEINCAPS));
	printf("打开的输入设备：%s\n", wic.szPname);

	// prepare buffer   
	static WAVEHDR wh[FRAGMENT_NUM];
	for (int i = 0; i<FRAGMENT_NUM; i++)
	{
		wh[i].lpData = new char[FRAGMENT_SIZE];//波形缓冲数据(传入首地址)
		wh[i].dwBufferLength = FRAGMENT_SIZE;//缓冲区长度
		wh[i].dwBytesRecorded = 0;//指明录音时缓冲区容量
		wh[i].dwUser = NULL;//用户数据
		wh[i].dwFlags = 0;//提供缓冲区标示
		wh[i].dwLoops = 1;//循环次数
		wh[i].lpNext = NULL;
		wh[i].reserved = 0;

		waveInPrepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));//准备缓冲区
		waveInAddBuffer(hWaveIn, &wh[i], sizeof(WAVEHDR));//添加一个缓冲区
	}
	// record   
	printf("Start to Record...\n");

	buf_count = 0;
	waveInStart(hWaveIn);//启动输出
	start = clock();
	while (voiceCaptureControl)
	{
		Sleep(1);
	}

	printf("Record Over!\n\n");
	end = clock();
	endtime = (double)(end - start) / CLOCKS_PER_SEC;
	//-----------------------------------
	// clean   
	waveInStop(hWaveIn);
	waveInReset(hWaveIn);
	for (int i = 0; i<FRAGMENT_NUM; i++)
	{
		waveInUnprepareHeader(hWaveIn, &wh[i], sizeof(WAVEHDR));
		delete wh[i].lpData;
	}
	waveInClose(hWaveIn);

	//system("pause");
	//printf("\n");

	/* 放音 */

	// Device   
	//nReturn = waveOutGetNumDevs();
	//printf("\n输出设备数目：%d\n", nReturn);
	//for (int i = 0; i<nReturn; i++)
	//{
	//	WAVEOUTCAPS woc;
	//	waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS));
	//	printf("#%d\t设备名：%s\n", i, wic.szPname);
	//}

	//// open   
	//HWAVEOUT hWaveOut;
	//waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);

	//WAVEOUTCAPS woc;
	//waveOutGetDevCaps((UINT_PTR)hWaveOut, &woc, sizeof(WAVEOUTCAPS));
	//printf("打开的输出设备：%s\n", wic.szPname);

	//// prepare buffer   
	//WAVEHDR wavhdr;
	//wavhdr.lpData = (LPSTR)buffer;
	//wavhdr.dwBufferLength = BUFFER_SIZE;
	//wavhdr.dwFlags = 0;
	//wavhdr.dwLoops = 0;

	//waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));

	//// play   
	//printf("Start to Play...\n");

	//buf_count = 0;
	//waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	//while (buf_count < (44100 * 16 * 2 / 8 * endtime))
	//{
	//	Sleep(1);
	//}

	//// clean   
	//waveOutReset(hWaveOut);
	//waveOutUnprepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	//waveOutClose(hWaveOut);

	//printf("Play Over!\n\n");


}

void CrecordDlg::voiceCaptureOff()
{

}

