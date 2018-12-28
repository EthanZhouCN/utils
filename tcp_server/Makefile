######################################
#
######################################
#source file
#源文件，自动找所有.c文件，并将目标定义为同名.o文件
SOURCE  := $(wildcard *.c) 
#OBJS  := rent_config.o rent_data.o rent_main.o rent_report.o rent_socket.o rent_timer.o
			
OBJS    := $(patsubst %.c,%.o,$(patsubst %.c,%.o,$(SOURCE)))
  
#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
TARGET  := server
  
#compile and lib parameter
#编译参数
CC      := gcc
LIBS    := 
LDFLAGS := -lpthread
DEFINES :=
INCLUDE := -I.
CFLAGS  := -g -O1 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS)
  
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
  
all : $(TARGET)

objs : $(OBJS)

rebuild: veryclean everything
             
clean:
	rm -fr *.so
	rm -fr *.o
	rm -fr $(TARGET)
veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
