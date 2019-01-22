#include "internal.h"

#define CR	0x0D
#define LF	0x0A

//TODO use strtod
gboolean rayem_parser_read_single_line(FILE *input,GString *str,
		int max_str_len,
		int *eos){
	clearerr(input);
	g_string_truncate(str,0);

	int ret,i=0;
	gboolean exit_ret=TRUE;
	if(eos!=NULL)*eos=0;

	int cr_detected=0;
	while(1){
		uint8_t ch;
		ret=fgetc(input);
		if(feof(input)){
			exit_ret=TRUE;
			if(eos!=NULL)*eos=1;
			goto my_readLine_exit;
		}else if(ferror(input)){
			return FALSE;
		}
		ch=ret;

		if(ch==CR){
			if(cr_detected){
				return FALSE;
			}
			cr_detected=1;
			continue;
		}
		if(ch==LF){
			exit_ret=TRUE;
			goto my_readLine_exit;
		}
		if(cr_detected){
			return FALSE;
		}
		if((max_str_len>0 && i>=max_str_len)){
			return FALSE;
		}
		g_string_append_c(str,ch);
		cr_detected=0;
	}
my_readLine_exit:
	return exit_ret;
}
