/* /% C %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Source file input.c
 ************************************************************************
 * Description:
 *  Interactive interface frontend
 ************************************************************************
 * Copyright(c) 1995~1999  Masaharu Goto (MXJ02154@niftyserve.or.jp)
 *
 * Permission to use, copy, modify and distribute this software and its 
 * documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 ************************************************************************/

#include "common.h"

#ifndef G__OLDIMPLEMENTATION1078
int G__quiet=0;
#endif


#ifdef G__GNUREADLINE
extern char *readline G__P((char* prompt));
extern int add_history G__P((char* str));

/************************************************************
* G__input_history()
*
*  command history with file store
*************************************************************/
void G__input_history(state,string)
int *state;
char *string;
{
  char G__oneline[G__LONGLINE*2];
  char G__argbuf[G__LONGLINE*2];
  /* int  G__null_fgets=1; */
  char *arg[G__ONELINE];
  int argn;
#ifdef G__TMPFILE
  char tname[G__MAXFILENAME];
#else
  char tname[L_tmpnam+10];
#endif
  
  static char prevstring[G__ONELINE];
  static char histfile[G__ONELINE];
  char *homehist=".cint_hist";
  int line=0;
  
  FILE *fp,*tmp;
  
  if(*state==0) {
    /********************************************************
     * read history file and set command history
     ********************************************************/
    *state = 1;
    prevstring[0]='\0'; /* sprintf(prevstring,""); */
    sprintf(histfile,"%s/%s",getenv("HOME"),homehist);
    fp=fopen(histfile,"r");
    if(fp) {
      while(G__readline(fp,G__oneline,G__argbuf,&argn,arg)!=0){
	add_history(arg[0]);
	strcpy(prevstring,arg[0]);
	*state = (*state)+1;
      }
      fclose(fp);
    }
    else {
      fp=fopen(histfile,"w");
      fclose(fp);
    }
    return;
  }
  else if(strcmp(string,prevstring)!=0) {
    /********************************************************
     * append command history to file
     ********************************************************/
    add_history(string);
    fp=fopen(histfile,"a+");
    fprintf(fp,"%s\n",string);
    fclose(fp);
    *state = (*state)+1;
    strcpy(prevstring,string);
    if(*state<51) return;
  }
  else {
    return;
  }
  
  *state=1;
  
  /********************************************************
   * shrink history file (using tmpfile)
   ********************************************************/
  fp=fopen(histfile,"r");
  do {
    G__tmpnam(tname);
    tmp=fopen(tname,"w");
  } while((FILE*)NULL==tmp && G__setTMPDIR(tname));
  if(tmp&&fp) {
    while(G__readline(fp,G__oneline,G__argbuf,&argn,arg)!=0){
      ++line;
      if(line>30) fprintf(tmp,"%s\n",arg[0]);
    }
  }
  if(tmp) fclose(tmp);
  if(fp) fclose(fp);
  
  /* copy back to history file */
  fp=fopen(histfile,"w");
  tmp=fopen(tname,"r");
  if(tmp&&fp) {
    while(G__readline(tmp,G__oneline,G__argbuf,&argn,arg)!=0){
      fprintf(fp,"%s\n",arg[0]);
    }
  }
  if(tmp) fclose(tmp);
  if(fp) fclose(fp);
  remove(tname);
}
#endif

