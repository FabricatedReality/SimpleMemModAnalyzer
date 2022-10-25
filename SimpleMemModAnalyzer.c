#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SCOPE "global"
#define TYPES "char", "int", "float"
#define TYPES_SIZE 3
#define MAX_VAR_LEN 10
#define MAX_LINE_LEN 128

//Linked List
typedef struct Node {
  char *data;
  struct Node *next;
} node;

//Creating a node of Linked List, returns the head of the list
node* createNode(char *data){
  node *newNode = malloc(sizeof(node));
  newNode->data = malloc(strlen(data)+sizeof(char));
  strcpy(newNode->data, data);
  newNode->next = NULL;
  return newNode;
}

//Add to the tail of the linked list, returns the head of the list
node* addLast(node *head, node *newNode) {
  if(head == NULL){
    return newNode;
  }
  node *cur = head;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = newNode;
  return head;
}

node *addLast2(node *head, char *data) {
  return addLast(head, createNode(data));
}

//Free the memory allocated for the node
void freeNode(node *node){
  if(node != NULL){
    free(node->data);
    free(node);
  }
}

//Free the entire list
void freeList(node *head){
  node *cur = head;
  node *prev;
  while(cur != NULL){
    prev = cur;
    cur = cur->next;
    freeNode(prev);
  }
}

//remove the last element of the list, returns the head of the list
node* removeLast(node *head) {
  if(head == NULL || head->next == NULL){
    freeNode(head);
    return NULL;
  }
  node *cur = head;
  while(cur->next->next != NULL){
    cur = cur->next;
  }
  free(cur->next);
  cur->next = NULL;
  return head;
}

void printList(node *head){
  node *cur = head;
  while(cur != NULL){
    printf("%s\n",cur->data);
    cur = cur->next;
  }
}

//data for the function will be stored in the following format: function_name number_of_lines number_of_variables
void getFuncData(node* node, char *funcName, int *numLine, int *numVar){
  strcpy(funcName, node->data);
  strtok(funcName, " ");
  
  int num = strtol(strtok(NULL, " "), NULL, 10);
  if(numLine != NULL)
    *numLine = num;
  if(numVar != NULL)
    *numVar = strtol(strtok(NULL, " "), NULL, 10);
}

//print STATS part of the Memory Model Layout
void printStats(node *functions){
  node *cur = functions;
  int numFunc = 0;
  for(;cur != NULL; numFunc++, cur = cur->next);
  printf("  - Total number of functions: %d\n", numFunc);
  if(functions != NULL)
    printf("    ");
  
  cur = functions;
  while(cur != NULL){
    char funcName[64];
    getFuncData(cur, funcName, NULL, NULL);
    printf("%s", funcName);
    if(cur->next != NULL)
      printf(", ");
    cur = cur->next;
  }

  cur = functions;
  printf("\n  - Total number of lines per functions:\n");
  while(cur != NULL){
    int numLines;
    char funcName[64];
    getFuncData(cur, funcName, &numLines, NULL);
    printf("    %s: %d\n", funcName, numLines);
    cur = cur->next;
  }

  cur = functions;
  printf("  - Total number of variables per function:\n");
  while(cur != NULL){
    int numVar;
    char funcName[64];
    getFuncData(cur, funcName, NULL, &numVar);
    printf("    %s: %d\n", funcName, numVar);
    cur = cur->next;
  }

  freeList(functions);
}

//print out the Memory Model Layout
void printMemModel(char *progName, node *ROData, node *staticData, node *heap, node *stack, node *functions, int numLines){
  printf(">>> Memory Model Layout <<<\n\n");
  printf("***  exec // test ***\n");
  printf("   %s\n\n", progName);

  printf("### ROData ###       scope  type  size\n");
  printList(ROData);
  freeList(ROData);
  
  printf("\n### static data ###\n");
  printList(staticData);
  freeList(staticData);

  printf("\n### heap ###\n");
  printList(heap);
  freeList(heap);
  
  printf("\n####################\n");
  printf("### unused space ###\n");
  printf("####################\n");

  printf("\n### stack ###\n");
  printList(stack);
  freeList(stack);
  
  printf("\n**** STATS ****\n");
  printf("  - Total number of lines in the file: %d\n", numLines);
  printStats(functions);
  printf("//////////////////////////////\n");
}

