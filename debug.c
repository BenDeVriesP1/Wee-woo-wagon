/*
 * debug.c
 *
 *  Created on: Mar 11, 2024
 *      Author: Ben deVries
 */


#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"


#define WORKINGOUTBUFFERLEN 256

#define STOREDINPUTS		8

#define WORKINGINBUFFLEN	128
#define MAXARGUMENTS		16

#define RUNUNITTEST			0

#if RUNUNITTEST
void debugUnitTest(void);
#endif

void (*sendCmd)(const char*const) = NULL;


static char workingBuffer[WORKINGOUTBUFFERLEN];


static inline void sendStr(const char *str)
{
	if(sendCmd != NULL)
	{
		sendCmd(str);
	}
}

void vdebug_print(const char *str,va_list arg)
{
	vsnprintf (workingBuffer, WORKINGOUTBUFFERLEN, str,arg);
	sendStr(workingBuffer);
}

void debug_print(const char *str,...)
{
	va_list arg;
	va_start(arg,str);
	vdebug_print(str,arg);
	va_end(arg);
}


void echoChar(const char c)
{
	static char str[2] = {'a','\0'};
	str[0] = c;
	sendStr(str);
}


static const char delstring[] = "\x1b[1D \x1b[1D";

static const vtseq_t vtBytilda[25] = {VT_UNKOWN,VT_HOME,VT_INSERT,VT_DELETE,
									  VT_END,VT_PGUP,VT_PGDOWN,VT_HOME,
									  VT_F4,VT_UNKOWN,VT_UNKOWN,VT_F1,
									  VT_F2,VT_F3,VT_F4,VT_F5,
									  VT_UNKOWN,VT_F6,VT_F7,VT_F8,
									  VT_F9,VT_F10,VT_UNKOWN,VT_F11,
									  VT_F12};


static debugFunctionSub_t FunctionKeySubs[N_FUNKEYS] = {0};

static void callFNCFNC(vtseq_t fKey)
{
	char *noStr[] = {""};
	if(FunctionKeySubs[fKey].debugFun != NULL)
	{
		FunctionKeySubs[fKey].debugFun->fnc(0,noStr);
		//Hand it something in case someone is dumb enought to try and parse it
	}
	else
	{
		debug_print("No Function Bound to F%d\r\n",fKey);
	}
}

static char storedInputs[STOREDINPUTS][WORKINGOUTBUFFERLEN];
static size_t storedInputOrder[STOREDINPUTS];

void initStoredInputs(void)
{
	for(size_t i = 0;i<STOREDINPUTS;i++)
	{
		memset(storedInputs[i],0x00,WORKINGOUTBUFFERLEN);
		storedInputOrder[i] = 0;
	}
}

static void replace_current_line(char* inputBuffer, size_t* inputIndex, char *newStr)
{
	sendStr("\x1b[2K\r"); //Escape code to erase the full line, carriage return to return cursor position
	sendStr(newStr);

	strcpy(inputBuffer, newStr);
	*inputIndex = strlen(inputBuffer);
}


size_t get_save_index(char *str,size_t *listOrder,size_t max_list_len)
{
	size_t ret = 1;
	size_t savedInputIndex = 0;
	for(ret = 1;ret<(max_list_len-1);ret++) //First Item always one last item cannot be max_list_len so should be max-1;
	{
		savedInputIndex = listOrder[ret];
		if(savedInputIndex == 0)
		{
			break;//End of saved values
		}
		if(stringMatch(str,storedInputs[savedInputIndex]))
		{
			break;
		}
	}
	return ret;
}


