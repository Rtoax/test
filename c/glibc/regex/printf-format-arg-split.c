#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>



//TODO 2021年6月11日 荣涛 匹配 printf 格式化字符串
//
//"^%([-+ #0]+)?([\\d]+|\\*)?(\\.(\\d+|\\*))?(hh|h|l|ll|j|z|Z|t|L)?([diuoxXfFeEgGaAcspn])"
//"^%([-+ #0]+)?([0-9]+|\\*)?(\\.([0-9]+|\\*))?(hh|h|l|ll|j|z|Z|t|L)?([diuoxXfFeEgGaAcspn])"


//https://snyk.io/vuln/SNYK-JS-PRINTF-1072096
//`/\%(?:\(([\w_.]+)\)|([1-9]\d*)\$)?([0 +\-\#]*)(\*|\d+)?(\.)?(\*|\d+)?[hlL]?([\%bscdeEfFgGioOuxX])/g`

const char format_arg_pattern[] = {
    "^%"
    "([-+ #0]+)?"           // Flags (Position 1)
    "([0-9]+|\\*)?"         // Width (Position 2)
    "(\\.([0-9]+|\\*))?"    // Precision (Position 4; 3 includes '.')
    "(hh|h|l|ll|j|z|Z|t|L)?"// Length (Position 5)
    "([diuoxXfFeEgGaAcspn])"// Specifier (Position 6)
};
    
char *format_strings[] = {
    "Hello, %d %s %ld %f %lf %x %lx.\n",
    "Hello, %*.*s  %*.*d %5.3f.\n",
    NULL
};


int test2_printf_format(int idx)
{
	int status = 0, i = 0;
	int flag = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
    
	const char *pattern = format_arg_pattern;
	char *buf = format_strings[idx];//success

    printf("############ TEST %d ############ \n", idx);

	regcomp(&reg, pattern, flag);

    int len = strlen(buf);
    char *fmt = buf;

    while(len-->1) {
        
    	status = regexec(&reg, fmt, nmatch, pmatch, 0);
        
    	if(status == REG_NOMATCH) {
            
//    		printf("no match\n");
            
    	} else if(status == 0) {
    	
    		printf("match success. ");

            for(i = pmatch[0].rm_so; i < pmatch[0].rm_eo; i++) {
    			putchar(fmt[i]);
    		}
    		putchar('\n');
    	}
        fmt++;
        
    }
    
	regfree(&reg);

	return 0;
    
}


int main(void)
{
    int idx = 0;
    
    while(format_strings[idx]) {
        test2_printf_format(idx);
        idx++;
    }
}

