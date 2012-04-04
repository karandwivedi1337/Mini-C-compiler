//remove single and multiline comments as well as blank spaces(starting and trailing) from a source file
//input: source file path e.g E:\B1-75\LAB2\test.cpp
//output: E:\B1-75\compiled.cpp
//limitation-1: cannot process strings like cout<<"a" <spaces> ; or if(a==5) <spaces> {  }
//limitation-2: escape characters are not supported within string literals e.g. " \" "
//limitation-3: limited identifer length to a maximum of 32
//limitation-4: declaration list should be terminated by ;\n
//limitation-5: assigment via condition operator should involve all identifiers of the same type
//condition to be avoided: c   o u ttt <<a;  //will make separate entries for c,o,u ttt in symbol table
//above condition may over-write some existing identifiers in symbol table
//note:maximum one space may occur after the delimitor and before \n
#include<iostream.h>
#include<stdlib.h>
#include<conio.h>
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#define hash_size 256      //size of hash table
#define keywords_length 32  //size of keyword array
#define reserved_length 4  //size of reserved length array
#define types_length 2     //size of types array

void statement();
void statement_list();
int listundec();
void skip();

void asm_declarations();
void asm_statement();
void asm_statement_list();

FILE *f,*f1,*f2,*f3,*f4,*f5,*f6,*f7;
char name[200]="E:\\B1-75\\LAB2\\test.cpp"; //the source file
char output[200]="E:\\B1-75\\compiled.cpp";
char tokens[200]="E:\\B1-75\\tokens.txt"; //repetition in file
char key[200]="E:\\B1-75\\keywords.txt";//non-repeating in file
char ops[200]="E:\\B1-75\\operators.txt";//repetition in file
char numbers[200]="E:\\B1-75\\numbers.txt"; //repetition in file
char special[200]="E:\\B1-75\\special.txt"; //repetition in file
char identify[200]="E:\\B1-75\\identify.txt"; //repetition in file
//char identifier[200]="E:\\B1-75\\identifiers.txt";
//f is the original code
//f1 is the updated file
//f2 is the tokens file (keywords + identifiers)
//f3 is the keywords file
//f4 is the operators file
//f5 is the numbers file
//f6 is the special characters file
//f7 is the identifiers file
char keywords[][8]={"auto","break","case","char","const","continue","default","do","double","else","enum","extern","float","for","goto","if","int","long","register","return","short","signed","sizeof","static","struct","switch","typedef","union","unsigned","void","volatile","while"};
//length of keyword array=keywords_length=32
char reservedwords[][32]={"main","cout","cin","exit"};
//length of reserved array=reserved_length=4
char types[][32]={"char","int"};
//length of types array=types_length=2
int printkey[keywords_length]; //printed[i]=1 if ith keyword has been printed
//char defkey[][10]={"auto ","break ","break;","case ","char ","const ","continue ","continue;","default:","do{","double ","else ","else{","enum{","extern ","float ","for(","goto ","if(","int ","long ","register ","return ","return;","short ","signed ","sizeof(","static ","struct ","switch(","typedef ","union ","unsigned ","void ","volatile ","while("};
char c,tok[32];

struct node{
char name[32]; //lexeme name
char type;  //token type (e.g. keywords, identifiers, numbers, special symbols
char datatype; // char(c), int(i), char array(d), int array(j)
int size;   //size of data type (char=1, int=2, char array,int array=size in [])
int verified; //check if an identifier has been declared on not
struct node* next;
};

node* asm_term(char from[]);
void asm_simple_expn(char from[]);
node* asm_tprime(char from[]);

node* t=NULL;
int linerr=1,openbrac=0,closebrac=0; //parser counters for scope mismatches
int dec_label_count=0, loop_label_count=0;
// counters for assembly labels for decision and looping statements

node* getnode(){
node* p=(node*)malloc(sizeof(node));
return p;
}

node* hash[hash_size]; //hash table used as a data-structure for symbol table

void overwrite(){ //copies changes back to test.cpp

f=fopen(name,"w");
f1=fopen(output,"r");
char c=getc(f1);

while(c!=EOF){
putc(c,f);
c=getc(f1);
}//while close
fclose(f);
fclose(f1);

}//overwrite() close

void inittable(){
for(int i=0;i<hash_size;i++){
hash[i]=NULL; //initialize hash pointers to NULL
}
}

int calchash(char buf[]){

int val=0,pos=0;

while(buf[pos]!='\0'){
val+=(int)buf[pos];
pos++;
}
return (val%hash_size);

}//calchash close

void addtotable(char symbol[], char t, char dt, int s, int pos){ //pos=location in hash table
				//type,datatype,size
node *p=getnode();

strcpy(p->name,symbol);
p->type=t;
p->datatype=dt;
p->size=s;
p->next=NULL;
p->verified=0;

if(hash[pos]==NULL) hash[pos]=p;

else if(hash[pos]!=NULL){
node* prev=hash[pos];
while(prev->next!=NULL) prev=prev->next;
prev->next=p;
}

}//addtotable close

int searchtable(char key[]){

int pos=calchash(key);

node* t=hash[pos];

while(t!=NULL){
if(strcmp(key,t->name)==0) return 1;
t=t->next;
}//while close

return 0;
}


node* findintable(char key[]){

int pos=calchash(key);

node* t=hash[pos];

while(t!=NULL){
if(strcmp(key,t->name)==0) return t;
t=t->next;
}//while close

return NULL;
}


