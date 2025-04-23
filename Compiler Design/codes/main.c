#include "lex.h"
#include "parse.h"
#include "run.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <windows.h>  // Windows-specific headers

static void print(FILE *output_file, const struct token *const tokens,
    const size_t ntokens, const int error)
{
    for (size_t i = 0, alternate = 0; i < ntokens; ++i) {
        const struct token token = tokens[i];

        if (token.token == token_FBEG || token.token == token_FEND) {
            continue;
        }

        if (token.token != token_WSPC && token.token != token_LCOM && token.token != token_BCOM){
            alternate++;
        }

        const int len = token.end - token.beg;

        if (i == ntokens - 1 && error == LEX_UNKNOWN_TOKEN) {
            fprintf(output_file, "%.*s < Unknown token\n", len ?: 1, token.beg);
        } else if (token.token == token_LCOM || token.token == token_BCOM){
            fprintf(output_file, "%.*s", len, token.beg);
        } else if (alternate % 2) {
            fprintf(output_file, "%.*s", len, token.beg);
        } else {
            fprintf(output_file, "%.*s", len, token.beg);
        }
    }
}

int main(int argc, char **argv)
{
    HANDLE hFile, Mapping;
    LPVOID mapped;
    DWORD size;
    int exit_status = EXIT_FAILURE;

    if (argc != 2) {
        return fprintf(stderr, "Usage: %s <file>\n", argv[0]), exit_status;
    }

    // Open the file
    hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        perror("CreateFile");
        return exit_status;
    }

    // Get file size
    size = GetFileSize(hFile, NULL);
    if (size == 0) {
        fprintf(stderr, "‘%s‘: The file is empty\n", argv[1]);
        CloseHandle(hFile);
        return exit_status;
    }

    // Create file mapping
    Mapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, size, NULL);
    if (Mapping == NULL) {
        perror("CreateFileMapping");
        CloseHandle(hFile);
        return exit_status;
    }

    // Map the file into memory
    mapped = MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, size);
    if (mapped == NULL) {
        perror("MapViewOfFile");
        CloseHandle(Mapping);
        CloseHandle(hFile);
        return exit_status;
    }

    // Create the outputs directory if it doesn't exist
    CreateDirectory("outputs", NULL);

    // Construct the output file path
    char output_file_path[MAX_PATH];
    const char *input_file_name = strrchr(argv[1], '\\'); // Extract the file name from the path
    if (!input_file_name) {
        input_file_name = strrchr(argv[1], '/');
    }
    if (!input_file_name) {
        input_file_name = argv[1];
    } else {
        input_file_name++; // Skip the slash
    }

    // Remove the .txt extension from the input file name
    char base_name[MAX_PATH];
    strncpy(base_name, input_file_name, MAX_PATH);
    char *dot = strrchr(base_name, '.');
    if (dot && strcmp(dot, ".txt") == 0) {
        *dot = '\0'; // Remove the .txt extension
    }

    // Construct the final output file name
    snprintf(output_file_path, MAX_PATH, "outputs/%s_output.txt", base_name);

    // Open the output file
    FILE *output_file = fopen(output_file_path, "w");
    if (!output_file) {
        perror("Failed to open output file");
        UnmapViewOfFile(mapped);
        CloseHandle(Mapping);
        CloseHandle(hFile);
        return exit_status;
    }

    fprintf(output_file, "\n---*** Lexing ***---\n\n");
    struct token *tokens;
    size_t ntokens;
    const int lex_error = lex((const uint8_t *)mapped, size, &tokens, &ntokens);

    if (!lex_error || lex_error == LEX_UNKNOWN_TOKEN) {
        print(output_file, tokens, ntokens, lex_error);
    } else if (lex_error == LEX_NOMEM) {
        fprintf(output_file, "The lexer could not allocate memory.\n");
    }

    if (!lex_error) {
        fprintf(output_file, "\n\n\n---*** Parsing ***---\n\n");
        const struct node root = parse(tokens, ntokens, output_file);

        if (!parse_error(root)) {
            fprintf(output_file, "\n\n---*** Running ***---\n\n");
            run(&root, output_file);
            collapse_tree(root);
            exit_status = EXIT_SUCCESS;
        }
    }
    
    // Print the output file location to the terminal
    printf("The output is saved to %s in the outputs folder\n\n", output_file_path);
    free(tokens);
    UnmapViewOfFile(mapped);
    CloseHandle(Mapping);
    CloseHandle(hFile);
    fclose(output_file); // Close the output file
    return exit_status;
}