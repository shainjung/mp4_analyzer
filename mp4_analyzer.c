// mp4_analyze.cpp : 定義主控台應用程式的進入點。
//

#include <stdio.h>
#include <string.h>

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

void parse_box(FILE* fp, int indent, unsigned int end_position)
{
    while(!feof(fp))
    {
        unsigned int start_offset;
        unsigned int size;
        unsigned char tmp[5];

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
        fread(tmp, 1, 4, fp);
        tmp[4] = 0;

        char indent_str[256];
        int i;
        for(i=0;i<indent;i++)
            indent_str[i] = '\t';
        indent_str[i] = 0;
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
            fread(uuid, 16, 1, fp);
            printf("%suuid:", indent_str);
            print_uuid(uuid);
            fseek(fp, size-8-16, SEEK_CUR);
        }
        else
            fseek(fp, size-8, SEEK_CUR);
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: mp4_analyzer <mp4 file>\n");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");
    if(fp == NULL)
    {
        printf("Open file %s failed\n", argv[1]);
        return -1;
    }

    parse_box(fp, 0, -1);

	return 0;
}




