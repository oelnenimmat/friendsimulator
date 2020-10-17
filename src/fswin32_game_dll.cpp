/*
Leo Tamminen

Dynamically loaded game code dll
*/

// Todo(Leo): Change to wide strings???
constexpr internal char const * gamecode_dll_file_name 		= "friendsimulator.dll";
constexpr internal char const * gamecode_dll_file_name_temp = "friendsimulator_temp.dll";
constexpr internal char const * gamecode_update_func_name	= "update_game";
constexpr internal char const * gamecode_checkfilename 		= "friendsimulator_game_dll_built_checkfile";

struct Win32Game
{
	// Note(Leo): these are for hot reloading game code during development
	// Todo(Leo): Remove functionality from final game
	HMODULE dllHandle;
	FILETIME dllWriteTime;
	bool32 shouldReInitializeGlobalVariables;

	UpdateGameFunc * update;	
};

internal bool32 fswin32_game_is_loaded(Win32Game * game)
{
	bool32 loaded   = game->dllHandle != nullptr
					&& game->update != nullptr;
	return loaded;
}

internal FILETIME fswin32_file_get_write_time(const char * fileName)
{
    // Todo(Leo): Make this function sensible
    WIN32_FILE_ATTRIBUTE_DATA fileInfo = {};
    if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileInfo))
    {
    }
    else
    {
        // Todo(Leo): Now what??? Getting file time failed --> file does not exist??
    }    
    FILETIME result = fileInfo.ftLastWriteTime;
    return result;
}


internal void fswin32_game_load_dll(Win32Game * game)
{
	Assert(fswin32_game_is_loaded(game) == false && "Game code is already loaded, unload before reloading");

	CopyFileA(gamecode_dll_file_name, gamecode_dll_file_name_temp, false);
	game->dllHandle = LoadLibraryA(gamecode_dll_file_name_temp);
	if(game->dllHandle != nullptr)
	{
		FARPROC procAddress = GetProcAddress(game->dllHandle, gamecode_update_func_name);
		game->update        = reinterpret_cast<UpdateGameFunc*>(procAddress);
		game->dllWriteTime  = fswin32_file_get_write_time(gamecode_dll_file_name);

		DeleteFileA(gamecode_checkfilename);
	}

	Assert(fswin32_game_is_loaded(game) && "Game not loaded");
}

internal void fswin32_game_unload_dll(Win32Game * game)
{
	FreeLibrary(game->dllHandle);
	game->dllHandle =  nullptr;

	DeleteFileA(gamecode_dll_file_name_temp);
}

internal void fswin32_game_reload(Win32Game & game)
{
	FILETIME dllLatestWriteTime = fswin32_file_get_write_time(gamecode_dll_file_name);
	if (CompareFileTime(&dllLatestWriteTime, &game.dllWriteTime) > 0)
	{
		HANDLE checkfile = CreateFileA(gamecode_checkfilename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, nullptr);

		log_application(1, "Attempting to reload game");
		
		if (checkfile != INVALID_HANDLE_VALUE)
		{
    		CloseHandle(checkfile);          

			fswin32_game_unload_dll(&game);
			fswin32_game_load_dll(&game);

			log_application(0, "Reloaded game");

			game.shouldReInitializeGlobalVariables = true;
		}
		else
		{
            // Todo(Leo): Do we need to CloseHandle here???
			log_application(1, "Did not reload the game: checkfile not available");
		}
	}
}