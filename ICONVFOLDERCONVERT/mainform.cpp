/*****************************************************************************************************************
made by @vain430
Email vain430@gmail.com

History
2014.05.06 UPLOAD Version 0.9
twitter : @vain430
*****************************************************************************************************************/



#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include <process.h>
#include <string>
#include <list>
#include <stdio.h>
#include <iconv.h>
#include <CommCtrl.h>

#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"libiconv.lib")
#pragma comment(linker,"/manifestdependency:\" type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0'  processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"") 


#ifndef MAX_LENGTH
#define MAX_LENGTH 260
#endif

#undef MAX_PATH
#define MAX_PATH 512

#define MAX_BUFF 1024

#ifndef tstring
#ifdef _UNICODE
#define tstring std::wstring
#else
#define tsting std::string
#endif
#endif

#define WM_SETMAXPROGRESS WM_USER+100 


BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
void DoEncodeing(void *p);
void GetStringFromComboBox(HWND hWnd, int id, char *str); /// No LIMIT LENGTH! 


HWND hMain;

#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_DROPFILES
#define WM_DROPFILES 0x0233
#endif

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	ChangeWindowMessageFilter(WM_COPYGLOBALDATA, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD); //for uac

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, MainDlgProc);
	return 0;
}




BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_INITDIALOG:
		hMain = hDlg;
		InitCommonControls();
		{
			const char* arr[] = { "ASCII", "ISO-8859-1", "ISO-8859-2", "ISO-8859-3", "ISO-8859-4", "ISO-8859-5",
				"ISO-8859-7", "ISO-8859-9", "ISO-8859-10", "ISO-8859-13", "ISO-8859-14", "ISO-8859-15",
				"ISO-8859-16", "KOI8-R", "KOI8-U", "KOI8-RU", "CP1250", "CP1251", "CP1252", "CP1253", "CP1254", "CP1257",
				"CP850", "CP866", "MacRoman", "MacCentralEurope", "MacIceland", "MacCroatian", "MacRomania", "MacCyrillic",
				"MacUkraine", "MacGreek", "MacTurkish", "Macintosh", "ISO-8859-6", "ISO-8859-8", "CP1255", "CP1256",
				"CP862", "MacHebrew", "MacArabic", "EUC-JP", "SHIFT_JIS", "CP932", "ISO-2022-JP", "ISO-2022-JP-2",
				"ISO-2022-JP-1", "EUC-CN", "HZ", "GBK", "GB18030", "EUC-TW", "BIG5", "CP950", "BIG5-HKSCS",
				"ISO-2022-CN", "ISO-2022-CN-EXT", "EUC-KR", "CP949", "ISO-2022-KR", "JOHAB", "ARMSCII-8",
				"Georgian-Academy", "Georgian-PS", "KOI8-T", "TIS-620", "CP874", "MacThai", "MuleLao-1", "CP1133",
				"VISCII", "TCVN", "CP1258", "HP-ROMAN8", "NEXTSTEP", "UTF-8", "UCS-2", "UCS-2BE", "UCS-2LE", "UCS-4",
				"UCS-4BE", "UCS-4LE", "UTF-16", "UTF-16BE", "UTF-16LE", "UTF-32", "UTF-32BE", "UTF-32LE", "UTF-7",
				"C99", "JAVA", "UCS-2-INTERNAL", "UCS-4-INTERNAL", "char", "wchar_t", "CP437", "CP737", "CP775", "CP852",
				"CP853", "CP855", "CP857", "CP858", "CP860", "CP861", "CP863", "CP865", "CP869", "CP1125", "CP864", "EUC-JISX0213",
				"Shift_JISX0213", "ISO-2022-JP-3", "TDS565", "RISCOS-LATIN1" };
				for (int i = 0; i<115; i++) {
					SendMessageA(GetDlgItem(hDlg, IDC_COMBO_DSTCHAR), CB_ADDSTRING, 0, (LPARAM)arr[i]);
					SendMessageA(GetDlgItem(hDlg, IDC_COMBO_ORGCHAR), CB_ADDSTRING, 0, (LPARAM)arr[i]);
				}

				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_EXTFILTER), _T("*.*"));
				SetWindowText(GetDlgItem(hDlg, IDC_DRAGHERE), _T("Drag&drop or write or press button"));
				
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_CONVERT:
			_beginthread(DoEncodeing, 0, (void *)hDlg);
			return TRUE;
		case IDCLOSE:
		case IDCANCEL:
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		return FALSE;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			int uCount = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
			int nLength = DragQueryFile((HDROP)wParam, 0, NULL, 0);
			TCHAR *buffer = new TCHAR[nLength+2];
			memset(buffer, 0, sizeof(sizeof(TCHAR)*nLength+2));
			DragQueryFile((HDROP)wParam, 0, buffer, nLength +1);
			SetWindowText(GetDlgItem(hDlg, IDC_DRAGHERE), buffer);
			delete buffer;
			SendMessage(GetDlgItem(hDlg, IDC_DRAGHERE), EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength);
		}
		return true;
	}
	return FALSE;
}



