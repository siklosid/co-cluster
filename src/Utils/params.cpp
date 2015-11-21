#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "getopt.h"

#define PARAMS_CC
#include "params.h"

using std::string;
/*#include "log.h"

#ifndef log_err
#define log_err(fmt,arg...) {fprintf(stderr,fmt"\n", ##arg);}
#endif*/

static void change_slashes(char *fn) {
#ifdef WIN32
    const char src = '/';
    const char dest = '\\';
#else
    const char src = '\\';
    const char dest = '/';
#endif
    char *p = strchr(fn, src);
    while (p!=NULL) {
        *p = dest;
        p = strchr(p+1,src);
    }
#ifdef WIN32
	// Windows's stat does not like \ at the end of directory names
	if (strlen(fn) != 0) {
		p = fn + strlen(fn) - 1;
		if (*p == '\\')
			*p = '\0';
	}
#endif
}


Params_Base::Params_Base(){
   m__argc=0;
   m__argv=NULL;
   //m__envp=NULL;
   m__nitems=0;
   m__helpind=-1;
   m__conffnind=-1;
   m__conffn=NULL;
   for(int i=0;i<PAR_MAXOPTIONS;i++){
      m__keywords[i]=NULL;
      //TODO options init
      m__flags[i]=0;
      m__values[i]=NULL;
      m__poskw[i]=-1;
      m__arrsizes[i]=0;
   }
   m__nvars=0;
   for(int i=0;i<PAR_MAXVARS;i++){
      m__vars[i]=NULL;
      m__varvalues[i]=NULL;
   }
}

Params_Base::Params_Base(Params_Base &pb){
   m__argc=pb.m__argc;
   m__argv=pb.m__argv;
   //	m__envp=pb.m__envp;
   m__prog=0;
   m__nitems=pb.m__nitems;
   m__helpind=pb.m__helpind;
   m__conffnind=pb.m__conffnind;
   if(pb.m__conffn!=NULL)
      m__conffn=strdup(pb.m__conffn);
   else m__conffn=NULL;
   strcpy(m__shortoptions,pb.m__shortoptions);
   for(int i=0;i<PAR_MAXOPTIONS;i++){
      m__keywords[i]=NULL;
      //TODO options init
      m__flags[i]=0;
      m__values[i]=NULL;
      m__poskw[i]=0;
      m__arrsizes[i]=0;
   }
   m__nvars=0;
   for(int i=0;i<PAR_MAXVARS;i++){
      m__vars[i]=NULL;
      m__varvalues[i]=NULL;
   }
   for(int i=0;i<m__nitems;i++){
      if(pb.m__keywords[i]!=NULL)
         m__keywords[i]=strdup(pb.m__keywords[i]);
      else m__keywords[i]=NULL;
      m__options[i]=pb.m__options[i];
      m__flags[i]=pb.m__flags[i];
      if(pb.m__values[i]!=NULL)
         m__values[i]=strdup(pb.m__values[i]);
      else m__values[i]=NULL;
      m__poskw[i]=pb.m__poskw[i];
   }
   copytofields();
}

Params_Base::~Params_Base(){
   deletestruct();
   delvars();
   if(m__conffn!=NULL)free(m__conffn);
   m__conffn = NULL;
}

void Params_Base::Fetch(const char *conffn,int argc, char *const argv[], ProgMask prog){
   if(m__conffn!=NULL)free(m__conffn);
   if(conffn!=NULL)m__conffn=strdup(conffn);
   else m__conffn=NULL;
   Fetch(argc,argv,prog);
}
void Params_Base::Fetch(int argc, char *const argv[], ProgMask prog){
   //	if(m__conffn!=NULL)free(m__conffn);
   //	m__conffn=strdup(conffn);
   m__argc=argc;
   m__argv=argv;
   //	m__envp=envp;
   m__prog=prog;
   initstruct();
   if(m__conffn==NULL){
      readcommandline(); // try get configfile from commandline
      if((m__conffnind!=-1)&&(m__values[m__conffnind]!=NULL))
         m__conffn=strdup(m__values[m__conffnind]);
   }


   if(m__conffn!=NULL)
      readconfigfile();
   else log_err("No configfile defined");
   readcommandline();
   if((m__helpind!=-1)&&(m__values[m__helpind]!=NULL)){
      deletestruct();
      printhelp();
      if(m__conffn!=NULL)free(m__conffn);
      m__conffn = NULL;
      exit(0);
   }
   copytofields();
   deletestruct();
}

