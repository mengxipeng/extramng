
PROJECT=extract_mng

LDFLAGS=-lz -lmng
extract_mng:extract_mng.o
	$(CC) -o $@ $^ $(LDFLAGS) 