void createtable(FILE* curfile, char type, char datatype, int size){

//f7=fopen(identify,"r");

char c,buf[32];
int bufpos=0;
c=getc(curfile);

//in every file, for every string\n, search in hash table, if not found, find position and insert into table
while(c!=EOF){
bufpos=0;

while(c!='\n'){
buf[bufpos++]=c;
c=getc(curfile);
}

buf[bufpos++]='\0';

//puts(buf);

int found=searchtable(buf);

if(found==0){
if(datatype=='c') addtotable(buf,type,datatype,strlen(buf),calchash(buf));
else if(datatype=='n') addtotable(buf,type,datatype,strlen(buf)*2,calchash(buf));
else addtotable(buf,type,datatype,size,calchash(buf));
}

c=getc(curfile);
}//while close

fclose(curfile);

}//createtable() close

void populatetable(){
f7=fopen(identify,"r"); //type, datatype, size
createtable(f7,'i','i',9);
f6=fopen(special,"r");
createtable(f6,'s','c',1);
f5=fopen(numbers,"r");
createtable(f5,'n','i',2); //numbers by default taken as integers
f4=fopen(ops,"r");
createtable(f4,'o','c',1);
f3=fopen(key,"r");
createtable(f3,'k','c',9);
}//populatetable cose


void edittable(char buf[], char t, char dt, int s){

//int loc=calchash(buf);

node* n=hash[calchash(buf)];

while(n!=NULL){
if(strcmp(buf,n->name)==0){
n->type=t;
n->datatype=dt;
n->size=s;
break;
}
n=n->next;
}//while close

}//edittable() close

void displaytable(){
int print=0;
for(int i=0;i<hash_size;i++){

if(hash[i]!=NULL){
cout<<"At "<<i<<" ";
node* t=hash[i];

while(t!=NULL){
print++;
int pos=0;
while(t->name[pos]!='\0') cout<<t->name[pos++];
cout<<" "<<t->type<<" "<<t->datatype<<" "<<t->size<<" "<<"   ";
t=t->next;
if(print%4==0) cout<<endl;
}//while close

}//if close

}//for close

}//displaytable() close

void displaynode(node* t){
int p=0;
while(t->name[p]!='\0') cout<<t->name[p++];
cout<<t->type<<t->datatype<<t->size<<"  ";
}


int reserved(char buf[]){

for(int i=0;i<reserved_length;i++){ //4=length of reservedwords array
if(strcmp(buf,reservedwords[i])==0) return 1;
}
return 0;
}

void listidentifiers(){
f2=fopen(tokens,"r");
f7=fopen(identify,"w");

char c,buf[32],bufpos=0;
int flag=1; //if flag=1 assumes that current token is an identifier

c=getc(f2);

//for each token, check if it matches a keyword. If not, then write to f7
while(c!=EOF){
bufpos=0;
flag=1;

while(c!='\n'){
buf[bufpos++]=c;
c=getc(f2);}
buf[bufpos++]='\0';

//puts(buf);

for(int i=0;i<keywords_length;i++){  //32=length of keywords array
if(strcmp(buf,keywords[i])==0 || reserved(buf)==1){
flag=0; //0 = not an identifier
break;}
}

if(flag==1){

for(int j=0;j<strlen(buf);j++){
putc(buf[j],f7);}

putc('\n',f7);

}

c=getc(f2);

}//while c!=EOF close

fclose(f2);
fclose(f7);
}//listidentifiers() close

void listoperators(){
char c,d,e;
int flag=0;
f1=fopen(output,"r");
f4=fopen(ops,"w");
f6=fopen(special,"w");
c=getc(f1);

while(c!=EOF){
flag=0;

if(c=='"'){ //bypass string literals
c=getc(f1);
while(c!='"'){
c=getc(f1);
}
c=getc(f1);
}//if c=='"' close

else if(c=='~' || c==','|| c=='(' || c==')' || c=='!' || c==';'){
putc(c,f4);
flag=1;}


else if(c=='+'){
putc(c,f4);

d=getc(f1);

if(d=='='|| d=='+') putc(d,f4);

flag=1;
}//c=='+' close

else if(c=='-'){
putc(c,f4);

d=getc(f1);

if(d=='=' || d=='-') putc(d,f4);
flag=1;
}//c=='-' close

else if(c=='=' || c=='*' || c=='/' || c=='%' || c=='^'){
putc(c,f4);

d=getc(f1);
if(d=='=') putc(d,f4);
flag=1;
}

else if(c=='&'){
putc(c,f4);

d=getc(f1);

if(d=='&'|| d=='=') putc(d,f4);

flag=1;
}//c=='&' close


else if(c=='|'){
putc(c,f4);

d=getc(f1);

if(d=='|' || d=='=') putc(d,f4);

flag=1;
}//c=='|' close


else if(c=='<'){
putc(c,f4);

d=getc(f1);
if(d=='=') putc(d,f4);
else if(d=='<'){
putc(d,f4);
e=getc(f1);
if(e=='=') putc(e,f4);
else ungetc(e,f1);
}
flag=1;
}//c=='<' close

else if(c=='>'){
putc(c,f4);

d=getc(f1);
if(d=='=') putc(d,f4);
else if(d=='>'){
putc(d,f4);
e=getc(f1);
if(e=='=') putc(e,f4);
else ungetc(e,f1);
}
flag=1;
}//c=='>' close

/*
else if(c=='?'){ //check for conditional operator
d=getc(f1);
while(d!=':' & d!='\n') d=getc(f1); //characters may be present in between!
if(d==':'){ putc('?',f4); putc(d,f4); }
flag=1;
}
*/

else if(!isdigit(c) & !isalpha(c) & c!=';' & c!=' ' & c!='\n' & c!='\t'){
putc(c,f6);
putc('\n',f6);
}

if(flag==1) putc('\n',f4);

c=getc(f1);

}//while close
fclose(f1);
fclose(f4);
fclose(f6);

}//listoperators() close

