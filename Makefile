# Makefile for cminus

## generic
CC 				= gcc
C_FLAG 			= -Iinclude

## build dir
BUILD_DIR 		= build

## test
TEST_DIR		= test_src

## helper
HELPER_SRC_DIR	= helper
HELPER_OUT_DIR	= ${BUILD_DIR}/${HELPER_SRC_DIR}
HELPER_C_FILE	= ${wildcard ${HELPER_SRC_DIR}/*.c}
HELPER_OBJ		= ${patsubst %.c, ${BUILD_DIR}/%.o, ${HELPER_C_FILE}}

## lexer
### flex
LEXER_FLEX 		= flex

### dir
LEXER_SRC_DIR	= lexer
LEXER_OUT_DIR	= ${BUILD_DIR}/${LEXER_SRC_DIR}

### source
LEXER_C_FILE 	= ${wildcard ${LEXER_SRC_DIR}/*.c}
LEXER_FLEX_FILE = ${wildcard ${LEXER_SRC_DIR}/*.l}
LEXER_FLEX_OUT	= ${LEXER_OUT_DIR}/lex.yy.c

### obj
LEXER_FLEX_OBJ 	= ${patsubst %.c, %.o, ${LEXER_FLEX_OUT}}
LEXER_C_OBJ		= ${patsubst %.c, ${BUILD_DIR}/%.o, ${LEXER_C_FILE}}
LEXER_OBJ 		= ${LEXER_C_OBJ} ${LEXER_FLEX_OBJ}

### test
LEXER_TEST_SRC	= ${TEST_DIR}/lex_test.c
LEXER_TEST_OBJ	= ${LEXER_OUT_DIR}/lex_test.o
LEXER_TEST_OUT	= ${BUILD_DIR}/lex_test


all: prepare lex_test

## Flex rules

### Flex test 

lex_test: ${LEXER_OBJ} ${LEXER_TEST_OBJ} ${HELPER_OBJ}
	${CC} ${C_FLAG} ${LEXER_OBJ} ${LEXER_TEST_OBJ}  ${HELPER_OBJ} -o ${LEXER_TEST_OUT} 

${LEXER_TEST_OBJ}: ${LEXER_TEST_SRC}
	${CC} ${C_FLAG} -c -o ${LEXER_TEST_OBJ} ${LEXER_TEST_SRC}


### Flex

${LEXER_C_OBJ}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAG} -c -o $@ $<

${LEXER_FLEX_OBJ}: flex_comp
	${CC} ${C_FLAG} -c -o ${LEXER_FLEX_OBJ} ${LEXER_FLEX_OUT}

flex_comp:
	${LEXER_FLEX} -o ${LEXER_FLEX_OUT} ${LEXER_FLEX_FILE}


## helper rules

${HELPER_OBJ}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAG} -c -o $@ $<

prepare:
	mkdir -p ${BUILD_DIR}
	mkdir -p ${LEXER_OUT_DIR}
	mkdir -p ${HELPER_OUT_DIR}

clean:
	rm -rf ${BUILD_DIR}