#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

const char format_arg_pattern[] = {
"^%"
"([-+ #0]+)?"           // Flags (Position 1)
"([\\d]+|\\*)?"         // Width (Position 2)
"(\\.(\\d+|\\*))?"      // Precision (Position 4; 3 includes '.')
"(hh|h|l|ll|j|z|Z|t|L)?"// Length (Position 5)
"([diuoxXfFeEgGaAcspn])"// Specifier (Position 6)
};
char format_string[] = {"Hello, %d %s %*.*s %ld %f %lf %x %lx.\n"};

//TODO 2021年6月11日 荣涛 匹配 printf 格式化字符串
//

int main(void)
{
	int status = 0, i = 0;
	int flag = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
    
	const char *pattern = "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$";
	char *buf = "123456789@qq.com";//success

	regcomp(&reg, pattern, flag);
    
	status = regexec(&reg, buf, nmatch, pmatch, 0);
    
	if(status == REG_NOMATCH) {
        
		printf("no match\n");
        
	}else if(status == 0){
	
		printf("match success\n");

        for(i = pmatch[0].rm_so; i < pmatch[0].rm_eo; i++){
			putchar(buf[i]);
		}
		putchar('\n');
	}
	regfree(&reg);

	return 0;
}