void Params_Base::Fetch_conf( const char *conffn, ProgMask prog){
   if(m__conffn!=NULL)free(m__conffn);
   if(conffn!=NULL)m__conffn=strdup(conffn);
   else m__conffn=NULL;
   Fetch_conf(prog);
}
void Params_Base::Fetch_conf( ProgMask prog){
   //	m__envp=envp;
   m__prog=prog;
   //	if(m__conffn!=NULL)free(m__conffn);
   //	m__conffn=strdup(conffn);
   initstruct();
   if(m__conffn==NULL){
      readcommandline(); // try get configfile from commandline
      if((m__conffnind!=-1)&&(m__values[m__conffnind]!=NULL))
         m__conffn=strdup(m__values[m__conffnind]);
   }
   if(m__conffn!=NULL)
      readconfigfile();
   else log_err("No configfile defined");

   copytofields();
   deletestruct();
}

void Params_Base::Fetch_comm(int argc, char *const argv[],ProgMask prog){
   m__argc=argc;
   m__argv=argv;
   m__prog=prog;
   initstruct();
   readcommandline();
   if((m__helpind!=-1)&&(m__values[m__helpind]!=NULL)){
      deletestruct();
      printhelp();
      if(m__conffn!=NULL)free(m__conffn);
      exit(0);
   }
   copytofields();
   deletestruct();
}

void Params_Base::initstruct(){
   for(int i=0;i<PAR_MAXOPTIONS;i++){
      m__keywords[i]=NULL;
      m__options[i].name=NULL;
      m__values[i]=NULL;
      m__flags[i]=0;
      m__shortoptions[i]='\0';
      m__poskw[i]=-1;
      m__arrsizes[i]=0;
   }
   m__nitems=0;
   m__helpind=-1;
   m__conffnind=-1;
}
void Params_Base::deletestruct(){
   for(int i=0;i<PAR_MAXOPTIONS;i++){
      if(m__keywords[i]!=NULL) free( m__keywords[i]);
      if(m__values[i]!=NULL) free( m__values[i]);
      m__keywords[i]=NULL;
      m__options[i].name=NULL;
      m__values[i]=NULL;
      m__flags[i]=0;
      m__shortoptions[i]='\0';
      m__poskw[i]=-1;
      m__arrsizes[i]=0;
   }
   m__nitems=0;
   m__helpind=-1;
   m__conffnind=-1;
}


// XXX: next five methods are very similar, but not the same!
void Params_Base::additem(char *keyword, char ch,int arr_size){
   if(m__nitems>=PAR_MAXOPTIONS){
      log_err("More than %d parameter",PAR_MAXOPTIONS);
      exit(1);
   }
   m__keywords[m__nitems]=strdup(keyword);
   sprintf(m__shortoptions,"%s%c:",m__shortoptions,ch);
   m__options[m__nitems].name=m__keywords[m__nitems];
   m__options[m__nitems].has_arg=1;
   m__options[m__nitems].flag=&(m__flags[m__nitems]);
   m__options[m__nitems].val=1;

   m__flags[m__nitems]=0;

   m__arrsizes[m__nitems]=arr_size;

   m__nitems++;
}

void Params_Base::addfileitem(char *keyword, int pos){
   if(m__nitems>=PAR_MAXOPTIONS){
      log_err("More than %d parameter",PAR_MAXOPTIONS);
      exit(1);
   }
   m__keywords[m__nitems]=strdup(keyword);
   sprintf(m__shortoptions,"%s%c:",m__shortoptions,255); //dummy shortopt
   m__options[m__nitems].name=m__keywords[m__nitems];
   m__options[m__nitems].has_arg=1;
   m__options[m__nitems].flag=&(m__flags[m__nitems]);
   m__options[m__nitems].val=1;

   m__flags[m__nitems]=0;
   m__poskw[pos]=m__nitems;

   m__arrsizes[m__nitems]=0;

   m__nitems++;
}


