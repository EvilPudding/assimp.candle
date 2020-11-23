#include "assimp.h"
#include "../candle/components/model.h"
#include "../candle/components/timeline.h"
#include "../candle/components/bone.h"
#include "../candle/components/skin.h"
#include "../candle/components/light.h"
#include "../candle/components/node.h"
#include "../candle/components/name.h"
#include "../candle/systems/sauces.h"
#include "../candle/utils/file.h"
#include "../candle/utils/mafs.h"

#include "aiw.h"

#include <stdio.h>
#include <stdlib.h>

/* #define OBJ 1199244993 */
/* #define PLY 1842197252 */
/* #define GLTF 1170648920 */
/* #define BIN 1516167604 */
/* #define DAE 1928915418 */
/* #define FBX 804230617 */

c_assimp_t *c_assimp_new()
{
	/* sauces_loader(ref("obj"), NULL); */
	/* sauces_loader(ref("mtl"), NULL); */
		/* sauces_loader(ref("ply"), NULL); */
	sauces_loader(ref("fbx"), NULL);
	sauces_loader(ref("dae"), NULL);
	sauces_loader(ref("bin"), NULL);
	sauces_loader(ref("gltf"), NULL);
	aiw_init();

	c_assimp_t *self = component_new(ct_assimp);
	return self;
}

void mesh_load_scene(mesh_t *self, const void *grp)
{
	mesh_lock(self);
	const struct aiMesh *group = grp;
	strcpy(self->name, "load_result");
	/* self->has_texcoords = 0; */

	int offset = vector_count(self->verts);
	int j;

	if(!group->mTextureCoords[0])
	{
		self->has_texcoords = 0;
	}
	/* for(j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++) */
	/* { */
		/* printf("j %d = %u\n", j, group->mNumUVComponents[j]); */
	/* } */


	for(j = 0; j < group->mNumVertices; j++)
	{
		mesh_add_vert(self, VEC3(_vec3(group->mVertices[j])));
	}
	struct aiVector3D *normals = group->mNormals;
	normals = NULL;
	struct aiVector3D *texcoor = group->mTextureCoords[0];
	for(j = 0; j < group->mNumFaces; j++)
	{
		const struct aiFace *face = &group->mFaces[j];
		unsigned int *indices = face->mIndices;

		if(face->mNumIndices == 3)
		{
			int i0 = indices[0] + offset;
			int i1 = indices[1] + offset;
			int i2 = indices[2] + offset;
			mesh_add_triangle(self,
					i0, normals?vec3(_vec3(normals[i0])):Z3,
					texcoor?vec2(_vec2(texcoor[i0])):Z2,

					i1, normals?vec3(_vec3(normals[i1])):Z3,
					texcoor?vec2(_vec2(texcoor[i1])):Z2,

					i2, normals?vec3(_vec3(normals[i2])):Z3,
					texcoor?vec2(_vec2(texcoor[i2])):Z2);
		}
		else if(face->mNumIndices == 4)
		{
			int i0 = indices[0] + offset;
			int i1 = indices[1] + offset;
			int i2 = indices[2] + offset;
			int i3 = indices[3] + offset;
			mesh_add_quad(self,
					i0, normals?vec3(_vec3(normals[i0])):Z3,
					texcoor?vec2(_vec2(texcoor[i0])):Z2,

					i1, normals?vec3(_vec3(normals[i1])):Z3,
					texcoor?vec2(_vec2(texcoor[i1])):Z2,

					i2, normals?vec3(_vec3(normals[i2])):Z3,
					texcoor?vec2(_vec2(texcoor[i2])):Z2,

					i3, normals?vec3(_vec3(normals[i3])):Z3,
					texcoor?vec2(_vec2(texcoor[i3])):Z2);
		}
	}

	mesh_unlock(self);
}
static void load_node_children(entity_t entity, const struct aiScene *scene,
		const struct aiNode *anode, int depth);
static void load_comp_children(
		entity_t entity,
		const struct aiScene *scene,
		const struct aiNode *anode,
		c_node_t *root
);


