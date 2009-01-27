// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include <string.h>
#include <stdlib.h>
#include "Common.hpp"
#include "LexParser.hpp"
#include "Analyzer.hpp"
#include "Codegen_C.hpp"

void debug_print(Program *p, Scope *ns, std::string prepend) {
    if (ns->parent == NULL) {
        std::cout << prepend << "scope: " << ns << " Owner: " << ns->owner << std::endl;
    }
    else {
        std::cout << prepend << " " << ns << " Parent: " << ns->parent << " Owner: " << ns->owner << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_funs.begin(), end = ns->local_funs.end(); iter != end; ++iter) {
        std::cout << prepend << " fun: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_types.begin(), end = ns->local_types.end(); iter != end; ++iter) {
        std::cout << prepend << " type: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_vars.begin(), end = ns->local_vars.end(); iter != end; ++iter) {
        std::cout << prepend << " var: " << iter->first << " def: " << iter->second << " type: " << p->vars[iter->second]->type_def_num << " Usage: "
            << p->vars[iter->second]->usage_start.line
            << " " << p->vars[iter->second]->usage_start.col
            << ", " << p->vars[iter->second]->usage_end.line
            << " " << p->vars[iter->second]->usage_end.col  << std::endl;
    }
    for (std::map<std::string, Scope*>::iterator iter = ns->namespaces.begin(), end = ns->namespaces.end(); iter != end; ++iter) {
        std::cout << prepend << " child: " << iter->first;
        debug_print(p, iter->second, prepend + "  ");
    }
}

void debug_print(Program *p, std::string prepend) {
    //For now let's just print the namespaces
    for (unsigned int i = 0; i < p->var_sites.size(); ++i) {
        std::cout << "--" << i << " " << p->var_sites[i].size() << std::endl;
        for (unsigned int j = 0; j < p->var_sites[i].size(); ++j) {
            std::cout << prepend << p->var_sites[i][j] << std::endl;
        }
    }
    //for (std::vector<Var_Def*>::iterator iter = p->vars.begin(), end = p->vars.end(); iter != end; ++iter) {
    for (unsigned int i = 0; i < p->vars.size(); ++i) {
        std::cout << prepend << " var: " << i << " def: " << p->vars[i] << " type: " << p->vars[i]->type_def_num << " Usage: "
            << p->vars[i]->usage_start.line
            << " " << p->vars[i]->usage_start.col
            << ", " << p->vars[i]->usage_end.line
            << " " << p->vars[i]->usage_end.col  << std::endl;
    }

    debug_print(p, p->global, prepend);
}

void debug_print_vars(Program *p, Token *token) {
    if (token->scope != NULL) {
        Scope *scope = token->scope;
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
            std::cout << iter->first << " " << iter->second << "(" << p->vars[iter->second]->usage_start.line << ", " << p->vars[iter->second]->usage_start.col << ") "
                << "(" << p->vars[iter->second]->usage_end.line << ", " << p->vars[iter->second]->usage_end.col << ") " << std::endl;
        }
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print_vars(p, token->children[i]);
    }
}

void debug_print_def(Program *p, Token *token, std::string prepend) {
    if (token->contents == "") {
        std::cout << prepend << "(" << token->type << " def:" << token->definition_number << " type:" << token->type_def_num << " "
            << token->scope << " " << token->start_pos.line << " " << token->start_pos.col << " to " << token->end_pos.line << " " << token->end_pos.col << ")" << std::endl;
    }
    else {
        std::cout << prepend << token->contents << " (" << token->type << " def:" << token->definition_number
            << " type:" << token->type_def_num << " " << token->scope << " " << token->start_pos.line << " " << token->start_pos.col << " to "
            << token->end_pos.line << " " << token->end_pos.col << ")" << std::endl;
    }

    if (token->scope != NULL) {
        debug_print(p, token->scope, prepend + "  ");
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print_def(p, token->children[i], "   " + prepend);
    }
}

