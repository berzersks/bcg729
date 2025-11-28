PHP_ARG_ENABLE(bcg729, whether to enable bcg729 support,
[  --enable-bcg729           Enable bcg729 extension])

if test "$PHP_BCG729" != "no"; then
  PHP_NEW_EXTENSION(bcg729, bcg729.c, $ext_shared)
  PHP_ADD_LIBRARY(bcg729, 1, bcg729)
fi