static mat_t *load_material(const struct aiMaterial *mat,
		const struct aiScene *scene)
{
	char buffer[512] = "unnamed";
	struct aiString name;

	struct aiString path;
	if(aiwGetMaterialString(mat, AI_MATKEY_NAME, &name) ==
			aiReturn_SUCCESS)
	{
		strncpy(buffer, name.data, sizeof(buffer));
		to_lower_case(buffer);
	}
	strcat(buffer, ".mat");
	mat_t *material = sauces(buffer);
	if(!material)
	{
		material = mat_new(buffer, "default");

		enum aiTextureMapping mapping;
		unsigned int uvi = 0;
		enum aiTextureOp op;
		float blend;
		enum aiTextureMapMode mode;
		unsigned int flags;

		if(aiwGetMaterialTexture( mat, aiTextureType_NORMALS, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				mat1t(material, ref("normal.texture"), texture);
				mat1f(material, ref("normal.blend"), 1.0f - blend);
			}
		}
		if(aiwGetMaterialTexture( mat, aiTextureType_DIFFUSE, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				mat1t(material, ref("albedo.texture"), texture);
				mat1f(material, ref("albedo.blend"), 1.0f - blend);
			}
		}

		if(aiwGetMaterialTexture( mat, aiTextureType_AMBIENT, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				mat1t(material, ref("metalness.texture"), texture);
				mat1f(material, ref("metalness.blend"), 1.0f - blend);
			}
		}

		if(aiwGetMaterialTexture( mat, aiTextureType_SHININESS, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				mat1t(material, ref("roughness.texture"), texture);
				mat1f(material, ref("roughness.blend"), 1.0f - blend);
			}
		}

		vec4_t color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		if (AI_SUCCESS == aiwGetMaterialColor(mat, AI_MATKEY_COLOR_TRANSPARENT,
					(void*)&color))
		{
			mat4f(material, ref("absorve.color"),
			      vec4(1.0f - color.x, 1.0f - color.y,
			           1.0f - color.z, 1.0f - color.w));
		}
		if (AI_SUCCESS == aiwGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE,
					(void*)&color))
		{
			mat4f(material, ref("albedo.color"), color);
		}

/* 		int j; for(j = 0; j < mat->mNumProperties; j++) { */
/* 			printf("%s\n", mat->mProperties[j]->mKey.data); */
/* 		} */
		sauces_register(material->name, NULL, material);
	}
	return material;
}

static inline mat4_t mat4_from_ai(const struct aiMatrix4x4 m)
{
	mat4_t r;
	float *v = (float*)&m;
	r = *(mat4_t*)v;

	r = mat4_transpose(r);
	return r;
}


void load_comp(entity_t entity, const struct aiScene *scene,
		const struct aiNode *anode, c_node_t *root)
{
	int m;
	c_model_t *mc = c_model(&entity);
	if(!mc && anode->mNumMeshes)
	{
		entity_add_component(entity, c_model_new(mesh_new(),
					sauces("_default.mat"), 1, 1));
		mc = c_model(&entity);
	}

	int last_vertex = 0;
	for(m = 0; m < anode->mNumMeshes; m++)
	{
		const struct aiMesh *mesh = scene->mMeshes[anode->mMeshes[m]];

		mesh_lock(mc->mesh);
		mesh_load_scene(mc->mesh, mesh);
		if(mesh->mNumBones)
		{
			int b;
			c_skin_t *skin = c_skin(&entity);
			if(!skin)
			{
				entity_add_component(entity, c_skin_new());
				skin = c_skin(&entity);
			}

			for(b = 0; b < mesh->mNumBones; b++)
			{
				int w;
				const struct aiBone* abone = mesh->mBones[b];
				entity_t bone = c_node_get_by_name(root, ref(abone->mName.data));

				if (!bone || abone->mNumWeights == 0)
					continue;

				int bone_index = skin->info.bones_num;

				skin->info.bones[bone_index] = bone;
				skin->info.bones_num++;

				if (!c_bone(&bone))
				{
					entity_add_component(bone, c_bone_new(entity,
								mat4_from_ai(abone->mOffsetMatrix)));
				}

				for(w = 0; w < abone->mNumWeights; w++)
				{
					mc->mesh->has_skin = 1;
					const struct aiVertexWeight *vweight = &abone->mWeights[w];
					int real_id = (vweight->mVertexId + last_vertex);
					vertex_t *vert = vector_get(mc->mesh->verts, real_id);

					int i;
					for(i = 0; i < 4; i++)
					{
						if(((float*)&vert->wei)[i]  == 0.0f)
						{
							((float*)&vert->wei)[i] = vweight->mWeight;
							((float*)&vert->bid)[i] = bone_index;
							break;
						}
					}
					if(i == 4) printf("TOO MANY WEIGHTS\n");
					mesh_modified(mc->mesh);
				}
			}
		}
		last_vertex += mesh->mNumVertices;
		mesh_unlock(mc->mesh);

		if(mesh->mMaterialIndex >= scene->mNumMaterials) continue;
		const struct aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
		mat_t *material = load_material(mat, scene);
		c_model_set_mat(mc, material);
	}

	load_comp_children(entity, scene, anode, root);
}