void listkeywords(){
f2=fopen(tokens,"r");
f3=fopen(key,"w");
char buf[10];
int pos,i,k=0;
char c=getc(f2);

cout<<"Keywords:\n";

while(c!=EOF){
pos=0;

//f2=getline(buf,f2,'\n');

while(c!='\n'){
buf[pos++]=c;
c=getc(f2);}

buf[pos]='\0';
//puts(buf);

for(i=0;i<keywords_length;i++){ //32=length of keywords array
k=0;
if(strcmp(buf,keywords[i])==0){
if(printkey[i]==0){
//int position=calchash(buf);
//addtotable(buf,'k',9,position);
while(buf[k]!='\0'){
cout<<(char)toupper(buf[k]);
putc(buf[k],f3);
k++;}
cout<<endl;
putc('\n',f3);
}
printkey[i]=1;
}
}//for close


c=getc(f2);
}//while close
fclose(f2);
fclose(f3);

}//listkeywords() close

void listtokens(){
char c;
int i,j;

f=fopen(name,"r");
f2=fopen(tokens,"w");
f5=fopen(numbers,"w");

c=getc(f);

while(c!=EOF){

if(c=='"'){ //bypass string literals
c=getc(f);
while(c!='"'){
c=getc(f);
}
c=getc(f);
}//if c=='"' close

else if(isalpha(c)){
putc(c,f2);
c=getc(f);
while(isdigit(c) || isalpha(c)){
putc(c,f2);
c=getc(f);
}//while close
putc('\n',f2);
}//else if (isalpha() close)

else if(isdigit(c)){
putc(c,f5);
c=getc(f);
while(isdigit(c)){
putc(c,f5);
c=getc(f);
}
putc('\n',f5);
}//else if isdigit(c) close

else { c=getc(f); }

}//while c!=EOF close

fclose(f);
fclose(f2);
fclose(f5);
}//listtokens() close

void removelines(){
char c,d,e;
int flag=0;
f=fopen(name,"r");
f1=fopen(output,"w");

c=getc(f);

while(c!=EOF){
flag=0;

if(c=='"'){
putc(c,f1);

d=getc(f);

while(d!='"'){
putc(d,f1);
d=getc(f);}

c=d;
}//c==' " ' close

else if(c=='\n'){

while(c=='\n') c=getc(f);
flag=1;

}

if(flag==1){
putc('\n',f1);
if(c!=EOF) putc(c,f1);
}
else putc(c,f1);
c=getc(f);
}
fclose(f);
fclose(f1);

}//removelines() close

void removespaces(){
//new function

char c,d;
int pos=0,len=0;
f=fopen(name,"r");
f1=fopen(output,"w");

char buf[5000]; //maximum size of 1 line=5000 characters

c=getc(f);
while(c!=EOF){

buf[pos++]=c;

if(c=='\n'){
//buf[pos++]='\0';
//remove leading extra spaces
for(int i=0;i<pos;i++){
if(buf[0]!=' ' & buf[0]!='\t') break;
if(buf[i]==' '|| buf[i]=='\t') continue;

len=i;
for(int j=0;j<pos;j++) buf[j]=buf[len+j]; //shift characters to left
pos=pos-len;
}//for close

//print truncated line
for(i=0;i<pos;i++){
if(pos==1) break; //do not print single \n
putc(buf[i],f1);
}


for(i=0;i<5000;i++) buf[i]=NULL;
pos=0;
len=0;
}//if c=='\n' close

c=getc(f);

}//while(c!=EOF) close

fclose(f1);
fclose(f);

//remove other extra spaces
overwrite();

//remove other and leading spaces

int extra=0;

f=fopen(name,"r");
f1=fopen(output,"w");

c=getc(f);

while(c!=EOF){
extra=0;

if(c=='"'){
while(c!='"'){
putc(c,f1);
c=getc(f);}
}

while(c==' ' || c=='\t'){ //all consecutive spaces and tabs are replaced by a single space
c=getc(f);
extra=1;
}
if(extra==1) putc(' ',f1);

putc(c,f1);

c=getc(f);
}

fclose(f);
fclose(f1);

//new function

}//removespaces() close

void removecomments(FILE* f){
char c,d,e,g;
int comm=0,multi=0,flag=0;

f1=fopen(output,"w");

c=getc(f);

while(c!=EOF){
flag=0;

if(c=='"'){  //comments within quotes are allowed!
putc(c,f1);
c=getc(f);
while(c!='"'){
putc(c,f1);
c=getc(f);}
flag=0;
}//if c=='"' close

if(c=='/'){

d=getc(f);

if(d=='/'){ //single line comment detected
flag=1;
e=getc(f);
comm++;

while(e!='\n'){
e=getc(f);
}

}//d=='/' close

else if(d=='*'){ //multiline comment detected
flag=1;
e=getc(f);
multi++;        /*      */
			  //cd      eg
g=getc(f);

while(1){
if(e=='*' & g=='/') break;
else{
e=g;
g=getc(f);
}

}

}//if d=='*' close

else{
flag=1;
putc(c,f1);
putc(d,f1);
}

}//if c=='/' close

if(flag==0)
putc(c,f1);

c=getc(f);

}//while close
fclose(f);
fclose(f1);
//cout<<"Single line comments:"<<comm<<"\n";
//cout<<"Multiple line comments:"<<multi<<"\n";
}//removecomments() close