//returns pointer to the first character that is not a white space, assume str is a well defined string
//returns NULL if it reaches end of string (not including string delimiter)
char *skipSpace(char *str){
  char *ptr = str;
  int len = strlen(str);
  for(int i = 0; i < len; i++, ptr += 1){
    if(*ptr != ' '){
      return ptr;
    }
  }
  return NULL;
}

//strstr but only checks for first n characters in str1 and don't return pointer
//returns 1 when str2 exist in first n character of str1, else return 0
int strnstr(char *str1, char*str2, int n){
  char tempStr[n + 1];
  strncpy(tempStr, str1, n);
  tempStr[n] = '\0';
  char *position = strstr(tempStr, str2);
  return position != NULL;
}

//check for pointers starting from ptr, if it is a pointer, then change type to TYPE *
//returns the first character after the * symbol, if there are no * symbol return first character after whitespace
char *formatPointer(char *ptr, char *type){
  //Assume that endPtr does not have only space after, since it is not possible to have TYPE on its own in a line
  char *endPtr = skipSpace(ptr);
  if(*endPtr == '*'){
    strcat(type, "*");
  }
  return endPtr;
}

//returns the pointer to the first element after the type declaration, returns NULL if there are no declaration
char *identifyType(char *curLine, char *type){
  char *types[] = {TYPES};
  char *endPtr = NULL;
  char *start = skipSpace(curLine);

  int i = 0;
  for(; i < TYPES_SIZE; i++){
    //in case of special case such as strcpy(a, "int")
    if(strnstr(start, types[i], strlen(types[i]))){
      strcpy(type, types[i]);
      endPtr = start + strlen(types[i]);
      break;
    }
  }

  if(endPtr == NULL)
    return NULL;

  endPtr = formatPointer(endPtr, type);
  //Special case of char** argv in main 
  if(*(endPtr + 1) == '*' && strcmp(type, "char*") == 0){
    strcat(type, "*");
  }
  
  //check for arrays starting from ptr, if it is an array, then change type to TYPE []
  //If it is type TYPE var[], then endPtr should be pointing to first char of var[]
  if(strchr(endPtr, '[') != NULL){
    strcat(type, "[]");
  }
  
  return skipSpace(endPtr);  
}

//Removes the tailing space of a string
void removeTailingSpace(char *str){
  int endIndex = strlen(str) - 1;
  for(int i = endIndex; i >= 0; i--){
    if(str[i] != ' '){
      str[i + 1] = '\0';
	  return;
    }
  }
}

/*
gets the name of the variable and copies it over to varName
identifier represents the character after the variable name ends
assume that str is a valid string and starts at the beginning of the variable
*/
void identifyName(char *str, char *varName, char identifier){
  char *nameEnd = strchr(str, identifier);
  char *lineEnd = strchr(str, ';');
  if(nameEnd != NULL){
    int varLen = strchr(str, identifier) - str;
    strncpy(varName, str, varLen);
    varName[varLen] = '\0';
  } else if(lineEnd != NULL){
    int varLen = strlen(str);
    strncpy(varName, str, varLen - 1);
    varName[varLen] = '\0';
  }else{
    strcpy(varName, str);
  }
  removeTailingSpace(varName);
}

//gets the name of the variable from str and store it in varName, type = "f" if it is a function
void getName(char *str, char *varName, char *type){
  if(strchr(type, '[') != NULL){
    identifyName(str, varName, '[');
  } else if(strchr(type, 'f') != NULL) {
    identifyName(str, varName, '(');
  } else{
    identifyName(str, varName, '=');
  }
  if(strchr(type, '*')){
    if(strstr(type, "**")){
      strcpy(varName, varName + 2);
    } else{
      strcpy(varName, varName + 1);
    }
  }

  char *endPtr = strchr(varName, ';');
  if(endPtr != NULL)
    *endPtr = '\0';
  
  endPtr = strchr(varName, '\r');
  if(endPtr != NULL)
    *endPtr = '\0';
  
  endPtr = strchr(varName, ')');
  if(endPtr != NULL)
    *endPtr = '\0';
  
  removeTailingSpace(varName);
  char temp[32];
  strcpy(temp, skipSpace(varName));
  strcpy(varName, temp);
}

