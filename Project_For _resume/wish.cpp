#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

   vector<string> paths = {"/bin"};

        string findpath(string cmd) {
           for (unsigned i = 0; i < paths.size(); i++) {
                string fullPath = paths[i] + "/" + cmd;
                if (access(fullPath.c_str(), X_OK) == 0) {
                   return fullPath;
                    }
                }
              return "";
            }


       // error funcation
          void Error() {
              char error_message[30] = "An error has occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
        }



          vector<string> parseInput(string line) {
            string word;
            istringstream iss(line);
            vector<string> args;

            while (iss >> word) {
            args.push_back(word);
        }

        return args;
  }

    int main(int argc, char* argv[]) {
      string line;

       if (argc > 2) {
         Error();
         exit(1);
       }

      ifstream batchFile;

         if (argc == 2) {
          batchFile.open(argv[1]);
           if (!batchFile.is_open()) {
            Error();
            exit(1);
        }
    }

        while (true) {

           if (argc == 1) {
            cout << "wish> ";
            }

           if (argc == 1) {
              if (!getline(cin, line)) {
                  exit(0);
                }
            } else {
                 if (!getline(batchFile, line)) {
                      exit(0);
                      }
                    }

            // add spaces around >
            
            
             string newLine = "";
             for (unsigned i = 0; i < line.size(); i++) {
                 if (line[i] == '>') {
                    newLine += " > ";
                   } else {
                 newLine += line[i];
                }
            }

              line = newLine;
              
              
              

               // split by &
              vector<string> commands;
              string current = "";

              for (unsigned i = 0; i < line.size(); i++) {
                  if (line[i] == '&') {
                      commands.push_back(current);
                      current = "";
                    } else {
                      current += line[i];
                    }
                }

                  commands.push_back(current);
                  
                  
                  

                 // store all child pids
               vector<pid_t> pids;

               // loop through each command
                 for (unsigned c = 0; c < commands.size(); c++) {

                     vector<string> args = parseInput(commands[c]);
                          if (args.empty()){
                              continue;
                          } 
                          
                          

                    // find redirection
               string outputFile = "";
                 int redirectPos = -1;
                 int redirectCount = 0;

                  for (unsigned i = 0; i < args.size(); i++) {
                        if (args[i] == ">") {
                           redirectCount++;
                           redirectPos = i;
                        }
                     }

                 if (redirectCount > 1) {
                     Error();
                     continue;
                    }

                 if (redirectPos != -1) {
                     if (redirectPos + 1 >= (int)args.size()) {
                     Error();
                     continue;
                     } else if (redirectPos + 2 < (int)args.size()) {
                                Error();
                                continue;
                 } else {
                         outputFile = args[redirectPos + 1];
                         args.resize(redirectPos);
                       }
                   }



            // check empty after parsing
              if (args.empty()) {
                Error();
                continue;
                 
              }  

              if (args[0] == ">") {
                  Error();
                   continue;
               }

              // built-in commands
              if (args[0] == "exit") {
                 if (args.size() != 1) {
                      Error();
                  } else {
                        exit(0);
                  }
                }
                else if (args[0] == "cd") {
                    if (args.size() != 2) {
                       Error();
                    } else {
                           if (chdir(args[1].c_str()) != 0) {
                                  Error();
                            }
                       }
                  }
                else if (args[0] == "path") {
                        paths.clear();
                        for (unsigned i = 1; i < args.size(); i++) {
                            paths.push_back(args[i]);
                         }
                       }
                else {
                       string fullPath = findpath(args[0]);

                           if (fullPath == "") {
                                Error();
                            } else {

                     vector<char*> argv;
                      for (unsigned i = 0; i < args.size(); i++) {
                            argv.push_back(const_cast<char*>(args[i].c_str()));
                        }
                            argv.push_back(NULL);

                            pid_t pid = fork();

                             if (pid < 0) {
                                   Error();
                               }
                             else if (pid == 0) {
                                   if (outputFile != "") {
                                     int fd = open(outputFile.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644); 
                                     if (fd < 0){
                                     Error();
                                     exit(1);
                                    }

                                  dup2(fd, STDOUT_FILENO);
                                  dup2(fd, STDERR_FILENO);
                                  close(fd);
                                }

                             execv(fullPath.c_str(), argv.data());
                             Error();
                             exit(1);
                           }
                         else {
                        pids.push_back(pid);
                   }
               }
           }

              } // end command loop

// wait for ALL children
            for (unsigned i = 0; i < pids.size(); i++) {
              int status;
              waitpid(pids[i], &status, 0);
               }
              }

 return 0;
}
