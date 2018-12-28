#include "stdio.h"
#include "stdint.h"
#include "string.h"

#define CONFIG_LINE_SIZE    1024

uint8_t save_file_dir[50];



int find_number_symbol_next(char *str, int number, char symbol)
{
	char *_buff_ptr = str;
	unsigned int _symbol_count = 0;

	do{
		if(((*str != '\0') || (*str != '\n')|| (*str != '\r')) && (str - _buff_ptr < CONFIG_LINE_SIZE))
		{
			
			if(*str == symbol)
			{
				
				_symbol_count++;
			}
		
			++str;
		
		}
		else
		{
		
			return -1;
		}
	}while(_symbol_count<number);

	
	return str - _buff_ptr;
}




void save_string_to_file(uint8_t *dir, uint8_t *string)
{
	
	FILE *fp = fopen(dir,"a");
	if(NULL == fp)//打开配置文件失败
		return ;
	fputs((const void *)string, fp);

	fclose(fp);
	
}

void rent_get_config(uint8_t *dir)
{
	int ret = 0;
	
	FILE *fp = NULL;
	uint8_t strline[CONFIG_LINE_SIZE];
	
	fp=fopen(dir,"r");
	if(NULL == fp)//打开配置文件失败
		return ;
	
	while(!feof(fp)){
	
		memset(strline, 0, CONFIG_LINE_SIZE);
	
		fgets(strline,CONFIG_LINE_SIZE,fp);

		if( strncmp(strline, "$GNRMC", 6) == 0)
		{
		
			ret = find_number_symbol_next(strline, 2, ',');
			
			if(ret == -1)
			{
				continue;
			}
			else
			{
				if(strline[ret] == 'A')
				{
					ret = find_number_symbol_next(strline, 9, ',');
					if(ret == -1)
					{
						continue;
					}
					else
					{
						if((ret + 6 < CONFIG_LINE_SIZE) && (strncmp(strline+ret, "261218", 6) !=0) && (strncmp(strline+ret, "271218", 6) !=0) )
						{
							printf("%s", strline);
							save_string_to_file(save_file_dir, strline+ret);
							save_string_to_file(save_file_dir, strline);
						}
					}
				}
			}
		}
		else if(strline[0] != '$')
		{
			printf("%s", strline);
			save_string_to_file(save_file_dir, strline);
		}
	}
	
	fclose(fp);
}

int main(int args, char **argv)
{
	memset(save_file_dir, 0, sizeof(save_file_dir));

	strcpy(save_file_dir, argv[2]);

	rent_get_config(argv[1]);

	return 0;
}
