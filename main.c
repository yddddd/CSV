//
// Created by Administrator on 2022/12/4.
//
#include <stdio.h>
#include <stdlib.h>

typedef enum CSV_ENCLOSURE_STATUS_ENUM
{
    CSV_ENCLOSURE_NONE,
    CSV_ENCLOSURE_ENTER,
    CSV_ENCLOSURE_EXIT
}CSV_ENCLOSURE_STATUS_ENUM;

#define DELIMITER_CHAR ','
#define ENCLOSURE_CHAR '"'
#define CSV_MAX_LINE 512

typedef struct csv_string
{
    char buf[52];
    short len;
}pcsv_string;

typedef struct user
{
    pcsv_string family[5];
    int pos;
}puser;

void push_string (char ch, pcsv_string *str)
{
    if (str->len > 50) {
        return;
    }

    str->buf[str->len] = ch;
    str->len++;
    return;
}
CSV_ENCLOSURE_STATUS_ENUM StateAction(char ch, CSV_ENCLOSURE_STATUS_ENUM quote_status, pcsv_string *str, puser *user)
{
    if ((DELIMITER_CHAR == ch) && (CSV_ENCLOSURE_ENTER != quote_status)) {
        memcpy((char *)(&(user->family[user->pos])), str->buf, str->len);
        user->family[user->pos].len = str->len;
        memset(str, 0, sizeof(pcsv_string));
        if((++(user->pos)) > 4) {
            return -1;
        }
        return CSV_ENCLOSURE_NONE;
    }

    if (ENCLOSURE_CHAR == ch) {
        if (CSV_ENCLOSURE_EXIT == quote_status) {
            push_string(ch, str);
            return CSV_ENCLOSURE_ENTER;
        } else {

            /*CSV_ENCLOSURE_NONE -> CSV_ENCLOSURE_ENTER
                     CSV_ENCLOSURE_ENTER -> CSV_ENCLOSURE_EXIT*/
            return quote_status + 1;
        }
    }

    push_string(ch, str);
    return quote_status % CSV_ENCLOSURE_EXIT;
}


static int my_fgets(char *string, int n, FILE* stream)
{
    int i;
    char ch;

    for (i = 0; i < n - 1; i++) {
        if (fread((void *) &ch, 1, 1, stream) != 1) {
            return 0;
        }
        *(string + i) = (char) ch;

        if (ch == '\n') {
            i++;
            break;
        }
    }

    *(string + i) = '\0';

    return i;
}

void ParseRecord()
{
    CSV_ENCLOSURE_STATUS_ENUM quote_status= CSV_ENCLOSURE_NONE;
    struct csv_string str;
    struct user filed;
    FILE *fp;
    int count, i;

    char line[CSV_MAX_LINE];
    fp = fopen("test_data.csv", "r");
    if(fp ==NULL ) {
        return;
    }
    //if (my_fgets(line, CSV_MAX_LINE, fp) == 0) {
    //   fclose(fp);
    //  return;
    //}

    memset(line, 0, CSV_MAX_LINE);
    int fistline=0;//记录recor的行数
    while ((count = my_fgets(line, CSV_MAX_LINE, fp)) != 0) {
        if(CSV_ENCLOSURE_ENTER != quote_status){//若循环时，quote_status为ENTER
            //状态，说明在引号内出现了换行，这时一条记录还没完成，需要继
            //续读，故此时不能清空record（user结构体）和缓冲区（string结构体）
            memset(&filed, 0, sizeof(struct user));
            memset(&str, 0, sizeof(struct csv_string));
            quote_status = CSV_ENCLOSURE_NONE;
        }
        for (i = 0; i < count; i++) {
            quote_status = StateAction(line[i], quote_status, &str, &filed);
            if((i==count-1)&& quote_status != CSV_ENCLOSURE_ENTER){//但是该记录最后一个逗号后的内容无法存储到记录中。
                //可在该行循环到末尾时判断，若不在ENTER状态，则
                //可确定该条记录已经结束，将缓冲区内容压入record中。
                memcpy((char *)(&(filed.family[filed.pos])), str.buf, str.len);
                filed.family[filed.pos].len = str.len;
                memset(&str, 0, sizeof(pcsv_string));
                if((++(filed.pos)) > 4) {
                    quote_status= -1;
                }

            }
            if (quote_status == -1) {
                break;
            }
        }

        if (CSV_ENCLOSURE_ENTER != quote_status) {  /*Record finished*/
            if(fistline==0) fistline++;//变量 fistline记录recor的行数
            else
                printf("Record%d\n %s || %s || %s \n", fistline++,filed.family[0].buf, filed.family[1].buf, filed.family[2].buf);

        }

    }

    fclose(fp);
    return;
}

int main()
{
    printf("RecordNumber:");
    ParseRecord();
    char a[10];
    scanf("%s",a);
    return 0;
}

