#include <sys/mman.h>

char *snortfile = "/usr/local/IDS/snort.conf";
char *rulespath = "/usr/local/IDS/rules/";
char *tmp_file = "/tmp/idsrules.txt";

char *ver_head = "#Version v";
char *rules_head = "include $RULE_PATH/";
char *rules_tail = ".rules";
char *msg_head = "(msg:\"";
char *msg_tail = "\";";
char *rulefile_ext = ".rules";
char *class_head = "classtype:";
char *class_tail = "; ";
char *sid_head = "sid:";
char *sid_tail = "; ";
char *f_start = NULL;

int get_classtype_sid(char *rulePtr, char *classBuf, char *sidBuf)
{
    char *p, *ptr, *ret, *head, *tail;

    *classBuf = '\0';
    *sidBuf = '\0';
    ptr = rulePtr;
    ret = classBuf;
    head = class_head;
    tail = class_tail;

    if ((ptr=strstr(ptr, head)))
    {
        p = strstr(ptr, tail);
        if (p)
        {
            ptr += strlen(head);
            strncpy(ret, ptr, (p-ptr));
            ret[p-ptr] = '\0';
        }
        head = sid_head;
        tail = sid_tail;
        ptr = strstr(ptr, head);
        p = strstr(ptr, tail);
        if (p)
        {
            strncpy(sidBuf, ptr, (p-ptr));
            sidBuf[p-ptr] = '\0';
            //break;
        }
        return 1;
    }

    return 0;
}

char* get_action(char *rulePtr, char *actionBuf)
{
    char *protocol_lists[] = { "ip", "icmp", "tcp", "udp", NULL };
    char *p=NULL, *act, *ptr=rulePtr, Buf[128], *result=NULL;
    int i=0;

    act = actionBuf ? actionBuf : Buf;
    *act = '\0';

    while ((*ptr != 0xa))
    {
        for (i=0; i<4; i++)
        {
            if (!strncmp(ptr, protocol_lists[i], strlen(protocol_lists[i])))
            {
                p = ptr;
                break;
            }
        }
        ptr--;
        if (p)
        {
            while ((*ptr!=0xa)&&(ptr!=f_start))
                ptr--;
            ptr == f_start ? ptr : ptr++;
            if ((*(ptr)=='#')&&(*(ptr+1)!='o'))
                result=NULL;
            else
                result = ptr;
            strncpy(act, ptr, (p-ptr-1));
            act[p-ptr-1] = '\0';
            break;
        }
    }
    return result;
}

