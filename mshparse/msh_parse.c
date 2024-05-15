#include <msh_parse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// more comments for final commit
struct msh_sequence{
	struct msh_pipeline* s;
	int sElements;
	int allocated;
	//ill store the strings later
};

struct msh_pipeline{
	struct msh_command* p;
	int pElements;
	int needsfree;
    int flag;

    int symbol; 
    int appended;
    char * file_out;
    char * file_err;
	//ill store the strings later
};

struct msh_command{
    int symbol;
	char** List;
	int cElements;
	int isthere;
	//ill store the strings later
    char * program;
    pid_t pid;
};

void
msh_pipeline_free(struct msh_pipeline *p)
{
    if(p){
        free(p);
    }
	// for(int i = 0;i< MSH_MAXCMNDS;i++){
	// 	struct msh_command* cc = &p->p[i];
	// 	for(int j = 0;j< cc->cElements;j++){
	// 		free(cc->List[j]);
	// 	}
	// 	free(&p->p[i].List);
	// 	free(&p->p[i]);
	// }
	// p->pElements = 0;
	// free(p->p);
	// p->needsfree = 1;
}

void
msh_sequence_free(struct msh_sequence *s)
{
    if( s){
        
    }
	// for(int i = 0;i< MSH_MAXBACKGROUND;i++){
	// 	if(s->s[i]->needsfree != 1){
	// 		msh_pipeline_free(s->s[i]);
	// 	}
	// 	free(s->s[i]);
	// }
	// free(s->s);
	free(s);
}

struct msh_sequence *
msh_sequence_alloc(void)
{
	struct msh_sequence *sequence = malloc(sizeof(struct msh_sequence));
    sequence->s = calloc(MSH_MAXBACKGROUND , sizeof(struct msh_pipeline));
    sequence->sElements = 0;
    for (int i = 0; i < 16; i++) {
        sequence->s[i].p = (struct msh_command *) malloc(MSH_MAXCMNDS * sizeof(struct msh_command));
        sequence->s[i].pElements = 0;
        for (int j = 0; j < 16; j++) {
            sequence->s[i].p[j].isthere = 0; 
        }
    }
    return sequence;
}

char *
msh_pipeline_input(struct msh_pipeline *p)
{
	(void)p;

	return NULL;
}
msh_err_t
msh_sequence_parse(char *str, struct msh_sequence *seq)
{
    msh_sequence_free(seq);
    seq = msh_sequence_alloc();
    if(seq == NULL){
        return 0;
    }
    if(str[0] == ' '){
	// 	// printf("empty string");
		return 0;
	}
    int n = -1;
    size_t i;
    for (i = 0; i < strlen(str); i++){
        if(str[i] != ' '){
            n = 1;
            break;
        }
    }
    if (n == -1){
        
        return 0;
    }
    seq->sElements = 0; 
    char* the_string = strdup(str);

    //pointers for strtok_r
    char *seq_ptr;
    char *pipe_ptr;
    char *com_ptr;
    //strings 
    char *token;
    char *token2;
    char *token3;

    for (token = strtok_r(the_string, ";", &seq_ptr); token != NULL; token = strtok_r(seq_ptr, ";", &seq_ptr))
    {
        struct msh_pipeline* pip = &seq->s[seq->sElements];
        seq->sElements++;
        pip->pElements = 0;
        pip->symbol =0 ;

        for (token2 = strtok_r(token, "|", &pipe_ptr); token2 != NULL; token2 = strtok_r(pipe_ptr, "|", &pipe_ptr))
        {
            struct msh_command *comm = &pip->p[pip->pElements];

            char *program = strtok_r(token2, " ", &com_ptr);
            comm->cElements = 1;
            comm->program = strdup(program); 

            comm->List = malloc(comm->cElements * sizeof(char *));
            comm->List[0] = comm->program;

            for (token3 = strtok_r(NULL, " ", &com_ptr); token3 != NULL; token3 = strtok_r(com_ptr, " ", &com_ptr))
            {
                comm->cElements++;
                if (strcmp(token3, "&") == 0){
                    comm->cElements--;
                    pip->flag = 1;
                }
                else if(strcmp(token3, "1>") == 0){
                    comm->cElements--;
                    pip->symbol = 1 ;
                }
                else if(strcmp(token3, "1>>") == 0){
                    comm->cElements--;
                    pip->symbol = 1 ;
                    pip->appended = 1;
                }
                else if(strcmp(token3, "2>") == 0){
                    comm->cElements--;
                    pip->symbol = 2 ;
                }
                else if(strcmp(token3, "2>>") == 0){
                    comm->cElements--;
                    pip->symbol = 2 ;
                    pip->appended = 1;
                }
                
                else if(pip->symbol == 1 || pip->symbol == 2){//if symbol was found
                    comm->cElements--;
                    if (pip->symbol == 2){
                        pip->file_err = strdup(token3);
                    }
                    if (pip->symbol == 1){
                        pip->file_out = strdup(token3);
                    }
                    //then file name is here
                    // printf("is this eveyr going here\n");
                    // pip->file_name = strdup(token3);
                }
                else{
                   
                    
                    comm->List = realloc(comm->List, comm->cElements * sizeof(char *));
                    comm->List[comm->cElements - 1] = strdup(token3);
                    
                }
                
            }
            comm->cElements++;
            comm->List[comm->cElements - 1] = NULL;
            pip->pElements++;
        }
        pip->p[pip->pElements - 1].isthere = 1; 
    }
    //free the duped items
	free(the_string);

    return 0;
}


struct msh_pipeline *
msh_sequence_pipeline(struct msh_sequence *s)
{
    if (s == NULL || s->sElements == 0) {
        return NULL;
    }
    struct msh_pipeline *returned = calloc(MSH_MAXCMNDS, sizeof(struct msh_pipeline));

    memcpy(returned, &s->s[0], sizeof(struct msh_pipeline));
    int i;
    for (i = 0; i < s->sElements; i++) {
            s->s[i] = s->s[i+1];
    }

    // memset(&s->s[s->sElements - 1], 0, sizeof(struct msh_pipeline));

    s->sElements--;

    return returned;
}

struct msh_command *
msh_pipeline_command(struct msh_pipeline *p, size_t nth)
{
	if((size_t) p->pElements > nth){
		return &p->p[nth];
	}

	return NULL;
}

int
msh_pipeline_background(struct msh_pipeline *p)
{
    if (p->flag == 1){
        return 1;
    }
	return 0;
}

int
msh_command_final(struct msh_command *c)
{
	return c->isthere;
}

void
msh_command_file_outputs(struct msh_command *c, char **stdout, char **stderr)
{
	(void)c;
	(void)stdout;
	(void)stderr;
}

char *
msh_command_program(struct msh_command *c)
{
	if(c->cElements != 0){
		return c->List[0];
	}
	return NULL;
}

char **
msh_command_args(struct msh_command *c)
{
	return c->List;
}

void
msh_command_putdata(struct msh_command *c, void *data, msh_free_data_fn_t fn)
{
	(void)c;
	(void)data;
	(void)fn;
}

void *
msh_command_getdata(struct msh_command *c)
{
	(void)c;

	return NULL;
}   