void OnFindFile(TCHAR *str) //arg0: FILE name
{
//	SendMessage(GetDlgItem(hMain, IDC_LIST1), LB_ADDSTRING, 0,(LPARAM)str);
}


bool Filterling(TCHAR *filename, TCHAR *nameFilter, TCHAR *extFilter)
{
	if (_tcscmp(nameFilter, _T("*")) == 0)
	{
		if (_tcscmp(extFilter, _T("*")) == 0)
		{
			return true;
		}
	}

	//filname인자의 예 test.txt *.*
	//extFilter인자의 예 *.* txt.* *.txt
	TCHAR *ext=0;
	for (int i = _tcslen(filename)-1 ; i > 0; i--)
	{
		if (filename[i]==_T('.'))
		{
			ext = &filename[i+1];
			filename[i] = 0;
			break;
		}
	}

	if (_tcsncmp(nameFilter, _T("*"), 1) == 0)	//ext가 *이 아니다
	{
		if (ext) //null도아닐때
		{
			if (_tcscmp(extFilter, ext) == 0)
				return true;
		}
		
	}else	{									//nameFilter가 *이 아니다

		if (_tcscmp(nameFilter, filename) == 0)
			return true;
	}
	return false;
}


void GetFilesFromDirectory(TCHAR *param_path, TCHAR *nameFilter,TCHAR *extFilter, std::list<tstring> &list_arg)
{
	TCHAR path[MAX_PATH];
	TCHAR file_path[MAX_PATH];
	WIN32_FIND_DATA file_data;

	_stprintf_s(path,MAX_PATH, _T("%s\\*.*"), param_path);

	HANDLE search_handle = FindFirstFile(path, &file_data);



	if (INVALID_HANDLE_VALUE != search_handle){
		// 파일 목록을 성공적으로 구성한 경우..
		if (_tcscmp(file_data.cFileName, _T(".")) == 0) //skip .
		{
			FindNextFile(search_handle, &file_data); //skip ..
			if (!FindNextFile(search_handle, &file_data))  //if there is no more file list
			{
				FindClose(search_handle); //close and return;
				return;
			}
		}
		do {

			_stprintf_s(file_path, MAX_PATH, _T("%s\\%s"), param_path, file_data.cFileName);

			if ((FILE_ATTRIBUTE_DIRECTORY & file_data.dwFileAttributes)) //디렉터리면
				GetFilesFromDirectory(file_path, nameFilter, extFilter, list_arg);
			else
			{
				TCHAR filename[120];
				int len = _tcslen(file_path);
				for (int i = len - 1; i > 0;i--)
					if (file_path[i] == _T('\\'))
					{
						_tcscpy_s(filename, 120, file_path + i + 1);
						break;
					}
				if (Filterling(filename, nameFilter, extFilter)) //filename will be damaged
					list_arg.push_back(file_path);
				OnFindFile(file_path);
			}

		// if FindNextFile cant get file_data from search_handle return INVALID_HANDLE_VALUE(0)
		} while (FindNextFile(search_handle, &file_data));
		FindClose(search_handle);
	}
}


