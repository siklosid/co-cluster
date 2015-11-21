#ifndef PARAMS_H
#define PARAMS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "Utils/log.h"

#ifndef log_err
#define log_err(fmt,arg...) {fprintf(stderr,fmt"\n", ##arg);}
#endif

/*#ifdef HAVE_STRING
#include <string>
	using namespace std;
	typedef string params_filename;
#else
	typedef char params_filename[PATH_MAX];
#endif
*/
typedef char params_filename[PATH_MAX];

#define PAR_MAXOPTIONS	1024 
#define PAR_MAXVARS	128
#define PAR_ARRAY_SEPARATOR ','	

#define PAR_MAX_SPACES	256
//#ifndef PARFILE
//#error "PARFILE not defined"
//#endif

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long ProgMask;


class Params_Base{
public:
	Params_Base();
	Params_Base(Params_Base &pb);
	virtual ~Params_Base();
	
	void Fetch(const char *conffn,int argc, char *const argv[],
			ProgMask prog=0);
	void Fetch(int argc, char *const argv[],
			ProgMask prog=0);//default configfile
	void Fetch_conf(const char *conffn,
			ProgMask prog=0);//from configfile
	void Fetch_conf(ProgMask prog=0);//from default configfile
	void Fetch_comm(int argc,char *const argv[],ProgMask prog=0);// from argv
	char *m__conffn;
	int m__argc;
	char *const *m__argv;
//	char *const *m__envp;
	
	char *m__env_path; // TODO: ezt nem igy kellen
protected:
	char *m__keywords[PAR_MAXOPTIONS];
	struct option m__options[PAR_MAXOPTIONS];
	int m__flags[PAR_MAXOPTIONS];
	char m__shortoptions[2*PAR_MAXOPTIONS+1];
	char *m__values[PAR_MAXOPTIONS];
	int m__poskw[PAR_MAXOPTIONS];
	int m__arrsizes[PAR_MAXOPTIONS];
	virtual	void initstruct();
	virtual void copytofields(){};
	void deletestruct();
	void additem(char *keyword,char ch,int arrsize);
	void addfileitem(char *keyword,int pos);
	void addswitchitem(char *keyword,char ch);
	void addhelpitem(char *keyword,char ch);
	void addconfitem(char *keyword, int pos,char *path,char *def);
	void readcommandline();
	void readconfigfile();
	bool readconfigfile_arrayitem(char *kw_begin,char *val_begin,
                    int lineno,int nosub);
	
	int m__nitems;
	virtual void printhelp(){};
	int m__helpind;
	int m__conffnind;
	ProgMask m__prog;

	int str_to_int(const char *s){return atoi(s);}
	uint str_to_uint(const char *s){return atoi(s);}
	uint32_t str_to_uint32_t(const char *s){return atoi(s); }
	long str_to_long(const char *s){return atol(s);}
	ulong str_to_ulong(const char *s){return atol(s);}
	float str_to_float(const char *s){return static_cast<float>(atof(s));}
	double str_to_double(const char *s){return atof(s);}
	char str_to_char(const char *s){return s[0];}
/*
#ifdef HAVE_STRING
	string str_to_string(char *s){string ss=s;return ss;};
#endif
*/
	int str_to_switch(char *s);


	char *replace_env_var(char *s,char *const *envp);
	void setvar(char *,char *);
	char *getvar(char *);
	char *getnextarrayitem(char **p);
	void delvars();
	int m__nvars;
	char *m__vars[PAR_MAXVARS];
	char *m__varvalues[PAR_MAXVARS];

	int m__cflineno;
	
	void tolower(char *s);
	char *skipwspace(char *p);
	char *skipwspaceoreq(char *p,int *waseq);
	char *findwspace(char *p);
	char *findwspaceoreq(char *p);
};

#endif //PARAMS_H

#ifndef PARAMS_CC



