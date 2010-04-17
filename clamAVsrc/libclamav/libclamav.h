// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBCLAMAV_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBCLAMAV_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBCLAMAV_EXPORTS
#define LIBCLAMAV_API __declspec(dllexport)
#else
#define LIBCLAMAV_API __declspec(dllimport)
#endif

// This class is exported from the libclamav.dll
class LIBCLAMAV_API Clibclamav {
public:
	Clibclamav(void);
	// TODO: add your methods here.
};

extern LIBCLAMAV_API int nlibclamav;

LIBCLAMAV_API int fnlibclamav(void);
