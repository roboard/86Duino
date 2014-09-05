/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
   Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
   Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   
   Usage of this library is not restricted in any way.  Standard public
   symbols are in dlfcn.h.  Contributed to the DJGPP project. */

#ifndef __dj_include_dxe_h_
#define __dj_include_dxe_h_

typedef struct {
  long magic;
  long symbol_offset;
  long element_size;
  long nrelocs;
} dxe_header;

#define DXE_MAGIC 0x31455844

#ifdef __cplusplus
extern "C" {
#  define EXTERN_C extern "C"
#else
#  define EXTERN_C extern
#endif

/* the legacy support */
void *_dxe_load (char *filename);

#if 0
-------------------------------------------------------------------------------
    The overall structure of the DXE3 file:

    < header >
      /* see the dxe_header structure */

    < exported symbols table >
      {
        long offset;                            /* symbol offset */
        char symbol_name [];			/* symbol name */
      } [dxe_header.n_exp_syms];

    < .text, .data, ... >
      /* this is the actual code, data, etc. of the module. All the offsets
         below (in relocation records) are relative to the beginning of this
         section. */

    < dependency table >
      {
        char dep_name [];			/* module name */
      } [dxe_header.n_deps];

    < unresolved symbols table >
      {
        unsigned short relative_reloc_count;	/* number of relative relocs */
        unsigned short absolute_reloc_count;	/* number of absolute relocs */
        char symbol_name [];			/* symbol name */
        long rel_relocs [relative_reloc_count];	/* the offsets at which we
                                                   should add the absolute
                                                   value of the symbol */
        long abs_relocs [absolute_reloc_count];	/* the offsets at which we
                                                   should add the relative
                                                   value of the symbol */
      } [dxe_header.n_unres_syms]

    < relocations >
      {
        long offset;				/* the offset at which we
                                                   should add the base address
                                                   of .text section */
      } [dxe_header.n_relocs];

    < description >
      /* Optional ASCIIZ string, containing a description of the module */

-------------------------------------------------------------------------------
#endif

typedef struct {
        /* DXE1 header */

        long magic;
        long symbol_offset;
        long element_size;
        long nrelocs;

        /* extended header */

        long hdr_size;		/* Header size in bytes */

        char major;
        char minor;
        short flags;		/* Capabilities */

        long reloc_table;	/* File offset to relocation table */

        long n_exp_syms;	/* Number of exported symbols */
        long exp_table;		/* File offset to exported symbols table */
        long exp_size;

        long n_deps;		/* Number of dependencies */
        long dep_table;		/* File offset to dependency table */
        long dep_size;

        long n_unres_syms;	/* Number of unresolved symbols */
        long unres_table;	/* File offset to unresolved table */
        long unres_size;

        long sec_size;		/* Size of combined code+data+bss section */
        long sec_f_size;	/* Physical section size (the rest is zeroed) */

        long _init;
        long _fini;
} __attribute__((packed)) dxe3_header;

typedef struct
{
  const char *name;	/* Symbol name: "printf", "strlen" and so on */
  void *offset;		/* Symbol offset */
} dxe_symbol_table;

/* Register (yet another) table of symbols to be exported into the loaded
 * modules. You can register any number of such tables. When a module with
 * unresolved external symbols is loaded, all these tables are searched
 * for the respective symbol. If no one is found, the last-resort handler
 * is called. If even the last-resort handler cannot return an address,
 * an error condition is raised.
 */
int dlregsym (const dxe_symbol_table *symtab);

/* Unregister the respective symbol table. */
int dlunregsym (const dxe_symbol_table *symtab);

/* The following variable contains a pointer to a function that is being
 * called when an unresolved symbol cannot be found in all the symbol tables
 * that the dynamic loader have at his disposition. The handler should return
 * NULL to rise a error condition, otherwise it should return a valid address.
 */
extern void *(*dlsymresolver) (const char *symname);

/* The following variable contains a pointer to a function that is being
 * called when static linking fails because of missing module. Note that
 * due to delayed nature of static linkage, the error can pop up very
 * late! If you want to check it at startup, call the "load_MODULENAME"
 * function explicitly. The function should never return.
 */
extern void (*dlerrstatmod) (const char *module);

/* The following variable contains a pointer to a function that is being
 * called when during static linking the dynamic loader finds that some symbol
 * is missing from dynamic module. The function should never return.
 */
extern void (*dlerrstatsym) (const char *module, const char *symbol);

/* These are used by code generated by dxe3gen */
void dlstatbind (const char *module, void **handle, char *stubs, char *syms);
void dlstatunbind (const char *module, void **handle, char *stubs, char *syms, long loader);

/*
 * Use the following macros to build a table of symbols that are exported
 * to dynamically loaded modules. This table should be passed to dlregsym().
 * Usage example:
 *
 *  DXE_EXPORT_TABLE (st)
 *    DXE_EXPORT (printf)
 *    DXE_EXPORT (strlen)
 *    DXE_EXPORT (strchr)
 *  DXE_EXPORT_END
 *
 *  dlregsym (st);
 *  void *h = dlopen ("my.dxe", RTLD_GLOBAL);
 *  if (!h)
 *  {
 *    fprintf (stderr, "my.dxe: %s\n", dlerror ());
 *    abort ();
 *  }
 *
 * The DXE_EXPORT_TABLE_AUTO macro can be used to register the
 * table with the dynamic loader automatically during startup.
 * To use it, just use DXE_EXPORT_TABLE_AUTO instead of DXE_EXPORT_TABLE.
 */
#define DXE_EXPORT_TABLE(name)	static dxe_symbol_table name [] = {
#define DXE_EXPORT_TABLE_AUTO(name) \
  static __attribute__((constructor)) void name##_auto_register () \
  { \
   extern void *__alias__##name __asm("_" #name); \
   dlregsym ((void *)&__alias__##name); \
  } \
  static __attribute__((unused)) dxe_symbol_table name [] = {
#define DXE_EXPORT(symbol)	{ "_" #symbol, (void *)&symbol },
#define DXE_EXPORT_END		{ 0, 0 }};

/*
 * Alternatively, one can build the export table this way:
 *
 *  extern_asm(_printf);
 *  extern_asm(_strlen);
 *  extern_asm(_strchr);
 *  DXE_EXPORT_TABLE (st)
 *    DXE_EXPORT_ASM (_printf)
 *    DXE_EXPORT_ASM (_strlen)
 *    DXE_EXPORT_ASM (_strchr)
 *  DXE_EXPORT_END
 */
#define extern_asm(n) extern void *__asm__##n __asm(#n)
#define DXE_EXPORT_ASM(symbol)	{ #symbol, &__asm__##symbol },

/*
 * Use the following macros to declare the load-on-demand and unload-on-demand
 * functions for a statically-linked dynamic module. Name must be uppercase and
 * any illegal character converted to underscore. Usage example:
 *
 *  DXE_DEMAND (MYDXE);
 *  somefunc()
 *  {
 *    dlload_MYDXE ();
 *    ...
 *    dlunload_MYDXE ();
 *  }
 */
#define DXE_DEMAND(name) \
  EXTERN_C void dlload_##name (void); \
  EXTERN_C void dlunload_##name (void)

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_dxe_h_ */
