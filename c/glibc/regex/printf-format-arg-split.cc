#include <algorithm>

#include <bits/algorithmfwd.h>
#include <regex>
#include <vector>

#include <stdio.h>

/**
 * Describes how to interpret the dynamic log stream and partially
 * reconstruct the original log message.
 */
struct PrintFragment {
    // The type of the argument to pull from the dynamic buffer to the
    // partial format string (formatFragment)
    uint8_t argType:5;

    // Indicates that the fragment requires a dynamic width/precision
    // argument in addition to one required by the format specifier.
    bool hasDynamicWidth:1;
    bool hasDynamicPrecision:1;

    //TODO(syang0) is this necessary? The format framgnet is null-terminated
    // Length of the format fragment
    uint16_t fragmentLength;

    // A fragment of the original LOG statement that contains at most
    // one format specifier.
    char formatFragment[];
} __attribute__((packed));


int main()
{
    size_t i = 0;
    int consecutivePercents = 0;
    std::cmatch match;
    PrintFragment *pf = nullptr;

    char formatString[] = {"Hello, %d %s %*.*s %ld %f %lf %x %lx.\n"};
    
    size_t formatStringLength = strlen(formatString) + 1; // +1 for NULL
    
    std::regex regex("^%"
                     "([-+ #0]+)?" // Flags (Position 1)
                     "([\\d]+|\\*)?" // Width (Position 2)
                     "(\\.(\\d+|\\*))?"// Precision (Position 4; 3 includes '.')
                     "(hh|h|l|ll|j|z|Z|t|L)?" // Length (Position 5)
                     "([diuoxXfFeEgGaAcspn])"// Specifier (Position 6)
                     );
    
    while (i < formatStringLength) {
        
        char c = formatString[i];
        
        // Skip the next character if there's an escape
        if (c == '\\') {
            i += 2;
            continue;
        }
        
        if (c != '%') {
            ++i;
            consecutivePercents = 0;
            continue;
        }
        
        if (++consecutivePercents % 2 == 0
                || !std::regex_search(formatString + i, match, regex))
        {
            ++i;
            continue;
        }
        
        // Advance the pointer to the end of the specifier
        i += match.length();

        
        std::string width = match[2].str();
        std::string precision = match[4].str();
        std::string length = match[5].str();
        char specifier = match[6].str()[0];
        
        if (width.begin() != width.end()) { // 确保s非空
            auto it = width.begin();    // it表示s的第一个字符
            printf("%c", it);
        }
        if (precision.begin() != precision.end()) { // 确保s非空
            auto it = precision.begin();    // it表示s的第一个字符
            printf("%c", it);
        }

    //TODO  rtoax  2021年6月11日
//        printf("%c%c%c%c\n", width[0], precision[0], length[0], specifier);
    }               
}