int 
print_rules(char *filename, int idx, char *ruleBuf, char *actionBuf, char *classBuf, char *sidBuf)
{
    int i, fd, cnt=0;
    char *f, *start, *ptr, *p, *ret, *act, *cla, *sid, *head, *tail;
    struct stat sb;
    char buf[128], filenameBuf[128], actBuf[80], claBuf[128], sidBuffer[80];

    ret = ruleBuf ? ruleBuf : buf;
    act = actionBuf ? actionBuf : actBuf;
    cla = classBuf ? classBuf : claBuf;
    sid = sidBuf ? sidBuf : sidBuffer;

    if (!strcmp(filename, "rules") || !strcmp(filename, "version"))
    {
        f = snortfile;
        head = rules_head;
        tail = rules_tail;
    }
    else
    {
        sprintf(filenameBuf, "%s%s%s", rulespath, filename, rulefile_ext);
        f = filenameBuf;
        head = msg_head;
        tail = msg_tail;
    }

    if ((fd = open(f,O_RDONLY)) < 0)
    {
        printf("Unable to open %s (%d)\n", f, fd);
        return 0;
    }
    fstat(fd,&sb);
    start = mmap(NULL, sb.st_size, PROT_READ,MAP_PRIVATE, fd, 0);
    if(start == MAP_FAILED)
    {
        printf("Unable to mmap %s\n", f);
        close(fd);
        return 0;
    }
    //printf("%s", start);
    f_start = start;
    ptr = start;

    if (!strcmp(filename, "version"))
    {
        *ret = 0x30;
        *(ret+1) = '\0';
        if (ptr=strstr(ptr, ver_head))
        {
            for (i=0; i<20 ; i++)
            {
                if (ptr[i]==0xd||ptr[i]==0xa)
                    break;
            }
            ptr += strlen(ver_head);
            strncpy(ret, ptr, i-strlen(ver_head));
            ret[i-strlen(ver_head)] = '\0';
            if (ruleBuf==NULL)
            {
                printf(ret);
                printf("\n");
            }
        }
    }
    else if (idx == 0)
    {
        while (ptr=strstr(ptr, head))
        {
            if (*(ptr-1) == '#')
            {
                ptr++;
                continue;
            }
            p = strstr(ptr, tail);
            if (p)
            {
                if (!strcmp(filename, "rules"))
                {
                    if (!strncmp(ptr-4, "#ox ", 4))
                       strcpy(act, "disable");
                    else
                       strcpy(act, "enable");
                    ptr += strlen(head);
                    strncpy(buf, ptr, (p-ptr));
                    buf[p-ptr] = '\0';
                    if (ruleBuf==NULL)
                        printf("%s %s\n", act, buf);
                    cnt++;
                }
                else if (get_action(ptr, act))
                {
                    ptr += strlen(head);
                    strncpy(buf, ptr, (p-ptr));
                    buf[p-ptr] = '\0';
                    get_classtype_sid(ptr, cla, sid);
                    if (ruleBuf==NULL)
                        printf("%s %s    (%s, %s)\n", act, buf, cla, sid);
                    cnt++;
                    *act = '\0';
                }
                else
                    ptr++;
            }
        }
        if (ruleBuf==NULL)
            printf("\n%d rules\n", cnt);
    }
    else
    {
        while ((ptr=strstr(ptr, head)) && cnt != idx)
        {
            if (*(ptr-1) == '#')
            {
                ptr++;
                continue;
            }
            if (strcmp(filename, "rules"))
            {
                if (get_action(ptr, act))
                    cnt++;
            }
            else
            {
                cnt++;
                if (!strncmp(ptr-4, "#ox ", 4))
                   strcpy(act, "disable");
                else
                   strcpy(act, "enable");
            }
            if (cnt == idx)
            {
                p = strstr(ptr, tail);
                if (p)
                {
                    ptr += strlen(head);
                    strncpy(ret, ptr, (p-ptr));
                    ret[p-ptr] = '\0';
                    get_classtype_sid(ptr, cla, sid);
                    if (ruleBuf==NULL)
                    {
                        if (!strcmp(filename, "rules"))
                            printf("%s %s\n", act, ret);
                        else
                            printf("%s %s    (%s, %s)\n", act, ret, cla, sid);
                    }
                    break;
                }
            }
            else
                ptr++;
        }
        if (cnt < idx)
            cnt = 0;
    }
    munmap(start, sb.st_size);
    close(fd);
    return cnt;
}

int set_risk_action(char *filename, char *actionBuf)
{
    int i, fd, tmp_fd, cnt=0;
    char *f, *start, *ptr, *p, *act, *head, *tail, *p0, *p1;
    struct stat sb;
    char buf[128], filenameBuf[128], actBuf[80];

    head = msg_head;
    tail = msg_tail;
    act = actBuf;
    f = filename;
    if ((fd = open(f,O_RDONLY)) < 0)
    {
        printf("Unable to open %s (%d)\n", f, fd);
        return 0;
    }
    fstat(fd,&sb);
    start = mmap(NULL, sb.st_size, PROT_READ,MAP_PRIVATE, fd, 0);
    if(start == MAP_FAILED)
    {
        printf("Unable to mmap %s\n", f);
        close(fd);
        return 0;
    }
    //printf("%s", start);
    f_start = start;
    ptr = start;

    unlink(tmp_file);
    umask(0);
    if ((tmp_fd = open(tmp_file, O_RDWR | O_CREAT | O_EXCL, 0666))<0)
    {
        printf("Unable to open tmp file (%d)\n", tmp_fd);
        munmap(start, sb.st_size);
        close(fd);
        return 0;
    }

    while (ptr=strstr(ptr, head))
    {
        if (*(ptr-1) == '#')
        {
            ptr++;
            continue;
        }

        if (p0=get_action(ptr, act))
            cnt++;
        else
        {
            ptr++;
            continue;
        }

        p = strstr(ptr, tail);
        if (p)
        {
            int f_size=0;

            p1 = strstr(p, ";)");
            if (write(tmp_fd, actionBuf, strlen(actionBuf))<0)
                printf("\nWrite error !\n");
            f_size = p1-p0-strlen(act)+2;
            if (write(tmp_fd, p0+strlen(act), f_size)<0)
                printf("\nWrite error !\n");
            write(tmp_fd, "\n", 1);
        }
        ptr++;
    }
    munmap(start, sb.st_size);
    close(tmp_fd);
    close(fd);
    sprintf(buf, "cp -f %s %s", tmp_file, f);
    system(buf);
    unlink(tmp_file);
    return cnt;
}

