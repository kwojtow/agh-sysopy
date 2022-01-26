#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    if(argc == 2){
        FILE* mail;
        if(strcmp(argv[1], "sender") == 0){
            mail = popen("mail -H | sort -k 3 -df", "r");
        }
        else if(strcmp(argv[1], "date") == 0){
            mail = popen("mail -H", "r");
        }
        else{
            exit(1);
        }
        char buf[65536];
        int read_bytes = 0;
        while((read_bytes = fread(buf, 1, sizeof(buf), mail)) != 0){
            printf("%s", buf);
        }
        printf("\n");
        pclose(mail);
    }
    else if(argc == 4){
        char* address = argv[1];
        char* subject = argv[2];
        char* content = argv[3];

        char command[1000];
        snprintf(command, sizeof(command), "mail -s %s %s", subject, address);
        FILE* mail = popen(command, "w");
        fwrite(content, sizeof(content[0]), sizeof(content), mail);
        pclose(mail);
        printf("Wysłano wiadomość na adres %s\n", address);
    }
    else{
        exit(2);
    }

    return 0;
}
