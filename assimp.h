#ifndef CAN_ASSIMP_H
#define CAN_ASSIMP_H

#include "../candle/ecs/ecm.h"

typedef struct c_assimp_t
{
	c_t super;
} c_assimp_t;

void ct_assimp(ct_t *self);
DEF_CASTER(ct_assimp, c_assimp, c_assimp_t)

c_assimp_t *c_assimp_new(void);

#endif /* !CAN_ASSIMP_H */
