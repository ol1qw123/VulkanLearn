#include "Classheader.hpp"
namespace Alloctor {
    void* CustomAllocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        void* ptr = malloc(size);
        printf("Allocated %zu bytes\n", size);
        return ptr;
    }

    // Custom reallocation function
    void* CustomReallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        void* ptr = realloc(pOriginal, size);
        printf("Reallocated to %zu bytes\n", size);
        return ptr;
    }

    // Custom free function
    void CustomFree(void* pUserData, void* pMemory) {
        free(pMemory);
        printf("Freed memory\n");
    }
}