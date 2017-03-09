/*
h-fomat.c


format:
struct bin
{
	struct block_head
	{
		char start_flag[4];
		....
	}
	struct block_item[]
	{
		char item1[4];
		char item2[4];
		...
	}
	....
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ITEM_LENGTH_TYPE_SINGLE    1   //单节点
#define ITEM_LENGTH_TYPE_LIMITED   2   //有限数量节点
#define ITEM_LENGTH_TYPE_UNLIMITED   3   //循环节点

#define ITEM_STATUS_INIT 0
#define ITEM_STATUS_TYPE_START 1
#define ITEM_STATUS_TYPE_END 2
#define ITEM_STATUS_NAME_START 3
#define ITEM_STATUS_NAME_END 4
#define ITEM_STATUS_CLOSE	5

struct item_t
{
	int length;			//
	int status;			//0:init 1:start 2:type end 3:name_start 4:name end 5:close
	int length_type;	//
	int byte_num;
	int offset;
	char type_name[32];
	char var_name[32];

	struct item_t *next;
	struct item_t *parent;
	struct item_t *son;
	struct item_t *pre;
};

static struct item_t g_root_item;
static char seperators[]={' ','	',0xa,0xd,0x0};
static char *type_array[]={"char","short","int",NULL};
static char start_end_char[]={'{','}',';',0x0};
/******************************   local function ************************************/

static void dump_item(struct item_t *item){
	printf("***************************\n");
	printf("item->length:%d \n",item->length);
	printf("item->byte_num:%d \n",item->byte_num);
	printf("item->offset:%d \n",item->offset);
	printf("item->type_name:%s \n",item->type_name);
	printf("item->status:%d \n",item->status);
	printf("item->name:%s \n",item->var_name);
	printf("***************************\n\n");
}

static void printf_space(int num)
{
	if(num<=0)
		return ;
	
	int i=0;
	for(i=0;i<num;i++)
		printf(" ");
}

static void printf_son_prefix(int level)
{
	char *son_prefix="|    ";
	if(level<=0)
		return ;
	
	int i=0;
	for(i=0;i<level;i++)
		printf("%s",son_prefix);
}

static void dump_item_level(int level,struct item_t *item){
	char att_prefix[]={'|','-','-',0x0};
	char son_prefix[]={'|',' ',' ',0x0};
	
	int space_num=0;
	printf_space((level>0?1:0)*5);printf_son_prefix(level-1);
	printf("%s name:%s level:%d addr:0x%08x\n",att_prefix,item->var_name,level,item);
	
	if(item->next == NULL && item->son == NULL)
		space_num=2;
	
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("%s length:%d \n",att_prefix,item->length);
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("%s byte_num:%d \n",att_prefix,item->byte_num);
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("%s offset:%d \n",att_prefix,item->offset);
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("%s type_name:%s \n",att_prefix,item->type_name);
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("%s status:%d \n",att_prefix,item->status);
	printf_space(5);printf_son_prefix(level+1-space_num);printf_space(5*space_num);
	printf("\n");
}

static int is_seperator(char ch)
{
	
	int i=0;
	while(seperators[i] != 0x0){
		if(seperators[i] == ch){
			return 1;
		}
		i++;
	}
	return 0;
}

static int is_start_end(char ch)
{
	
	int i=0;
	while(start_end_char[i] != 0x0){
		if(start_end_char[i] == ch){
			return 1;
		}
		i++;
	}
	return 0;
}

/*
brief:s_parse_length
return:
	0:means unlimit or dynamic length,for example: char var[];
	1:single item,for example: int lenght
	>1:limited item, for example: char name[16];
	-1:not find item length.
*/
static int s_parse_length(char *name)
{
	//printf("%s %d name:%s\n",__FUNCTION__,__LINE__,name);
	
	int len=strlen(name);
	char buffer[32]={0};
	int flag=0;
	int i=0;
	int pos=0;
	
	//1.search for '[' and ']'
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
	//printf("%s %d flag:%d buffer:%s\n",__FUNCTION__,__LINE__,flag,buffer);
	
	//2.get lenght charatistic.
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
	return -1;
}

int s_get_type_length(char *type_name)
{
	int i=0;
	int len=1;
	while(type_array[i]!=NULL){
		if(strcmp(type_array[i],type_name)==0){
			break;
		}
		len *= 2;
		i++;
	}
	return len;
}

static struct item_t *create_a_son(struct item_t *parent)
{
	struct item_t *son=NULL;
	//dump_item(parent);
	son=(struct item_t *)malloc(sizeof(struct item_t));
	if(son !=NULL){
		memset(son,0,sizeof(struct item_t));
		parent->son=son;
		son->offset=parent->offset;
		son->parent=parent;
	}
	return son;
}

static struct item_t *create_a_brother(struct item_t *brother)
{
	struct item_t *new_brother=NULL;
	//dump_item(brother);
	
