#ifndef MIN_BLOCK_ORDER
#define MIN_BLOCK_ORDER 5
#endif

// TODO: I think I am missing size checks?

void* Win32Allocate(size_t Size);

typedef struct transient_block 
{
    struct transient_block* Next;
} transient_block;

typedef struct transient_allocator
{
    uint8_t             PoolOrder;
    uint8_t             MinBlockOrder;
    uint8_t*            Memory;
    size_t              Capacity;
    transient_block**   FreeList;
} transient_allocator;

typedef struct 
{
    uint8_t* Memory;
    size_t   Size;
    size_t   At;
    size_t   Capacity;
} transient_stack;

static uint32_t
SizeToOrder(transient_allocator* Allocator, size_t Size)
{
    size_t Needed = (Size + ((1ULL << Allocator->MinBlockOrder) - 1))
                    >> Allocator->MinBlockOrder;

    uint32_t Order = 0;
    while ((1ULL << Order) < Needed)
    {
        Order++;
    }
    return Order;
}

static void
SplitBlock(transient_allocator* Allocator, uint32_t Order)
{
    transient_block* Block = Allocator->FreeList[Order];
    Assert(Block);
    Allocator->FreeList[Order] = Block->Next;

    size_t HalfSize = (1ULL << (Order + Allocator->MinBlockOrder - 1));
    transient_block* Buddy = (transient_block*)( (uint8_t*)Block + HalfSize );

    Block->Next = Buddy;
    Buddy->Next = Allocator->FreeList[Order - 1];
    Allocator->FreeList[Order - 1] = Block;
}

static transient_block*
GetBuddy(transient_allocator* Allocator, transient_block* Block, uint32_t Order)
{
    size_t Offset    = (uint8_t*)Block - Allocator->Memory;
    size_t BlockSize = (1ULL << (Order + Allocator->MinBlockOrder));
    size_t BuddyOff  = Offset ^ BlockSize;
    return (transient_block*)(Allocator->Memory + BuddyOff);
}

static transient_allocator
InitializeTransientAllocator(size_t Size)
{
    transient_allocator Transient = { 0 };

    u8 PoolOrder = 0;
    size_t MaxBlockSize = 1llu << MIN_BLOCK_ORDER;
    while ((MaxBlockSize << 1) <= Size)
    {
        MaxBlockSize <<= 1;
        PoolOrder++;
    }

    Transient.MinBlockOrder = MIN_BLOCK_ORDER; // NOTE: 2^5 = 32 bytes (consider making this bigger)
    Transient.PoolOrder     = PoolOrder;
    Transient.Memory        = (u8*)Win32Allocate(Size);
    Transient.FreeList      = (transient_block**)Win32Allocate(sizeof(transient_block*) * (PoolOrder + 1));
    Transient.Capacity      = Size;

    transient_block* Initial = (transient_block*)Transient.Memory;
    Initial->Next = NULL;

    for (u8 Index = 0; Index < PoolOrder; Index++)
    {
        Transient.FreeList[Index] = NULL;
    }
    Transient.FreeList[PoolOrder] = Initial;

    return Transient;
}

static void*
AllocateTransient(transient_allocator* Allocator, size_t Size)
{
    uint32_t Order = SizeToOrder(Allocator, Size);
    if (Order > Allocator->PoolOrder) return NULL;

    uint32_t EmptyOrder = Order;
    while (EmptyOrder <= Allocator->PoolOrder &&
           Allocator->FreeList[EmptyOrder] == NULL) 
    {
        EmptyOrder++;
    }
    if (EmptyOrder > Allocator->PoolOrder) return NULL;

    while (EmptyOrder > Order) 
    {
        SplitBlock(Allocator, EmptyOrder);
        EmptyOrder--;
    }

    transient_block* Block     = Allocator->FreeList[Order];
    Allocator->FreeList[Order] = Block->Next;

    memset(Block, 0, Size);
    return Block;
}

static void
FreeTransient(transient_allocator* Allocator, void* Pointer, size_t Size)
{
    Assert(Pointer && Allocator->Memory);

    uint32_t Order         = SizeToOrder(Allocator, Size);
    transient_block* Block = (transient_block*)Pointer;

    while (Order < Allocator->PoolOrder) 
    {
        transient_block* Buddy = GetBuddy(Allocator, Block, Order);

        transient_block** PrevPtr = &Allocator->FreeList[Order];
        transient_block*  Current = *PrevPtr;
        while (Current && Current != Buddy) {
            PrevPtr = &Current->Next;
            Current =  Current->Next;
        }

        if (!Current) break;

        *PrevPtr = Buddy->Next;

        if (Buddy < Block) 
        {
            Block = Buddy;
        }
        Order++;
    }

    Block->Next                = Allocator->FreeList[Order];
    Allocator->FreeList[Order] = Block;
}
