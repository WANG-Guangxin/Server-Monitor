# 编译器
CC = g++

# 头文件路径
INC_DIR = include

# 源文件路径
SRC_DIR = src

# 目标文件路径
OBJ_DIR = obj

# 静态库路径
LIB_DIR = #

# 静态库
LIBS = -lpthread -lcrypto -lssl -lcurl

# 可执行文件名
TARGET = server-monitor-app

# 源文件列表
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# 目标文件列表
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# 编译选项
CFLAGS = -Wall -I$(INC_DIR) -Iinclude/json -O2 -std=c++14 -g

# 链接选项
LDFLAGS = -L$(LIB_DIR) $(LIBS)

# 默认编译规则
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

# 生成目标文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) 
