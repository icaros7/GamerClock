#ifdef NVENC_EXPORTS
#define NVENC_API extern "C" __declspec(dllexport)
#else
#define NVENC_API extern "C" __declspec(dllimport)
#endif

int GetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nDefault);
void SetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nValue);