void identifydatatypes(){

f1=fopen(output,"r"); //open compiled.cpp
int idsize,pos=0,flag;
char buf[32],dt;

char c=getc(f1);

while(c!=EOF){
idsize=9;
pos=0;
flag=0;

if(c=='"'){ //bypass string literals
c=getc(f);
while(c!='"'){
c=getc(f);
}
//c=getc(f);
}//if c=='"' close


else if(isalpha(c)){

buf[pos++]=c;
c=getc(f1);

while(isalpha(c)){
buf[pos++]=c;
c=getc(f1);}

buf[pos]='\0';
//0 1 2 3
//1 2 4 6
for(int i=0;i<types_length;i++){
if(strcmp(buf,types[i])==0){
if(i==0){
idsize=1;  //size of char=1
dt='c';}
else{
idsize=i*2;  //size of int=2
dt='i';
}
flag=1; //flag==1 denotes that a valid datatype is found
break;
}
}//for close

//int p=0;
//while(buf[p]!='\0') cout<<buf[p++];
//cout<<"\t";
}//if(isalpha(c)) close


if(flag==1){
char id[32];
int idpos=0,num;//num=size of arrays in declaration list

//c is now one character ahead of datatype
c=getc(f1); //move to next character in the declaration list

while(c!='\n' & c!=';'){

idpos=0;
num=0;

while(isalpha(c)){
id[idpos++]=c;

c=getc(f1);

while(isdigit(c) || isalpha(c)){
id[idpos++]=c;
c=getc(f1);}

//support for array identification can be added here

if(c=='['){

num=0; //array size
c=getc(f1);

while(isdigit(c)){
num=num*10+((int)c-48);
c=getc(f1);
}

//idsize*=num;
//dt++;

//c=getc(f1);

}//if c=='[' close


id[idpos]='\0';


int f=searchtable(id);


if(f==1){
//int p=0;
//while(buf[p]!='\0') cout<<buf[p++];
//cout<<"\t";
if(num>0)
edittable(id,'i',dt+1,idsize*num);
else
edittable(id,'i',dt,idsize);
}

}//while(isalpha(c)) close

c=getc(f1);

}//while(c!='\n' & c!=';') close

}//if flag==1 close

c=getc(f1);
}//while(c!=EOF) close

fclose(f1);
}//identifydatatypes() close

void neutralize(){
for(int i=0;i<32;i++) tok[i]='\0';
}//neutralize() close

node* lex(FILE* f1){

char d,e,pos=0;
node* t=NULL;
//int p=0;
c=getc(f1);

while(c!=EOF){
neutralize();
t=NULL;
pos=0;

if(c==' '){
//c=getc(f1);
//cout<<"space\n";
//return NULL;
tok[0]=c;
tok[1]='\0';
break;
}

else if(c=='\n'){
//c=getc(f1);
//cout<<"newline\n";
//return NULL;
tok[0]=c;
tok[1]='\0';
break;
}

else if(c=='"'){ //bypass string literals
//cout<<"String literal\n";
c=getc(f1);

while(c!='"'){
c=getc(f1);
}
strcpy(tok,"String");
break;
//return NULL;
}//if c=='"' close

else if(isalpha(c) || isdigit(c)){
tok[pos++]=c;
c=getc(f1);

while(isalpha(c) || isdigit(c)){
tok[pos++]=c;
c=getc(f1);
}//while close

ungetc(c,f1);

tok[pos]='\0';

t=findintable(tok);

if(t!=NULL) return t;
else break;
}//else if (isalpha(c) || isdigit(c) close)

else{   //check for special characters and operators
//cout<<"SS\t";

if(c=='+'){
tok[pos++]=c;
//cout<<c;

d=getc(f1);

if(d=='='|| d=='+'){
tok[pos++]=d;
}

else ungetc(d,f1);

}//c=='+' close

else if(c=='-'){
tok[pos++]=c;
//cout<<c;

d=getc(f1);

if(d=='='|| d=='-'){
tok[pos++]=d;
}

else ungetc(d,f1);

}//c=='+' close


else if(c=='~' || c==','|| c=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']' || c==';'){
tok[pos++]=c;
}

else if(c=='<'){
tok[pos++]=c;
//putc(c,f4);

d=getc(f1);
if(d=='=') tok[pos++]=d;
else if(d=='<'){
tok[pos++]=d;
e=getc(f1);
if(e=='=') tok[pos++]=e;
else ungetc(e,f1);
}
else ungetc(d,f1);
}//c=='<' close


else if(c=='>'){
tok[pos++]=c;

d=getc(f1);
if(d=='=') tok[pos++]=d;
else if(d=='>'){
tok[pos++]=d;
e=getc(f1);
if(e=='=') tok[pos++]=e;
else ungetc(e,f1);
}
else ungetc(d,f1);
}//c=='>' close


else if(c=='|'){
tok[pos++]=c;
//putc(c,f4);

d=getc(f1);

if(d=='|' || d=='=') tok[pos++]=d;
else ungetc(d,f1);

}//c=='|' close

else if(c=='&'){
tok[pos++]=c;
//putc(c,f4);

d=getc(f1);

if(d=='&'|| d=='=') tok[pos++]=d;

else ungetc(d,f1);

}//c=='&' close

else if(c=='=' || c=='*' || c=='/' || c=='%' || c=='^' || c=='!'){
tok[pos++]=c;
//putc(c,f4);

d=getc(f1);
if(d=='=') tok[pos++]=d;
else ungetc(d,f1);
}

tok[pos]='\0';

t=findintable(tok);

if(t!=NULL) return t;
else break;
}//else close for special characters and operators

}//while(c!=EOF) close


t=getnode();
if(c==EOF) strcpy(tok,"EOF");
strcpy(t->name,tok);
t->type='r';
t->datatype='r';
t->size=strlen(tok);
return t;

}//lex() close

