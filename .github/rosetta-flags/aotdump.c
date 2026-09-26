#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <libproc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
__attribute__((constructor)) static void dumpAot(void) {
    const char *want = getenv("AOTDUMP_NAME");
    if (!want) return;
    mach_vm_address_t addr = 0; mach_vm_size_t size; natural_t depth = 0;
    for (;;) {
        struct vm_region_submap_info_64 info; mach_msg_type_number_t cnt = VM_REGION_SUBMAP_INFO_COUNT_64;
        if (mach_vm_region_recurse(mach_task_self(), &addr, &size, &depth, (vm_region_recurse_info_t)&info, &cnt)) break;
        char path[4096] = {0};
        proc_regionfilename(getpid(), addr, path, sizeof(path));
        if (strstr(path, want)) {
            char name[128]; snprintf(name, sizeof name, "gaot_%llx_off%llx_prot%d.bin", (unsigned long long)addr, (unsigned long long)info.offset, info.protection);
            FILE *f = fopen(name, "wb"); fwrite((void*)addr, 1, size, f); fclose(f);
            fprintf(stderr, "AOTDUMP %s size=%llu path=%s\n", name, (unsigned long long)size, path);
        }
        addr += size;
    }
}
