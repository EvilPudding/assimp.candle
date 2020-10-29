#include "aiw.h"

#ifdef _WIN32
#include <windows.h>
#elif !defined(__EMSCRIPTEN__)
#include <unistd.h>
#include <dlfcn.h>
#endif
#include "../candle/systems/sauces.h"


C_ENUM aiReturn (*aiwGetMaterialString)(const C_STRUCT aiMaterial* pMat,
    const char* pKey,
    unsigned int type,
    unsigned int index,
    C_STRUCT aiString* pOut);

C_ENUM aiReturn (*aiwGetMaterialColor)(const C_STRUCT aiMaterial* pMat,
    const char* pKey,
    unsigned int type,
    unsigned int index,
    C_STRUCT aiColor4D* pOut);

C_ENUM aiReturn (*aiwGetMaterialString)(const C_STRUCT aiMaterial* pMat,
    const char* pKey,
    unsigned int type,
    unsigned int index,
    C_STRUCT aiString* pOut);

unsigned int (*aiwGetMaterialTextureCount)(const C_STRUCT aiMaterial* pMat,
                                           C_ENUM aiTextureType type);

C_ENUM aiReturn (*aiwGetMaterialTexture)(
    const C_STRUCT aiMaterial* mat,
    C_ENUM aiTextureType type,
    unsigned int  index,
    C_STRUCT aiString* path,
    C_ENUM aiTextureMapping* mapping    /*= NULL*/,
    unsigned int* uvindex               /*= NULL*/,
    ai_real* blend                      /*= NULL*/,
    C_ENUM aiTextureOp* op              /*= NULL*/,
    C_ENUM aiTextureMapMode* mapmode    /*= NULL*/,
    unsigned int* flags                 /*= NULL*/);

const C_STRUCT aiScene* (*aiwImportFile)(
        const char *pFile,
        unsigned int pFlags);

void (*aiwReleaseImport)(
        const C_STRUCT aiScene *pScene);

void aiw_init(void)
{
#ifdef _WIN32
#define ailib(l) LoadLibrary(l)
#define aisym(v, type, l, s) v = (type)GetProcAddress(l, #s)
#define aiclose(l)
#elif defined(__EMSCRIPTEN__)
#define ailib(l)
#define aisym(v, type, l, s) v = (type)s
#define aiclose(l)
#else
#define ailib(l) dlopen(l, RTLD_NODELETE)
#define aisym(v, type, l, s) v = (type)dlsym(l, #s)
#define aiclose(l)
#endif

	FILE *fp;
#ifdef _WIN32
	HINSTANCE ailib;
	char lib_filename[MAX_PATH];  
	strcpy(lib_filename, "assimp.dll");
	fp = fopen(lib_filename, "r");
	if (fp == NULL)
	{
		char temp_path[MAX_PATH];
		HANDLE fd, file_map;
		LPVOID address;
		size_t bytes_num;

		resource_t *sauce = c_sauces_get_sauce(c_sauces(&SYS), sauce_handle(lib_filename));
		char *bytes = c_sauces_get_bytes(c_sauces(&SYS), sauce, &bytes_num);

		GetTempPath(MAX_PATH, temp_path);
		GetTempFileName(temp_path, TEXT("tempoai"), 0, lib_filename);

		fd = CreateFile(lib_filename, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		file_map = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, bytes_num, NULL);	    
		address = MapViewOfFile(file_map, FILE_MAP_WRITE, 0, 0, 0);	    
		CopyMemory(address, bytes, bytes_num);	    
		UnmapViewOfFile(address);
		CloseHandle(file_map);
		CloseHandle(fd);
	}
	ailib = ailib(lib_filename);
#elif !defined(__EMSCRIPTEN__)
	void *ailib;
	char lib_filename[PATH_MAX];  
	strcpy(lib_filename, "libassimp.so");
	fp = fopen(lib_filename, "r");
	if (fp == NULL)
	{
		char temp_name[] = "XXXXXXX";
		int fd = mkstemp(temp_name);
		size_t bytes_num;

		resource_t *sauce = c_sauces_get_sauce(c_sauces(&SYS), sauce_handle(lib_filename));
		char *bytes = c_sauces_get_bytes(c_sauces(&SYS), sauce, &bytes_num);

		if (write(fd, bytes, bytes_num) == -1)
		{
			printf("Failed to write to ai shared library temp file.\n");
			exit(1);
		}
		close(fd);
	}
	ailib = ailib(lib_filename);
#else
	void *ailib;
#endif

	aisym(aiwGetMaterialString, C_ENUM aiReturn (*)(const C_STRUCT aiMaterial* pMat,
	                                                const char* pKey,
	                                                unsigned int type,
	                                                unsigned int index,
	                                                C_STRUCT aiString* pOut),
	                                                ailib, aiGetMaterialString);

	aisym(aiwGetMaterialColor, C_ENUM aiReturn (*)(const C_STRUCT aiMaterial* pMat,
				const char* pKey,
				unsigned int type,
				unsigned int index,
				C_STRUCT aiColor4D* pOut), ailib, aiwGetMaterialColor);

	aisym(aiwGetMaterialString, C_ENUM aiReturn (*)(const C_STRUCT aiMaterial* pMat,
				const char* pKey,
				unsigned int type,
				unsigned int index,
				C_STRUCT aiString* pOut), ailib, aiwGetMaterialString);

	aisym(aiwGetMaterialTextureCount, unsigned int (*)(const C_STRUCT aiMaterial* pMat,
			C_ENUM aiTextureType type), ailib, aiGetMaterialTextureCount);

	aisym(aiwGetMaterialTexture, C_ENUM aiReturn (*)(
			const C_STRUCT aiMaterial* mat,
			C_ENUM aiTextureType type,
			unsigned int index,
			C_STRUCT aiString* path,
			C_ENUM aiTextureMapping* mapping,
			unsigned int* uvindex,
			ai_real* blend,
			C_ENUM aiTextureOp* op,
			C_ENUM aiTextureMapMode* mapmode,
			unsigned int* flags), ailib,
		aiGetMaterialTexture);

	aisym(aiwImportFile, const C_STRUCT aiScene* (*)(const char *pFile,
			unsigned int pFlags), ailib, aiwImportFile);

	aisym(aiwReleaseImport, void (*)(const C_STRUCT aiScene *pScene), ailib, aiReleaseImport);
	aiclose(ailib);

#undef ailib
#undef aisym
}