void error(char errmsg[]){
cout<<errmsg<<" at line "<<linerr<<"\n";

while(strcmp(t->name,"EOF")!=0){
if(strcmp(tok,"{")==0) openbrac++;
else if(strcmp(tok,"}")==0) closebrac++;
t=lex(f1);
}

if(openbrac>closebrac) cout<<"Scope mismatch. } expected\n";
else if(openbrac<closebrac) cout<<"Scope mismatch. { expected\n";
//cout<<"Syntax error at line "<<linerr<<" "<<errmsg<<"\n";
listundec();

getch();
exit(1);
}

void passspnew(){

while(strcmp(t->name," ")==0 | strcmp(t->name,"\n")==0){
//cout<<" pass ";
if(strcmp(t->name,"\n")==0){ linerr++; }
t=lex(f1);
}

}//passspnew() close


int match(char expectedtoken[],char errmsg[]){
if(strcmp(tok,"{")==0) openbrac++;
else if(strcmp(tok,"}")==0) closebrac++;
if(strcmp(tok,expectedtoken)==0){
//cout<<t->name<<" ";
t=lex(f1);
//cout<<"1";
return 1;}

else error(errmsg);
return 0;
}//match() close


void id_list(){
//cout<<"id_list called ";
char errmsg[]="Declaration syntax error";

if(t->datatype=='i' | t->datatype=='c' | t->datatype=='j' | t->datatype=='d'){
node* tt=t;
//cout<<tt->name;

match(t->name,errmsg);

tt->verified++;

if(strcmp(t->name,",")==0){

match(t->name,errmsg);

id_list();

}//comma after id close

else if(strcmp(t->name,"[")==0){
match("[",errmsg);

if(t->type=='n') match(t->name,errmsg);

match("]",errmsg);

if(strcmp(t->name,",")==0) match(t->name,errmsg);

else if(strcmp(t->name,";")==0){ match(t->name,errmsg); return;}

id_list();

}//array declaration after id close


else{ //id_list ends with a ;
match(";",errmsg);
//else error(errmsg);
}//else close

}//if(t->datatype=='i' | t->datatype=='c') close

else error(errmsg); //corresponds to first token not being an identifier after "int "

}//id_list() close

void factor(){
//cout<<"Factor called ";
char errmsg[]="Identifier or number expected";
passspnew();

if(t->type=='i') match(t->name,errmsg);
else if(t->type=='n') match(t->name,errmsg);
else error(errmsg);

}//factor() close

void tprime(){
//cout<<"tprime called ";
char errmsg[]="Invalid expression";
//tprime->mulop factor tprime

passspnew();

if(strcmp(t->name,"*")==0 || strcmp(t->name,"/")==0 || strcmp(t->name,"%")==0)
match(t->name,errmsg);
else return;

passspnew();

factor();

tprime();

}//tprime() close

void term(){
//cout<<"term called ";

factor();

tprime();

}

void seprime(){
//cout<<"seprime called ";
char errmsg[]="Invalid expression";

passspnew();
//seprime->addop term seprime
//cout<<"X"<<t->name<<"X";
if(strcmp(t->name,"-")==0 || strcmp(t->name,"+")==0) match(t->name,errmsg);
else return;
//if(strcmp(t->name,"+")==0) match(t->name,errmsg);
//else if(strcmp(t->name,"-")==0) match(t->name,errmsg);
//else return;

term();

passspnew();

seprime();

}

void simple_expn(){
//cout<<"simple_expn called ";

term();

seprime();

}

void eprime(){

//cout<<"eprime called ";
char errmsg[]="Unsupported relational operator";
passspnew();

if(strcmp(t->name,"!=")==0) match(t->name,errmsg);
else if(strcmp(t->name,"==")==0) match(t->name,errmsg);
else if(strcmp(t->name,"<=")==0) match(t->name,errmsg);
else if(strcmp(t->name,">=")==0) match(t->name,errmsg);
else if(strcmp(t->name,">")==0) match(t->name,errmsg);
else if(strcmp(t->name,"<")==0) match(t->name,errmsg);
//if(strcmp(t->name,"!=")==0) match(t->name,errmsg);

else return;

simple_expn();
}

void expn(){
//cout<<"expn called ";
//cout<<"expn called\n";

passspnew();

simple_expn();

eprime();

//passspnew();

}//expn() close


void dprime(){
//cout<<"dprime called ";
char errmsg[]="Invalid conditional statement";
passspnew();
//cout<<"X"<<t->name<<"X";

if(strcmp(t->name,"else")==0) match(t->name,errmsg);
else return;

statement();

}//dprime() close

void assign_statement(){
//cout<<"assign_statement called ";
char errmsg[]="Incorrect assignment";
match(t->name,errmsg);

//t=lex(f1);
passspnew();

match("=",errmsg);
passspnew();

expn();

match(";",errmsg);
}

void assignloop_statement(){
//cout<<"assignloop_statement called ";
char errmsg[]="Incorrect assignment";
match(t->name,errmsg);

//t=lex(f1);
passspnew();

match("=",errmsg);
passspnew();

expn();

//match(";",errmsg);
}


void decision_statement(){
//cout<<"decision statement called ";
char errmsg[]="Invalid decision statement";
match(t->name,errmsg);

passspnew();

match("(","( expected");

expn();

match(")",") expected");

statement();
//match(";",errmsg);

passspnew();

dprime();

}//decision_statement close

