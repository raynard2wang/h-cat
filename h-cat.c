/*

format:
<head>
</head>
<body>
	<para>
		<item></item>
	</para>
</body>
<tail>
</tail>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_MAX_SIZE 1024


#define ITEM_TYPE_SINGLE    1   //单节点
#define ITEM_TYPE_LIMITED   2   //有限数量节点
#define ITEM_TYPE_UNLIMITED   3   //循环节点

#define PARAGRAPH_ITEM

#if 1
	#define GO_TO_SON printf("go to son!\n");
	#define GO_TO_PARENT printf("go to parent!\n");
	#define GO_TO_BROTHER printf("go to brother!\n");
	#define FIND_NEW_ITEM printf("FIND new item!\n");
#else
	#define GO_TO_SON
	#define GO_TO_PARENT 
	#define GO_TO_BROTHER 
	#define FIND_NEW_ITEM 
#endif

struct item_t
{
	int length;
	int offset;
	char  type_name[32];
	int item_type; 
	int flag; //0:init 1:start 2:type end 3:name_start 4:name end 5:close
	char name[32];
	struct item_t *next;
	struct item_t *parent;
	struct item_t *son;
	struct item_t *pre;
};

typedef struct input_para_t
{
	char *hex_filename;
	char *format_filename;
}input_para,*p_input_para;

static input_para s_para;
static struct item_t g_item_root;


void dump_item(struct item_t *item){
	printf("%s %d item->length:%d \n",__FUNCTION__,__LINE__,item->length);
	printf("%s %d item->offset:%d \n",__FUNCTION__,__LINE__,item->offset);
	printf("%s %d item->type_name:%s \n",__FUNCTION__,__LINE__,item->type_name);
	printf("%s %d item->item_type:%d \n",__FUNCTION__,__LINE__,item->item_type);
	printf("%s %d item->flag:%d \n",__FUNCTION__,__LINE__,item->flag);
	printf("%s %d item->name:%s \n",__FUNCTION__,__LINE__,item->name);
}

static int get_length(char *name)
{
	printf("%s %d name:%s\n",__FUNCTION__,__LINE__,name);
	
	int len=strlen(name);
	char buffer[32]={0};
	int flag=0;
	int i=0;
	int pos=0;
	
	while(i<len){
		if(flag == 0){
			if(name[i] =='['){
				flag=1;
			}
			
			i++;
			continue;
		}
		if(flag==1){
			if(name[i] == ']'){
				flag=2;
				break;
			}
			buffer[pos] = name[i];
			pos++;
			i++;
			continue;
		}
	}
	printf("%s %d flag:%d buffer:%s\n",__FUNCTION__,__LINE__,flag,buffer);
	
	if(flag == 0){
		return 1;
	}
	if(flag==1){
		return -1;
	}
	if(flag== 2){
		if(pos==0){
			return 0;
		}
		return atoi(buffer);
	}
	return -2;
}

static int parse(char *format_buffer,struct item_t *item)
{
	int ret=0;
	int i=0;
	int len=strlen(format_buffer);
	
	//init root
	struct item_t *current_item=item;
	memset(current_item,0,sizeof(struct item_t ));
	
	struct item_t *new_item=(struct item_t *)malloc(sizeof(struct item_t));
	memset(new_item,0,sizeof(struct item_t));
	int pos=0;
	
	while(i<len)
	{
		printf("%s %d new_item->flag:%d i:%d format_buffer[i]:%c-0x%02x\n",__FUNCTION__,__LINE__,new_item->flag,i,format_buffer[i],format_buffer[i]);
		if(new_item->flag==0){
			if(format_buffer[i] == '}'){
				dump_item(current_item);
				if(memcmp(current_item->type_name,"struct",6)==0){
					if(current_item->flag!=5){
						current_item->flag=5;
					}else{
						current_item=current_item->parent;
						current_item->flag=5;
						GO_TO_PARENT;
					}
				}else{
					current_item=current_item->parent;
					current_item->flag=5;
					GO_TO_PARENT;
				}
				if(current_item == item){
					break;
				}
				i++;
				continue;
			}
			if(format_buffer[i] == ' '
				 ||  format_buffer[i] == '\t'
				 || format_buffer[i] == 0x9
				 || format_buffer[i] == 0x11
				 || format_buffer[i] == 0xa 
				 || format_buffer[i] == 0xd ){
				i++;
				continue;
			}
			new_item->type_name[pos]=format_buffer[i];
			pos++;
			i++;
			new_item->flag=1;
			continue;
		}
		if(new_item->flag == 1){
			if(format_buffer[i] != ' '
				 &&  format_buffer[i] != '\t'
				 && format_buffer[i] != 0xa 
				 && format_buffer[i] != 0xd )
			{
				new_item->type_name[pos]=format_buffer[i];
				pos++;
				i++;
				continue;
			}
			i++;
			new_item->flag=2;
			pos=0;
			continue;
		}
		if(2 == new_item->flag){
			if(format_buffer[i] == ' '
				 || format_buffer[i] == 0x9
				 || format_buffer[i] == 0x11
				 || format_buffer[i] == 0xa 
				 || format_buffer[i] == 0xd){
				i++;
				continue;
			}
			new_item->name[pos]=format_buffer[i];
			pos++;
			i++;
			new_item->flag=3;
			continue;
		}
		if(3 == new_item->flag){
			if(format_buffer[i] != ' '
				 &&  format_buffer[i] != '\t'
				 &&  format_buffer[i] != 0x9
				 &&  format_buffer[i] != 0x11
				 && format_buffer[i] != 0xa 
				 && format_buffer[i] != 0xd 
				 && format_buffer[i] !='{' 
				 && format_buffer[i] !=';'){
				new_item->name[pos]=format_buffer[i];
				pos++;
				i++;
				continue;
			}
			pos=0;
			new_item->flag=4;
			continue;
		}
		if(4 == new_item->flag)
		{
			int length=get_length(new_item->name);
			if(length == 0){
				new_item->item_type = ITEM_TYPE_UNLIMITED;
				new_item->length = 0;
			}else if(length == 1){
				new_item->item_type = ITEM_TYPE_SINGLE;
				new_item->length = 1;
			}else{
				new_item->item_type = ITEM_TYPE_LIMITED;
				new_item->length = length;
			}
			
			if(format_buffer[i] == '{'){
				//insert to tree
				current_item->son=new_item;
				new_item->parent=current_item;
				current_item=new_item;
				
				new_item=(struct item_t *)malloc(sizeof(struct item_t));
				memset(new_item,0,sizeof(struct item_t));
				
				dump_item(current_item);
				GO_TO_SON;
				continue;
			}
			if(format_buffer[i] == ';'){
				new_item->flag=5;
				continue;
			}
			i++;
			continue;
		}
		if(5 == new_item->flag){
			if(memcmp(current_item->type_name,"struct",6)==0){
				current_item->son=new_item;
				new_item->parent=current_item;
			}else{
				current_item->next=new_item;
				GO_TO_BROTHER;
			}
			
			current_item=new_item;
			new_item=(struct item_t *)malloc(sizeof(struct item_t));
			memset(new_item,0,sizeof(struct item_t));
			
			dump_item(current_item);
			i++;
			
			
			continue;
		}		
	}
	return ret;
}

static void output_hex(char *buffer, int len)
{
	printf("******** block start ******************\n");
	int i=0;
	for(i=0;i<len;i++){
		printf("%02x ",buffer[i]);
		if(i % 16 == 15 ){
			printf("\n");
		}
	}
	if(i % 16 != 15)
		printf("\n");
	
	printf("******** block end   ******************\n");
}

static void read_format(char *filename, struct item_t* item_root)
{
	int ret=0;
	char buffer[1024]={0};
	
	memset(item_root,0,sizeof(struct item_t));
	
	FILE *fp=fopen(filename,"rb");
	if(fp==NULL){
		printf("file:%s doesn't exist!\n",s_para.hex_filename);
		return;
	}
	
	ret=fread(buffer,1,1024,fp);
	if(ret>0){
		//parse(buffer,item_root);
		my_read_format(buffer);
	}
	
	fclose(fp);
	return;
}

static int read_parameter(int argc,char *argv[])
{
	int i=0;
	memset(&s_para,0,sizeof(s_para));
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-f")==0){
			i++;
			if(i>=argc){
				break;
			}
			s_para.format_filename=argv[i];
			continue;
		}
		if(strcmp(argv[i],"-i")==0){
			i++;
			if(i>=argc){
				break;
			}
			s_para.hex_filename=argv[i];
			//printf("file:%s i=%d!\n",s_para.hex_filename,i);
			continue;
		}
	}
	//printf("hex_filename file:%s i=%d!\n",s_para.hex_filename,i);
	//printf("format_filename file:%s i=%d!\n",s_para.format_filename,i);
	if(s_para.format_filename == NULL && s_para.hex_filename==NULL)
	{
		return 1;
	}
	return 0;
}

int main(int argc,char *argv[])
{
	char buffer[BLOCK_MAX_SIZE]={0};
	
	//1.read parameter
	int ret=read_parameter(argc,argv);
	if(ret!=0){
		printf("parameter is wrong! The format of this command is like below:\n");
		printf("./%s -f formart_file -i hex_file!\n",argv[0]);
		return -1;
	}
	
	//2.read format file
	read_format(s_para.format_filename,&g_item_root);
	
	//3.read hex data 
	FILE *fp=fopen(s_para.hex_filename,"rb");
	if(fp==NULL){
		printf("file:%s doesn't exist!\n",s_para.hex_filename);
		return 0;
	}
	
	while(!feof(fp)){
		memset(buffer,0,sizeof(buffer));
		ret=fread(buffer,1,1024,fp);
		if(ret>0){
			output_hex(buffer,ret);
		}else{
			break;
		}
	}

	fclose(fp);
	printf("file:%s output end!\n",s_para.hex_filename);
	return 0;
}
