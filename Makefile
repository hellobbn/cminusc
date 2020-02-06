# Makefile for cminus

## generic
CC 				= gcc
C_FLAG 			= -Iinclude  -Ibuild/generated -Wall -Wextra -Wno-unused-parameter
C_FLAG 		   += -Wno-unused-function

## build dir
BUILD_DIR 		= build
GENERATED_DIR	= ${BUILD_DIR}/generated

## test
TEST_DIR		= test_src
TEST_OUT_DIR	= testout

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
LEXER_FLEX_TEST_OBJ = ${LEXER_OUT_DIR}/lex_flex_test.o
LEXER_TEST_OUT_DIR = ${TEST_OUT_DIR}/${LEXER_SRC_DIR}
LEXER_TEST_SRC	= ${TEST_DIR}/lex_test.c
LEXER_TEST_OBJ	= ${LEXER_OUT_DIR}/lex_test.o
LEXER_TEST_OUT	= ${BUILD_DIR}/lex_test
LEXER_TEST_CFLAG	= -DLEX_TEST

## syntax tree
### dir
SYNTREE_SRC_DIR	= syntax_tree
SYNTREE_OUT_DIR	= ${BUILD_DIR}/${SYNTREE_SRC_DIR}

### source
SYNTREE_C_FILE 	= ${wildcard ${SYNTREE_SRC_DIR}/*.c}

### obj
SYNTREE_OBJ		= ${patsubst %.c, ${BUILD_DIR}/%.o, ${SYNTREE_C_FILE}}

### test
SYNTREE_TEST_SRC	= ${TEST_DIR}/syntree_test.c
SYNTREE_TEST_OBJ 	= ${SYNTREE_OUT_DIR}/syntree_test.o
SYNTREE_TEST_OUT	= ${BUILD_DIR}/syntree_test

## parser

### the parser
BISON_EXEC		= bison
BISON_FLAGS		= -d -Wno-conflicts-sr
PARSER 			= parser

### dir
PARSER_SRC_DIR	= parser
PARSER_OUT_DIR	= ${BUILD_DIR}/${PARSER_SRC_DIR}
PARSER_HEADER	= ${GENERATED_DIR}/${PARSER}.tab.h

### source
BISON_IN_SOURCE	= ${PARSER_SRC_DIR}/${PARSER}.y
BISON_OUT_SOURCE = ${PARSER_OUT_DIR}/${PARSER}.tab.c
PARSER_C_SRC	= ${wildcard ${PARSER_SRC_DIR}/*.c}

### obj
PARSER_BISON_OBJ	= ${PARSER_OUT_DIR}/${PARSER}.o
PARSER_C_OBJ		= ${patsubst %.c, ${BUILD_DIR}/%.o, ${PARSER_C_SRC}}
PARSER_OBJ			= ${PARSER_BISON_OBJ} ${PARSER_C_OBJ}

### test
PARSER_TEST_OUT_DIR	= ${TEST_OUT_DIR}/${PARSER_SRC_DIR}
PARSER_TEST_SRC	= ${TEST_DIR}/parser_test.c
PARSER_TEST_OBJ	= ${PARSER_OUT_DIR}/parser_test.o
PARSER_TEST_OUT	= ${BUILD_DIR}/parser_test

## All Dir
DIRS = ${BUILD_DIR} ${LEXER_OUT_DIR} ${HELPER_OUT_DIR} ${LEXER_TEST_OUT_DIR} \
       ${PARSER_TEST_OUT_DIR} ${GENERATED_DIR} ${PARSER_OUT_DIR} ${SYNTREE_OUT_DIR}

all: prepare lex_test syntree_test bison_test
	$(info )
	$(info ----------------------------)
	$(info All Done)
	$(info ----------------------------)

## parser rules

### bison test
bison_test: ${PARSER_TEST_OBJ} ${SYNTREE_OBJ} ${HELPER_OBJ} ${LEXER_OBJ} ${PARSER_OBJ}
	${CC} ${C_FLAG} ${PARSER_TEST_OBJ} ${PARSER_BISON_OBJ} ${SYNTREE_OBJ} ${HELPER_OBJ} ${LEXER_OBJ} -o ${PARSER_TEST_OUT}

${PARSER_TEST_OBJ}: ${PARSER_TEST_SRC}
	${CC} ${C_FLAG} -c -o ${PARSER_TEST_OBJ} ${PARSER_TEST_SRC} 

### bison generate
${PARSER_C_OBJ}: ${BUILD_DIR}/%.o : %.c
	${CC} ${C_FLAG} -c -o $@ $<

${PARSER_BISON_OBJ}: ${BISON_OUT_SOURCE}
	${CC} ${C_FLAG} -c -o ${PARSER_BISON_OBJ} ${BISON_OUT_SOURCE}

${BISON_OUT_SOURCE}: bison_compile
	mv ${PARSER}.tab.c ${BISON_OUT_SOURCE}

${PARSER_HEADER}: bison_compile
	mv ${PARSER}.tab.h ${PARSER_HEADER}

bison_compile: ${BISON_IN_SOURCE}
	${BISON_EXEC} ${BISON_FLAGS} ${BISON_IN_SOURCE}

## Syntree rules

### syntree test
syntree_test: ${SYNTREE_TEST_OBJ} ${SYNTREE_OBJ}
	${CC} ${C_FLAG} ${SYNTREE_TEST_OBJ} ${SYNTREE_OBJ} -o ${SYNTREE_TEST_OUT}

${SYNTREE_TEST_OBJ}: ${SYNTREE_TEST_SRC}
	${CC} ${C_FLAG} -c -o ${SYNTREE_TEST_OBJ} ${SYNTREE_TEST_SRC}

### syntree
${SYNTREE_OBJ}: ${BUILD_DIR}/%.o : %.c
	${CC} ${C_FLAG} -c -o $@ $<

## Flex rules

### Flex test 

lex_test: ${LEXER_FLEX_TEST_OBJ} ${LEXER_TEST_OBJ} ${HELPER_OBJ} ${SYNTREE_OBJ}
	${CC} ${C_FLAG} ${LEXER_C_OBJ}  ${LEXER_FLEX_TEST_OBJ} ${LEXER_TEST_OBJ} ${HELPER_OBJ} ${SYNTREE_OBJ} -o ${LEXER_TEST_OUT} 

${LEXER_TEST_OBJ}: ${LEXER_TEST_SRC}
	${CC} ${C_FLAG} -c -o ${LEXER_TEST_OBJ} ${LEXER_TEST_SRC}

${LEXER_FLEX_TEST_OBJ}: flex_comp ${PARSER_HEADER}
	${CC} ${C_FLAG} ${LEXER_TEST_CFLAG} -c -o ${LEXER_FLEX_TEST_OBJ} ${LEXER_FLEX_OUT}

### Flex

${LEXER_C_OBJ}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAG} -c -o $@ $<

${LEXER_FLEX_OBJ}: flex_comp ${PARSER_HEADER}
	${CC} ${C_FLAG} -c -o ${LEXER_FLEX_OBJ} ${LEXER_FLEX_OUT}

flex_comp:
	${LEXER_FLEX} -o ${LEXER_FLEX_OUT} ${LEXER_FLEX_FILE}


## helper rules

${HELPER_OBJ}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAG} -c -o $@ $<

prepare:
	mkdir -p ${DIRS}

clean:
	rm -rf ${BUILD_DIR}
	rm -rf ${TEST_OUT_DIR}