void looping_statement(){
//cout<<"looping statement called ";
char errmsg[]="Invalid looping statement";

if(strcmp(t->name,"while")==0){
match(t->name,errmsg);

match("(","( expected");

expn();

match(")",") expected");

statement();
}

else if(strcmp(t->name,"for")==0){
match(t->name,errmsg);

match("(","( expected");

assign_statement();

expn();

match(";","; expected");

assignloop_statement();

match(")",") expected");

statement();

}

else error(errmsg);

}//looping_statement() close

void statement(){
//cout<<"statement called ";
char errmsg[]="Invalid statement";
passspnew();

if(t->type=='i'){ //assign statement
assign_statement();
}//assign_statement close

else if(strcmp(t->name,"if")==0){ //decision statement
decision_statement();
}//decision statement close

else if(strcmp(t->name,"while")==0 || strcmp(t->name,"for")==0){
looping_statement();
}//looping statement close

else error(errmsg);

}//statement() close

void statement_list(){
//cout<<"statement list called ";
//char errmsg[]="from statement_list";
passspnew();

if(strcmp(t->name,"if")==0 || strcmp(t->name,"for")==0 || strcmp(t->name,"while")==0 || t->type=='i'){
statement();
//match(";",errmsg);
statement_list();
}
else return;

}//statement_list() close

void declarations(){
//cout<<"declarations called";
char errmsg[]="Incorrect declaration";
passspnew();

if(strcmp(t->name,"int")==0 | strcmp(t->name,"char")==0)
match(t->name,errmsg);
else return;
passspnew(); //int a,b; (space required after datatype)

id_list();

declarations();

}//declarations() close

void skip(){
//cout<<"skip called ";
while(strcmp(t->name,"}")!=0) t=lex(f1);

}//skip() close

int program(){
//cout<<"program called ";
char errmsg[]="Syntax error";

match("main",errmsg);
match("(",errmsg);
match(")",errmsg);
match("{",errmsg);

declarations();

statement_list();

//skip();

match("}",errmsg);

int ret=listundec();
if(ret==0){
cout<<"No syntax errors\n";
return 1;}
return 0;

}//program() close

int parser(){
f1=fopen(output,"r"); //parser will open file and lex will return next token

t=lex(f1);

int ret=program();
fclose(f1);
return ret;

//getch();
}//parser() close

int listundec(){
int flag=0;
for(int i=0;i<hash_size;i++){

node* p=hash[i];

while(p!=NULL){
if(p->datatype=='i' || p->datatype=='j' || p->datatype=='c' || p->datatype=='d'){
if(p->type=='i' & p->verified<=0){
cout<<p->name<<" undeclared\n";
flag=1;}
else if(p->type=='i' & p->verified>1){
cout<<"Multiple declaration of "<<p->name<<endl;
flag=1;}}
p=p->next;
}

}//for close
return flag;
}//listundec() close


void asm_error(char errmsg[]){
cout<<errmsg<<" at line "<<linerr<<"\n";
getch();
exit(1);
}

int asm_match(char expectedtoken[],char errmsg[]){

if(strcmp(tok,expectedtoken)==0){
//cout<<t->name<<" ";
t=lex(f1);
return 1;}

else asm_error(errmsg);
return 0;
}//asm_match() close

void asm_factor(char from[]){
//cout<<"Factor called ";
char errmsg[]="Identifier or number expected";
passspnew();

if(strcmp(from,"expn")==0){ cout<<"mov ax,"; puts(t->name); asm_match(t->name,errmsg); }
else if(strcmp(from,"seprime")==0 || strcmp(from,"tprime")==0 || strcmp(from,"eprime")==0)
{ cout<<"mov bx,"; puts(t->name); asm_match(t->name,errmsg); }
//else asm_error(errmsg);

}//asm_factor() close


node* asm_tprime(char from[]){
//cout<<"tprime called ";
char errmsg[]="Invalid expression";
node* op=NULL;
//tprime->mulop factor tprime

passspnew();

if(strcmp(t->name,"*")==0 || strcmp(t->name,"/")==0 || strcmp(t->name,"%")==0){
op=t;
asm_match(t->name,errmsg);}
else return op;

passspnew();

asm_factor("tprime");

asm_tprime(from);

return op;
}//tprime() close

node* asm_eprime(){

//cout<<"eprime called ";
char errmsg[]="Unsupported relational operator";
passspnew();
node* op=NULL;

if(strcmp(t->name,"!=")==0){ op=t; asm_match(t->name,errmsg);          }
else if(strcmp(t->name,"==")==0){ op=t; asm_match(t->name,errmsg);    }
else if(strcmp(t->name,"<=")==0){ op=t; asm_match(t->name,errmsg);   }
else if(strcmp(t->name,">=")==0){ op=t; asm_match(t->name,errmsg);  }
else if(strcmp(t->name,">")==0){ op=t; asm_match(t->name,errmsg);  }
else if(strcmp(t->name,"<")==0){ op=t; asm_match(t->name,errmsg); }
//if(strcmp(t->name,"!=")==0) match(t->name,errmsg);

else return op;

asm_simple_expn("eprime");
return op;
}//asm_eprime() close


node* asm_seprime(){
//cout<<"seprime called ";
char errmsg[]="Invalid expression";

passspnew();
node* op=NULL;
//seprime->addop term seprime
//cout<<"X"<<t->name<<"X";
if(strcmp(t->name,"-")==0 || strcmp(t->name,"+")==0){
//if(strcmp(t->name,"+")==0) cout<<"add ax,bx\n";
//if(strcmp(t->name,"-")==0) cout<<"sub ax,bx\n";
op=t;
asm_match(t->name,errmsg);}
else return op;
//if(strcmp(t->name,"+")==0) match(t->name,errmsg);
//else if(strcmp(t->name,"-")==0) match(t->name,errmsg);
//else return;

asm_term("seprime");

passspnew();

asm_seprime();
return op;

}//asm_seprime() close


