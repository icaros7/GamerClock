#ifdef VCE_EXPORTS
#define VCE_API extern "C" __declspec(dllexport)
#else
#define VCE_API extern "C" __declspec(dllimport)
#endif

int GetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nDefault);
void SetConfigInt(LPCSTR lpSection, LPCSTR lpName, int nValue);
