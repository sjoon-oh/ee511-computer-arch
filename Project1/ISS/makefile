CC = gcc
TARGET = krpiss
OBJECTS = krpiss.o 

all : $(TARGET)
	@echo "Compilation completed!"

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ 

krpiss.o : ./krpiss.c 	
	$(CC) $(CFLAGS) -c ./krpiss.c 


clean :
	rm -f $(TARGET)
	rm -f $(TARGET).o
