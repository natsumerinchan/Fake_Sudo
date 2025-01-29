#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define VERSION "1.0"

static struct option long_options[] = {
    {"user", required_argument, NULL, 'u'},
    {"group", required_argument, NULL, 'g'},
    {"supp-group", required_argument, NULL, 'G'},
    {"shell", required_argument, NULL, 's'},
    {"login", no_argument, NULL, 'i'},
    {"background", no_argument, NULL, 'b'},
    {"preserve-env", no_argument, NULL, 'E'},
    {"edit", required_argument, NULL, 'e'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {NULL, 0, NULL, 0}
};

char* join_args(int count, char** args) {
    if (count <= 0) return NULL;
    
    size_t total_len = 1; // For null terminator
    for (int i = 0; i < count; i++) 
        total_len += strlen(args[i]) + 1;
    
    char* buffer = malloc(total_len);
    if (!buffer) return NULL;
    
    char* ptr = buffer;
    for (int i = 0; i < count; i++) {
        size_t len = strlen(args[i]);
        memcpy(ptr, args[i], len);
        ptr += len;
        *ptr++ = ' ';
    }
    ptr[-1] = '\0';
    
    return buffer;
}

void print_help() {
    printf("Android sudo wrapper for MagiskSU\n\n");
    printf("Usage: sudo [options] [command]\n");
    printf("Options:\n");
    printf("  -u, --user=USER        run command as specified user (default: 0)\n");
    printf("  -g, --group=GRP        specify primary group\n");
    printf("  -G, --supp-group=GRP   specify supplementary group\n");
    printf("  -s, --shell=SHELL      use specified shell\n");
    printf("  -i, --login            run login shell\n");
    printf("  -b, --background       run command in background\n");
    printf("  -E, --preserve-env     preserve environment variables\n");
    printf("  -e, --edit=FILE        edit file with nano\n");
    printf("  -h, --help             show this help message\n");
    printf("  -V, --version          display version\n");
}

int main(int argc, char* argv[]) {
    char* target_user = NULL;
    char* command = NULL;
    char* shell = NULL;
    char* edit_file = NULL;
    int login = 0;
    int preserve_env = 0;
    int background = 0;
    int edit_mode = 0;
    int opt;

    // Parse command line options
    while ((opt = getopt_long(argc, argv, "+u:g:G:s:ibEe:hV", long_options, NULL)) != -1) {
        switch (opt) {
        case 'u': target_user = optarg; break;
        case 'g': /* TODO: Handle groups */ break;
        case 'G': /* TODO: Handle supp groups */ break;
        case 's': shell = optarg; break;
        case 'i': login = 1; break;
        case 'b': background = 1; break;
        case 'E': preserve_env = 1; break;
        case 'e': edit_mode = 1; edit_file = optarg; break;
        case 'h': print_help(); return 0;
        case 'V': printf("sudo-android %s\n", VERSION); return 0;
        default:  return 1;
        }
    }

    // Handle edit mode
    if (edit_mode) {
        if (optind < argc) {
            fprintf(stderr, "Error: Can't combine -e with command\n");
            return 1;
        }
        command = malloc(strlen(edit_file) + 6);
        sprintf(command, "nano %s", edit_file);
        if (!target_user) target_user = "0";
    }

    // Handle remaining arguments as command
    if (!edit_mode && optind < argc) {
        command = join_args(argc - optind, &argv[optind]);
    }

    // Check if a command is needed
    int need_command = 1;
    if (login && !command && !edit_mode) {
        // If only login (-i) is specified, launch an interactive shell
        need_command = 0;
    }

    if (need_command && !command && !edit_mode) {
        print_help();
        return 0;
    }

    // Build su command
    char* su_argv[32] = { "su" };
    int arg_count = 1;
    
    if (target_user) su_argv[arg_count++] = target_user;
    if (preserve_env) su_argv[arg_count++] = "--preserve-environment";
    if (login) su_argv[arg_count++] = "-l";
    if (shell) {
        su_argv[arg_count++] = "--shell";
        su_argv[arg_count++] = shell;
    }
    if (command) {
        su_argv[arg_count++] = "-c";
        su_argv[arg_count++] = command;
    }
    su_argv[arg_count] = NULL;

    // Execute command
    if (background) {
        pid_t pid = fork();
        if (pid == 0) {
            setsid();
            execvp("su", su_argv);
            perror("execvp");
            exit(1);
        } else if (pid < 0) {
            perror("fork");
            return 1;
        }
    } else {
        execvp("su", su_argv);
        perror("execvp");
        return 1;
    }

    return 0;
}