#include "assimp.h"
#include <components/model.h>
#include <components/timeline.h>
#include <components/bone.h>
#include <components/skin.h>
#include <components/light.h>
#include <components/node.h>
#include <components/name.h>
#include <systems/sauces.h>
#include <utils/file.h>
#include <utils/mafs.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/metadata.h>
#include <assimp/postprocess.h>

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

	c_assimp_t *self = component_new("assimp");
	return self;
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
	if(aiGetMaterialString(mat, AI_MATKEY_NAME, &name) ==
			aiReturn_SUCCESS)
	{
		strncpy(buffer, name.data, sizeof(buffer));
		to_lower_case(buffer);
	}
	strcat(buffer, ".mat");
	mat_t *material = sauces(buffer);
	if(!material)
	{
		material = mat_new(buffer);

		enum aiTextureMapping mapping;
		unsigned int uvi = 0;
		enum aiTextureOp op;
		float blend;
		enum aiTextureMapMode mode;
		unsigned int flags;

		if(aiGetMaterialTexture( mat, aiTextureType_NORMALS, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				material->normal.texture = texture;
				material->normal.texture_blend = 1.0f - blend;
				material->normal.texture_scale = 1.0f;
			}
		}
		if(aiGetMaterialTexture( mat, aiTextureType_DIFFUSE, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				material->albedo.texture = texture;
				material->albedo.texture_blend = 1.0f - blend;
				material->albedo.texture_scale = 1.0f;
			}
		}

		if(aiGetMaterialTexture( mat, aiTextureType_SHININESS, 0, &path,
					&mapping, &uvi, &blend, &op, &mode, &flags) ==
				aiReturn_SUCCESS)
		{
			strncpy(buffer, path.data, sizeof(buffer));
			char *fname = filter_sauce_name(buffer);
			texture_t *texture = sauces(fname);
			if(texture)
			{
				material->roughness.texture = texture;
				material->roughness.texture_blend = 1.0f - blend;
				material->roughness.texture_scale = 1.0f;
			}
		}

		vec4_t color = vec4(0,0,0,1);
		if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_TRANSPARENT,
					(void*)&color))
		{
			material->transparency.color = vec4(1-color.x, 1-color.y, 1-color.z, 1-color.a);
		}
		if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE,
					(void*)&color))
		{
			material->albedo.color = color;
		}

		/* int j; for(j = 0; j < mat->mNumProperties; j++) { printf("%s\n", mat->mProperties[j]->mKey.data); } */
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
			c_skin_vert_prealloc(skin, vector_count(mc->mesh->verts));

			for(b = 0; b < mesh->mNumBones; b++)
			{
				int w;
				const struct aiBone* abone = mesh->mBones[b];
				entity_t bone = c_node_get_by_name(root, ref(abone->mName.data));
				if(!bone) continue;
				int bone_index = skin->bones_num;

				skin->bones[bone_index] = bone;
				mat4_t offset = mat4_from_ai(abone->mOffsetMatrix);
				skin->off[bone_index] = offset;
				skin->bones_num++;

				if(!c_bone(&bone))
				{
					entity_add_component(bone, c_bone_new());
				}
				for(w = 0; w < abone->mNumWeights; w++)
				{
					int i;
					const struct aiVertexWeight *vweight = &abone->mWeights[w];
					int real_id = (vweight->mVertexId + last_vertex);
					for(i = 0; i < 4; i++)
					{
						if(skin->wei[real_id]._[i] == 0.0f)
						{
							skin->wei[real_id]._[i] = vweight->mWeight;
							skin->bid[real_id]._[i] = bone_index;
							break;
						}
					}
					if(i == 4) printf("TOO MANY WEIGHTS\n");
				}
			}
		}
		last_vertex += mesh->mNumVertices;

		if(mesh->mMaterialIndex >= scene->mNumMaterials) continue;
		const struct aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
		mat_t *material = load_material(mat, scene);
		mc->layers[0].mat = material;
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
	c_spacial_t *spacial = c_spacial(&entity);

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

	if(inherit_type)
	{
		c_spacial_set_model(spacial, mat4_from_ai(anode->mParent->mTransformation));
	}
	else
	{
		c_spacial_set_model(spacial, mat4_from_ai(anode->mTransformation));
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
		entity_t n = entity_new(c_name_new(name), c_node_new());
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
	resource_t *sauce = c_sauces_get_sauce(c_sauces(&SYS), info->filename);
	if(!sauce) return STOP;

	const struct aiScene *scene = aiImportFile(sauce->path,
			/* aiProcess_CalcTangentSpace  		| */
			aiProcess_Triangulate			|
			/* aiProcess_GenSmoothNormals		| */
			aiProcess_JoinIdenticalVertices 	|
			aiProcess_SortByPType);
	if(!scene)
	{
		printf("failed to load %s\n", info->filename);
		return CONTINUE;
	}

	int anim_only = 1;
	if(!entity_exists(*target))
	{
		anim_only = 0;
		*target = entity_new(c_name_new(info->filename), c_node_new());
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
				entity_add_component(node, c_light_new(40.0f, color, 256));
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
		c_spacial_set_scale(c_spacial(target), vec3(info->scale));
	}

	aiReleaseImport(scene);
    return STOP;
    /* return HANDLED; */
}

REG()
{
	ct_t *ct = ct_new("assimp", sizeof(c_assimp_t), NULL, NULL, 0);

	ct_listener(ct, WORLD | 100, sig("load"), c_assimp_load);
}

