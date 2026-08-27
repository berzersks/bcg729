PHP_ARG_ENABLE(bcg729, whether to enable bcg729 support,
[  --enable-bcg729           Enable bcg729 extension])

if test "$PHP_BCG729" != "no"; then

  dnl === GSM ===
  AC_CHECK_HEADER(gsm.h, [], [
    AC_MSG_ERROR([gsm.h not found. libgsm is required])
  ])

  AC_CHECK_LIB(
    gsm,
    gsm_create,
    [],
    [AC_MSG_ERROR([libgsm not found or missing gsm_create()])]
  )

  AC_DEFINE(HAVE_LIBGSM, 1, [libgsm support enabled])

  dnl === EXTENSION ===
  PHP_NEW_EXTENSION(bcg729, bcg729.c, $ext_shared)

  dnl === LINKAGEM ===
  PHP_ADD_LIBRARY(bcg729, 1, BCG729_SHARED_LIBADD)
  PHP_ADD_LIBRARY(gsm, 1, BCG729_SHARED_LIBADD)

  PHP_SUBST(BCG729_SHARED_LIBADD)
fi