void debug_print_position(Position &pos) {
    std::cout << "f: " << pos.filename << " l: " << pos.line << " c: " << pos.col;
}

void debug_print_extents(Program *p) {
    for (unsigned int i = 0; i < p->vars.size(); ++i) {
        std::cout << "var: " << i;
        Extent *extent = p->vars[i]->extent;
        while (extent != NULL) {
            std::cout << "[";
            switch (extent->type) {
                case (Extent_Type::DECLARE) : std::cout << "Decl"; break;
                case (Extent_Type::ELSEIF_START) : std::cout << "Elseif"; break;
                case (Extent_Type::ELSE_START) : std::cout << "Else"; break;
                case (Extent_Type::IF_JOIN) : std::cout << "EndIf"; break;
                case (Extent_Type::IF_START) : std::cout << "If"; break;
                case (Extent_Type::LOOP_JOIN) : std::cout << "EndLoop"; break;
                case (Extent_Type::LOOP_START) : std::cout << "Loop"; break;
                case (Extent_Type::READ) : std::cout << "Read"; break;
                case (Extent_Type::WRITE) : std::cout << "Write"; break;
            }
            std::cout << " ";
            debug_print_position(extent->start_pos);
            std::cout << " ";
            debug_print_position(extent->end_pos);
            std::cout << "]" << std::endl;
            extent = extent->next;
        }
        std::cout << std::endl;
    }
}

class Compiler {
    Analyzer an;
    Lex_Parser lp;
    Codegen c;
    Program *p;

    Token *app;

public:
    Compiler() {
        app = new Token(Token_Type::APPLICATION);
        p = new Program();
    }

    std::string load_file(const char *filename) {
        std::ifstream infile (filename, std::ios::in | std::ios::ate);
        if (!infile.is_open()) {
            std::cerr << "Can not open " << filename << std::endl;
            exit(0);
        }

        std::streampos size = infile.tellg();
        infile.seekg(0, std::ios::beg);

        std::vector<char> v(size);
        infile.read(&v[0], size);

        std::string ret_val (v.empty() ? std::string() : std::string (v.begin(), v.end()).c_str());

        return ret_val;
    }

    void translate_file(std::string filename) {
        Token *t;

        std::string contents = load_file(filename.c_str());
        t = lp.lexparse_file(filename, contents);

        app->children.push_back(t);
        p->files.push_back(t);

        return;
    }

    void analyze_files() {
        Scope *start;

        for (unsigned int i = 0; i < p->files.size(); ++i) {
            Token *t = p->files[i];

            an.analyze_strays(t);

            //Start building app
            start = p->global;
            an.analyze_type_blocks(p, t, &start);
        }


        for (unsigned int i = 0; i < p->files.size(); ++i) {
            Token *t = p->files[i];

            start = p->global;
            an.analyze_fun_blocks(p, t, &start);
        }


        Token *t = app;

        //start = p->global;

        an.add_implied_constructors(p);
        an.analyze_var_type_and_scope(p, t, p->global);

        an.analyze_token_types(p, t, p->global);

        an.analyze_ports_of_entry(p, t, NULL, p->global, false, false);
        an.analyze_implied_this(p, t, p->global);
        an.analyze_return_calls(p, t, 0);
        an.analyze_var_visibility(p, t);
        an.analyze_freeze_resume(p, t, p->global);
        an.analyze_copy_delete(p, t, NULL, p->global);

        an.analyze_usage_extents(p, t, NULL, p->global);
        an.analyze_usage_extent_colors(p);

        //debug_print_extents(p);

        //debug_print_def(p, t, "");
        //debug_print_vars(p, t);
        //debug_print(p, "");

    }
    void compile_program(int argc, char *argv[], char *output_file, std::string include_dir,
            const std::vector<std::string> &lib_dirs, const std::vector<std::string> &libs, const std::string optimization_level) {

        std::ostringstream output;

        c.codegen(p, p->files[0], output);
        if (output_file == NULL) {
            std::cout << output.str();
        }
        else {
            std::ofstream outfile;
            outfile.open("tmpXXXXX.c");
            outfile << output.str();
            outfile.close();
            std::ostringstream exe_cmdline;

#if defined (__SVR4) && defined (__sun)
            exe_cmdline << "g++ -ggdb -O" << optimization_level << " -o \"" << output_file << "\" tmpXXXXX.c -Werror -I\"" << include_dir << "\" ";
#else
            exe_cmdline << "gcc -ggdb -O" << optimization_level << " -o \"" << output_file << "\" tmpXXXXX.c -Werror -I\"" << include_dir << "\" ";
#endif

            for (unsigned int i = 0; i < lib_dirs.size(); ++i) {
                exe_cmdline << "-L\"" << lib_dirs[i] << "\" ";
            }

            for (unsigned int i = 0; i < libs.size(); ++i) {
                exe_cmdline << "-l" << libs[i] << " ";
            }

            if (system(exe_cmdline.str().c_str()) == 0) {
                remove("tmpXXXXX.c");
            }
            else {
                std::cout << "For commandline: ";
                for (int i = 0; i < argc; ++i) {
                    std::cout << argv[i] << " ";
                }
                std::cout << std::endl;
                std::cout << "Used compile line: " << exe_cmdline.str() << std::endl;
                exit(-1);
            }
        }
    }

