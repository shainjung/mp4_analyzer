// mp4_analyze.cpp : print ATOMs in MP4 file.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>

void print_uuid(unsigned char* uuid)
{
    char hex[]="0123456789abcdef";
    int i;
    for(i=0;i<4;i++)
    {
        printf("%c%c", hex[(*uuid)>>4], hex[(*uuid)&0xF]);
        uuid++;
    }
    printf("-");
    for(i=0;i<2;i++)
    {
        printf("%c%c", hex[(*uuid)>>4], hex[(*uuid)&0xF]);
        uuid++;
    }
    printf("-");
    for(i=0;i<2;i++)
    {
        printf("%c%c", hex[(*uuid)>>4], hex[(*uuid)&0xF]);
        uuid++;
    }
    printf("-");
    for(i=0;i<2;i++)
    {
        printf("%c%c", hex[(*uuid)>>4], hex[(*uuid)&0xF]);
        uuid++;
    }
    printf("-");
    for(i=0;i<6;i++)
    {
        printf("%c%c", hex[(*uuid)>>4], hex[(*uuid)&0xF]);
        uuid++;
    }
    printf("\n");
}

void generate_indept_str(char* buffer, int indent)
{
    memset(buffer, ' ', indent*4);
    buffer[indent*4] ='\0';
}

int isfourcc(unsigned char* fourcc)
{
    int i;
    for(i=0;i<4;i++)
    {
        if(isalpha(fourcc[i]) || isdigit(fourcc[i]))
            continue;
        return 0;
    }

    return 1;
}

int guess_box(FILE* fp, unsigned int size)
{
    /* guess atom */
    unsigned char tmp[5];
    unsigned int guess_size;

    if(fread(tmp, 4, 1, fp) <= 0)
        return 0;
    guess_size = tmp[0];
    guess_size = (guess_size<<8) | tmp[1];
    guess_size = (guess_size<<8) | tmp[2];
    guess_size = (guess_size<<8) | tmp[3];

    if(fread(tmp, 4, 1, fp) <= 0)
        return 0;
    tmp[4] = 0;

    fseek(fp, -8, SEEK_CUR);

    if(guess_size < size && isfourcc(tmp))
        return 1;
    else
        return 0;
}

void parse_box(FILE* fp, int indent, unsigned int end_position)
{
    while(!feof(fp))
    {
        unsigned int start_offset;
        unsigned int size;
        unsigned char tmp[5];
        char indent_str[256];

        /* get start offset */
        start_offset = ftell(fp);
        if(end_position > 0 && start_offset >= end_position)
            return;

        /* read size */
        if(fread(tmp, 1, 4, fp) <= 0)
            return;

        size = tmp[0];
        size = (size<<8) | tmp[1];
        size = (size<<8) | tmp[2];
        size = (size<<8) | tmp[3];

        /* read fourcc */
        if(fread(tmp, 1, 4, fp) <= 0)
            return;
        tmp[4] = 0;

        /* indent */
        generate_indept_str(indent_str, indent);

        /* print ATOM */
        printf("%stype:%s\n", indent_str, tmp);
        printf("%ssize:%d\n", indent_str, size);
        printf("%sstart_offset:0x%x\n", indent_str, start_offset);

        /* check all contents */
        if(strcmp((const char*)tmp, "moov") == 0)
            parse_box(fp, indent+1, start_offset + size);
        else if(strcmp((const char*)tmp, "trak") == 0)
            parse_box(fp, indent+1, start_offset + size);
        else if(strcmp((const char*)tmp, "mdia") == 0)
            parse_box(fp, indent+1, start_offset + size);
        else if(strcmp((const char*)tmp, "minf") == 0)
            parse_box(fp, indent+1, start_offset + size);
        else if(strcmp((const char*)tmp, "uuid") == 0)
        {
            unsigned char uuid[16];
            if(fread(uuid, 16, 1, fp) <= 0)
                return;
            printf("%suuid:", indent_str);
            print_uuid(uuid);
            fseek(fp, size-8-16, SEEK_CUR);
        }
        else if(guess_box(fp, size))
        {
            parse_box(fp, indent+1, start_offset + size);
        }
        else
        {
            fseek(fp, size-8, SEEK_CUR);
        }
    }
}

int main(int argc, char* argv[])
{
    FILE* fp;
    if(argc < 2)
    {
        printf("Usage: mp4_analyzer <mp4 file>\n");
        return 0;
    }

    fp = fopen(argv[1], "rb");
    if(fp == NULL)
    {
        printf("Open file %s failed\n", argv[1]);
        return -1;
    }

    parse_box(fp, 0, -1);

	return 0;
}