//change varSize to the string of the size of one of the 3 base types: int, float, char
//return the size as string literal
const char *getBaseSize(char *varSize, char *type){
  if(strstr(type, "char") != NULL){
    sprintf(varSize, "%ld", sizeof(char));
    return "sizeof(char)";
  } else if(strstr(type, "int") != NULL){
    sprintf(varSize, "%ld", sizeof(int));
    return "sizeof(int)";
  } else{
    sprintf(varSize, "%ld", sizeof(float));
    return "sizeof(float)";
  }
}

int getArrayLen(char *arrayStart){
	if(strchr(arrayStart, '{') + 1 == strchr(arrayStart, '}')){
		return 0;
	}
	char *commaPtr = strchr(arrayStart, ',');
	int count = 1;
	for(;commaPtr != NULL;count++, commaPtr = strchr(commaPtr + 1, ','));
	return count;
}

//returns the length of the string literal if there is one, 0 if there is no string literal assigned to the string
int identifySize(char *str, char *varSize, char *type){
  const char *sizeStr = getBaseSize(varSize, type);
  if(strchr(str, '"') != NULL){
    char *strStart = strchr(str, '"');
    char *strEnd = strchr(strStart + 1, '"');
    int strLen = strEnd - strStart;
    if(strchr(type, '*')){
      //This is a string literal
      sprintf(varSize, "%ld", sizeof(char *));
      return strLen;
    } else{
      char *brkStart = strchr(str, '[');
      brkStart += 1;
      brkStart = skipSpace(brkStart);
      if(*brkStart == ']'){
        sprintf(varSize, "%d*%s", strLen, sizeStr);
        return 0;
      }
    }
  }
  if(strchr(type, '[') != NULL){
    char *lenStart = strchr(str, '[') + 1;
    char *lenEnd = strchr(str, ']');
    //variable in the form TYPE var[N]
    if(lenStart != lenEnd){
      int lenSize = lenEnd - lenStart;
      char tempStr[32];
      strncpy(tempStr, lenStart, lenSize);
      tempStr[lenSize] = '\0';
      sprintf(varSize, "%s*%s", tempStr, sizeStr);
    } else{
      //variable in the form TYPE var[] = {1, 2, 3}
      int ArrayLen = getArrayLen(lenEnd);
      sprintf(varSize, "%d*%s", ArrayLen, sizeStr);
    }
  } else if(strchr(type, '*') != NULL){
    if(strstr(type, "**") != NULL){
      sprintf(varSize, "%ld", sizeof(char **));
    }else if(strstr(type, "char") != NULL){
      sprintf(varSize, "%ld", sizeof(char *));
    } else if(strstr(type, "int") != NULL){
      sprintf(varSize, "%ld", sizeof(int *));
    } else{
      sprintf(varSize, "%ld", sizeof(float *));
    }
  }
  return 0;
}

/*
takes in str and split it into different variable declarations
returns a pointer to the first variable and changes ',' to '\0', str is set to the rest
example: int a, b, c is split into varStr = int a\0, str = b, c
*str is a proper string or NULL
*/
char *splitName(char **str){
  if(*str == NULL)
    return NULL;
  
  char *firstComma = strchr(*str, ',');
  char *temp = *str;
  if(firstComma == NULL){
    *str = NULL;
    return temp;
  }
  
  char delim[2];
  if(strchr(temp, '"') != NULL){
	  delim[0] = '"';
	  delim[1] = '"';
  } else if(strchr(temp, '{') != NULL) {
	  delim[0] = '{';
	  delim[1] = '}';
  } else if(strchr(temp, '\'') != NULL){
	  delim[0] = '\'';
	  delim[1] = '\'';
  } else if(strstr(temp, "alloc") != NULL){
	  delim[0] = '(';
	  delim[1] = ')';
  } 
  char *firstDelim = strchr(temp, delim[0]);
  
  //str in the form var1, var2 or var1, var2 = {1,2}
  if(firstDelim != NULL && firstComma < firstDelim){
    *str = firstComma;
  } else {
    //str in the form var1[] = {1, 2}, var2 or var1[] = {2,3};
    char *closeDelim = strchr(firstDelim + 1, delim[1]);
    *str = strchr(closeDelim + 1, ',');
  }
  if(*str != NULL){
    **str = '\0';
    *str = skipSpace(*str + 1);
  }
  return temp;
}

