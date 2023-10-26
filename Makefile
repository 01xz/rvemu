TARGET ?= rvemu

SRC_DIR = src

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
EXE_DIR = $(BUILD_DIR)

HDRS = $(wildcard $(SRC_DIR)/*.h)
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

CFLAGS += -O3
CFLAGS += -Wall -Werror -Wimplicit-fallthrough

LDFLAGS += -lm

$(EXE_DIR)/$(TARGET): $(OBJS) | $(EXE_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE_DIR):
	@mkdir -p $@

$(OBJ_DIR):
	@mkdir -p $@

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: clean