int set_rules_action(char *filename, int idx, char *actionBuf)
{
    char *action_lists[] = { "alert", "log", "drop", "sdrop", "disable", "enable", NULL };
    int i, fd, tmp_fd, cnt=0, action_flag=0;
    char *f, *start, *ptr, *p, *act, *head, *tail, *p0, *p1;
    struct stat sb;
    char buf[128], filenameBuf[128], actBuf[80];

    if ((actionBuf==NULL)||(strlen(actionBuf)==0)||(filename==NULL))
        return 0;

    act = actBuf;

    for (i=0; i<6; i++)
    {
        if (strstr(actionBuf, action_lists[i]))
        {
            action_flag = 1;
            break;
        }
    }

    if (action_flag == 0)
        return 0;

    if (!strcmp(filename, "rules"))
    {
        sprintf(filenameBuf, "%s", snortfile);
        head = rules_head;
        tail = rules_tail;
    }
    else
    {
        sprintf(filenameBuf, "%s%s%s", rulespath, filename, rulefile_ext);
        head = msg_head;
        tail = msg_tail;
    }
    f = filenameBuf;

    if (!strcmp(filename, "high"))
    {
        return(set_risk_action(f, actionBuf));
    }
    else if (!strcmp(filename, "medium"))
    {
        return(set_risk_action(f, actionBuf));
    }
    else if (!strcmp(filename, "low"))
    {
        return(set_risk_action(f, actionBuf));
    }

    if ((fd = open(f,O_RDONLY)) < 0)
    {
        printf("Unable to open %s (%d)\n", f, fd);
        return 0;
    }
    fstat(fd,&sb);
    start = mmap(NULL, sb.st_size, PROT_READ,MAP_PRIVATE, fd, 0);
    if(start == MAP_FAILED)
    {
        printf("Unable to mmap %s\n", f);
        close(fd);
        return 0;
    }
    //printf("%s", start);
    f_start = start;
    ptr = start;

    unlink(tmp_file);
    umask(0);
    if ((tmp_fd = open(tmp_file, O_RDWR | O_CREAT | O_EXCL, 0666))<0)
    {
        printf("Unable to open tmp file (%d)\n", tmp_fd);
        munmap(start, sb.st_size);
        close(fd);
        return 0;
    }

    while ((ptr=strstr(ptr, head)) && cnt != idx)
    {
        if (*(ptr-1) == '#')
        {
            ptr++;
            continue;
        }

        if (strcmp(filename, "rules"))
        {
            if (p0=get_action(ptr, act))
                cnt++;
        }
        else
            cnt++;

        if (cnt == idx)
        {
            p = strstr(ptr, tail);
            if (p)
            {
                int f_size=0;

                p1 = ptr;
                ptr += strlen(head);
                strncpy(buf, ptr, (p-ptr));
                buf[p-ptr] = '\0';

                if (!strcmp(filename, "rules"))
                {
                    f_size = sb.st_size-(p1-f_start);
                    if (!strcmp(actionBuf, "enable"))
                    {
                        if (!strncmp(p1-4, "#ox ", 4))
                        {
                            if (write(tmp_fd, f_start, p1-f_start-4)<0)
                                printf("\nWrite error !\n");
                        }
                        else
                        {
                            if (write(tmp_fd, f_start, p1-f_start)<0)
                                printf("\nWrite error !\n");
                        }
                        if (write(tmp_fd, p1, f_size)<0)
                            printf("\nWrite error !\n");
                    }
                    else if (!strcmp(actionBuf, "disable"))
                    {
                        if (write(tmp_fd, f_start, p1-f_start)<0)
                            printf("\nWrite error !\n");
                        if (strncmp(p1-4, "#ox ", 4))
                        {
                            if (write(tmp_fd, "#ox ", 4)<0)
                                printf("\nWrite error !\n");
                        }
                        if (write(tmp_fd, p1, f_size)<0)
                            printf("\nWrite error !\n");
                    }
                }
                else
                {
                    if (write(tmp_fd, f_start, p0-f_start)<0)
                        printf("\nWrite error !\n");
                    if (write(tmp_fd, actionBuf, strlen(actionBuf))<0)
                        printf("\nWrite error !\n");
                    f_size = sb.st_size-(p0-f_start)-strlen(act);
                    if (write(tmp_fd, p0+strlen(act), f_size)<0)
                        printf("\nWrite error !\n");
                }
                //printf("%s %s \n", actionBuf, buf);
                break;
            }
        }
        else
            ptr++;
    }
    if (cnt < idx)
        cnt = 0;
    
    munmap(start, sb.st_size);
    close(tmp_fd);
    close(fd);
    sprintf(buf, "cp -f %s %s", tmp_file, f);
    system(buf);
    unlink(tmp_file);
    return cnt;
}