void Params_Base::addswitchitem(char *keyword, char ch){
   if(m__nitems>=PAR_MAXOPTIONS){
      log_err("More than %d parameter",PAR_MAXOPTIONS);
      exit(1);
   }
   m__keywords[m__nitems]=strdup(keyword);
   sprintf(m__shortoptions,"%s%c:",m__shortoptions,ch);
   m__options[m__nitems].name=m__keywords[m__nitems];
   //	m__options[m__nitems].has_arg=2; //optional argument
   m__options[m__nitems].has_arg=1; //required argument
   m__options[m__nitems].flag=&(m__flags[m__nitems]);
   m__options[m__nitems].val=1;

   m__flags[m__nitems]=0;

   m__arrsizes[m__nitems]=0;

   m__nitems++;
}
void Params_Base::addhelpitem(char *keyword, char ch){
   if(m__nitems>=PAR_MAXOPTIONS){
      log_err("More than %d parameter",PAR_MAXOPTIONS);
      exit(1);
   }
   m__keywords[m__nitems]=strdup(keyword);
   sprintf(m__shortoptions,"%s%c",m__shortoptions,ch);
   m__options[m__nitems].name=m__keywords[m__nitems];
   m__options[m__nitems].has_arg=0;
   m__options[m__nitems].flag=&(m__flags[m__nitems]);
   m__options[m__nitems].val=1;

   m__flags[m__nitems]=0;

   m__arrsizes[m__nitems]=0;

   m__helpind=m__nitems;
   m__nitems++;
}
void Params_Base::addconfitem(char *keyword, int ch,char *path,char *def){
   if(m__nitems>=PAR_MAXOPTIONS){
      log_err("More than %d parameter",PAR_MAXOPTIONS);
      exit(1);
   }
   m__keywords[m__nitems]=strdup(keyword);
   sprintf(m__shortoptions,"%s%c:",m__shortoptions,ch);
   m__options[m__nitems].name=m__keywords[m__nitems];
   m__options[m__nitems].has_arg=1;
   m__options[m__nitems].flag=&(m__flags[m__nitems]);
   m__options[m__nitems].val=1;

   m__flags[m__nitems]=0;

   m__arrsizes[m__nitems]=0;

   // TODO: a tobbi default erteket is itt kellene
   if(m__values[m__nitems]!=NULL)free(m__values[m__nitems]);
   // NULL pointer check added by ST
   m__env_path = getenv(path);
   if( m__env_path == NULL ) // not set
   {
     m__env_path = (char *)"./"; // set it to local dir
   }
   m__values[m__nitems]=(char*)malloc(strlen(m__env_path)+strlen(def)+2);
   if((strlen(m__env_path)>0)&&(m__env_path[strlen(m__env_path)-1]=='/'))
   {
      sprintf(m__values[m__nitems],"%s%s",m__env_path,def);
      change_slashes(m__values[m__nitems]);
   }else
   {
      sprintf(m__values[m__nitems],"%s/%s",m__env_path,def);
      change_slashes(m__values[m__nitems]);
   }
   //fprintf(stderr,	"%s=%s\n",path,env_path);
   m__conffnind=m__nitems;
   m__nitems++;
}


