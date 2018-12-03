#pragma once

#define PQ_PARENT(i) ((i-1) >> 1)
#define PQ_LEFT_CHILD(i) ((i<<1) + 1)
#define PQ_RIGHT_CHILD(i) ((i<<1) + 2)


