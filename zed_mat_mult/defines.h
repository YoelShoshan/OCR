#ifndef ZED_MAT_DEFINES
#define ZED_MAT_DEFINES

// Thread block size
//TODO: restore both to 16
#define BLOCK_SIZE 16
#define TILE_SIZE  16

#define PREC_TYPE double

//#define VERBOSE_DBG
//#define VERBOSE_ALLOCATIONS


// impl

#ifdef VERBOSE_DBG
#define VERBOSE_DBG_PRINT(...) \
	printf(__VA_ARGS__);
#else
#define VERBOSE_DBG_PRINT(...)
#endif

#ifdef VERBOSE_ALLOCATIONS
#define VERBOSE_ALLOC_PRINT(...) \
	printf(__VA_ARGS__);
#else
#define VERBOSE_ALLOC_PRINT(...)
#endif


#define ZMM_CHECK_CUDA_ERROR(x,operation_str) \
	{ \
	if (x!=cudaSuccess)\
		{\
			printf("Cuda operation failed! Attempted [%s]\n", operation_str);\
		}\
	}

#define ZMM_CHECK_CUDA_ERROR_RET(x,operation_str) \
	{ \
	if (x!=cudaSuccess)\
		{\
			printf("Cuda operation failed! Attempted [%s]\n", operation_str);\
			return false;\
		}\
	}



#endif