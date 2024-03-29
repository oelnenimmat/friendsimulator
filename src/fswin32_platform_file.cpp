/*
Leo Tamminen

platform_file_XXXX functions' implementations.
*/

PlatformFileHandle platform_file_open(char const * filename, FileMode fileMode)
{
	DWORD access;
	// Todo(Leo): this should only be enabled in development
	DWORD share = 0;
	DWORD creation;

	if(fileMode == FileMode_read)
	{
		access 		= GENERIC_READ;
		share 		= FILE_SHARE_READ;
		creation 	= OPEN_EXISTING;
	}
	else if (fileMode == FileMode_write)
	{
		access 		= GENERIC_WRITE;
		creation 	= OPEN_ALWAYS;
	}

	// Todo(Leo): check lpSecurityAttributes
	HANDLE file = CreateFile(	filename,
								access,
								share,
								nullptr,
								creation,
								FILE_ATTRIBUTE_NORMAL,
								nullptr);

	DWORD error = 0;
	if(file == INVALID_HANDLE_VALUE)
	{
		error = GetLastError();
	}


	SetFilePointer((HANDLE)file, 0, nullptr, FILE_BEGIN);

	// Todo(Leo): this may be unwanted
	if (fileMode == FileMode_write)
	{
		SetEndOfFile((HANDLE)file);
	}

	PlatformFileHandle result = (PlatformFileHandle)file;
	return result;
}

void platform_file_close(PlatformFileHandle file)
{
	CloseHandle((HANDLE)file);
}

void platform_file_write (PlatformFileHandle file, s64 position, s64 count, const void * memory)
{
	LARGE_INTEGER POSITION; // Note(Leo): lol
	POSITION.QuadPart = position;
	SetFilePointer(file, POSITION.LowPart, &POSITION.HighPart, FILE_BEGIN);

	DWORD bytesWritten;
	WriteFile((HANDLE)file, memory, count, &bytesWritten, nullptr);
}

void platform_file_read (PlatformFileHandle file, s64 position, s64 count, void * memory)
{
	LARGE_INTEGER POSITION;
	POSITION.QuadPart = position;
	SetFilePointer(file, POSITION.LowPart, &POSITION.HighPart, FILE_BEGIN);

	DWORD bytesRead;
	ReadFile((HANDLE)file, memory, count, &bytesRead, nullptr);
}

s64 platform_file_get_size(PlatformFileHandle file)
{
	LARGE_INTEGER fileSize;
	// GetFileSizeEx((HANDLE)file, &fileSize);

	DWORD highPart = 0;
	DWORD lowPart = GetFileSize((HANDLE)file, &highPart);
	DWORD error;
	if (lowPart == 0xffffffff)
	{
		error = GetLastError();
	}

	fileSize.LowPart = lowPart; 
	fileSize.HighPart = highPart;

	Assert(fileSize.QuadPart < max_value_s64);

	return fileSize.QuadPart;
}
