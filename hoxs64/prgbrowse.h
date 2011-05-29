#ifndef __CPRGBrowse_H__
#define	__CPRGBrowse_H__

typedef class CArrayElement<struct C64Filename> CDirectoryElement;
typedef class CArray<struct C64Filename> CDirectoryArray;


class CPRGBrowse
{
public:
	enum filetype {ALL=0,FDI=1,G64=2,D64=4,TAP=8,T64=16,PRG=32,P00=64,SID=128};
	CPRGBrowse();
	~CPRGBrowse();
	HRESULT Init(bit8 *charGen);
	BOOL Open(HINSTANCE hInstance, OPENFILENAME *pOF, enum filetype filetypes);
	LRESULT ChildDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	void PopulateList(HWND hDlg);
	void CreateControls(HWND hDlg);
	CDirectoryArray dataList;

	void DrawC64String(HDC, int, int, BYTE[], int, bool);

	HINSTANCE hInstance;
	int SelectedListItem;
	int SelectedDirectoryIndex;
	bool SelectedQuickLoadDiskFile;
	bool SelectedAlignD64Tracks;
	bit8 SelectedC64FileName[C64DISKFILENAMELENGTH];
	int SelectedC64FileNameLength;

	enum filetype mAllowTypes;
	static const int HEADERLINES = 1;
	bool DisableQuickLoad;

private:
	static const int MAXLISTITEMCOUNT = 1000;
	int miLoadedListItemCount;
	enum FIS
	{
		COMPLETED = 0,
		WORKING = 1
	};
	void CancelFileInspector();
	HRESULT BeginFileInspector(HWND hWndDlg, TCHAR *fileName);
	static DWORD WINAPI StartFileInspectorThread(LPVOID lpParam);
	DWORD StartFileInspector();
	HWND mhWndInspector;
	TCHAR mptsFileName[MAX_PATH+1];
	HANDLE mhEvtComplete;
	CRITICAL_SECTION mCrtStatus;
	bool mbSectionOK;
	HANDLE mhEvtQuit;
	bool mbDestroyCalled;
	FIS mFileInspectorStatus;
	HRESULT mFileInspectorResult;
	void InspectorCompleteFail();
	void InspectorCompleteOK();
	void InspectorStart();

	HBRUSH m_hbrush;
	void ReSize(HWND hDlg, LONG w, LONG h);
	void CleanUp();
	bit8 *charGen;
	C64File c64file;
	HWND hParent;
	HWND hListBox;
	HWND hCheckQuickLoad;
	HWND hCheckAlignD64Tracks;
	int idListBox;
	int mgapListBoxBottom;
	int mgapCheckBoxQuickLoadBottom;
	int mgapCheckBoxAlignD64TracksBottom;
	bool mbGapsDone;
};



#endif