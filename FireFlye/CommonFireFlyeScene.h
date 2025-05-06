#pragma once
#include <set>
#include <cctype>
#include <shlobj.h>
#include <windows.h>
#include <shobjidl_core.h>

#include "BaseScene.h"
#include "RipeGrain/UILayer.h"

#include "VideoReader.h"
#include "RecordManagerModule.h"

const std::string MEDIA_DIRECTORY = "./Resources/";

class CommonFireFLyeScene : public BaseScene
{
protected:
	RipeGrain::SceneObject background;
protected:
	RipeGrain::UILayer ui_layer;
	RipeGrain::ObjectsLayer background_layer;
public:
	void Initialize() override
	{
		SetViewPortSize(750, 650);
		background.AddSprite(CreateSprite(Image{ MEDIA_DIRECTORY + "fireflyebg.jpg" }));
		background.SetPosition(370, 250);
		background_layer.AddObject(&background);
		ui_layer.ui_clip_buffer = getCoreEngine().CreateStencilBuffer(750, 650);
		AddLayer(&background_layer);
		AddLayer(&ui_layer);
		RegisterEvent(RipeGrain::CreateUIRegisterEvent(&ui_layer));
	}
public:
	virtual ~CommonFireFLyeScene() = default;
};

inline std::string convert_to_string(const std::wstring& wstr)
{
	if (wstr.empty())
		return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
inline std::wstring convert_to_wstring(const std::string& str)
{
	if (str.empty())
		return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
inline std::optional<std::string> ShowSelectFolderOrFileDialogue()
{
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = "Select a File or Directory";
	bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != 0)
	{
		char path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			std::string result(path);
			CoTaskMemFree(pidl);
			return result;
		}
		CoTaskMemFree(pidl);
	}
	return std::nullopt;
}

// Function to open the Save File dialog and get the file path 
inline std::optional<std::wstring> ShowOpenSaveFileDialogue() {
	IFileSaveDialog* pFileSave = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, (void**)&pFileSave);
	if (SUCCEEDED(hr)) {
		hr = pFileSave->Show(NULL);
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem;
			hr = pFileSave->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pszFilePath;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					std::wstring filePath = pszFilePath;
					CoTaskMemFree(pszFilePath);
					pItem->Release();
					pFileSave->Release();
					CoUninitialize();
					return filePath;
				}
				pItem->Release();
			}
		}
		pFileSave->Release();
	}
	return std::nullopt;
}
// file_filter_map: key = file Type name, value = file extension (seperated by ';')
// returns: file path if user selected a file, std::nullopt if user cancelled
// example usage: std::optional<std::string> file_path = OpenFile("Select a file", { {"Text Files", "*.txt"}, {"All Files", "*.*"} });
inline std::optional<std::string> ShowOpenFileDialogue(const std::initializer_list<std::pair<std::string, std::string>>& file_filter_map, HWND parent = nullptr)
{
	OPENFILENAME ofn;
	//maximum length of a file name
	char szFile[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = parent;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	std::vector<char> file_filter_buffer;
	for (auto& [type, ext] : file_filter_map)
	{
		file_filter_buffer.insert(file_filter_buffer.end(), type.begin(), type.end());
		file_filter_buffer.push_back('\0');
		file_filter_buffer.insert(file_filter_buffer.end(), ext.begin(), ext.end());
		file_filter_buffer.push_back('\0');
	}
	//when the api detects 2 null characters it stops reading the string
	file_filter_buffer.push_back('\0');
	ofn.lpstrFilter = file_filter_buffer.data();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		return ofn.lpstrFile;
	}
	return std::nullopt;
}
inline void MarkPathInExplorer(const std::filesystem::path& p)
{
	auto pa = p.string();
	//ShellExecute(NULL, "open", pa.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	ShellExecute(nullptr, "open", "explorer.exe", ("/select," + pa).c_str(), nullptr, SW_SHOWDEFAULT);
}
inline void OpenFolderInsideExplorer(const std::filesystem::path& p)
{
	auto pa = p.string();
	ShellExecute(nullptr, "open", pa.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}