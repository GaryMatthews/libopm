/* vim: set shiftwidth=3 softtabstop=3 expandtab: */ 

#ifndef DEFS_H
#define DEFS_H

/* Miscellaneous defines that don't belong anywhere else. */

/* Stuff to shut up warnings about rcsid being unused. */
#define USE_VAR(var)    static char sizeof##var = sizeof(sizeof##var) + sizeof(var)
/* RCS tag. */
#define RCSID(x)        static char rcsid[] = x; USE_VAR(rcsid);

#endif /* DEFS_H */