//Store variable string in the list, id = 'n' for normal variables, id = 'h' for variables in the heap, id = 'r' for ROData
void storeVar(char *varName, char *scope, char *type, char *varSize, node **ROData, node **staticData, node **heap, node **stack, char id){
	char displayStr[64];
	sprintf(displayStr, "   %s   %s   %s   %s", varName, scope, type, varSize);
  if(id == 'h'){
    *heap = addLast2(*heap, displayStr);
  } else if(id == 'r'){
    *ROData = addLast2(*ROData, displayStr);
  } else if(strcmp(scope, "global") == 0){
	  *staticData = addLast2(*staticData, displayStr);
	} else {
		*stack = addLast2(*stack, displayStr);
	}
}

void findType(char *str, char *type){
  if(strstr(str, "int") != NULL){
    strcpy(type, "int");
  } else if(strstr(str, "char") != NULL){
    strcpy(type, "char");
  } else if(strstr(str, "float") != NULL){
    strcpy(type, "float");
  }
}

void getHeapType(char *varName, char *type, node* stack){
    node *cur = stack;
    while(cur != NULL){
      //since varName = *varname
      if(strstr(cur->data, varName) != NULL){
        findType(cur->data, type);
        return;
      }
      cur = cur->next;
    }
    strcpy(varName, "void");
}

void getHeapName(char *str, char *varName){
  char temp[1] = {'\0'};
  getName(str, varName + 1, temp);
  *(varName) = '*';
}

void getHeapSize(char* str, char *size){
  char *bktStart = strchr(str, '(') + 1;
  char *comma = strchr(str, ',');
  int bktCount = 1;
  int i = 0;
  for(; *(bktStart + i) != '\0'; i++){
    if(*(bktStart + i) == '(')
      bktCount += 1;
    else if(*(bktStart + i) == ')')
      bktCount -= 1;
    if(bktCount == 0)
      break;
  }
  //since malloc only takes in 1 argument
  if(comma != NULL){
    char temp[32];
    //set temp to first parameter of calloc
    strncpy(temp, bktStart, comma - bktStart);
    temp[comma - bktStart] = '\0';
    sprintf(size, "(%s)*(", temp);
    
    char *tempPtr = skipSpace(comma + 1);
    //set temp to second parameter of calloc
    strncpy(temp, tempPtr, bktStart + i - tempPtr);
    temp[bktStart + i - tempPtr] = '\0';
    strcat(size, temp);
    strcat(size, ")");
  } else {
    strncpy(size, bktStart, i);
    *(size + i) = '\0';
    removeTailingSpace(size);
  }
}

void processAlloc(char *str, char *scope, char *varName, char *type, node **stack, node **heap){
  char *alloc = strstr(str, "alloc");
  char *quote = strchr(str, '"');
  if(alloc != NULL && (quote == NULL || (quote != NULL && alloc < quote))){
    char heapName[32];
    char heapType[32];
    char heapSize[32];
    getHeapSize(str, heapSize);
    
    if(varName == NULL){
      getHeapName(str, heapName);
    } else{
      sprintf(heapName, "*%s", varName);
    }
	  if(type == NULL){
      getHeapType(heapName + 1, heapType, *stack);
    } else{
      strcpy(heapType, type);
      heapType[strlen(type)-1] = '\0';
    }
    storeVar(heapName, scope, heapType, heapSize, NULL, NULL, heap, NULL, 'h');
  }
}

void processLine(char *curLine, char *scope, int *numVar, node **ROData, node **staticData, node **heap, node **stack){
  if(curLine == NULL)
    return;
  char type[MAX_VAR_LEN];
  //start is the pointer that points to the first variable name
  char *start = identifyType(curLine, type);
  
  
  if(start == NULL){
    processAlloc(curLine, scope, NULL, NULL, stack, heap);
    return;
  }
  
  char *varStr = splitName(&start);
  char varName[32];
  char varSize[32];

  do{
    //store normal variables
    getName(varStr, varName, type);
    int strLiteral = identifySize(varStr, varSize, type);
    storeVar(varName, scope, type, varSize, ROData, staticData, heap, stack, 'n');
    //store allocs
    processAlloc(varStr, scope, varName, type, stack, heap);
    //store string literals
    if(strLiteral > 0){
      char tempSize[16];
      sprintf(tempSize, "%d*sizeof(char)", strLiteral);
      char tempType[8] = "char[]";
      storeVar(varName, scope, tempType, tempSize, ROData, staticData, heap, stack, 'r');
    }
    *numVar += 1;
	
    varStr = splitName(&start);
  } while(varStr != NULL);
}