void Params_Base::readcommandline(){
   //#ifndef WIN32
   int retval=0;
   int ind=-1;
   optind=1;
   int find=1;
   while (retval!=-1){
      while((optind<m__argc)&&(m__argv[optind][0]!='-')){
         if(find>=PAR_MAXOPTIONS){
            log_err("Too many parameter");
            deletestruct();
            if(m__conffn!=NULL)free(m__conffn);
            exit(1);
         }
         ind=m__poskw[find];
         find++;
         if(ind==-1){
            //TODO:log_err("Too many filename parameter");
         }else{
            if(m__values[ind]!=NULL)free(m__values[ind]);
            m__values[ind]=strdup(m__argv[optind]);
         }
         optind++;
      }
      retval=getopt_long_only(m__argc,m__argv,m__shortoptions,
            m__options,&ind);
      if(retval==-1)break;
      if(retval==0){
         if(ind>=PAR_MAXOPTIONS){
            log_err("Too many options");
            deletestruct();
            if(m__conffn!=NULL)free(m__conffn);
            exit(1);}
         if(m__values[ind]!=NULL)free(m__values[ind]);
         if((m__options[ind].has_arg==1)||
               ((m__options[ind].has_arg==2)&&(optarg!=NULL)))
            m__values[ind]=strdup(optarg);
         else
            m__values[ind]=strdup("yes");
         continue;
      }
      if(retval!='?'){
         ind=0;
         unsigned int sp=0;
         while (sp<strlen(m__shortoptions) &&
               (m__shortoptions[sp]!=retval)){
            ind++;
            sp++;
            if(m__shortoptions[sp]==':')
               sp++;
         }
         if(m__shortoptions[sp]==retval){
            if(m__values[ind]!=NULL)free(m__values[ind]);
            if((m__shortoptions[sp+1]==':')&&(optarg!=NULL))
               m__values[ind]=strdup(optarg);
            else
               m__values[ind]=strdup("yes");
            continue;
         }
         else{ // it is impossible
            deletestruct();
            if(m__conffn!=NULL)free(m__conffn);
            exit(1);
         }
      }
      if(m__helpind!=-1)
         log_err("Try '%s --%s' for more information.",
               m__argv[0],m__keywords[m__helpind]);
      deletestruct();
      if(m__conffn!=NULL)free(m__conffn);
      m__conffn=NULL;
      exit(1);
   }
   //#endif  // WIN32
}

void Params_Base::readconfigfile(){
   char buf[1000];
   int nosub=0;
   FILE *f=fopen(m__conffn,"r");
   m__cflineno=0;
   bool error=false;

   //fprintf(stderr,"configfile: %s\n",m__conffn);

   if(f==NULL){
      log_err("Warning: configfile not found: %s.",m__conffn);
      return;
   }
   while(fgets(buf,1000,f)!=NULL){
      m__cflineno++;
      nosub=0;
      char *kw_begin,*kw_end,*val_begin,*val_end;
      int waseq=0;
      kw_begin=skipwspace(buf);
      if( ((*kw_begin)=='\0') || ((*kw_begin)=='#') )continue;
      kw_end=findwspaceoreq(kw_begin);
      val_begin=skipwspaceoreq(kw_end,&waseq);
      if((val_begin[0]!='"')&&(val_begin[0]!='\''))
         val_end=findwspace(val_begin);
      else{
         if(val_begin[0]=='\'')nosub=1;
         val_begin++;
         val_end=strpbrk(val_begin,"\"\'");
         if(val_end==NULL)val_end=strchr(val_begin,'\0');
      }
      (*kw_end)='\0';
      (*val_end)='\0';
      if(waseq){
         //	fprintf(stderr,"setvar( %s , %s )\n",kw_begin,val_begin);
         setvar(kw_begin,val_begin);
         continue;
      }
      if(strchr(kw_begin,'[')!=NULL){
         readconfigfile_arrayitem(kw_begin,val_begin,m__cflineno,nosub);
         continue;
      }
      int ind=0;
      while((ind<m__nitems) &&
            (strcmp(kw_begin,m__keywords[ind])!=0))
         ind++;
      if((ind<m__nitems)&&(strcmp(kw_begin,m__keywords[ind])==0)){
         //TODO: ez igy nem engedi felulirni a configfajlt a commandline-nal!
         //			if(m__values[ind]!=NULL){
         //				log_err("Multiple definition of %s line: %d",
         //						m__keywords[ind],m__cflineno);
         //				error=true;break;
         //			}
         m__values[ind]=strdup(val_begin);
         //XXX:memory leak
         //	if(m__envp!=NULL){
         if(!nosub){
            m__values[ind]=
                  replace_env_var(m__values[ind],NULL/*m__envp*/);
         }
         //	}
         continue;
      }
      log_err("Invalid keyword in configfile line %d: %s",
            m__cflineno,kw_begin);
      error=true;
      break;
   }
   if(error){
      fclose(f);
      deletestruct();
      if(m__conffn!=NULL)free(m__conffn);
      m__conffn=NULL;
      exit(1);
   }
   delvars();
   fclose(f);
   m__cflineno=0;
}