class
#define CLASS(x) x
#define CONFIGFILE(path,def,keyword,ch,help)
#define P(type,var,def,keyword,ch,help,prog)	
#define PA(type,size,var,def,keyword,ch,help,prog)	
#define S(len,var,def,keyword,ch,help,prog)	
#define FN(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyword,ch,help,prog)	
#define SA(len,size,var,def,keyword,ch,help,prog)	
#define F(len,var,def,keyword,pos,help)	
#define SW(var,def,keyword,ch,help,prog)		
#define HELP(keyword,ch,begin,end)
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
:public Params_Base{ public:

#define CLASS(x) x():Params_Base(){};
#define CONFIGFILE(path,def,keyword,ch,help)
#define P(type,var,def,keyword,ch,help,prog)	
#define PA(type,size,var,def,keyword,ch,help,prog)	
#define S(len,var,def,keyword,ch,help,prog)	
#define FN(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyword,ch,help,prog)	
#define SA(len,size,var,def,keyword,ch,help,prog)	
#define F(len,var,def,keyword,pos,help)	
#define SW(var,def,keyword,ch,help,prog)		
#define HELP(keyword,ch,begin,end)
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP

#define CLASS(x)
#define CONFIGFILE(path,def,keyword,ch,help) // no public field for configfile 
#define P(type,var,def,keyword,ch,help,prog)	type var;	\
					int sw_##var;
#define PA(type,size,var,def,keyword,ch,help,prog) type var[size];\
					int sw_##var;   \
    					int size_##var;
#define S(len,var,def,keyword,ch,help,prog)	char var[len];	\
					int sw_##var;
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyw,ch,help,prog)   S(len,var,def,keyw,ch,help,prog)	
#define SA(len,size,var,def,keyword,ch,help,prog) char var[size][len];\
					int sw_##var;   \
                    			int size_##var;
#define F(len,var,def,keyword,pos,help)	char var[len];	\
					int sw_##var;
#define SW(var,def,keyword,ch,help,prog)	int var;
#define HELP(keyword,ch,begin,end)
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
private:
	void initstruct(){
		Params_Base::initstruct();
#define CLASS(x) 
#define CONFIGFILE(path,def,keyword,ch,help) addconfitem((char*)keyword, ch, (char*)path, (char*)def);
#define P(type,var,def,keyword,ch,help,prog)	additem((char*)keyword,ch,0);
#define PA(type,size,var,def,keyword,ch,help,prog) additem((char*)keyword,ch,size);
#define S(len,var,def,keyword,ch,help,prog)	additem((char*)keyword,ch,0);
#define PWD(len,var,def,keyw,ch,help,prog)   S(len,var,def,keyw,ch,help,prog)	
#define SA(len,size,var,def,keyword,ch,help,prog)additem((char*)keyword,ch,size);
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	
#define F(len,var,def,keyword,pos,help)	addfileitem((char*)keyword,pos);
#define SW(var,def,keyword,ch,help,prog)		addswitchitem((char*)keyword,ch);
#define HELP(keyword,ch,begin,end)	addhelpitem((char*)keyword,ch);
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef SA
#undef F
#undef SW
#undef HELP
};

	void copytofields(){int l__i=0;
#define CLASS(x) 
#define CONFIGFILE(path,def,keyword,ch,help)l__i++;// no public field for configfile 
// TODO: copy the default value into m__values[l__i] in P and PA
#define P(type,var,def,keyword,ch,help,prog)var=def;sw_##var=0;		\
					if(m__values[l__i]!=NULL){	\
					var=str_to_##type(m__values[l__i]); \
					sw_##var=1;			\
					}l__i++;
#define PA(type,size,var,def,keyword,ch,help,prog)sw_##var=0;           \
					{char *p,*q=NULL;size_##var=size; \
					if(m__values[l__i]!=NULL){      \
					    p=m__values[l__i];       \
					    sw_##var=1;			\
					}else{q=p=strdup(def);}		\
					int i=0;                        \
					while(p!=NULL){                 \
					   if(i>=size){log_err("too many items, size of %s is %d",keyword,size);exit(1);}\
					   var[i++]=str_to_##type(     \
						getnextarrayitem(&p));} \
					if(i!=size){log_err("size of %s is %d not %d",keyword,size,i);exit(1);}\
					l__i++;if(q!=NULL)free(q);};
#define S(len,var,def,keyword,ch,help,prog)strncpy(var,def,len);sw_##var=0; 	\
					if(m__values[l__i]!=NULL){	\
					strncpy(var,m__values[l__i],len); \
					sw_##var=1;			\
					}else{ char *def2=strdup(def);	\
					char *defr=replace_env_var(def2,\
						NULL/*m__envp*/);		\
					if(defr!=NULL){			\
					m__values[l__i]=strdup(defr);	\
					strncpy(var,defr,len);free(defr);}}\
					l__i++;
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	\
                change_slashes(var); \
                change_slashes(m__values[l__i-1]);
