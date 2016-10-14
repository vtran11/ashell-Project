
//http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html
//canonical and noncaninical:http://www.gnu.org/software/libc/manual/html_node/Canonical-or-Not.html
//tcsetattr: http://pubs.opengroup.org/onlinepubs/009695399/functions/tcsetattr.html
//read:http://www.tutorialspoint.com/unix_system_calls/read.htm https://en.wikipedia.org/wiki/Read_(system_call)
//write:http://www.tutorialspoint.com/unix_system_calls/write.htm https://en.wikipedia.org/wiki/Write_(system_call)

#include <iostream>
#include <dirent.h>  //ls
#include <unistd.h> //chdir(),dup2(),fork(), getcwd(),read(), write()
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <pwd.h>
#include <vector>

using namespace std;

//Noncanonical mode offers special parameters called MIN and TIME for controlling
//whether and how long to wait for input to be available. You can even use them to
//avoid ever waiting to return immediately with whatever input is available, or with no input.
//int tcsetattr(int fildes, int optional_actions const struct termios *termios_p);
//TCSANOW, the change shall occur immediately.
//TCSADRAIN, the change shall occur after all output written to fildes is transmitted. This function should be used when changing parameters that affect output.
//TCSAFLUSH, the change shall occur after all output written to fildes is transmitted, and all input so far received but not read shall be discarded before the change is made.

