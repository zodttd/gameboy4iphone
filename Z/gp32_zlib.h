#ifndef ZLIBGP32
#define ZLIBGP32

/*gp32 -----------------------------v*/

extern void *gm_malloc(unsigned int);
extern void gm_free(void *);

/*
voidpf zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) items += size - size; /
    return (voidpf)gm_malloc(items * size);
}

void  zcfree (voidpf opaque, voidpf ptr)
{
    gm_free(ptr);
    if (opaque) return;
}
*/

#undef free
#undef alloc
#undef realloc
#undef malloc
#undef memset
#undef memcpy
#undef memcmp
#undef strcpy
#undef strncpy
#undef strcat
#undef strncat
#undef strlen
//#undef sprintf

#endif
