# SecMalloc
Decription : Trying to recode the classic memory alocation function of langage C but in a secure way. Recreation of malloc, free, calloc and realloc.
Every test will unitary and made with [criterion](https://criterion.readthedocs.io/en/master/intro.html).

### Logic used :
Inspect the man of each function to undestand how they work and what they do.

#### Notes :
This code will do the job of a memory block
```c
#define MEMORY_SIZE 10000

typedef struct block {
    size_t size;
    struct block* next;
} block_t;

block_t* free_list = NULL;
char memory[MEMORY_SIZE];
```

# my_malloc :
Allocates size bytes of uninitialized storage. 

## Syntax
```c
void* my_malloc(
    [int]    SIZE_T size
);
```
## Parameters
`[in] size_t size`
Size to allocate. This value should not be NULL, equal to 0 or less.

## Return value
If the function succeeds, the return value return pointer to the start of allocated block.

## Remarks
- Check minimum size
- Search if the memory bloc has a minimum size
- If there are no free block return NULL
- If the block is too big we divide it by 2 (poor optimisation)
- Update free_block list (aka free_list)
- Return the pointer to the allocated space.

# my_free :
Deallocates the space previously allocated by `my_malloc`, `my_calloc` ot `my_realloc`

## Syntax
```c
void my_free(
    [in]    void* ptr
);
```
## Parameters 
`[out] void* ptr`
The returned pointer of an allocated function like `my_malloc`, `my_calloc` ot `my_realloc`. This value can't be NULL.

## Return value
If the function secceed there are no returned value.

## Remarks
- Check if the ptr is valid
- Check if the ptr is on allocated memory with my_malloc()
- Retreive if the block is associated to a pointer
- Check if memory block is valided
- Add the block to the list of free block
- Renitialize allocated memory with memset() to avoid other memory leak

# my_calloc :
Allocates memory for an array of num objects of size and initializes all bytes in the allocated storage to zero.

## Syntax
```c
void* my_calloc(
    [in]    SIZE_T nmemb,
    [in]    INT size_t size
);
```


## Parameters 
`[in] nmemb`

`[in] size`
The size of the region, in bytes. If the size is NULL, 0 or less the function return NULL.

## Return value
If the function succeeds, the return value return a pointer of my_malloc function.

## Remarks
- Valid parametes check
- Calculate the total memory size to allocate
- Check total memory size is not too big
- Alloc memory with my_alloc()
- Initilise memory allocated to 0 with memset()    

# my_realloc :
## Syntax
```c
void* my_realloc(
    [in]    void* ptr,
    [in]    INT size_t size
);
```

## Parameters 
`[in] ptr`
Pointer of the malloc block to reallocate.
`[in] size`
The size of the region, in bytes. If the size is NULL, 0 or less the function return NULL.

## Return value
Return a pointer to the start of tha realocated block.

## Remarks
- Retreive the associated block
- Allocate a new block using my_malloc
- Copy the data from the old block to the new bloc
- Free the old block of memory using my_free
