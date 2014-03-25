#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define DEBUG TRUE

#define log_arquivo() printf("Arquivo: %s, linha: %d\n",__FILE__, __LINE__)
#define log_error(M) printf("%s\n", M)
#define log_error_code(CODE) printf("Codigo: (%d)\n", CODE)

#define check(A,M) if(!A){log_error(M); log_arquivo();  exit(EXIT_FAILURE);}
#define check_cl(STATUS,M) if(STATUS!=CL_SUCCESS){ log_error(M); log_error_code(STATUS); log_arquivo(); exit(EXIT_FAILURE); }

char *trim(char *s);

#endif // UTILS_H_INCLUDED