    void print_positions(std::string &contents, Position &start_pos, Position &end_pos) {
        unsigned int line, col;
        line = 1; col = 1;

        std::string::iterator p = contents.begin();

        while (line < start_pos.line) {
            if (*p == '\n') {
                ++line;
            }
            ++p;
        }

        std::string::iterator reset = p;

        //while we're in the right area, print the line, then print the highlight
        while (line <= end_pos.line) {
            col = 1;

            //First pass, print the line:
            reset = p;
            while ((*p != '\n') && (p != contents.end())) {
                std::cout << *p;
                ++p;
            }
            std::cout << std::endl;

            //Second pass, print the highlight:
            p = reset;
            while ((*p != '\n') && (p != contents.end())) {
                if ((start_pos.line == end_pos.line) && (col >= start_pos.col) && (col < end_pos.col)) {
                    std::cout << '^';
                }
                else if ((start_pos.line != end_pos.line) && (((line == start_pos.line) && (col >= start_pos.col))
                        || ((line > start_pos.line) && (line < end_pos.line))
                        || ((line == end_pos.line) && (col < end_pos.col)))) {
                    std::cout << '^';
                }
                else if ((start_pos == end_pos) && (line == start_pos.line) && (col == start_pos.col)) {
                    std::cout << '^';
                }
                else {
                    std::cout << ' ';
                }
                if (*p != '\n') {
                    ++col;
                }
                ++p;
            }
            ++line;
            ++p;
            std::cout << std::endl;
        }

    }

    void print_error(Compiler_Exception &ce) {
        //Reload file to see the error
        if ((ce.where_begin.filename != NULL) && (ce.where_begin.line > 0)) {
            std::string contents = load_file(ce.where_begin.filename);

            print_positions(contents, ce.where_begin, ce.where_end);

            std::cerr << "Error: " << ce.reason << " at line " << ce.where_begin.line << " col "
                << ce.where_begin.col << " in " << ce.where_begin.filename << std::endl;
        }
        else {
            std::cerr << "Error: " << ce.reason << std::endl;
        }
    }
};

void print_help() {
    printf("Usage: minnowc <options> <filename(s)>\n");
    printf("-o <filename>   : compile and output to a binary file\n");
    printf("-O <level>      : set the optimization level\n");
    printf("-L <lib dir>    : add a library directory to the library search path\n");
    printf("-l <library>    : add a library to link against\n");
    printf("-C              : output the generated C file to the stdout\n");
    printf("-h              : this help\n");
}

