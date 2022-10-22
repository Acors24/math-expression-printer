#include <stdbool.h>

//typedef struct node Node;

typedef struct node
{
    bool isOperator;
    char operator;
    char* operand;
    struct node* l;
    struct node* r;
} Node;

typedef Node* ExprTree;

ExprTree new_tree();

ExprTree parse(const char* expr, int* error_code);

void print_tree(const ExprTree tree);

int get_width(const ExprTree tree);

int get_height(const ExprTree tree);

void free_tree(ExprTree tree);

bool in(char c, char *s);
