ifndef HDF5_HOME
 $(error HDF5_HOME must be set to the location of your cloned and installed hdf5-group repository)
endif

W_OPTS         = -Wall -Wno-strict-aliasing -Wno-parentheses -Wextra -Werror -Wsign-compare
I_OPTS         = -I${HDF5_HOME}/include -Isrc
L_OPTS         = -L${HDF5_HOME}/lib -Wl,-rpath=${HDF5_HOME}/lib -lhdf5 -lz -lpthread -lssl -shared
OPTS           = -DKXVER=3 -fPIC

MS             = $(shell getconf LONG_BIT)

ifeq ($(shell uname),Linux)
 LNK     = -lrt
 OSFLAG  = l
 OSXOPTS =
else ifeq ($(shell uname),Darwin)
 LNK     =
 OSFLAG  = m
 OSXOPTS = -undefined dynamic_lookup  -mmacosx-version-min=10.12
endif

QARCH = $(OSFLAG)$(MS)
QLIB  = $(QHOME)/$(QARCH)

all: hdf5.so

hdf5.so: src/kdb_util.o src/hdf5_utils.o src/hdf5_create.o src/hdf5_general.o src/hdf5_read.o src/hdf5_write.o src/hdf5_ls.o src/hdf5_groups.o src/hdf5_links.o src/hdf5_del.o
	$(CC) src/kdb_util.o src/hdf5_utils.o src/hdf5_create.o src/hdf5_general.o src/hdf5_read.o src/hdf5_write.o src/hdf5_ls.o src/hdf5_groups.o src/hdf5_links.o src/hdf5_del.o $(L_OPTS) $(LNK) -o hdf5.so

src/kdb_util.o: src/kdb_util.c src/k.h
	$(CC) src/kdb_util.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/kdb_util.o

src/hdf5_utils.o: src/hdf5_utils.c src/k.h
	$(CC) src/hdf5_utils.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_utils.o

src/hdf5_create.o: src/hdf5_create.c src/k.h
	$(CC) src/hdf5_create.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_create.o

src/hdf5_general.o: src/hdf5_general.c src/k.h
	$(CC) src/hdf5_general.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_general.o

src/hdf5_read.o: src/hdf5_read.c src/k.h
	$(CC) src/hdf5_read.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_read.o

src/hdf5_write.o: src/hdf5_write.c src/k.h
	$(CC) src/hdf5_write.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_write.o

src/hdf5_ls.o: src/hdf5_ls.c src/k.h
	$(CC) src/hdf5_ls.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_ls.o

src/hdf5_groups.o: src/hdf5_groups.c src/k.h
	$(CC) src/hdf5_groups.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_groups.o

src/hdf5_links.o: src/hdf5_links.c src/k.h
	$(CC) src/hdf5_links.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_links.o

src/hdf5_del.o: src/hdf5_del.c src/k.h
	$(CC) src/hdf5_del.c -m$(MS) $(OPTS) $(I_OPTS) $(W_OPTS) -c -o src/hdf5_del.o

src/k.h:
	curl -s -L https://github.com/KxSystems/kdb/raw/master/c/c/k.h -o src/k.h

install: hdf5.so
	install hdf5.so $(QLIB)
	install hdf5.q $(QHOME)

clean:
	rm -f src/k.h src/*.o hdf5.so