int main(int argc, char *argv[]) {
    Compiler compiler;

    char *output_file;
    output_file = (char*)malloc(sizeof(char) * 7);
    strcpy(output_file, "noname");

    std::string current_bin = argv[0];

    if (argc < 2) {
        print_help();
        printf("Please specify the file to compile\n");
        exit(0);
    }

    std::vector<std::string> libs;
    libs.push_back("aquarium");

    std::string prelude_dir = "";
    std::string prefix_dir = "";
    std::string lib_dir = ".";
    std::string include_dir = "aquarium";
    std::string optimization_level = "0";

    #ifdef INSTALLPREFIX
        prefix_dir = INSTALLPREFIX;
        prelude_dir = prefix_dir + "share/minnow/";
        lib_dir = prefix_dir + "lib";
        include_dir = prefix_dir + "include/minnow";
    #endif

    std::vector<std::string> lib_dirs;
    lib_dirs.push_back(lib_dir);

    try {
        compiler.translate_file(prelude_dir + "prelude.mno");

        //for (int i = 1; i < argc; ++i) {
        int i = 1;
        while (i < argc) {
            /*
            if (strcmp(argv[i], "-o") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    output_file = argv[i+1];
                    i = i + 2;
                }
                else {
                    printf("Missing output filename:  Use -o <filename> to output a binary\n");
                    exit(0);
                }
            }
            else if (strcmp(argv[i], "-L") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    std::string libdir = argv[i+1];
                    lib_dirs.push_back(libdir);
                    i = i + 2;
                }
                else {
                    printf("Missing library directory:  Use -L <director> to add a library directory\n");
                    exit(0);
                }
            }
            else if (strcmp(argv[i], "-l") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    std::string libname = argv[i+1];
                    libs.push_back(libname);
                    i = i + 2;
                }
                else {
                    printf("Missing library filename:  Use -l <filename> to add a library\n");
                    exit(0);
                }
            }
            */
            if (argv[i][0] == '-') {
                int arglen = strlen(argv[i]);
                if (arglen > 1) {
                    switch(argv[i][1]) {
                        case ('h') :
                            print_help();
                            exit(0);
                        break;
                        case ('o') :
                            if (arglen == 2) {
                                //grab output file
                                if ((i+1) < argc) {
                                    output_file = argv[i+1];
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing output filename:  Use -o <filename> to output a binary\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                output_file = (char *)(argv[i] + 2);
                                ++i;
                            }
                        break;
                        case ('C'):
                            output_file = NULL;
                            ++i;
                        break;
                        case ('L') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    std::string libdir = argv[i+1];
                                    lib_dirs.push_back(libdir);
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing library directory:  Use -L <director> to add a library directory\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                std::string libdir = (char *)(argv[i] + 2);
                                lib_dirs.push_back(libdir);
                                ++i;
                            }
                        break;
                        case ('l') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    std::string libname = argv[i+1];
                                    libs.push_back(libname);
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing library filename:  Use -l <filename> to add a library\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                std::string libname = (char *)(argv[i] + 2);
                                libs.push_back(libname);
                                ++i;
                            }
                        break;
                        case ('O') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    optimization_level = argv[i+1];
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing optimization level:  Use -O <level> to set the optimization level\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                optimization_level = (char *)(argv[i] + 2);
                                ++i;
                            }
                        break;
                        default:
                            printf("Unknown option '%c'\n", argv[i][1]);
                            print_help();
                            exit(0);

                    }
                }

            }
            else {
                compiler.translate_file(argv[i]);
                ++i;
            }
        }
        //Once we're parsed and ready, analyze what we have
        compiler.analyze_files();

        //Then, start outputting code
        compiler.compile_program(argc, argv, output_file, include_dir, lib_dirs, libs, optimization_level);

    }
    catch (Compiler_Exception &ce) {
        //debug_print_def(p, t, "");
        //debug_print_vars(p, t);
        //debug_print(p, "");
        compiler.print_error(ce);
        exit(0);
    }
}