/************************************************************
* char *G__input(char *prompt)
*
*  command input frontend
*************************************************************/
char *G__input(prompt)
char *prompt;
{
  static char line[G__ONELINE];
  char *pchar;
#ifdef G__GNUREADLINE
  static int state=0;
#endif

#ifndef G__OLDIMPLEMENTATION1078
  if(G__quiet) prompt="";
#endif
  
  if(G__Xdumpreadline[0]) {
    pchar=G__xdumpinput(prompt);
    strcpy(line,pchar);
  }
  else {
    
#ifdef G__GNUREADLINE
    
    if(state==0) {
      G__init_readline();
      G__input_history(&state,"");
    }
    pchar=readline(prompt);
    while(pchar&&strlen(pchar)>G__ONELINE-5) {
      G__fprinterr("!!! User command too long !!!\n");
      pchar=readline(prompt);
    }
    if(pchar) strcpy(line,pchar);
    else line[0]=0;
#ifdef G__MEMTEST
    /* memory parity checker can not handle the pointer malloced in readlie */
    G__DUMMY_Free((void*)pchar);
#else
    free(pchar);
#endif
    if(line[0]!='\0') {
      G__input_history(&state,line);
    }
    
#else /* of G__GNUREADLINE */
    
    if(G__Xdumpreadline[0]) {
      pchar=G__xdumpinput(prompt);
      strcpy(line,pchar);
    }
    else {
      fprintf(G__stdout,"%s",prompt);
      /* scanf("%s",line); */
      fgets(line,G__ONELINE-5,G__stdin);
    }
    
#endif /* of G__GNUREADLINE */
    
    if(G__in_pause) { /*G__in_pause is used for G__input and G__valuemonitor*/
      /***********************************************
       * For ATEC replacement
       * If execute readline dump file mode, get
       * debug command from the readline dump file
       * else get command from stdin.
       ***********************************************/
      switch(line[0]) {
      case 'Z':
      case 'Y':
      case 'z':
      case 'y':
	break;
      case 'N':
      case 'n':
	if(G__dumpreadline[0]) {
	  line[0]='<';
	  G__dumpinput(line);
	}
	break;
      case EOF:
	line[0]='Q';
	break;
      default:
	G__dumpinput(line);
	break;
      }
    }
    else {
      G__dumpinput(line);
    }
  }
#ifndef G__OLDIMPLEMENTATION447
  clearerr(G__sin);
#endif
  return(line);
}


/****************************************************************
*  Support libraries for GNU readline completion
*
*  Inter face to readline completion
*
*   Copyright of source code bolow and GNU readline library 
*  belongs to Free Software Foundation. Refer to GNU GENERAL 
*  PUBLIC LICENSE and GNU LIBRARY GENERAL PUBLIC LICENSE.
* 
****************************************************************/

#ifdef G__GNUREADLINE
/* Tell the GNU readline library how to complete. We want to try to
* complete on compiled function names (if this is the first word in 
* the line, or on filenames if not.) */

char **completion_matches();

char **G__funcname_completion(text,start,end)
char *text;
int start,end;
{
  char **matchs;
  matchs = (char **)NULL;
  /* If this word is at the start of the line, then it is a function name */
  matchs = completion_matches(text,G__search_next_member);
  return(matchs);
}
#endif

/* Attempt to complete on the contents of TEXT. START and END show the
* region of TEXT that contaions the ford to complete. We can use the
* entire line in case we want to do some simple parsing. Return the
* array of matches , or NULL if there aren't any */



#ifdef G__GNUREADLINE
extern char **(*rl_attempted_completion_function)();
extern char *rl_basic_word_break_characters;

int G__init_readline()
{
  /* Tell the completer that we want a crack first */
  rl_attempted_completion_function = G__funcname_completion;
  rl_basic_word_break_characters = " \t\n\"\\'`@$=;|&{(+*/%^~!,";
  return(0);
}
#endif

/****************************************************************
*  End of Support libraries for GNU readline completion
****************************************************************/



char *G__strrstr(string1,string2)
char *string1,*string2;
{
  char *p=NULL,*s,*result=NULL;
  s=string1;
  while((p=strstr(s,string2))) {
    result=p;
    s=p+1;
  }
  return(result);
}



/*
 * Local Variables:
 * c-tab-always-indent:nil
 * c-indent-level:2
 * c-continued-statement-offset:2
 * c-brace-offset:-2
 * c-brace-imaginary-offset:0
 * c-argdecl-indent:0
 * c-label-offset:-2
 * compile-command:"make -k"
 * End:
 */