static void load_comp_children(entity_t entity, const struct aiScene *scene,
		const struct aiNode *anode, c_node_t *root)
{
	c_node_t *node = c_node(&entity);
	int i;
	for(i = 0; i < anode->mNumChildren; i++)
	{
		const struct aiNode *cnode = anode->mChildren[i];
		const char *name = cnode->mName.data;

		if(!name[0])
		{
			load_comp_children(entity, scene, cnode, root);
			continue;
		}
		entity_t n = c_node_get_by_name(node, ref(name));
		load_comp(n, scene, cnode, root);
	}
}

static void load_node(entity_t entity, const struct aiScene *scene,
		const struct aiNode *anode, int depth)
{
	int inherit_type = 0;
	c_spatial_t *spatial = c_spatial(&entity);

	const struct aiMetadata *meta = anode->mMetaData;
	if(meta)
	{
		int j;
		for(j = 0; j < meta->mNumProperties; j++)
		{
			const struct aiMetadataEntry *v = &meta->mValues[j];
			if(!strcmp(meta->mKeys[j].data, "InheritType"))
			{
				inherit_type = *(char*)v->mData;
			}
			continue;
			printf("%s: ", meta->mKeys[j].data);
			switch(v->mType)
			{
				case AI_BOOL: printf("%d\n", *(char*)v->mData); break;
				case AI_INT32: printf("%u\n", *(unsigned int*)v->mData); break;
				case AI_UINT64: printf("%lu\n", *(unsigned long*)v->mData); break;
				case AI_FLOAT: printf("%f\n", *(double*)v->mData); break;
				case AI_DOUBLE: printf("%lf\n", *(double*)v->mData); break;
				case AI_AISTRING: printf("'%s'\n", ((const struct aiString*)v->mData)->data); break;
				case AI_AIVECTOR3D: break;
				case AI_META_MAX: break;
				default: break;
			}
		}
	}
	/* if(inherit_type) */
	/* { */
		/* c_spatial_set_model(spatial, mat4_from_ai(anode->mParent->mTransformation)); */
	/* } */
	/* else */
	{
		c_spatial_set_model(spatial, mat4_from_ai(anode->mTransformation));
	}
	load_node_children(entity, scene, anode, depth);
}

static void load_node_children(entity_t entity, const struct aiScene *scene,
		const struct aiNode *anode, int depth)
{
	c_node_t *node = c_node(&entity);
	int i;
	for(i = 0; i < anode->mNumChildren; i++)
	{
		const struct aiNode *cnode = anode->mChildren[i];
		const char *name = cnode->mName.data;

		if(!name[0])
		{
			load_node_children(entity, scene, cnode, depth);
			continue;
		}
		entity_t n = entity_new({c_name_new(name); c_node_new();});
		c_node_add(node, 1, n);
		load_node(n, scene, cnode, depth + 1);
	}
}