char *line_dot_get(const char c)
{
	static uint8_t escapeState = 0,fNUM=0; //0 nothing 1 got esc 2 is got [
	vtseq_t vtOut = VT_UNKOWN;
	static char parsedString[WORKINGOUTBUFFERLEN] = {0};
	static size_t index = 0;

	static size_t totalStoredInputs = 0;
	static size_t selectedInput = 0;

	//Ok with any luck this should mirror the bourne  again shell as much as possible

	switch(escapeState)
	{
	case 0:
		switch(c)
		{
		case 0x1b: //<esc>
			escapeState = 1;
			break;
		case 0x08: //Backspace
		case 0x7f: //Delete
			if(index>0)
			{
				parsedString[index-1] = '\0';
				index = index-1;
				sendStr(delstring);
			}
			break;
		case '\r'://Carriage return AKA DING!
		case '\n'://New Line
			if(index>0)//Only return when you got something
			{
				parsedString[index++] = '\0'; // Add null terminator and return
				index=0;
				sendStr("\r\n");


				size_t saveLoc;



				size_t save_index = get_save_index(parsedString,storedInputOrder,STOREDINPUTS);

				if(save_index > totalStoredInputs)
				{
					totalStoredInputs = save_index;
					saveLoc = save_index;
				}
				else
				{
					saveLoc = storedInputOrder[save_index];
				}
				for(size_t i = (save_index-1);i>0;i--) //Last I should be 1
				{
					storedInputOrder[i+1] = storedInputOrder[i];
				}
				storedInputOrder[1] = saveLoc;







				memcpy(storedInputs[saveLoc],parsedString,WORKINGOUTBUFFERLEN);


				/* KEEPING FOR DEBUG PURPOSES
				debug_print("SI: %d\r\n",save_index);
				debug_print("SL: %d\r\n",saveLoc);
				debug_print("{");
				for(size_t i = 0;i<STOREDINPUTS;i++)
				{
					debug_print(" %d,",storedInputOrder[i]);
				}
				sendStr(delstring);
				debug_print("}\r\n");
				************************************/

				memset(parsedString,0x00,WORKINGOUTBUFFERLEN);
				memset(storedInputs[0],0x00,WORKINGOUTBUFFERLEN);
				selectedInput = 0;
				return storedInputs[saveLoc];


			}
			break;
		default:
			if(index < (WORKINGOUTBUFFERLEN-1)) //Save room for null terminator
			{
				parsedString[index++] = c;
				echoChar(c);
			}
			break;
		}

		break;
	case 1:
		switch(c)
		{
		case '[':
			fNUM=0;
			escapeState = 2;
			break;
		default:
			sendStr("<esc>");
			echoChar(c);
			escapeState = 0;
			break;
		}
		break;
	case 2:
		switch(c)
		{
			case 'A':
				if(selectedInput<totalStoredInputs)
				{
					parsedString[index] = '\0'; //Add a null terminator before storing
					strcpy(storedInputs[storedInputOrder[selectedInput]], parsedString); //Store the current string to the current selected input
					selectedInput++;
					replace_current_line(parsedString, &index, storedInputs[storedInputOrder[selectedInput]]);
				}

				escapeState = 0;
				break;
			case 'B':
				if(selectedInput>0)
				{
					parsedString[index] = '\0'; //Add a null terminator before storing
					strcpy(storedInputs[storedInputOrder[selectedInput]], parsedString); //Store the current string to the current selected input
					selectedInput--;
					replace_current_line(parsedString, &index, storedInputs[storedInputOrder[selectedInput]]);
				}

				escapeState = 0;
				break;
			case 'C':

				escapeState = 0;
				break;
			case 'D':

				escapeState = 0;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				fNUM*=10;
				fNUM += (c-'0');
				break;
			case '~':
				if(fNUM>24)
				{
					vtOut = VT_UNKOWN;
				}
				else
				{
					vtOut = vtBytilda[fNUM];
				}
				debug_print("F%d\r\n",vtOut);
				callFNCFNC(vtOut);
				escapeState = 0;
				break;
			default:
				echoChar(c);
				escapeState = 0;
				break;
		}
	}

	return NULL;

}