bool Params_Base::readconfigfile_arrayitem(char *kw_begin,char *val_begin,
      int lineno,int nosub){
   char *br_begin=strchr(kw_begin,'[');
   if(br_begin==NULL){
      log_err("no '[' in arrayitem in configfile line %d",lineno);
      return false;
   }
   char *br_end=strchr(kw_begin,']');
   if(br_end==NULL){
      log_err("no 'i]' in arrayitem in configfile line %d",lineno);
      return false;
   }
   int arr_index=atoi(br_begin+1);
   (*br_begin)='\0';
   int ind=0;
   while((ind<m__nitems) &&
         (strcmp(kw_begin,m__keywords[ind])!=0)){
      ind++;
   }
   if((ind<m__nitems)&&(strcmp(kw_begin,m__keywords[ind])==0)){
#define PAR_BUF_LEN	1024
      char tmp[PAR_BUF_LEN];
      memset(tmp,0,PAR_BUF_LEN);
      if(m__values[ind]==NULL){
         int i;
         //log_info("ai=%d",arr_index);
         for(i=0;(i<PAR_BUF_LEN)&&(i<arr_index);i++)
            tmp[i]=PAR_ARRAY_SEPARATOR;
         //log_info("i=%d",i);
         if(i>=PAR_BUF_LEN){
            log_err("array index %d is too big",arr_index);
            return false;
         }
         strncpy(tmp+arr_index,val_begin,PAR_BUF_LEN-arr_index-1);
         char *p=tmp+strlen(tmp);
         for(int i=arr_index+1;i<m__arrsizes[ind];i++){
            *p=PAR_ARRAY_SEPARATOR;
            p++;
         }
         (*p)='\0';
         m__values[ind]=strdup(tmp);
         //log_info("new:(%s)",m__values[ind]);
      }else{
         int i=0;
         char *p=m__values[ind];
         while((i<arr_index)&&(p!=NULL)){
            p=strchr(p,PAR_ARRAY_SEPARATOR);
            if(p!=NULL)p++;
            i++;
         }
         if(p!=NULL){
            //log_info("was:(%s)",m__values[ind]);
            char *p2=strchr(p,PAR_ARRAY_SEPARATOR);
            if(p2==NULL) p2= (char*)"";
            snprintf(tmp,1024,"%.*s%s%s",(int)(p-m__values[ind]),m__values[ind],
                  val_begin,p2);
            free(m__values[ind]);
            m__values[ind]=strdup(tmp);
            //log_info("new:(%s)",m__values[ind]);
         }else{
            //log_info("was:(%s)",m__values[ind]);
            strncpy(tmp,m__values[ind],PAR_BUF_LEN);
            p=tmp+strlen(tmp);
            for(;(p<tmp+PAR_BUF_LEN-1)&&(i<=arr_index);i++){
               (*p)=PAR_ARRAY_SEPARATOR;
               p++;
            }
            if(p>=tmp+PAR_BUF_LEN-1){
               log_err("array index %d is too big",arr_index);
               return false;
            }
            strncpy(p,val_begin,tmp+PAR_BUF_LEN-p-1);
            free(m__values[ind]);
            m__values[ind]=strdup(tmp);
            //log_info("new:(%s)",m__values[ind]);
         }
      }
      if(!nosub){
         m__values[ind]=
               replace_env_var(m__values[ind],NULL/*m__envp*/);
      }
      return true;
   }
   log_err("Invalid keyword in configfile line %d: %s",
         m__cflineno,kw_begin);
   return false;
}