#define PWD(len,var,def,keyw,ch,help,prog)   S(len,var,def,keyw,ch,help,prog)	
#define SA(len,size,var,def,keyword,ch,help,prog)sw_##var=0;            \
					{char *def2=strdup(def);size_##var=size;\
					char *defr=replace_env_var(def2,\
						NULL);                  \
					char *p,*q=NULL;                \
					if(m__values[l__i]!=NULL){      \
					    p=m__values[l__i];       \
					    sw_##var=1;			\
					}else{q=p=strdup(defr);		\
					m__values[l__i]=strdup(defr);}	\
					int i=0;                        \
					while(p!=NULL){                 \
					   if(i>=size){log_err("too many items, size of %s is %d",keyword,size);exit(1);}\
					   strncpy(var[i++],            \
						 getnextarrayitem(&p),len);}\
					if(i!=size){log_err("size of %s is %d not %d",keyword,size,i);exit(1);}\
					sw_##var=1;			\
					l__i++;free(defr);if(q!=NULL)free(q);};
		
#define F(len,var,def,keyword,pos,help)strncpy(var,def,len);sw_##var=0; \
					if(m__values[l__i]!=NULL){	\
					strncpy(var,m__values[l__i],len); \
					sw_##var=1;			\
					}else{ char *def2=strdup(def);	\
					char *defr=replace_env_var(def2,\
						NULL/*m__envp*/);		\
					if(defr!=NULL){			\
					m__values[l__i]=strdup(defr);	\
					strncpy(var,defr,len);free(defr);}}\
					l__i++;
#define SW(var,def,keyword,ch,help,prog)var=def;			\
					if(m__values[l__i]!=NULL){	\
					var=str_to_switch(m__values[l__i]); \
					}l__i++;
#define HELP(keyword,ch,begin,end)	l__i++;
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
};

	void printhelp(){
		char *help[PAR_MAXOPTIONS];
		char *help_kw[PAR_MAXOPTIONS];
		char help_ch[PAR_MAXOPTIONS];
		char help_prog[PAR_MAXOPTIONS];
		char *help_begin;
		char *help_end;
		char *help_usage[PAR_MAXOPTIONS]; 
		int i,j,k,longest,spacelen;
		char spaces[PAR_MAX_SPACES];

		i=j=0;
#define CLASS(x) 
#define CONFIGFILE(path,def,keyword,cha,hel) \
					help[i]=strdup(hel);            \
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=0;i++;
#define P(type,var,def,keyword,cha,hel,prog)help[i]=strdup(hel);	\
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=prog;i++;
#define PA(type,size,var,def,keyword,cha,hel,prog)help[i]=strdup(hel);  \
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=prog;i++;
#define S(len,var,def,keyword,cha,hel,prog)help[i]=strdup(hel);		\
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=prog;i++;
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyw,ch,help,prog)   S(len,var,def,keyw,ch,help,prog)	
#define SA(len,size,var,def,keyword,cha,hel,prog)help[i]=strdup(hel);   \
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=prog;i++;
#define F(len,var,def,keyword,pos,hel)	help_usage[j]=strdup(hel);j++;
#define SW(var,def,keyword,cha,hel,prog)help[i]=strdup(hel);		\
					help_kw[i]=strdup(keyword);	\
					help_ch[i]=cha;			\
					help_prog[i]=prog;i++;
