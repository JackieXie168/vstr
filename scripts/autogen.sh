#! /bin/sh

libtoolize -c --force && aclocal && automake -a -c --force-missing && autoconf

# Fix automake...
rm COPYING
ln Documentation/COPYING.LIB COPYING

gen_prefix=include/internal_syms_generated/vstr

# Files for internal symbol remapping...
rm -f $gen_prefix-alias-symbols.h
touch $gen_prefix-alias-symbols.h
rm -f $gen_prefix-cpp-symbols_fwd.h
touch $gen_prefix-cpp-symbols_fwd.h

# BEG: Create alias files...
perl -pe 'BEGIN { print "/* DO NOT EDIT THIS FILE */\n"; } \
          s/^(.*)\((.*)\)\n$/#ifdef $2\nVSTR__SYM($1)\n#endif\n\n/; \
          s/^([^#].*[^)])\n$/VSTR__SYM($1)\n/;' \
  include/vstr.exported_symbols > $gen_prefix-alias-symbols.h
# END: Create alias files...

# BEG: Create cpp mapping to internal names
perl -pe 'BEGIN { print "/* DO NOT EDIT THIS FILE */\n"; } \
        s/^(.*)\((.*)\)\n$/#ifdef $2\n#define vstr_$1 vstr_nx_$1\n#endif\n\n/; \
        s/^([^#].*[^)])\n$/#define vstr_$1 vstr_nx_$1\n/;' \
  include/vstr.exported_symbols > $gen_prefix-cpp-symbols_fwd.h
perl -pe 'BEGIN { print "/* DO NOT EDIT THIS FILE */\n"; } \
        s/^(.*)\((.*)\)\n$/#ifdef $2\n#undef vstr_$1\n#endif\n\n/; \
        s/^([^#].*[^)])\n$/#undef vstr_$1\n/;' \
  include/vstr.exported_symbols > $gen_prefix-cpp-symbols_rev.h
# END: Create cpp mapping to internal names