char *strchr_noesc(char *s,char c){
   char *p=strchr(s,c);
   while( (p!=NULL)&&(p!=s) && (*(p-1)=='\\') ){
      if((*p)!='\0'){
         p=strchr(p+1,c);
      }
   }
   return p;
}
char *strchr_esc(char *s,char c){
   char *p=strchr(s,c);
   while( (p!=NULL)&&((p==s) || (*(p-1)!='\\') )){
      if((*p)!='\0'){
         p=strchr(p+1,c);
      }
   }
   return p;
}

char *remove_backslash(char *s,char c){
   if(s==NULL)return NULL;
   char *buf=strdup(s);
   (*buf)='\0';
   char *from=s;
   char *bs=strchr_esc(from,c);
   while(bs!=NULL){
      (*(bs-1))='\0';
      strcat(buf,from);
      from=bs;
      if((*from)!='\0')
         bs=strchr_esc(from+1,c);
      else bs=NULL;
   }
   strcat(buf,from);
   char *new_s=strdup(buf);
   free(buf);
   free(s);
   return new_s;
}


char *Params_Base::replace_env_var(char *s,char *const *envp){
   char *env_begin;
   char *env_end;
   char *quo_begin;
   char *quo_end;
   char *other;
   //	char **env=envp;
   const char *env_val;
   char *rs=s;
   do{
      env_begin=strchr_noesc(rs,'$');
      quo_begin=strchr_noesc(rs,'\'');
      while((quo_begin!=NULL)&&(env_begin!=NULL)&&
            (quo_begin<env_begin)){
         quo_end=strchr(quo_begin+1,'\'');
         env_begin=strchr_noesc(quo_end+1,'$');
         quo_begin=strchr_noesc(quo_end+1,'\'');
      }//XXX: ezt tesztelni kene!

      if((env_begin!=NULL)&&(env_begin[1]!='(')){
         if(m__cflineno!=0){
            log_err("Missing left parenthesis in configfile line %d: %s",	m__cflineno, s);
         }else{
            log_err("Missing left parenthesis in default value: %s", s);
         }
         return NULL;
      }
      if(env_begin!=NULL){
         env_end=strchr(env_begin+2,')');
         if(env_end==NULL){
            if(m__cflineno!=0){
               log_err("Missing right parenthesis in configfile line %d: %s",	m__cflineno,s);
            }else{
               log_err("Missing left parenthesis in default value: %s", s);
            }
            return NULL;
         }
         *env_begin='\0';
         env_begin+=2;
         *env_end='\0';
         other=env_end+1;
         //	char **env=envp;
         //	while((*env)!=NULL){//TODO getenv
         //		if((strncmp(*env,env_begin,strlen(env_begin))
         //			==0)&&((*env)[strlen(env_begin)]=='='))
         //			break;
         //		env++;
         //	}
         //	if((*env)==NULL){
         //		fprintf(stderr,"$(%s) not defined\n",env_begin);
         //		return s;
         //	}
         //	env_val=(*env)+strlen(env_begin)+1;
         env_val=getvar(env_begin);
         if(env_val==NULL){
            log_err("unknown variable %s", env_begin);
            env_val = strdup(""); // XXX memleak
            // Registry reading
            // Ha nem talalja meg a kornyezeti valtozot, megprobalja, hatha a windows registryben taroltuk a cuccost.
            /*	try {
					#ifdef WIN32
						TCHAR* t_env_val, *t_env_begin;
						t_env_val = (TCHAR*) env_val;
						t_env_begin = (TCHAR*) env_begin;
							// Ez a harom sor az Unicode kodolas miatt kellett, amibel meg sok baj lesz
						RegistryHandling reghand;
						t_env_val= (TCHAR*) reghand.tget_value(t_env_begin).c_str(); // (env_begin itt AppDataPath vagy InstallPath)
					#else
						throw "";
					#endif
					}
				catch (const char * registry_error) { // Ha vegkepp kudarcot vall, kilep:
					log_err("Required variable or registry key $(%s) is not defined in line %d of configfile %s. Exiting.\n%s",
						  env_begin, m__cflineno, m__conffn, registry_error);
					exit(1);
					//return NULL;//XXX this is broken!
					}
             */
         }
         char *newrs=(char *)malloc(
               strlen(rs)+strlen(env_val)+strlen(other)+1);
         //XXX: memory leak
         sprintf(newrs,"%s%s%s",rs,env_val,other);
         char *q=rs;
         rs=newrs;
         free(q);
      }
   }while(env_begin!=NULL);
   return remove_backslash(rs,'$');
}

