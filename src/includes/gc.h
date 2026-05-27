#ifndef TBASIC_GC_H
#define TBASIC_GC_H

#include "objects.h"



#define BITSET_U8_BITS ((uint16_t)8)

typedef struct bitset_t {
    uint8_t *bits;
    uint16_t capacity;
} BitSet;

void BitSet_new(BitSet *self, uint16_t capacity);
void BitSet_del(BitSet *self);

uint16_t BitSet_len(const BitSet *self);

int8_t BitSet_get_at(const BitSet *self, uint16_t pos);
void BitSet_set_at(BitSet *self, uint16_t pos);
void BitSet_clear(BitSet *self);



// ? Implements a simple ring buffer to track int16_t object IDs, mainly used by the GC.
typedef struct id_queue {
    int16_t *items;
    int16_t head_pos;
    int16_t tail_pos;
    int16_t length;
    int16_t capacity;
} IdQueue;

void IdQueue_new(IdQueue *self, int16_t capacity);
void IdQueue_del(IdQueue *self);

int8_t IdQueue_exhausted(const IdQueue *self);

int8_t IdQueue_push(IdQueue *self, int16_t id);
int16_t IdQueue_pop(IdQueue *self);
void IdQueue_reset(IdQueue *self);



typedef struct ptr_queue {
    const void **ptrs;
    int16_t head_pos;
    int16_t tail_pos;
    int16_t length;
    int16_t capacity;
} PtrQueue;

void PtrQueue_new(PtrQueue *self, int16_t capacity);
void PtrQueue_del(PtrQueue *self);

int8_t PtrQueue_exhausted(const PtrQueue *self);

int8_t PtrQueue_push(PtrQueue *self, const void *p);
const void *PtrQueue_pop(PtrQueue *self);
void PtrQueue_reset(PtrQueue *self);




typedef struct gc_state_t {
    BitSet reach_bits;      // ? Compactly tracks which of 0 to N heap cells are reachable. The allocated bitset here MUST have at least the same bit count as the cell count.
    IdQueue next_ids;       // ? Tracks which heap IDs to mark next.
} GCState;

void GCState_new(GCState *self, int16_t max_object_ids);
void GCState_del(GCState *self);

void GCState_collect(GCState *self, ObjHeap *heap, const Value *stack_ptr, int stack_n);

#endif
