#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define my_sizeof(type) (char *)(&type+1)-(char*)(&type)


char * my_strstr(const char * needle, const char* haystack )
{
    int h_ptr = 0;
    int n_ptr = 0;
    int n_len = strlen(needle);
    int h_len = strlen(haystack);
    printf("n_len = %i\n", n_len);
    printf("h_len = %i\n", h_len);

    if (strlen(haystack) < strlen(needle))
    {
        return NULL;
    }
    while (h_ptr < strlen(haystack))
    {
        while ((haystack[h_ptr] != needle[0]) && (h_ptr < h_len))
        {
            h_ptr++; // Seek to next needle[0]
        }

        if (haystack[h_ptr] == needle[0]) // Else if we found a match of the start char, begin matching
        {
            
            // Begin matching
            printf("Begin match at h_ptr[%i]=%c\n", h_ptr, haystack[h_ptr]);
            if ((h_len - h_ptr) < n_len) // If the diff between left-h_len is less than n_len, then we will have no match, return null
            {
                printf("Remaining haystack len of (%i) can't fit in needle len (%i)\n", (h_len - h_ptr), (n_len));
                return NULL;
            }
            n_ptr = 0;
            int mismatch = 0;
            int h_ptr_cpy = h_ptr;
            while (n_ptr < n_len)
            {
                if (needle[n_ptr] == haystack[h_ptr])
                {
                    printf("@%02x match: %c \n", &haystack[h_ptr], needle[n_ptr]);
                    h_ptr++;
                    n_ptr++;
                }
                else
                {
                    printf("@%x mismatch: %c \n", &haystack[h_ptr], needle[n_ptr]);
                    mismatch = 1;
                    break; // When we break, h_ptr is at the location of last match
                }
            }
            if ((n_ptr < n_len-1) || mismatch)// Never fullfilled a full match (broke early)
            {
                // Revert to start, continue loop
                printf("mismatch , n_ptr=%i, h_ptr=%i\n", n_ptr, h_ptr);
                h_ptr = h_ptr_cpy+1; // Restore the haystack index, and incrememnt to next haystack index
                n_ptr = 0;
                continue;
            }
            else
            {
                // Found the substr, return pointer to first occurence
                printf("match , n_ptr=%i, h_ptr=%i\n", n_ptr, h_ptr_cpy);
                char * ret = &haystack[h_ptr_cpy];
                return ret;
            }
        }
        else
        {
            // No other matches of first needle char was found, return null
            return NULL;
        }
    }
    return NULL; // Invalid lengths
}


char * my_strstr2(const char * needle, const char* haystack )
{
    int n_len = strlen(needle);
    int h_len = strlen(haystack);
    for (int i = 0; i <= (h_len-n_len); i++)
    {
        int j = 0;
        for (j = 0; j < n_len; j++)
        {
            if (haystack[i+j] != needle[j])
            {
                break;
            }
        }
        if (j == n_len)
        {
            return &haystack[i];
        }
    }
    return NULL;
}


void * memcpy2(void * dest, void * src, size_t n)
{
    // Cast input to uint8_t*ptr
    char * destPtr = (char*)dest;
    char * srcPtr = (char*)src;
    for (int i = 0; i < n; i++)
    {
        *destPtr = *srcPtr;
        destPtr++;
        srcPtr++;
    }
    return src;
}


void * memcpy_overlap(void * dest, void * src, size_t n)
{
    char temp[1024]; // <-- make some huge buffer
    char * destPtr = (char*)dest;
    char * srcPtr = (char*)src;
    for (int i =0; i <n; i++)
    {
        temp[i] = srcPtr[i];
    }
    for (int i =0; i <n; i++)
    {
        destPtr[i] = temp[i];
    }
    return dest;
}


int testStrStr(void)
{
    char string1[] = "Hello My Name is AAdeello.";
    char string2[] = "Adeel";
    char * ret = my_strstr(string2, string1);
    if (ret)
    {
        printf("Found substr at %x = %s\n", ret, ret);
    }
    else
    {
        printf("did not find substr %s\n", string2);
    }
}


int main(void)
{

    testStrStr();
    return 0;

    unsigned int arr1[10] = {2, 4, 6, 8, 10, 12};
    unsigned int arr2[10] = {1, 1, 1, 1, 1, 1};

    // copy first 4 elements of arr2 into arr1
    unsigned int * retPtr;

    printf("Source array [");
    int i = 0;
    for (i = 0; i < 10-1; i++)
    {
        printf(" %lu,", arr1[i]);
    }
    printf( " %lu ]\n", arr1[i]);

    retPtr = (unsigned int*)memcpy_overlap((void*)&arr1[1], (void*)&arr1[0], 3*(sizeof(unsigned int)));

    printf("Result array [");
    i = 0;
    for (i = 0; i < 10-1; i++)
    {
        printf(" %lu,", arr1[i]);
    }
    printf( " %lu ]\n", arr1[i]);

    return 0;
}