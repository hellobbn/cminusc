# Makefile for cminus

# -----------------------------------------------------------------------------
# Defs
# -----------------------------------------------------------------------------

## generic
CC 				= gcc
CXX				= g++
AR				= ar
ASM 			= nasm
C_FLAG 			= -Iinclude  -Ibuild/generated -Wall -Wextra -Wno-unused-parameter
C_FLAG 		   += -Wno-unused-function
CXX_FLAG		= ${C_FLAG}
CXX_LINKFLAG	= -l LLVM ${CXX_FLAG} 
ASM_FLAG		= -felf64

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
SYNTREE_CXX_FILE = ${wildcard ${SYNTREE_SRC_DIR}/*.cpp}

### obj
SYNTREE_C_OBJ		= ${patsubst %.c, ${BUILD_DIR}/%.o, ${SYNTREE_C_FILE}}
SYNTREE_CXX_OBJ		= ${patsubst %.cpp, ${BUILD_DIR}/%.obj, ${SYNTREE_CXX_FILE}}
SYNTREE_OBJ			= ${SYNTREE_C_OBJ} ${SYNTREE_CXX_OBJ}

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

#
# cminus c main
#

### dir
CMINUSC_SRC_DIR	= cminusc
CMINUSC_OUT_DIR	= ${BUILD_DIR}/${CMINUSC_SRC_DIR}

### source
CMINUSC_SRC		= ${wildcard ${CMINUSC_SRC_DIR}/*.cpp}

### obj
CMINUSC_OBJ		= ${patsubst %.cpp, ${BUILD_DIR}/%.obj, ${CMINUSC_SRC}}

#
# Library
#

### dir
LIB_SRC_DIR	= lib
LIB_OUT_DIR	= ${BUILD_DIR}/${LIB_SRC_DIR}

### source
LIB_ASM_SRC	= ${wildcard ${LIB_SRC_DIR}/*.asm}

### obj
LIB_ASM_OBJ	= ${patsubst %.asm, ${BUILD_DIR}/%.obj, ${LIB_ASM_SRC}}

### lib
LIB_FINAL	= ${BUILD_DIR}/libcminusc.a

## All Dir
DIRS = ${BUILD_DIR} ${LEXER_OUT_DIR} ${HELPER_OUT_DIR} ${LEXER_TEST_OUT_DIR} \
       ${PARSER_TEST_OUT_DIR} ${GENERATED_DIR} ${PARSER_OUT_DIR} ${SYNTREE_OUT_DIR} \
	   ${CMINUSC_OUT_DIR} ${LIB_OUT_DIR}

# -----------------------------------------------------------------------------
# Rules
# -----------------------------------------------------------------------------

all: prepare cminusc
	$(info )
	$(info ----------------------------)
	$(info All Done)
	$(info ----------------------------)

# 
# cminusc main rules
#
cminusc: lex_all syntree_all parser_all helper_all lib_all ${CMINUSC_OBJ} 
	${CXX} ${CXX_LINKFLAG} -o ${BUILD_DIR}/cminus ${PARSER_OBJ} ${LEXER_OBJ} ${SYNTREE_OBJ} ${CMINUSC_OBJ} ${HELPER_OBJ}

${CMINUSC_OBJ}: ${BUILD_DIR}/%.obj : %.cpp
	${CXX} ${CXX_FLAG} -c -o $@ $<


#
# cminusc lib rules
#
lib_all: ${LIB_FINAL}

${LIB_FINAL}: ${LIB_ASM_OBJ}
	${AR} rcs ${LIB_FINAL} ${LIB_ASM_OBJ}

${LIB_ASM_OBJ} : ${BUILD_DIR}/%.obj : %.asm
	${ASM} ${ASM_FLAG} -o $@ $<

## parser rules

parser_all: bison_test ${PARSER_OBJ}

### bison test
bison_test: ${PARSER_TEST_OBJ} ${SYNTREE_C_OBJ} ${HELPER_OBJ} ${LEXER_OBJ} ${PARSER_OBJ}
	${CC} ${C_FLAG} ${PARSER_TEST_OBJ} ${PARSER_BISON_OBJ} ${SYNTREE_C_OBJ} ${HELPER_OBJ} ${LEXER_OBJ} -o ${PARSER_TEST_OUT}

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

syntree_all: syntree_test ${SYNTREE_OBJ}

### syntree test
syntree_test: ${SYNTREE_TEST_OBJ} ${SYNTREE_C_OBJ}
	${CC} ${C_FLAG} ${SYNTREE_TEST_OBJ} ${SYNTREE_C_OBJ} -o ${SYNTREE_TEST_OUT}

${SYNTREE_TEST_OBJ}: ${SYNTREE_TEST_SRC}
	${CC} ${C_FLAG} -c -o ${SYNTREE_TEST_OBJ} ${SYNTREE_TEST_SRC}

### syntree
${SYNTREE_C_OBJ}: ${BUILD_DIR}/%.o : %.c
	${CC} ${C_FLAG} -c -o $@ $<

${SYNTREE_CXX_OBJ}: ${BUILD_DIR}/%.obj : %.cpp
	${CXX} ${CXX_FLAG} -c -o $@ $<

## Flex rules

lex_all: lex_test ${LEXER_OBJ}

### Flex test 

lex_test: ${LEXER_FLEX_TEST_OBJ} ${LEXER_TEST_OBJ} ${HELPER_OBJ} ${SYNTREE_C_OBJ}
	${CC} ${C_FLAG} ${LEXER_C_OBJ}  ${LEXER_FLEX_TEST_OBJ} ${LEXER_TEST_OBJ} ${HELPER_OBJ} ${SYNTREE_C_OBJ} -o ${LEXER_TEST_OUT} 

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

helper_all: ${HELPER_OBJ}

${HELPER_OBJ}: ${BUILD_DIR}/%.o: %.c
	${CC} ${C_FLAG} -c -o $@ $<

prepare:
	mkdir -p ${DIRS}

clean:
	rm -rf ${BUILD_DIR}
	rm -rf ${TEST_OUT_DIR}