#define HELP(keyword,ch,begina,enda)	help_begin=strdup(begina);	\
					help_end=strdup(enda);
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
		
		fprintf(stderr,"Usage: %s [OPTIONS]...",m__argv[0]);
		for(k=0;k<j;k++){
			fprintf(stderr," %s",help_usage[k]);
			free(help_usage[k]);
		}
		fprintf(stderr,"\n");
		fprintf(stderr,"%s\n",help_begin);
		free(help_begin);

		memset(spaces,' ',PAR_MAX_SPACES);
		longest=0;for(k=0;k<i;k++){
			if((signed)strlen(help_kw[k])>longest)
				longest=(signed)strlen(help_kw[k]);
		}
		for(k=0;k<i;k++){
			//printf("%d %d\n",m__prog,help_prog[k]);
			spacelen=longest+1-static_cast<int>(strlen(help_kw[k]));
			if (spacelen<0)spacelen=0;
			if (spacelen>=PAR_MAX_SPACES)spacelen=PAR_MAX_SPACES-1;
			spaces[spacelen]='\0';
			if((m__prog&help_prog[k])!=0)
			{
			if(help_ch[k]!=0){
				fprintf(stderr,"-%c, --%s%s%s\n",
					help_ch[k],help_kw[k],spaces,help[k]);
			}else{
				fprintf(stderr,"    --%s%s%s\n",
					help_kw[k],spaces,help[k]);
			}
			}
			spaces[spacelen]=' ';
			free(help[k]);
			free(help_kw[k]);
		}
		fprintf(stderr,"%s\n",help_end);
		free(help_end);
	};

public:	void print(FILE *f,const char *prefix,const char *parname=NULL){
#define IFP(var) if((parname==NULL)||(strcmp(parname,keyword)==0));
#define CLASS(x) 
#define CONFIGFILE(path,def,keyword,ch,help)
#define P(type,var,def,keyword,ch,help,prog)		\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
			fprintf(f,"%s%s=%d\n",prefix,keyword,(int)var); }
#define PA(type,size,var,def,keyword,ch,help,prog)\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
			fprintf(f,"%s%s=\"",prefix,keyword);	\
			for(int i=0;i<size;i++)			\
				fprintf(f,"%d ",(int)var[i]);	\
			fprintf(f,"\"\n");			\
		}
#define S(len,var,def,keyword,ch,help,prog)			\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
			fprintf(f,"%s%s=\'%s\'\n",prefix,keyword,var);}	
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyw,ch,help,prog) // do not print this
#define SA(len,size,var,def,keyword,ch,help,prog)\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
			fprintf(f,"%s%s=\"",prefix,keyword);	\
			for(int i=0;i<size;i++)			\
				fprintf(f,"%s ",var[i]);	\
			fprintf(f,"\"\n");	}
#define F(len,var,def,keyword,pos,help)				\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
			fprintf(f,"%s%s=\'%s\'\n", prefix, keyword, var);}
#define SW(var,def,keyword,ch,help,prog)	\
		if((parname==NULL)||(strcmp(parname,keyword)==0)){ \
		  fprintf(f,"%s%s=\'%s\'\n",prefix,keyword,(var?"yes":"no"));}
#define HELP(keyword,ch,begin,end)	
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
	};

public:	void reconstruct(FILE *f){
#define CLASS(x) 
#define CONFIGFILE(path,def,keyword,ch,help)
// TODO: CSAK int-ekre mukodik jol a P(...)!
#define P(type,var,def,keyword,ch,help,prog) {	\
		fprintf(f,"%s %d\n",keyword,(int)var); } 
#define PA(type,size,var,def,keyword,ch,help,prog) {\
		for(int i=0;i<size;i++){			\
			fprintf(f,"%s[%d] %d\n",keyword,i,(int)var[i]);	\
		}}
#define S(len,var,def,keyword,ch,help,prog) {			\
		fprintf(f,"%s \'%s\'\n",keyword,var);}	
#define FN(len,var,def,keyword,ch,help,prog) \
                S(len,var,def,keyword,ch,help,prog)	
#define PWD(len,var,def,keyw,ch,help,prog) // do not print this
#define SA(len,size,var,def,keyword,ch,help,prog) {\
		for(int i=0;i<size;i++){			\
			fprintf(f,"%s[%d] \'%s\'\n",keyword,i,var[i]);	\
		}}
#define F(len,var,def,keyword,pos,help) {				\
		fprintf(f,"%s \'%s\'\n",keyword,var);}	
#define SW(var,def,keyword,ch,help,prog) {	\
		  fprintf(f,"%s \'%s\'\n",keyword,(var?"yes":"no"));}
#define HELP(keyword,ch,begin,end)	
#include PARFILE
#undef CLASS
#undef CONFIGFILE
#undef P
#undef PA
#undef S
#undef FN
#undef PWD
#undef SA
#undef F
#undef SW
#undef HELP
	};
protected:
};

// Allow more than one Param class in a file
#undef PARFILE

#endif //PARAMS_CC

