#include "ExprTree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ExprTree new_tree()
{
    ExprTree root = (ExprTree) malloc(sizeof(Node));
    root->isOperator = false;
    root->operator = '?';
    root->operand = NULL;
    root->l = NULL;
    root->r = NULL;
    return root;
}

ExprTree split_and_parse(const char* expr, int i, int* error_code)
{
    int len = strlen(expr);
    ExprTree root = new_tree();

    char sub1[i + 1];
    memcpy(sub1, expr, i);
    sub1[i] = '\0';
    char sub2[len - i + 1];
    memcpy(sub2, expr + i + 1, len - i);
    sub2[len - i] = '\0';

    root->isOperator = true;
    root->operator = expr[i];
    root->operand = NULL;
    ExprTree res = parse(sub1, error_code);
    if (res == NULL)
    {
        free_tree(root);
        return NULL;
    }
    root->l = res;

    res = parse(sub2, error_code);
    if (res == NULL)
    {
        free_tree(root);
        return NULL;
    }
    root->r = res;
    return root;
}

int count_spaces(const char* expr)
{
    int len = strlen(expr);
    int spaces = 0;
    for (int i = 0; i < len; i++)
        spaces += (expr[i] == ' ');

    return spaces;
}

const char* remove_spaces(const char* expr)
{
    int len = strlen(expr);
    int spaces = count_spaces(expr);

    if (!spaces) return expr;

    char* newExpr = (char*) malloc(sizeof(char) * (len - spaces + 1));
    newExpr[len - spaces] = '\0';
    for (int i = 0, ni = 0; i < len && ni < len - spaces; i++)
        if (expr[i] != ' ')
            newExpr[ni++] = expr[i];

    return newExpr;
}

ExprTree parse(const char* init_expr, int* error_code)
{
    if (init_expr == NULL) return NULL;

    int spaces;
    const char* expr;
    if ((spaces = count_spaces(init_expr)))
        expr = remove_spaces(init_expr);
    else
        expr = init_expr;

    int len = strlen(expr);
    int parAm = 0;
    if (expr[0] == '(' && expr[len - 1] == ')')
    {
        parAm = 1;
        for (int i = 1; i < len - 1 && parAm != 0; i++)
        {
            if (expr[i] == '(') parAm++;
            else if (expr[i] == ')') parAm--;
        }
        if (parAm)
        {
            char newExpr[len - 2];
            strcpy(newExpr, expr + 1);
            newExpr[len - 2] = '\0';
            if (spaces) free((void*) expr);
            return parse(newExpr, error_code);
        }
    }

    parAm = 0;
    int opI = -1;
    for (int i = 0; i < len; i++)
    {
        char c = expr[i];
        if (c == '(') parAm++;
        else if (c == ')') parAm--;
        else if (parAm == 0)
        {
            switch (c)
            {
            case '<':
            case '>':
            case '=':
            case '+':
            case '-':
                if (opI == -1 || !in(expr[opI], "<>="))
                    opI = i;
                break;
            case '*':
            case '/':
                if (opI == -1 || !in(expr[opI], "<>=+-"))
                    opI = i;
                break;
            case '^':
                if (opI == -1 || !in(expr[opI], "<>=+-*/^"))
                    opI = i;
                break;
            }
        }
    }

    if (parAm != 0)
    {
        if (spaces) free((void*) expr);
        *error_code = (parAm > 0 ? 1 : 2);
        return NULL;
    }

    if (opI != -1)
    {
        ExprTree root = split_and_parse(expr, opI, error_code);
        if (spaces) free((void*) expr);
        return root;
    }
    else
    {
        ExprTree root = new_tree();
        root->operand = (char*) malloc(len);
        strcpy(root->operand, expr);
        if (spaces) free((void*) expr);
        return root;
    }
}

void _print_tree(const ExprTree tree)
{
    if (tree->l != NULL) _print_tree(tree->l);
    if (tree->r != NULL) _print_tree(tree->r);
    if (tree->isOperator) printf("%c ", tree->operator);
    else printf("%s ", tree->operand);
}

void print_tree(const ExprTree tree)
{
    if (tree != NULL)
        _print_tree(tree);
    putchar('\n');
}

int get_width(const ExprTree tree)
{
    if (tree == NULL) return 0;

    if (tree->isOperator)
    {
        if (tree->operator == '/')
        {
            int l = get_width(tree->l),
                r = get_width(tree->r);
            return (l > r ? l : r);
        }
        else
            return get_width(tree->l) + get_width(tree->r) + (tree->operator != '^');
    }
    else
        return strlen(tree->operand);
}

int get_height(const ExprTree tree)
{
    if (tree == NULL) return 0;

    if (tree->isOperator)
    {
        int l = get_height(tree->l);
        int r = get_height(tree->r);
        int m = (l > r ? l : r);
        if (tree->operator == '/')
            return m * 2;
        else
            return m;
    }
    else
        return 1;
}

void free_tree(ExprTree tree)
{
    if (tree == NULL) return;

    free_tree(tree->l);
    free_tree(tree->r);
    free(tree->operand);
    free(tree);
}

bool in(char c, char *s)
{
    for (size_t i = 0; i < strlen(s); i++)
        if (c == s[i])
            return true;
    return false;
}
