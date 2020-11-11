#ifndef AIW_H
#define AIW_H

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/metadata.h>
#include <assimp/postprocess.h>

void aiw_init(void);

extern C_ENUM aiReturn (*aiwGetMaterialString)(const C_STRUCT aiMaterial* pMat,
                                               const char* pKey,
                                               unsigned int type,
                                               unsigned int index,
                                               C_STRUCT aiString* pOut);

extern C_ENUM aiReturn (*aiwGetMaterialColor)(const C_STRUCT aiMaterial* pMat,
                                              const char* pKey,
                                              unsigned int type,
                                              unsigned int index,
                                              C_STRUCT aiColor4D* pOut);

extern unsigned int (*aiwGetMaterialTextureCount)(const C_STRUCT aiMaterial* pMat,
                                                  C_ENUM aiTextureType type);

extern C_ENUM aiReturn (*aiwGetMaterialTexture)(
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

extern const C_STRUCT aiScene* (*aiwImportFile)(const char *pFile,
                                                unsigned int pFlags);

extern void (*aiwReleaseImport)(const C_STRUCT aiScene *pScene);

extern const C_STRUCT aiScene* (*aiwImportFileFromMemory)(const char *pBuffer,
        unsigned int pLength,
        unsigned int pFlags,
        const char *pHint);


#endif /* !AIW_H */
