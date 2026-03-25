/*
 * debug.h
 *
 *  Created on: Mar 11, 2024
 *      Author: Ben deVries
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum vtseq {
	VT_UNKOWN = 0,
	VT_F1,
	VT_F2,
	VT_F3,
	VT_F4,
	VT_F5,
	VT_F6,
	VT_F7,
	VT_F8,
	VT_F9,
	VT_F10,
	VT_F11,
	VT_F12,
	VT_HOME,
	VT_INSERT,
	VT_DELETE,
	VT_END,
	VT_PGUP,
	VT_PGDOWN,
}vtseq_t;

#define N_FUNKEYS 19


typedef struct debugFunction {
	//What to type to call function
	const char *name;

	//A brief description of the function
	const char *desc;

	//What function to be called
	void (*fnc)(int,char **);

	//This is used if the function requires only int type numbers
	const bool onlyNumeric;

	const int minArguments;

	const int maxArguments;
}debugFunction_t;

typedef struct debugFunctionSub {
	struct debugFunction *debugFun;
	struct debugFunctionSub *next;
}debugFunctionSub_t;




/*
 * @def DEFINE_DEBUG_SUB
 *
 * @brief helper macro to define debug subscriptions
 *
 * @param subname what you want to name the SUBSCRIPTION
 *
 * @param funct the function being used excpected to be in for void foo(int argc,char **argv)
 *
 * @param description a string giving a brief overview of the function
 *
 * @param onlyNum set to true if the only numaric arguements are to be expected
 */
#define DEFINE_DEBUG_SUB(subname,funct,description,onlyNum,minArgs,maxArgs) 			\
	const char funct##_namestring[] = #funct;											\
	const char funct##_descriptionstring[] = description;								\
	debugFunction_t funct##s_struct = {													\
			.name = funct##_namestring,													\
			.desc = funct##_descriptionstring,											\
			.fnc = funct,																\
			.onlyNumeric = onlyNum,														\
			.minArguments = minArgs,														\
			.maxArguments = maxArgs,														\
	};																					\
	debugFunctionSub_t subname = {														\
			.debugFun = &funct##s_struct,												\
			.next = NULL,																\
	}																					\


#define DEFINE_DEBUG_SUB_NO_ARGS(subname,funct,description) \
    DEFINE_DEBUG_SUB(subname,funct,description,false,0,0)


void vdebug_print(const char *str,va_list arg);

void debug_print(const char *str,...);

void startdebug(void (*sendstring)(const char*const));

void debugIn(const char c);

void subFnc(debugFunctionSub_t *newSub);

void subFncKey(debugFunctionSub_t *sub,vtseq_t key);

/*
 * @def isNumaric
 *
 * @brief Checks weather a string is a valid integer(positive or negitive)
 *
 * @param str the string being validated
 *
 * @return true if string only contains 0-9 and optionality a - in front
 *
 */
bool isNumeric(const char *str);


/*
 * @def strToInt
 *
 * @brief converts a string to a signed integer or 0 on non valid integer
 * (don't tell David but this is were i miss exceptions)
 *
 * @param str the string being converted
 *
 * @return a signed 32 bit number of the string
 *
 */
int32_t strToInt(const char *str);

/*
 * @def strToUint
 *
 * @brief converts a string to a unsigned integer or 0 on non valid integer
 *
 * @param str the string being converted
 *
 * @return a unsigned 32 bit number of the string
 *
 */
uint32_t strToUint(const char *str);

bool strToBool(char *str);


bool stringMatch(const char *str1,const char *str2);


//Makes all letters lower case and return pointer to str Note Mutates str
char *toLower(char *str);

//Makes all letters upper case and returns pointer to str Note Mutates str
char *toUpper(char *str);


/*
 * @def checkArgs
 *
 * @brief checks a string against a constant list of strings
 * example if argString is "input in i 10v" the function will return true if str is
 * "input" "in" "i" or "10v"
 *
 * @param str the string being checked
 *
 * @param argString list of strings to check against split with spaces so for example "input in i 10v"
 *
 * @return whether string is in the list
 *
 */
bool checkArgs(const char *str,const char *argString);


#endif /* INC_DEBUG_H_ */
