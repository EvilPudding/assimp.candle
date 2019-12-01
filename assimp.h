#ifndef CAN_ASSIMP_H
#define CAN_ASSIMP_H

#include <ecs/ecm.h>

typedef struct c_assimp_t
{
	c_t super;
} c_assimp_t;

DEF_CASTER("assimp", c_assimp, c_assimp_t)

c_assimp_t *c_assimp_new(void);

#endif /* !CAN_ASSIMP_H */