void Params_Base::setvar(char *var,char *value){
   if(m__nvars<PAR_MAXVARS){
      m__vars[m__nvars]=strdup(var);
      m__varvalues[m__nvars]=strdup(value);
      m__nvars++;
   }
   else{
      log_err("Too many variables");
   }
}
char *Params_Base::getvar(char *var){
   if(var==NULL)return NULL;
   for(int i=0;i<m__nvars;i++){
      if((m__vars[i]!=NULL)&&(strcmp(var,m__vars[i])==0))
         return m__varvalues[i];
   }
   for(int ind=0;ind<m__nitems;ind++){ // parametr ertek helyettesites
      if((m__keywords[ind]!=NULL)&&
            (strcmp(var,m__keywords[ind])==0)&&
            m__values[ind]!=NULL)
         return m__values[ind];
   }
   return getenv(var);
}



void Params_Base::delvars(){
   m__nvars=0;
   for(int i=0;i<PAR_MAXVARS;i++){
      if(m__vars[i]!=NULL){
         free(m__vars[i]);
         m__vars[i]=NULL;
      }
      if(m__varvalues[i]!=NULL){
         free(m__varvalues[i]);
         m__varvalues[i]=NULL;
      }
   }
}

char *Params_Base::getnextarrayitem(char **p){
   char *s=*p;
   char *q=strchr(s,PAR_ARRAY_SEPARATOR);
   if(q!=NULL){
      *q='\0';
      *p=q+1;
   }else{
      *p=NULL;
   }
   return s;
}




void Params_Base::tolower(char *p){
   while(((*p)!='\0')&&((*p)!='=')){
      if( ( 'A'<=(*p) )&&( (*p)<='Z' ) )
         (*p)=(*p)-'A'+'a';
      p++;
   }
}
char *Params_Base::skipwspace(char *p){
   while(((*p)!='\0') &&
         ( ( (*p)==' ' ) || ( (*p)=='\t' ) || ( (*p)=='\n' ) )){
      p++;
   }
   return p;
}
char *Params_Base::skipwspaceoreq(char *p,int *waseq){
   (*waseq)=0;
   while(((*p)!='\0') &&
         ( ( (*p)==' ' ) || ( (*p)=='\t' ) || ( (*p)=='\n' ) ||
               ( (*p)=='=' ) ) ){
      if((*p)=='=') (*waseq)=1;
      p++;
   }
   return p;
}
char *Params_Base::findwspace(char *p){
   while(((*p)!='\0') &&
         ( ( (*p)!=' ' ) && ( (*p)!='\t' ) && ( (*p)!='\n' ) ) ){
      p++;
   }
   return p;
}

char *Params_Base::findwspaceoreq(char *p){
   while(((*p)!='\0') &&
         ( ( (*p)!=' ' ) && ( (*p)!='\t' ) && ( (*p)!='\n' ) &&
               ( (*p)!='=' ) ) ){
      p++;
   }
   return p;
}


int Params_Base::str_to_switch(char *s){
   char *ss=strdup(s);
   int r=-1;
   tolower(ss);
   if(	(strcmp(ss,"yes")==0) ||
         (strcmp(ss,"1")==0) ||
         (strcmp(ss,"true")==0) ||
         (strcmp(ss,"on")==0)) r=1;
   if(	(strcmp(ss,"no")==0) ||
         (strcmp(ss,"0")==0) ||
         (strcmp(ss,"false")==0) ||
         (strcmp(ss,"off")==0)) r=0;
   free(ss);
   if((r==1)||(r==0))return r;
   log_err( "error: switch argument %s is invalid, please use one of "
         "{yes,no,true,false,on,off,1,0}", s );
   deletestruct();
   if(m__conffn!=NULL)free(m__conffn);
   m__conffn=NULL;
   exit(1);
}