char *getArgcArgV(char *string,int *argc,char **argv)
{
	bool LookingForSpace = true; //I thought this was cleaner when this variable was called LookingForNotSpace but whatever...
	*argc = 0;
	for(int i = 0;i<MAXARGUMENTS;i++)
	{
		argv[i] = NULL;
	}
	for(char *c = string;*c != '\0';c++)
	{
		if(LookingForSpace)
		{
			if(*c == ' ')
			{
				LookingForSpace = false;
				*c = '\0';
				if(*argc>=MAXARGUMENTS)
				{
					break;
				}
			}
		}
		else
		{
			if(*c != ' ')
			{
				LookingForSpace=true;
				argv[*argc] = c;
				*argc = *argc+1;
			}
			else
			{
				*c = '\0';
			}
		}
	}
	return string;
}

static void help(int argc,char **argv);

DEFINE_DEBUG_SUB(helpsub,help,"print a list of functions and descriptions",false,0,0);

static debugFunctionSub_t *subHead = &helpsub;



static void printFNC(debugFunctionSub_t *sub)
{
	debug_print("%s",sub->debugFun->name);
	if(strlen(sub->debugFun->name) >= 16)
	{
		debug_print("\t");
	}
	else if(strlen(sub->debugFun->name) >= 8)
	{
		debug_print("\t\t");
	}
	else
	{
		debug_print("\t\t\t");
	}
	debug_print("%s\r\n",sub->debugFun->desc);
}


static void help(int argc,char **argv)
{
	(void)argc;
	(void)argv;

	debugFunctionSub_t *sub;
	debug_print("\r\n------------------------------------------------------------\r\n");
	debug_print("Functions\r\n");
	debug_print("Name\t\t\tdescription\r\n");
	debug_print("------------------------------------------------------------\r\n");
	for(sub = subHead;sub != NULL;sub = sub->next)
	{
		printFNC(sub);
	}
	debug_print("\r\n\n");
	debug_print("Function Key Functions\r\n");
	debug_print("FNC\tName\t\t\tdescription\r\n");
	debug_print("------------------------------------------------------------\r\n");
	for(size_t i = 0;i<N_FUNKEYS;i++)
	{
		if(FunctionKeySubs[i].debugFun != NULL)
		{
			debug_print("F%d\t",i);
			//Yes this does say the Home key is F13 and PageUp if F14 ect..
			//But I have yet to see someone actually use those so...
			printFNC(&FunctionKeySubs[i]);
		}
	}
}

void subFnc(debugFunctionSub_t *newSub)
{
	debugFunctionSub_t *sub;
	for(sub = subHead;sub != NULL;sub = sub->next)
	{
		if(sub->next == NULL)
		{
			sub->next = newSub;
			break;
		}
	}
}



void subFncKey(debugFunctionSub_t *sub,vtseq_t key)
{
	if((key == VT_UNKOWN) || (key>=N_FUNKEYS) || (sub->debugFun->minArguments != 0))
	{
		debug_print("could not sub function");
		return;
	}
	FunctionKeySubs[key].debugFun = sub->debugFun;
}


int compareStr(const char* str1,const char* str2)
{
	const char *c1,*c2;
	for(c1 = str1,c2 = str2;(*c1 != '\0' && *c1 != '\0');c1++,c2++)
	{
		if(*c1 != *c2)
		{
			return -1;
		}
	}
	if(*c1 != *c2)
	{
		return -1;
	}
	return 0;
}