//*************************NonCanonical Mode***************************
void ResetCanonicalMode(int fd, struct termios *savedattributes)
{
    tcsetattr(fd, TCSANOW, savedattributes); ////fd:  fildes (an open file descriptor associated with a terminal)
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes)
{
    struct termios TermAttributes;
    
    // Make sure stdin is a terminal. The isatty() function tests whether fd is an open file descriptor referring to a terminal.
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later.
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes.
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

//**************** print path ****************************

void path()
{
    string currentDir = get_current_dir_name();                 //get current directory
    
    if(currentDir.length() == 16 || currentDir.length() < 16)  //print full directory is length =<16
    {
        write(1, currentDir.c_str(), currentDir.length());
        write(1, "% ", 2);
    }
    
    else //print "/.." instead if length of dir >16
    {
        write(1,"/...", 4);
        string lastSubstring = currentDir.substr(currentDir.find_last_of('/'));
        write(1,lastSubstring.c_str(), lastSubstring.length() );
        write(1, "% ", 2);
    }
}

//***************************ls***************************
//stat: http://pubs.opengroup.org/onlinepubs/007908799/xsh/sysstat.h.html
//permission: http://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
//dirent: http://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html
void lsDirectory(const char*dir)
{
    DIR *pdir = NULL;      // Pointer to a directory, initialize to NULL
    pdir = opendir(dir);   // refers to current directory
    struct dirent *ent = NULL; //refers to directory entry.
    if(pdir == NULL)
    {
        write(2,"Failed to open directory \"", 26);
        write(1,dir, strlen(dir));
        write(1,"/\"\n",3);
    }
    else
    {
        while((ent = readdir(pdir)) != NULL)
        {
            string currentDir = get_current_dir_name();
            currentDir = currentDir + "/../" + ent->d_name; //above directory + file_name
            struct stat perms;
            stat(currentDir.c_str(), &perms); //int stat(const char *path, struct stat *buf);
            int mode = perms.st_mode;
            if(S_ISDIR(mode)) write (1, "d", 1); //if file is a directory
            else write(1, "-", 1);               // else replace with "-"
            if(mode & S_IRUSR) write (1, "r", 1);//user, read
            else write(1, "-", 1);
            if(mode & S_IWUSR) write (1, "w", 1);//user, write
            else write(1, "-", 1);
            if(mode & S_IXUSR) write (1, "x", 1);//user, execute/search
            else write(1, "-", 1);
            if(mode & S_IRGRP) write (1, "r", 1);//group, read
            else write(1, "-", 1);
            if(mode & S_IWGRP) write (1, "w", 1);//group, write
            else write(1, "-", 1);
            if(mode & S_IXGRP) write (1, "x", 1);//group, excute/search
            else write(1, "-", 1);
            if(mode & S_IROTH) write (1, "r", 1);//other, read
            else write(1, "-", 1);
            if(mode & S_IWOTH) write (1, "w", 1);//other, write
            else write(1, "-", 1);
            if(mode & S_IXOTH) write (1, "x", 1);//other, execute/search
            else write(1, "-", 1);
            write(1, " ", 1);
            write(1, ent->d_name,strlen(ent->d_name));
            write(1, "\n", 1);
        }
    }
}

void lsDirectory(){
    DIR *pdir = NULL; // Pointer to a directory, initialize to NULL
    pdir = opendir(".");
    struct dirent *ent = NULL;
    if(pdir == NULL)
    {
        write(2,"Failed to open directory.", 25);
    }
    
    else
    {
        while((ent = readdir(pdir)) != NULL)
        {
            struct stat perms;
            stat(ent->d_name, &perms);
            int mode = perms.st_mode;
            if(S_ISDIR(mode)) write (1, "d", 1);
            else write(1, "-", 1);
            if(mode & S_IRUSR) write (1, "r", 1);
            else write(1, "-", 1);
            if(mode & S_IWUSR) write (1, "w", 1);
            else write(1, "-", 1);
            if(mode & S_IXUSR) write (1, "x", 1);
            else write(1, "-", 1);
            if(mode & S_IRGRP) write (1, "r", 1);
            else write(1, "-", 1);
            if(mode & S_IWGRP) write (1, "w", 1);
            else write(1, "-", 1);
            if(mode & S_IXGRP) write (1, "x", 1);
            else write(1, "-", 1);
            if(mode & S_IROTH) write (1, "r", 1);
            else write(1, "-", 1);
            if(mode & S_IWOTH) write (1, "w", 1);
            else write(1, "-", 1);
            if(mode & S_IXOTH) write (1, "x", 1);
            else write(1, "-", 1);
            write(1, " ", 1);
            write(1,ent->d_name,strlen(ent->d_name));
            write(1, "\n", 1);
        }
    }
}



//@@@@@@@@@@@@@@@@@@@@@@@@Recursion for ff @@@@@@@@@@@@@@@@@@@@@@@@@@@
//implement recursion: http://www.lemoda.net/c/recursive-directory/
void ffSubpath(const char *dir, string lastsub)
{
    DIR *pdir; // Pointer to a directory, initialize to NULL
    pdir = opendir(dir);
    struct dirent *entry;
    
    while(1)
    {
        if(!(entry = readdir(pdir)))  break;
        
        if (entry->d_type & DT_DIR)  //if there is subdirectory in current directory
        {
            if(strcmp(entry ->d_name,"..") != 0 && strcmp(entry ->d_name, ".") !=0) //check if this directory is not currdir or parent's dir
            {
                char path[PATH_MAX];
                strcpy(path, dir);
                strcat(path, "/");
                strcat(path, entry ->d_name);
                
                ffSubpath(path, lastsub); //recursively call the function again to test
            }
        }
        
        else  //if it is not a subdirectory but a file
        {
            if(entry ->d_name == lastsub)
            {
                write(1,dir, strlen(dir));
                write(1,"/",2);
                write(1, entry ->d_name, strlen(entry ->d_name));
                write(1,"\n",1);
            }
        } //close else
    }//clode while
    
    closedir(pdir);
}

//*************************main****************************
int main(int argc, char *argv[])
{
    struct termios SavedTermAttributes;
    string inputString;
    vector<string> inputs;
    int index = 0;
    char inputChar;
    while(1)
    {
        path();
        SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
        
        while(1) //deal with characters that we type in
        {
            read(STDIN_FILENO, &inputChar, 1); // read inputs
            if(0x04 == inputChar)
            { // Ctrl C or Ctr D
                break;
            }
            else if(0x1B == inputChar) // UP/DOWN/LEFT/RIGHT
            {
                read(STDIN_FILENO, &inputChar, 1); // read inputs
                if(0x5B == inputChar) // UP/DOWN/LEFT/RIGHT
                {
                    read(STDIN_FILENO, &inputChar, 1);
                    if(0x43 == inputChar || 0x44 == inputChar) //LEFT/RIGHT key do nothing
                        continue;
                    
                    else if (0x41 == inputChar) //UP
                    {
                        if(inputs.empty() || index + 1 > (int)inputs.size()) //if out of bound, bell sound
                        {
                            write(1,"\a", 1);
                            continue;
                        }
                        
                        for(int i = 0; i < (int)inputString.length(); i++)  //clear inputString (on screen)
                        {
                            write(1, "\b \b", 3);
                        }
                        index++;
                        inputString = inputs[inputs.size()-index];
                        write(1, inputString.c_str(), inputString.length());
                    }
                    
                    else if (0x42 == inputChar) //DOWN
                    {
                        if(inputs.empty() || index == 0) //if out of bound, bell sound
                        {
                            write(1,"\a", 1);
                            continue;
                        }
                        
                        for(int i = 0; i < (int)inputString.length(); i++) //clear inputString (on screen)
                        {
                            write(1, "\b \b", 3);
                        }
                        index--;
                        if(index == 0)
                            inputString ="";
                        else
                            inputString = inputs[inputs.size()-index];
                        write(1, inputString.c_str(), inputString.length());
                    }
                }
                
            }
            
            else if(0x0A == inputChar)  //'Enter' key
            {
                if (inputString == "") //if user press Enter without input, do nothing
                    continue;
                
                if (inputs.size() >= 10)
                    inputs.erase(inputs.begin()); //remove the one not in recent 10 inputString
                
                inputs.push_back(inputString);
                write(1, "\n", 1);
                break;
            }
            
            else if(0x7F == inputChar||0x08 == inputChar ||  '\b' == inputChar) //delete key
            {
                if(inputString == "")
                {
                    write(1,"\a",1);
                    continue;
                }
                
                else
                {
                    write(1,"\b \b",3);
                    inputString = inputString.erase(inputString.length()-1);
                }
            }
            
            else if(isprint(inputChar))  //character that can be print
            {
                write(1, &inputChar, 1);
                inputString += inputChar;
            }
            
            argv = new char*[(int)inputString.size() + 1];
        }//close small while
        
        ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
        
        if(inputString == "exit")
            break;
        
        
        else if(inputString == "pwd")  //pwd
        {
            write(1, get_current_dir_name(), strlen(get_current_dir_name())); //write current working dir
            write(1, "\n", 1);
        }
        
        else if(inputString.substr(0,2) == "cd")
        {
            if(inputString == "cd") //change directory
            {
                char* HOMEdir;
                HOMEdir = getenv("HOME"); //http://www.cplusplus.com/reference/cstdlib/getenv/
                chdir(HOMEdir);           // change directory to home. /home/userID%
            }
            else if(inputString.substr(2,1) == " ") //change directory
            {
                size_t pos = inputString.find_last_of(" ");
                string substring = inputString.substr(pos+1);
                
                DIR *ptrDir = NULL;                              // Pointer to a directory, initialize to NULL
                ptrDir = opendir(substring.c_str());            // refer to current directory
                
                if(ptrDir != NULL)                              // if pdir is NULL, Error.
                {
                    chdir(substring.c_str());                   // else change directory to "dir"
                }
                
                else
                    write(2,"Error changing directory.\n", 26); //value 2 = standard error
            }
        }
        
        else if(inputString.substr(0,2) == "ls")
        {
            if (inputString == "ls") //ls
            {
                lsDirectory();
            }
            
            else if(inputString.substr(2,1) == " ") //ls directory
            {
                size_t pos = inputString.find_last_of(" ");
                string substring = inputString.substr(pos+1);
                
                lsDirectory(substring.c_str());
            }
        }
        
        else if(inputString.substr(0,2) == "ff")
        {
            string lastSub;
            if (inputString == "ff") //ff
                write(2,"ff command requires a filename!\n", 33);
            
            else if(inputString.substr(2,1) == " ") // inputString = "ff file" or "ff file [directory]"
            {
                size_t pos = inputString.find_first_of(" ");
                string subString = inputString.substr(pos+1); //substring == "  file" or " file  [directory]"
                
                size_t posSub = subString.find_last_of(" ");
                string lastsub = subString.substr(posSub+1);  //lastsub = "file" or "[directory]"
                
                string currentDir = get_current_dir_name();
                
                if(lastsub.substr(0,1) != "/")  //lastsub is a file
                {
                    ffSubpath(currentDir.c_str(), lastsub);
                    
                }
                
                
                if(lastsub.substr(0,1) == "/")   //lastsub = [directory] and subString = "  file   [directory]"
                {
                    string fileString = subString.erase(subString.length()- lastsub.length()-1); //fileString = "  file  "
                    
                    for(int i =0; i < (int)fileString.length(); i++) //delete every space left in the substring
                    {
                        if( fileString[i] == ' ')
                            fileString = fileString.erase(i,1);  //finally fileString ="file" with no spaces
                    }
                    
                    ffSubpath(lastsub.c_str(), fileString);
                }
            }//close else if " "
        } //close ff
        
        
        else
        {
            write(2,"Failed to execute ", 18);
            write(1, inputString.c_str(), inputString.length());
            write(1,"\n",1);
        }
        
        inputString.clear();
        
    } //close global while
    
    return 0;
}


