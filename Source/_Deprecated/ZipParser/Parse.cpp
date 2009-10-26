
// Inefficient multithreading... we can't get the exact
// line from a FILE* because we don't know how long the
// lines are. So spawning another thread to read from
// another location of the FILE* isn't possible...

// Conclusion:
// The one thing that _can_ be done is to do the archive
// looping process outside the Window's thread while 
// implementing in the zip program.

#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <ctype.h>

#include <OS.h>
#include <List.h>
#include <Message.h>

int32 SkipSpaces (char *string, int32 location);
int32 SkipNonSpaces (char *string, int32 location);
static int32 ThreadOne (void *data);

int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Not enough arguments..." << endl << endl;
        cout << "Give zip file path" << endl;
        cout.flush ();
        
        return -1;
    }

    char *cmd = new char [strlen ("unzip -l ") + strlen (argv[1]) + 2];
    sprintf (cmd, "unzip -l \"%s\"", argv[1]);
    FILE *fp = popen (cmd, "r");
    delete[] cmd;


    double startTime = system_time();
        
    char data[768];

    int32 lineNum = 0;
    char *lenString;
    char *temp;
    int i;
    thread_id tID (0);

    while (fgets (data, 768, fp))
    {
        //lineNum ++;
        data[strlen (data) - 1] = '\0';
                
        if (lineNum > 3)
        {
            temp = data;
            temp [strlen(temp)] = '\0';
            temp += SkipSpaces (temp, 0);
            i = 1;
            while ((lenString = strtok (temp, " \t")) != NULL)
            {
//                cout << lenString << endl;
                int32 length = strlen (lenString) +  2;
                temp += length;
                i++;
                
                if (i == 4)
                {
                    lenString = strtok (temp, "\n");
                    temp += strlen (lenString);
//                    cout << ++lenString << endl;
                }
            }
        }
        
//        if (lineNum == 60)
//        {
//            BMessage msg;
//            msg.AddPointer ("pFile", (void*)fp);
//            tID = spawn_thread (ThreadOne, "ThreadOne",
//                                60, &msg);
//            resume_thread (tID);
//        }
    }
    
//    status_t result;
//    wait_for_thread (tID, &result);

    double endTime = system_time();
    cout.flush();
    
    cout << endl << endl;
    cout << "Time Taken: " << (endTime - startTime) / 1000000.0;
    cout << " seconds" << endl;
    cout.flush();
    
    return 0;
}

int32 SkipSpaces (char *string, int32 location)
{
    int32 i = location;
    while (isspace(string[i]))
        i++;

    return i;
}

int32 ThreadOne (void *arg)
{
    BMessage *message (reinterpret_cast<BMessage*>(arg));
    FILE *pFile;
    
    if (message->FindPointer ("pFile", reinterpret_cast<void**>(&pFile)) != B_OK)
        return B_ERROR;
    
    char data[768];
    char *lenString;
    char *temp;
    int i;
    FILE *fp = pFile;
    while (fgets (data, 768, fp))
    {
        data[strlen (data) - 1] = '\0';
                
        temp = data;
        temp [strlen(temp)] = '\0';
        temp += SkipSpaces (temp, 0);
        i = 1;
        while ((lenString = strtok (temp, " \t")) != NULL)
        {
            cout << lenString << endl;
            int32 length = strlen (lenString) +  2;
            temp += length;
            i++;
            
            if (i == 4)
            {
                lenString = strtok (temp, "\n");
                temp += strlen (lenString);
                cout << ++lenString << endl;
            }
            cout.flush();
        }
    }
    pclose (fp);

}
