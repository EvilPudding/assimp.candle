#ifndef CAN_ASSIMP_H
#define CAN_ASSIMP_H

#include <ecs/ecm.h>

typedef float(*collider_cb)(c_t *self, vec3_t pos);
typedef float(*velocity_cb)(c_t *self, vec3_t pos);

typedef struct c_assimp_t
{
	c_t super;
} c_assimp_t;

DEF_CASTER("assimp", c_assimp, c_assimp_t)

c_assimp_t *c_assimp_new(void);

#endif /* !CAN_ASSIMP_H */