node* asm_term(char from[]){
//cout<<"term called ";

asm_factor(from);

node* op=asm_tprime(from);
return op;
}//asm_term() close


void asm_simple_expn(char from[]){
//cout<<"simple_expn called ";

node* op1=asm_term(from);

node* op2=asm_seprime();

if(op1==NULL & op2==NULL) return;

if(op1!=NULL & op2==NULL){
//cout<<"mul or div case\n";
if(op1->name[0]=='*')
cout<<"mul bx\n";
else if(op1->name[0]=='/')
cout<<"div bx\n";
else if(op1->name[0]=='%')
cout<<"div bx\n";
cout<<"mov ax,dx\n";
}

else if(op1==NULL & op2!=NULL){
if(op2->name[0]=='+')
cout<<"add bx\n";
else if(op2->name[0]=='-')
cout<<"sub bx\n";
//cout<<"add or sub case\n";
}

//op1 and op2 both can be null e.g. b=a;
			      //b=a+-10
			      //b=a*/10
}//asm_simple_expn() close

node* asm_expn(){
//cout<<"expn called ";
//cout<<"expn called\n";
passspnew();

asm_simple_expn("expn");

node* op=asm_eprime();

return op;
//passspnew();

}//asm_expn() close


void asm_assign_statement(){
//cout<<"assign_statement called ";
char errmsg[]="Incorrect assignment";
char msg[50];
strcpy(msg,"mov ");
strcat(msg,t->name);
strcat(msg,",ax");
//strcat(msg,",ax");
asm_match(t->name,errmsg);

//t=lex(f1);
passspnew();

asm_match("=",errmsg);
passspnew();

asm_expn();
puts(msg);
//memset(msg,'\0')
match(";",errmsg);

}//asm_assign_statement()


void asm_dprime(){
//cout<<"dprime called ";
char errmsg[]="Invalid conditional statement";
passspnew();
//cout<<"X"<<t->name<<"X";

if(strcmp(t->name,"else")==0) asm_match(t->name,errmsg);
else return;

asm_statement();

}//dprime() close


void asm_decision_statement(){
//cout<<"decision statement called ";
char errmsg[]="Invalid decision statement";
asm_match(t->name,errmsg);

passspnew();

asm_match("(","( expected");

node* op=asm_expn();

cout<<"cmp ax,bx\n";
if(op->name[0]=='>') cout<<"jle dec_label"<<dec_label_count<<endl;
else if(op->name[0]=='<') cout<<"jge loop_label"<<++dec_label_count<<endl;
else if(strcmp(op->name,">=")==0) cout<<"jl loop_label"<<++dec_label_count<<endl;
else if(strcmp(op->name,"<=")==0) cout<<"jg loop_label"<<++dec_label_count<<endl;
else if(strcmp(op->name,"==")==0) cout<<"jne loop_label"<<++dec_label_count<<endl;
else if(strcmp(op->name,"!=")==0) cout<<"je loop_label"<<++dec_label_count<<endl;

asm_match(")","( expected");

asm_statement(); //statement inside if condition
//match(";",errmsg);

passspnew();
cout<<"jmp dec_label"<<++dec_label_count<<endl;

cout<<"dec_label"<<dec_label_count-1<<":\n";

asm_dprime();

cout<<"dec_label"<<dec_label_count<<":\n";
dec_label_count++;
}//decision_statement close

void asm_assignloop_statement(){
//cout<<"assignloop_statement called ";
char errmsg[]="Incorrect assignment";
asm_match(t->name,errmsg);

//t=lex(f1);
passspnew();

asm_match("=",errmsg);
passspnew();

asm_expn();

//match(";",errmsg);
}


void asm_loopfactor(){
char errmsg[]="Identifier or number expected";
passspnew();

for(int i=0;i<strlen(t->name);i++)
cout<<t->name[i];

asm_match(t->name,errmsg);

}//asm_loopfactor() close


node* asm_loopeprime(){

//cout<<"eprime called ";
char errmsg[]="Unsupported relational operator";
passspnew();
node* op=NULL;

if(strcmp(t->name,"!=")==0){ op=t; asm_match(t->name,errmsg);          }
else if(strcmp(t->name,"==")==0){ op=t; asm_match(t->name,errmsg);    }
else if(strcmp(t->name,"<=")==0){ op=t; asm_match(t->name,errmsg);   }
else if(strcmp(t->name,">=")==0){ op=t; asm_match(t->name,errmsg);  }
else if(strcmp(t->name,">")==0){ op=t; asm_match(t->name,errmsg);  }
else if(strcmp(t->name,"<")==0){ op=t; asm_match(t->name,errmsg); }
//if(strcmp(t->name,"!=")==0) match(t->name,errmsg);

else return op;

asm_loopfactor();
return op;
}//asm_loopeprime() close




node* asm_loopexpn(){
//cout<<"expn called ";
//cout<<"expn called\n";
passspnew();

asm_loopfactor();
cout<<",";
node* op=asm_loopeprime();

return op;
//passspnew();

}//asm_loopexpn() close