void debugIn(const char c)
{
	char *in;
	static char workingString[WORKINGOUTBUFFERLEN];
	static int argc = 0;
	static char *argv[MAXARGUMENTS];
	static char *argument;
	in = line_dot_get(c);
	debugFunctionSub_t *sub;
	if(in)
	{
		memcpy(workingString,in,WORKINGOUTBUFFERLEN);
		in = workingString;

		//In order to have the stored strings in the history remain the same we should only manipulate this
		//Working Copy

		argument = getArgcArgV(in,&argc,argv);
		for(sub = subHead;sub != NULL;sub = sub->next)
		{
			if(compareStr(argument,sub->debugFun->name)==0)
			{
				if(argc>sub->debugFun->maxArguments)
				{
					//Usesrs are responsible for ensure max arguments is >= than minArguments
					debug_print("to many arguments got %d max %d\r\n",argc,sub->debugFun->maxArguments);
				}
				else if(argc<sub->debugFun->minArguments)
				{
					debug_print("to few arguments got %d min %d\r\n",argc,sub->debugFun->minArguments);
				}
				else
				{
					if(sub->debugFun->onlyNumeric)
					{
						for(int i = 0;i<argc;i++)
						{
							if(!isNumeric(argv[i]))
							{
								debug_print("Argument %d should be numeric\r\n",i);
							}
						}
					}
					sub->debugFun->fnc(argc,argv);
				}

				break;
			}
		}
		if(sub == NULL)
		{
			debug_print("function %s not found\r\n",argument);
			help(0,NULL);
		}
	}
}




void startdebug(void (*sendstring)(const char*const))
{
	sendCmd = sendstring;

	initStoredInputs();

#if RUNUNITTEST
	debugUnitTest();
#endif
}


bool isNumeric(const char *str)
{
	if(*str == '-')
	{
		str++;
	}
	if(*str == '\0')
	{
		return false;
	}
	for(const char*c=str;*c != '\0';c++)
	{
		if(*c < '0' || *c > '9')
		{
			return false;
		}
	}
	return true;
}


int32_t strToInt(const char *str)
{
	bool flipsign = false;
	if(*str == '-')
	{
		flipsign = true;
		str++;
	}
	
	int32_t ret = (int32_t) strToUint(str); //Underflows possible
	
	if(flipsign)
	{
		ret *= -1;
	}
	return ret;
}

uint32_t strToUint(const char *str)
{
	uint32_t ret = 0;
	if(isNumeric(str) && *str != '-')
	{
		for(const char *c = str;*c != '\0';c++)
		{
			ret *= 10;
			ret += (*c-'0');
		}
	}
	return ret;
}

bool stringMatch(const char *str1,const char *str2)
{
	return((strcmp(str1,str2) == 0)? true : false);
}



static inline bool isAlphaC(char c)
{
	return(((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z')));
}

static inline char toLowerC(char c)
{
	if((c>='A')&&(c<='Z'))
	{
		//Why this works is pretty self explanatory
		return c + 0x20;
	}
	return c;
}

static inline char toUpperC(char c)
{
	if((c>='a')&&(c<='z'))
	{
		//Why this works is pretty self explanatory
		return c - 0x20;
	}
	return c;
}


char *toLower(char *str)
{
	for(char *c = str;*c != '\0';c++)
	{
		*c = toLowerC(*c);
	}
	return str;
}


//Makes all letters upper case and returns pointer to str Note Mutates str
char *toUpper(char *str)
{
	for(char *c = str;*c != '\0';c++)
	{
		*c = toUpperC(*c);
	}
	return str;
}


bool checkArgs(const char *str,const char *argString)
{
	const char *cin = str, *c;
	bool strEqual = true;
	for(c = argString;*c != '\0';c++)
	{
		switch(*c)
		{
			case ' ':
				if(strEqual && (*cin == '\0'))
				{
					return true;
				}
				cin = str;
				strEqual = true;
				break;
			default:
				if(strEqual && (*c == *cin))
				{
					cin++;
				}
				else
				{
					strEqual = false;
				}
				break;
		}
	}
	if(*cin == '\0')
	{
		return strEqual;
	}
	return false;
}


bool strToBool(char *str)
{
	size_t strlength = strlen(str);
	toLower(str);
	switch(strlength)
	{
		case 1:
			if(stringMatch(str,"t") || stringMatch(str,"y") || stringMatch(str,"1"))
			{
				return true;
			}
			return false;
		case 3:
			if(stringMatch(str,"yes"))
			{
				return true;
			}
			return false;
		case 4:
			if(stringMatch(str,"true"))
			{
				return true;
			}
			return false;
		default:
			return false;
	}
	return false;
}