	new_brother=(struct item_t *)malloc(sizeof(struct item_t));
	if(new_brother!=NULL){
		memset(new_brother,0,sizeof(struct item_t));
		brother->next=new_brother;
		new_brother->pre=brother;
		new_brother->parent=brother->parent;
		new_brother->offset = brother->offset + brother->byte_num;
	}
	//printf("%s[0x%08x] add a brother 0x%08x\n",brother->var_name,brother,new_brother);
	return new_brother;
}

static void releae_a_brother(struct item_t *brother)
{
	if(brother!=NULL){
		if(brother->pre!=NULL)
			brother->pre->next=NULL;
		memset(brother,0,sizeof(struct item_t));
	}
	free(brother);
}

/*
brief:

*/
static int parse(char *format_data,struct item_t *root_item)
{
	int ret=0;
	int i=0;
	int pos=0; //record the word length.
	int len=strlen(format_data);
	
	struct item_t *current_item=root_item;
	
	while(i<len){
		//printf("%s %d new_item->flag:%d i:%d pos:%d format_data[i]:%c-0x%02x \n",__FUNCTION__,__LINE__,current_item->status,i,pos,format_data[i],format_data[i]);
		if(ITEM_STATUS_INIT == current_item->status)
		{
			if(1 == is_seperator(format_data[i])){
				i++;
				continue;
			}
			if(1 == is_start_end(format_data[i])){
				if(current_item->parent == NULL || format_data[i] != '}')
				{
					ret=i;
					break;
				}
				struct item_t *brother=current_item;
				current_item = current_item->parent;
				releae_a_brother(brother);
				continue;
			}
			current_item->status=ITEM_STATUS_TYPE_START;
			pos=0;
		}
		if(ITEM_STATUS_TYPE_START == current_item->status)
		{
			if(1 == is_start_end(format_data[i])){
				break;
				ret=i;
			}
			if(0 == is_seperator(format_data[i])){
				current_item->type_name[pos]=format_data[i];
				//printf("%s %d current_item->type_name:%s\n",__FUNCTION__,__LINE__,current_item->type_name);
				pos++;
				i++;
				continue;
			}
			//printf("%s %d current_item->type_name:%s\n",__FUNCTION__,__LINE__,current_item->type_name);
			current_item->status=ITEM_STATUS_TYPE_END;
			pos=0;
		}
		if(ITEM_STATUS_TYPE_END == current_item->status)
		{
			if(1 == is_seperator(format_data[i])){
				i++;
				continue;
			}
			if(1 == is_start_end(format_data[i])){
				break;
				ret=i;
			}
			current_item->status=ITEM_STATUS_NAME_START;
		}
		if(ITEM_STATUS_NAME_START == current_item->status)
		{
			if(1 == is_start_end(format_data[i])){
				current_item->length=s_parse_length(current_item->var_name);
				current_item->status=ITEM_STATUS_NAME_END;
				pos=0;
				continue;
			}
					
			if(0 == is_seperator(format_data[i])){
				current_item->var_name[pos]=format_data[i];
				pos++;
				i++;
				continue;
			}
			current_item->length=s_parse_length(current_item->var_name);
			current_item->status=ITEM_STATUS_NAME_END;
			pos=0;
		}
		if(ITEM_STATUS_NAME_END == current_item->status)
		{
			//is there a son?
			if('{' == format_data[i]){
				current_item=create_a_son(current_item);
			}
			
			//is there a struct end?
			if('}' == format_data[i] ){
				//is root item
				if(current_item == root_item){
					break;
				}
			}
			if(';' == format_data[i])
			{
				current_item->byte_num=current_item->length * s_get_type_length(current_item->type_name);
				if(current_item->parent != NULL){
					//printf("%s %d current_item->parent->byte_num:%d\n",__FUNCTION__,__LINE__,current_item->parent->byte_num);
					current_item->parent->byte_num += current_item->byte_num;
					//printf("%s %d current_item->parent->byte_num:%d\n",__FUNCTION__,__LINE__,current_item->parent->byte_num);
					//dump_item(current_item->parent);
				}
			}
			if('}' == format_data[i]){
				if(current_item->parent != NULL){
					current_item->parent->byte_num += (current_item->byte_num * current_item->length);
					//dump_item(current_item->parent);
				}
			}
			//is there the end of the item member?
			if('}' == format_data[i]  || ';' == format_data[i]){
				current_item->status = ITEM_STATUS_CLOSE;
				current_item=create_a_brother(current_item);
			}
			i++;
			continue;
		}
	}
	
	return ret;
}

static void traversal(int level,struct item_t *item)
{
	dump_item_level(level,item);
	if(item->son!=NULL){
		traversal(level+1,item->son);
	}
	if(item->next!=NULL){
		traversal(level,item->next);
	}
}

/******************************   public function ************************************/
int my_read_format(char *format_data)
{
	int ret=0;
	memset(&g_root_item,0,sizeof(struct item_t));
	parse(format_data,&g_root_item);
	traversal(0,&g_root_item);
	return ret;
}


void output_with_format(char *buffer,int offset)
{
	
}