void asm_looping_statement(){
//cout<<"looping statement called ";
char errmsg[]="Invalid looping statement";

if(strcmp(t->name,"while")==0){
asm_match(t->name,errmsg);

asm_match("(","( expected");

//node* op=asm_loopexpn();

//cout<<"push ax\n";
//cout<<"push bx\n";
cout<<"looplabel"<<loop_label_count<<":\n";
//cout<<"cmp ax,bx\n";

cout<<"cmp ";
node* op=asm_loopexpn();
cout<<endl;

if(op->name[0]=='>') cout<<"jle loop_label"<<++loop_label_count<<endl;
else if(op->name[0]=='<') cout<<"jge loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,">=")==0) cout<<"jl loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"<=")==0) cout<<"jg loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"==")==0) cout<<"jne loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"!=")==0) cout<<"je loop_label"<<++loop_label_count<<endl;

asm_match(")",") expected");

asm_statement();

//cout<<"pop bx\n";
//cout<<"pop ax\n";
cout<<"jmp loop_label"<<loop_label_count-1<<endl;

}

else if(strcmp(t->name,"for")==0){
asm_match(t->name,errmsg);

asm_match("(","( expected");

asm_assign_statement();

cout<<"looplabel"<<loop_label_count<<":\n";
cout<<"cmp ";
node* op=asm_loopexpn();
cout<<endl;

if(op->name[0]=='>') cout<<"jle loop_label"<<++loop_label_count<<endl;
else if(op->name[0]=='<') cout<<"jge loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,">=")==0) cout<<"jl loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"<=")==0) cout<<"jg loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"==")==0) cout<<"jne loop_label"<<++loop_label_count<<endl;
else if(strcmp(op->name,"!=")==0) cout<<"je loop_label"<<++loop_label_count<<endl;

asm_match(";","; expected");

asm_assignloop_statement();

asm_match(")",") expected");

asm_statement();
cout<<"jmp loop_label"<<loop_label_count-1<<endl;

cout<<"loop_label"<<loop_label_count<<":\n";
}

}//asm_looping_statement() close



void asm_statement(){

//cout<<"statement called ";
//char errmsg[]="Invalid statement";
passspnew();

if(t->type=='i'){ //assign statement
asm_assign_statement();
}//assign_statement close


else if(strcmp(t->name,"if")==0){ //decision statement
asm_decision_statement();
}//decision statement close


else if(strcmp(t->name,"while")==0 || strcmp(t->name,"for")==0){
asm_looping_statement();
}//looping statement close

//else error(errmsg);

}//asm_statement() close


void asm_statement_list(){

passspnew();

if(t->type=='i' || strcmp(t->name,"if")==0 || strcmp(t->name,"for")==0 || strcmp(t->name,"while")==0){
asm_statement();
//match(";",errmsg);
asm_statement_list();
}
else return;

}//asm_statement_list() close


void asm_id_list(){
//cout<<"id_list called ";
passspnew();
char errmsg[]="Declaration syntax error";

if(t->datatype=='i' | t->datatype=='c'){
node* tt=t;
cout<<tt->name;
if(tt->datatype=='i') cout<< " DW 0\n";
else if(tt->datatype=='c') cout<<" DB 0\n";

asm_match(t->name,errmsg);

if(strcmp(t->name,",")==0){

asm_match(t->name,errmsg);

asm_id_list();

}//comma after id close

else{ //id_list ends with a ;
asm_match(";",errmsg);
//else error(errmsg);
}//else close

}
//else asm_error(errmsg); //corresponds to first token not being an identifier after "int "

}//id_list() close


void asm_declarations(){

char errmsg[]="Incorrect declaration";
passspnew();

if(strcmp(t->name,"int")==0){ asm_match(t->name,errmsg);}
else if(strcmp(t->name,"char")==0){ asm_match(t->name,errmsg);}
else{
cout<<"data ends\n";
return;}
passspnew(); //int a,b; (space required after datatype)

asm_id_list();

asm_declarations();

}//asm_declarations() close


void asm_program(){

char errmsg[]="Syntax error\n";

asm_match("main",errmsg);
asm_match("(",errmsg);
asm_match(")",errmsg);
asm_match("{",errmsg);

cout<<"data segment\n";

asm_declarations();

cout<<"code segment\nassume cs:code, ds:data\n";
cout<<"start:\nmov ax,data\nmov ds,ax\n";

asm_statement_list();
cout<<"mov ah,4ch\n";
cout<<"int 21h\n";
cout<<"code ends\n";
cout<<"end start\n";

}//asm_program() close

int gencode(){
linerr=1;
f1=fopen(output,"r");
t=lex(f1);

asm_program();
fclose(f1);
return 0;
}//gencode() close

void main(){
clrscr();
cout<<"Warning:This overwrites the source file!\n";
f=fopen(name,"r");

if(f==NULL){
cout<<"Unable to open file/invalid file name\n";
getch();
exit(1);}

removecomments(f);
overwrite(); //overwrites the source with the compiled changes
cout<<"Comments removed\n";
removelines();
cout<<"Extra lines removed\n";
overwrite();
removespaces(); //check for compatibility with looping statements --done
cout<<"Extra spaces removed\n";
overwrite();
cout<<"Source file overwritten\n";
inittable();
cout<<"Symbol table initialized\n";
listtokens();
listkeywords();
listoperators();
listidentifiers();
cout<<"Lexical analysis complete. Press any key to generate symbol table\n";
getch();
populatetable(); //populate symbol table with tokens by reading files
identifydatatypes(); //find datatype of identifiers and update their type & size
//displaytable();
cout<<endl<<endl;
int ret=parser();
//cout<<"Return value of parser:"<<ret<<endl<<endl;
gencode();
getch();
}//main() close