void FileEncode(iconv_t it, tstring path)
{
	FILE *in;
	FILE *out;
	int ret;


	tstring tmppath = path + _T("ICVTMP");
	_trename(path.c_str(), tmppath.c_str());

	_tfopen_s(&out, path.c_str(), _T("w"));
	if (!out)
	{
		return;
	}
	_tfopen_s(&in, tmppath.c_str(), _T("r"));
	if (!in)
	{
		fclose(out);
		return;
	}
	
	char inbuff[MAX_BUFF] = { 0, };
	char outbuff[MAX_BUFF] = { 0, };
	int err;
	const char *inPtr;
	char *outPtr;
	size_t in_size, out_size;

	out_size = MAX_BUFF;
	while (fgets(inbuff, MAX_BUFF, in))
	{
		inPtr = inbuff;
		outPtr = outbuff;
		in_size = strlen(inbuff);
		out_size = MAX_BUFF;
		while (in_size)
		{
			ret = iconv(it, &inPtr, &in_size, &outPtr, &out_size);
		}
		err = errno;
		

		fputs(outbuff, out);




		switch (err)
		{
		case EINVAL:
			break;
		case E2BIG:
			break;
		case EILSEQ:
			break;
		}
		err = 0;
		memset(inbuff, 0, sizeof(inbuff));
		memset(outbuff, 0, sizeof(outbuff));
	}



	fclose(in);
	fclose(out);
	_tremove(tmppath.c_str());

}

void DoEncodeing(void *p)
{
	using namespace std;
	HWND hDlg = (HWND)p;
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CONVERT), 0);

	//Get String from COMBOBOX
	char orgChar[MAX_LENGTH];
	GetStringFromComboBox(hDlg, IDC_COMBO_ORGCHAR, orgChar);
	char dstChar[MAX_LENGTH];
	GetStringFromComboBox(hDlg, IDC_COMBO_DSTCHAR, dstChar);
	
	TCHAR sDir[MAX_LENGTH];
	GetDlgItemText(hDlg, IDC_DRAGHERE, sDir, MAX_PATH);

	TCHAR nameFilter[MAX_LENGTH] = { 0, };
	TCHAR *extFilter=0;
	GetDlgItemText(hDlg, IDC_EDIT_EXTFILTER, nameFilter, MAX_PATH);
	if (nameFilter[0] == 0)
	{
		nameFilter[0] = _T('*');
		extFilter = nameFilter;
	}
	else
	{
		for (int i = _tcslen(nameFilter) - 1; i > 0; i--)
		{
			if (nameFilter[i] == _T('.'))
			{
				extFilter = &nameFilter[i+1];
				nameFilter[i] = 0;
			}
		}
	}

	list<tstring> list;
	GetFilesFromDirectory(sDir, nameFilter, extFilter, list);
	SendMessage(hDlg, WM_SETMAXPROGRESS, list.size(), 0);


	iconv_t it;
	it = iconv_open(dstChar, orgChar);
	if (it != (iconv_t)-1)
	{
		for (auto iter = list.begin(); iter != list.end(); iter++)
			FileEncode(it, *iter);
		iconv_close(it);
	}
	else
		MessageBox(NULL, _T("CANTOPEN ICONV"), _T("ERROR"), MB_OK);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CONVERT), 1);

}



void GetStringFromComboBox(HWND hWnd, int id, char *str) /// No LIMIT LENGTH! 
{
	HWND hCombo = GetDlgItem(hWnd, id);
	if (hCombo)
	{
		int i = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
		if (i>=0)
			SendMessageA(hCombo, CB_GETLBTEXT, i, (LPARAM)str);
	}
}