int nk_snortconfig(int argc, char argv[4][ARGV_SIZE], char *retBuf, char *actionBuf, \
                   char *classBuf, char *sidBuf)
{
    int   cnt=0, idx, i;
    char  *filename;

    /* Find any options.  argc is at least 1  the argv[0] is the program name*/
    argc--;  
    argv++;

    idx = atoi(argv[2]);
    filename = argv[1];

     /* Convert filename to lower case */
    for (i=0; i<strlen(filename); i++)
        *(filename+i) |= 0x20;

    switch ( argv[0][1] )
    {
        case 'p':
            cnt = print_rules(argv[1], idx, retBuf, actionBuf, classBuf, sidBuf);
	    break;

        case 'm':
            cnt = set_rules_action(argv[1], idx, argv[3]);
            if (actionBuf==NULL)
                printf("Set rule Done !\n");
	    break;

        default :
            cnt = 0;
    }

    return cnt;
}


int NK_IDSRules_Read(char* parm2, int idx, char *printBuf, char *actionBuf, \
                     char *classBuf, char *sidBuf)
{
    char argv[5][ARGV_SIZE];
    int cnt=0;

    if ((parm2 != (char *) NULL))
    {
        sprintf(argv[1], "-p");
	if (strlen(parm2) >= ARGV_SIZE) 
	{
	    //kd_Log("kd_IDSCommand:Too Big: %s",parm2);
	    return 0;
	}
        strcpy(argv[2], parm2);
        sprintf(argv[3], "%d", idx);
	cnt = nk_snortconfig(4, argv, printBuf, actionBuf, classBuf, sidBuf);
    }
    return cnt;
}

int NK_IDSRules_Write(char* parm2, int idx, char *actionBuf)
{
    char argv[5][ARGV_SIZE];
    int cnt=0;

    if ((parm2 != (char *) NULL))
    {
        sprintf(argv[1], "-m");
	if (strlen(parm2) >= ARGV_SIZE) 
	{
	    //kd_Log("kd_IDSCommand:Too Big: %s",parm2);
	    return 0;
	}
        strcpy(argv[2], parm2);
        sprintf(argv[3], "%d", idx);
        strcpy(argv[4], actionBuf);
	cnt = nk_snortconfig(4, argv, NULL, actionBuf, NULL, NULL);
    }
    return cnt;
}

 