static void load_timelines(entity_t entity, const struct aiScene *scene)
{
	c_node_t *root = c_node(&entity);
	int a;
	for(a = 0; a < scene->mNumAnimations; a++)
	{
		int n;
		const struct aiAnimation *anim = scene->mAnimations[a];

		for(n = 0; n < anim->mNumChannels; n++)
		{
			int k;
			const struct aiNodeAnim *node_anim = anim->mChannels[n];
			const char *name = node_anim->mNodeName.data;
			if(!name) continue;
			entity_t ent = c_node_get_by_name(root, ref(name));
			if(!ent) continue;

			c_timeline_t *tc = c_timeline(&ent);
			if(!tc)
			{
				entity_add_component(ent, c_timeline_new());
				tc = c_timeline(&ent);
			}
			tc->duration = anim->mDuration;
			tc->ticks_per_sec = anim->mTicksPerSecond;
			if(!tc->ticks_per_sec) tc->ticks_per_sec = 30;

			c_timeline_clear(tc);
			vector_alloc(tc->keys_pos, node_anim->mNumPositionKeys);
			vector_alloc(tc->keys_rot, node_anim->mNumRotationKeys);
			vector_alloc(tc->keys_scale, node_anim->mNumScalingKeys);

			for(k = 0; k < node_anim->mNumScalingKeys; k++)
			{
				const struct aiVectorKey *key = &node_anim->mScalingKeys[k];
				vec3_t vec = vec3(key->mValue.x, key->mValue.y, key->mValue.z);
				c_timeline_insert_scale(tc, vec, key->mTime);
			}
			for(k = 0; k < node_anim->mNumRotationKeys; k++)
			{
				const struct aiQuatKey *key = &node_anim->mRotationKeys[k];
				vec4_t quat = vec4(key->mValue.x, key->mValue.y, key->mValue.z,
						key->mValue.w);

				c_timeline_insert_rot(tc, quat, key->mTime);
			}
			for(k = 0; k < node_anim->mNumPositionKeys; k++)
			{
				const struct aiVectorKey *key = &node_anim->mPositionKeys[k];
				vec3_t vec = vec3(key->mValue.x, key->mValue.y, key->mValue.z);
				c_timeline_insert_pos(tc, vec, key->mTime);
			}

		}
		break;
	}
}

static int c_assimp_load(c_assimp_t *self,
		struct load_signal *info, entity_t *target)
{
	int i;
	printf("getting sauce %s\n", info->filename);
	resource_handle_t handle = sauce_handle(info->filename);
	resource_t *sauce = c_sauces_get_sauce(c_sauces(&SYS), handle);
	if(!sauce) return STOP;
	size_t bytes_num = 0;
	char *bytes = c_sauces_get_bytes(c_sauces(&SYS), sauce, &bytes_num);

	const struct aiScene *scene = aiwImportFileFromMemory(bytes, bytes_num,
			/* aiProcess_CalcTangentSpace  		| */
			aiProcess_Triangulate			    |
			/* aiProcess_GenSmoothNormals		| */
			aiProcess_JoinIdenticalVertices 	|
			aiProcess_SortByPType, strrchr(sauce->path, '.') + 1);
	if(!scene)
	{
		printf("failed to load %s\n", info->filename);
		return CONTINUE;
	}

	int anim_only = 1;
	if(!entity_exists(*target) || (*target) == SYS)
	{
		anim_only = 0;
		*target = entity_new({c_name_new(info->filename); c_node_new();});
	}
	/* c_model(&result)->mesh->transformation = */
	/* 	mat4_scale_aniso(c_model(&result)->mesh->transformation, vec3(scale)); */

	c_node_t *root = c_node(target);
	if(!anim_only)
	{
		load_node(*target, scene, scene->mRootNode, 0);
		load_comp(*target, scene, scene->mRootNode, root);
	}
	load_timelines(*target, scene);

	if(anim_only)
	{
		printf("Loading materials\n");
		for(i = 0; i < scene->mNumMaterials; i++)
		{
			const struct aiMaterial *mat = scene->mMaterials[i];
			load_material(mat, scene);
		}
	}

	for(i = 0; i < scene->mNumLights; i++)
	{
		const struct aiLight *light = scene->mLights[i];
		entity_t node = c_node_get_by_name(root, ref(light->mName.data));
		if(node)
		{
			c_light_t *lc = c_light(&node);
			if(!lc)
			{
				vec4_t color = vec4(
					light->mColorDiffuse.r,
					light->mColorDiffuse.g,
					light->mColorDiffuse.b,
					1.0f
				);
				entity_add_component(node, c_light_new(40.0f, color));
			}

			/* load_light(lc, light); */
		}
		else
		{
			printf("%s not found for light\n", light->mName.data);
		}
	}
	if(!anim_only)
	{
		c_spatial_set_scale(c_spatial(target), vec3(info->scale, info->scale, info->scale));
	}

	aiwReleaseImport(scene);
    return STOP;
    /* return HANDLED; */
}

void ct_assimp(ct_t *self)
{
	ct_init(self, "assimp", sizeof(c_assimp_t));

	ct_add_listener(self, WORLD, 100, ref("load"), c_assimp_load);
}

