## Automake template for standard include files.

INCLUDES = calyx_adjectives.h daemons.h easydoors.h hintsms.h links.h \
links.inf longint.h money.h nameable.h newbiegrammar.h printtime.h \
timepiece.h trinitystat.h whatis.h

EXTRA_NAMES = NewbieGrammar.h

DOCS = links.inf

include_HEADERS = $(INCLUDES) $(DOCS)

EXTRA_DIST = $(DOCS)

if CYGWIN
install-data-hook: add-localdir
uninstall-local: remove-localdir
else
install-data-hook: add-localdir add-names
uninstall-local: remove-localdir remove-names
endif

add-localdir:
	$(mkinstalldirs) $(DESTDIR)$(localincdir)

remove-localdir:
	rmdir $(DESTDIR)$(localincdir)

add-names:
	@ cd $(DESTDIR)$(includedir);					\
	for file in $(EXTRA_NAMES); do					\
	   realfile=`echo $$file | tr '[A-Z]' '[a-z]'`;			\
	   echo " linking $$file to $$realfile";			\
	   rm -f $$file;						\
	   $(LN_S) $$realfile $$file;					\
	done

remove-names:
	@ for file in $(EXTRA_NAMES); do				\
	   echo " unlinking $$file";					\
	   rm -f $(DESTDIR)$(includedir)/$$file;			\
	done

@make_include@

