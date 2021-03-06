/* parse_CGI.c:
 *
 * Copyright (C) 2009-2014  Alexander Reimer <alex_raw@rambler.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */



#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>		//used for malloc & free
#include <string.h>
//#include <ctype.h>
#include "include/httpd.h"
#include "parse_CGI.h"
#include "copy.h"


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define DEBUG

extern CGI_ENTRY cgi_entries[];
extern char *print200ok_mime;

//#include "include/httpd_config.h"		//for cfg_parse? struct

struct ARGS *args_ptr_local = NULL;
struct ARGS *args_ptr_global = NULL;

/*this is for CGI*/
char *get_arg(char *name, int flag){	//if flag = 0 -> global args are parsed, if 1 -> local args_ptr are parsed.
    struct ARGS *ptr;

    if(!name) return NULL;
    if(!flag) ptr = args_ptr_global;
    else ptr = args_ptr_local;	//must be preseted befor execution of this function.
    while(ptr && ptr->name && ptr->value){
	if(strcmp(name, ptr->name) == 0)
	    return ptr->value;
	ptr = ptr->next;
    }
    return NULL;
}

int handle_arg(int flag, char *input)
{
    struct ARGS **ptr;
    char *tmp;
    long long size;

    if(!input) return 0;	//nothing to parse
    if(!flag) ptr = &args_ptr_global;
    else ptr = &args_ptr_local;	//must be preseted befor execution of this function.
    while(*ptr) ptr=&((*ptr)->next);

    while (1){	//parse only booth '=' and '&' chars.
	if(! *ptr) *ptr = (struct ARGS *)malloc(sizeof(struct ARGS));
	if(*ptr == NULL){
	    printf("ERR: Allocate memory\n");
	    return 0;
	}
	(*ptr)->next = NULL;

	if((((*ptr)->name = w_strtok(&input, '=')) == NULL) || (((*ptr)->value = w_strtok(&input, '&')) == NULL )){
	    free(*ptr);
	    *ptr = NULL;
	    break;
	}	
	httpd_decode((*ptr)->value);
//**********
	tmp = parsestr1_((*ptr)->value, "??/[/*/]??/");	//if (args+i)->value == "file.inc?par=??_#par1??"
	if(tmp){						//if not find - erease par=??_#par1?? complete
	    tmp = get_var(&size, tmp);				//only writeable (!= 0) parameters will be linked!
	    if(tmp && size) {
		(*ptr)->value = tmp;
		//(args+i)->flag = 0;
	    } else {
//		free(*ptr);
		continue;
	    }
	}//(args+i)->flag = 1;
//**********
#ifdef DEBUG
//    printf("args[].name: %s args.value(after): %s, length=%d\n",(*ptr)->name,(*ptr)->value,strlen((*ptr)->value)+1);
    printf("args[].name: %s args.value length=%d\n",(*ptr)->name, strlen((*ptr)->value)+1);
#endif
	ptr = &((*ptr)->next);
    }
    return 1;
}


void free_arg_1(struct ARGS **ptr){
    if(!ptr || !*ptr) return;
    free_arg_1(&((*ptr)->next));
#ifdef DEBUG
    printf("free arg '%s'\n", (*ptr)->name);
#endif
    free(*ptr);	//at hier action
    *ptr = NULL;
}

void free_arg(int flag){
    if(!flag){
    free_arg_1(&args_ptr_global);
    }else{
    free_arg_1(&args_ptr_local);
    }
}


