lib_LTLIBRARIES = libzbxredis.la

libzbxredis_la_SOURCES = \
	libzbxredis.h \
	libzbxredis.c \
	redis.c

libzbxredis_la_CFLAGS = \
	$(ZABBIX_CPPFLAGS) \
	$(HIREDIS_CPPFLAGS)

libzbxredis_la_LDFLAGS = \
	-shared \
	-avoid-version \
	$(HIREDIS_LDFLAGS)

# Prevent install of the redundant *.la files
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/libzbxredis.la

# Fix "files left after uninstall" issue
uninstall-local:
	rm -f $(DESTDIR)$(libdir)/libzbxredis.so
