#pragma once
#include "cvirwindow.h"

class CDiagFileSaveD64
{
public:
	CDiagFileSaveD64();
	~CDiagFileSaveD64();

	HRESULT Init(int selectedNumberOfTracks);
	BOOL Open(HINSTANCE hInstance, OPENFILENAME *pOF);
	UINT ChildDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	void CreateControls(HWND hDlg);

	HINSTANCE hInstance;
	int SelectedNumberOfTracks;

private:
	void ReSize(HWND hDlg, LONG w, LONG h);
	void CleanUp();
	HWND hParent;
	HWND hStaticTracks;
	HWND hCheckTracks35;
	HWND hCheckTracks40;
	int idStaticTracks;
	int mgapStaticTracksBottom;
	int mgapCheckTracks35Bottom;
	int mgapCheckTracks40Bottom;
	bool mbGapsDone;
};