int parseargs(ARGUMENT *arg)
{
/*    int i, value_len, ret = 0;

    while (i < ARGS_MAX && args[i].name && args[i].value){

	while (arg->name != NULL){
	    if (strcmp(args[i].name, arg->name) == 0){
		ret = 1;	// something is matched
#ifdef DEBUG
    printf("p_arg->name: %s\n",arg->name);
    printf("p_arg->var(before): %s, length=%d\n",arg->var,strlen(arg->var));
#endif

		if (args[i].value == NULL) *arg->var = '\0';
		//else strncpy(arg->var, value, arg->len);
		else{
			value_len = strlen(args[i].value) + 1;
			strncpy(arg->var, args[i].value, MIN(value_len, (arg->len)-1));	//it seems to be OK, but is always p_arg->var[(p_arg->len)-1] == '\0'
		}

#ifdef DEBUG
    printf("p_arg->var(after): %s, length=%d\n",arg->var,strlen(arg->var));
#endif
		break;
	    }
	    arg++;
	}
	i++;
    }
    return ret;
*/return 0;
}

/* Given a www-form encoded string, restore the original: */
void httpd_decode(char *string)		/*had name unescape*/
{
    char *src = string, *dest = string;
    int digit1, digit2;

    while (*src!='\0'){
	switch (*src){
	    case '+':
		*dest = ' ';
		break;
	    case '%':
		src ++;
		if (*src >= '0' && *src <= '9') digit1 = *src - '0';
		else if (*src >= 'A' && *src <= 'F') digit1 = (*src - 'A') + 0xA;
		else if (*src >= 'a' && *src <= 'f') digit1 = (*src - 'a') + 0xA;
		else digit1 = 0;
		src ++;
		if (*src >= '0' && *src <= '9') digit2 = *src - '0';
		else if (*src >= 'A' && *src <= 'F') digit2 = (*src - 'A') + 0xA;
		else if (*src >= 'a' && *src <= 'f') digit2 = (*src - 'a') + 0xA;
		else digit2 = 0;
		*dest = (digit1 * 0x10) + digit2;
		break;
	    default:
		*dest = *src;
	}
	src ++;
	dest ++;
    }
    *dest = '\0';
    return;
}


/*find in *str character mark and replace them throw /0, set *str to next charakter and return pointer to begining of *str
 if charakter in string not found return NULL*/
char *w_strtok(char **str, char mark)
{

    char *begin = *str;

    if(*str)
    if(**str){
	while (**str != '\0'){
	    if (**str == mark){
		**str='\0';
		(*str)++;
		return begin;
	    }
	    (*str)++;
	}
    return begin;
    }
    return NULL;
}
/*
int check_digit( char *string )	//NEED TO BE USED!!!
{
    int i;

    if(!string) return 0;
    for (i=0; i<strlen(string); i++){
	if (isdigit( string[i] ))  return 0;
    }
    return 1;
}
*/
void delete_crlf( char *plag )	//find \n or \r and make end of string at this place!
{
    while ( *plag != '\0'){
	if ((*plag == '\n') || (*plag == '\r')){
	    *plag = '\0';
	    return;			
	}
	plag++;
    }
}


//mime - used 1 in httpd and 0 in include
void DoCGI(FILE *out, int mime, char *filename){

    CGI_ENTRY *p = cgi_entries;
//    char *data = arg;

#ifdef DEBUG
    printf("filename: %s\n",filename);
    printf("query: %s\n",arg);
#endif

    while((p->name[0] != ' ') && (p->name[0] != '\0')){
	if(strcmp(p->name, filename) == 0){
printf("%s %p\n",p->name, p->handler);
	    if(*filename != '_') fprintf(out, print200ok_mime, "text/html");
	    p->handler(out);
	    return;
	}
	p++;
    }
    if(get_cgi(out, filename)) return; //from copy_cgi.c
    if(mime){ 
	    fprintf(out, "HTTP/1.0 404 Not Found\n"
		 "Content-type: text/html\n\n"
		 "<html>\n"
		 "<head><title>404 Not Found</title></head>\n"
		 "<body><h1>404 Not Found</h1>\n"
		 "<p>The resource you have requested is not available."
		 "</p></body>\n"
		 "</html>\n");
	}else fprintf(out, "Not Found: %s\n", filename);
}
/*
<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>
<BODY><H2>404 Not Found</H2>
The requested object does not exist on this server.
The link you followed is either outdated, inaccurate,
or the server has been instructed not to let you have it.
</BODY></HTML>
*/
