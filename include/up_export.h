
#ifndef UP_EXPORT_H
#define UP_EXPORT_H

#ifdef UP_STATIC_DEFINE
#  define UP_EXPORT
#  define UP_NO_EXPORT
#else
#  ifndef UP_EXPORT
#    ifdef up_EXPORTS
        /* We are building this library */
#      define UP_EXPORT 
#    else
        /* We are using this library */
#      define UP_EXPORT 
#    endif
#  endif

#  ifndef UP_NO_EXPORT
#    define UP_NO_EXPORT 
#  endif
#endif

#ifndef UP_DEPRECATED
#  define UP_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef UP_DEPRECATED_EXPORT
#  define UP_DEPRECATED_EXPORT UP_EXPORT UP_DEPRECATED
#endif

#ifndef UP_DEPRECATED_NO_EXPORT
#  define UP_DEPRECATED_NO_EXPORT UP_NO_EXPORT UP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef UP_NO_DEPRECATED
#    define UP_NO_DEPRECATED
#  endif
#endif

#endif /* UP_EXPORT_H */