/*
counts the number of occurrance of c in str
returns 0 if there are no occurrances
str have to be a valid string 
*/
int countChars(char *str, char c){
  int numOccur = 0;
  int len = strlen(str);
  for(int i = 0; i < len;i++){
    if(str[i] == c){
      numOccur++;
    }
  }
  return numOccur;
}

void getScope(char *curLine, char *scope){
  char temp[2] = "f";
  getName(curLine, scope, temp);
  char *starPtr = strchr(scope, '*');
  char *spacePtr = strchr(scope, ' ');
  if(starPtr != NULL && spacePtr != NULL){
    if(starPtr > spacePtr)
      strcpy(scope, starPtr + 1);
    else
      strcpy(scope, spacePtr + 1);
  } else if(starPtr != NULL && spacePtr == NULL){
    strcpy(scope, starPtr + 1);
  } else if(starPtr == NULL && spacePtr != NULL){
    strcpy(scope, spacePtr + 1);
  }
}

void processFuncArgs(char *curLine, char *scope, int *numVar, node **stack){
  char *start = strchr(curLine, '(') + 1;
  char *varStr = splitName(&start);
  char varName[32];
  char varSize[32];

  do{
    char type[MAX_VAR_LEN];
    //varStr is now the pointer that points to the first variable name
    varStr = identifyType(varStr, type);
    if(varStr == NULL)
      return;
    
    getName(varStr, varName, type);
    identifySize(varStr, varSize, type);
    storeVar(varName, scope, type, varSize, NULL, NULL, NULL, stack, 'n');
    *numVar += 1;
    
    varStr = splitName(&start);
  } while(varStr != NULL);
}

//Identify variables and store them as read to print strings into linked lists
void analyzeFile(FILE *file, node **ROData, node **staticData, node **heap, node **stack, node **functions, int *numLines){
  char curLine[MAX_LINE_LEN];
  char nextLine[MAX_LINE_LEN];
  char scope[32] = DEFAULT_SCOPE;
  //use funcLines = -1 for whenever scope is global
  int funcLines = -1;
  int numVar = 0;
  //Number of curly brackets to keep tract of inside function or not
  int numBrkts = 0;
  
  //set default value
  if(fgets(curLine, MAX_LINE_LEN * sizeof(char), file) == NULL)
	  return;
  char *finished = fgets(nextLine, MAX_LINE_LEN * sizeof(char), file);
  //to make the loop run through all lines
  char *prevF = finished;
  
  do {
    *numLines += 1;
    numBrkts += countChars(nextLine, '{');
    
    //Check if it is function declaraction
    if(numBrkts > 0 && strcmp(scope, DEFAULT_SCOPE) == 0){
      funcLines = 0;
      numVar = 0;
      
      getScope(curLine, scope);
      processFuncArgs(curLine, scope, &numVar, stack);
    } else{
      //It is not a function declaration
      if(numBrkts == 0 && strcmp(scope, DEFAULT_SCOPE) != 0){
        char funcStr[64];
        sprintf(funcStr, "%s %d %d", scope, funcLines - 1, numVar);
        *functions = addLast2(*functions, funcStr);
        strcpy(scope, DEFAULT_SCOPE);
        funcLines = -1;
      }
      
      //Whenever scope is in a function, start counting the number of lines of this function
      if(funcLines != -1)
        funcLines += 1;
      //process line takes in pointer to curLine starting from first non-white space character
      processLine(curLine, scope, &numVar, ROData, staticData, heap, stack);
    }
    
    numBrkts -= countChars(nextLine, '}');
    
    strcpy(curLine, nextLine);
    prevF = finished;
    finished = fgets(nextLine, MAX_LINE_LEN * sizeof(char), file);
  } while(prevF != NULL);
}

int main(int argc, char **argv) {
  if(argc != 2){
    fprintf(stderr, "Error: Invalid Argument\n");
    return -1;
  }

  char path[64] = "./";
  strncat(path, argv[1], 61);
  FILE *file = fopen(path,"r");
  if(file == NULL){
    fprintf(stderr, "Error: Invalid File\n");
    return -1;
  }

  node *ROData = NULL;
  node *staticData = NULL;
  node *heap = NULL;
  node *stack = NULL;
  node *functions = NULL;
  int numLines = 0;
  analyzeFile(file, &ROData, &staticData, &heap, &stack, &functions, &numLines);
  printMemModel(argv[1], ROData, staticData, heap, stack, functions, numLines);
  return 0;
}
