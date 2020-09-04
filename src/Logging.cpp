#include <sstream>
#include <ostream>

struct LogInput
{
	// Todo(Leo): Do make use of our own string classes at some point
	// todo(Leo): these allocate randomly from wherever. It probably doesn't matter, since these should be disabled on
	// release anyway
	std::stringstream 	buffer;
	std::ostream * 		output;

	struct FileAddress
	{
		const char * 	file;
		s32 			line;
	} address;
	bool32 hasFileAddress = false;

	char const * title;
	s32 verbosity;

	bool doPrint = true;

	LogInput () 				= default;
	LogInput (LogInput &&) 		= default;

	LogInput (LogInput const &) = delete; 

	template<typename T>
	LogInput & operator << (T const & value)
	{
		if (doPrint)
			buffer << value;
	
		return *this;
	}

	template<>
	LogInput & operator << <FileAddress>(FileAddress const & value)
	{
		address 		= value;
		hasFileAddress 	= true;

		return *this;
	} 

	~LogInput()
	{
		if (doPrint)
		{
			std::stringstream header;
			header << "[" << title << ":" << verbosity;
			if (hasFileAddress)
			{
				header << ":" << address.file << ":" << address.line;
			}
			header << "]: ";	

			buffer << "\n";

			*output << header.str() << buffer.str();

			// // Note(Leo): We flush so we get immediate output to file.
			// // Todo(Leo): Heard this is unnecessay though, so find out more. Probably has to do with dctor not called when aborting or crashing..
			*output << std::flush;


			if (verbosity == 0 && output != & std::cout)
			{
				std::cout << header.str() << buffer.str();
			}
		}
	}
};

struct LogChannel
{
	char const * 	title;
	s32 			verbosity;

	// Todo(Leo): this is platform thing
	std::ostream * 	output = &std::cout;

	LogInput operator()(int verbosity = 1)
	{
		LogInput result;

		if (verbosity <= this->verbosity)
		{
			result.title 		= title;
			result.verbosity 	= verbosity;
			result.output 		= output;
		}
		else
		{
			result.doPrint = false;
		}

		return result;
	}
};

LogChannel logConsole 	= {"LOG", 5};

LogChannel logDebug 	= {"DEBUG", 5};
LogChannel logWarning	= {"WARNING", 5};
LogChannel logAnim 		= {"ANIMATION", 5};
LogChannel logVulkan 	= {"VULKAN", 5};
LogChannel logWindow	= {"WINDOW", 5};
LogChannel logSystem	= {"SYSTEM", 5};
LogChannel logNetwork	= {"NETWORK", 5};
LogChannel logAudio		= {"AUDIO", 5};

#define FILE_ADDRESS LogInput::FileAddress{__FILE__, __LINE__}
