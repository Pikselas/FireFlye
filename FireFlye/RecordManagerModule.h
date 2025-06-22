#pragma once
#include <memory>
#include <windows.h>
#include <cassert>
class RecordManagerModule
{
public:
	typedef struct
	{
		int id;
		char* name;
		int len;
	} Record, Tag, FireData;
	typedef struct
	{
		FireData* data;
		int len;
	}RecordList, TagList, FireDataList;
	struct RecordManager
	{
		int index;
		char* name;
		int len;
	};
	struct RecordManagerList
	{
		RecordManager* data;
		int len;
	};
	struct RecordPreviewData
	{
		int id;
		char* name;
		int len;
		char* preview;
		int preview_len;
	};
	struct RecordPreviewList
	{
		RecordPreviewData* data;
		int len;
	};
protected:
	typedef void(*fn_type_load_db)(const char*, int);
	typedef unsigned char (*fn_type_is_foreign_record_manager)(const char*, int);
	typedef void (*fn_type_import_foreign_tags)(const char*, int);
	typedef int (*fn_type_create_record)(int, const char*, int);
	typedef int (*fn_type_create_tag)(const char*, int);
	typedef RecordList(*fn_type_search_record_by_name)(int, const char*, int);
	typedef RecordList(*fn_type_search_record_by_tags)(int, int*, int);
	typedef TagList(*fn_type_search_tag_by_name)(const char*, int);
	typedef void (*fn_type_bind_record_to_tag)(int, int, int);
	typedef void (*fn_type_release_data_list)(FireDataList);
	typedef RecordManagerList(*fn_type_get_record_manager_list)();
	typedef void (*fn_type_release_record_manager_list)(RecordManagerList);
	typedef RecordPreviewList(*fn_type_search_record_with_preview_by_name)(int, const char*, int);
	typedef RecordPreviewList(*fn_type_get_previews_by_record_id)(int, int);
	typedef void (*fn_type_release_record_preview_list)(RecordPreviewList);
	typedef void (*fn_type_add_preview)(int , int, const char*, int);
public:
	fn_type_load_db load_db											= nullptr;
	fn_type_create_tag create_tag									= nullptr;
	fn_type_create_record create_record								= nullptr;
	fn_type_release_data_list release_data_list						= nullptr;
	fn_type_search_tag_by_name search_tag_by_name					= nullptr;
	fn_type_bind_record_to_tag bind_record_to_tag					= nullptr;
	fn_type_import_foreign_tags import_foreign_tags					= nullptr;
	fn_type_search_record_by_name search_record_by_name				= nullptr;
	fn_type_search_record_by_tags search_record_by_tags				= nullptr;
	fn_type_is_foreign_record_manager is_foreign_record_manager		= nullptr;
	fn_type_get_record_manager_list get_record_manager_list			= nullptr;
	fn_type_release_record_manager_list release_record_manager_list = nullptr;
	fn_type_search_record_with_preview_by_name search_record_with_preview_by_name = nullptr;
	fn_type_get_previews_by_record_id get_previews_by_record_id = nullptr;
	fn_type_release_record_preview_list release_record_preview_list = nullptr;
	fn_type_add_preview add_preview = nullptr;
private:
	HMODULE runtime_manager_module = nullptr;
private:
	void load_manager_runtime()
	{
		runtime_manager_module = LoadLibrary("FlyeManager.dll");
		assert(runtime_manager_module != nullptr);

		load_db = (fn_type_load_db)GetProcAddress(runtime_manager_module, "load_db");
		create_tag = (fn_type_create_tag)GetProcAddress(runtime_manager_module, "create_tag");
		create_record = (fn_type_create_record)GetProcAddress(runtime_manager_module, "create_record");
		release_data_list = (fn_type_release_data_list)GetProcAddress(runtime_manager_module, "release_data_list");
		search_tag_by_name = (fn_type_search_tag_by_name)GetProcAddress(runtime_manager_module, "search_tag_by_name");
		bind_record_to_tag = (fn_type_bind_record_to_tag)GetProcAddress(runtime_manager_module, "bind_record_to_tag");
		search_record_by_name = (fn_type_search_record_by_name)GetProcAddress(runtime_manager_module, "search_record_by_name");
		search_record_by_tags = (fn_type_search_record_by_tags)GetProcAddress(runtime_manager_module, "search_record_by_tags");
		is_foreign_record_manager = (fn_type_is_foreign_record_manager)GetProcAddress(runtime_manager_module, "is_foreign_record_manager");
		import_foreign_tags = (fn_type_import_foreign_tags)GetProcAddress(runtime_manager_module, "import_foreign_tags");
		get_record_manager_list = (fn_type_get_record_manager_list)GetProcAddress(runtime_manager_module, "get_record_manager_list");
		release_record_manager_list = (fn_type_release_record_manager_list)GetProcAddress(runtime_manager_module, "release_record_manager_list");
		search_record_with_preview_by_name = (fn_type_search_record_with_preview_by_name)GetProcAddress(runtime_manager_module, "search_record_with_preview_by_name");
		release_record_preview_list = (fn_type_release_record_preview_list)GetProcAddress(runtime_manager_module, "release_record_preview_list");
		add_preview = (fn_type_add_preview)GetProcAddress(runtime_manager_module, "add_preview");
		get_previews_by_record_id = (fn_type_get_previews_by_record_id)GetProcAddress(runtime_manager_module, "get_previews_by_record_id");
	}
private:
	RecordManagerModule() : runtime_manager_module(nullptr)
	{
		load_manager_runtime();
	}
	~RecordManagerModule()
	{
		if (runtime_manager_module != nullptr)
			FreeLibrary(runtime_manager_module);
	}
private:
	static RecordManagerModule manager_module;
public:
	static RecordManagerModule& GetInstance()
	{
		return manager_module;
	}
	RecordManagerModule& operator=(const RecordManagerModule&) = delete;
	RecordManagerModule(const RecordManagerModule&) = delete;
};

inline RecordManagerModule RecordManagerModule::manager_module;