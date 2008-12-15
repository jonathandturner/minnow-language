// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Common.hpp"
#include "Codegen_C.hpp"

void Codegen::codegen_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::LIST) {
        output << "Typeless_Vector__*";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : output << "void"; break;
            case (Internal_Type::BOOL) : output << "BOOL"; break;
            case (Internal_Type::INT) : output << "int"; break;
            case (Internal_Type::UINT) : output << "unsigned int"; break;
            case (Internal_Type::FLOAT) : output << "float"; break;
            case (Internal_Type::DOUBLE) : output << "double"; break;
            case (Internal_Type::STRING) : output << "Typeless_Vector__*"; break;
            case (Internal_Type::CHAR) : output << "char"; break;
            case (Internal_Type::POINTER) : output << "void*"; break;
            case (Internal_Type::OBJECT) : output << "void*"; break;
        }
    }
    else {
        output << "struct type__" << type_def_num << "*";
    }
}

void Codegen::codegen_typesig_no_tail(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::LIST) {
        output << "Typeless_Vector__";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : output << "void"; break;
            case (Internal_Type::BOOL) : output << "BOOL"; break;
            case (Internal_Type::INT) : output << "int"; break;
            case (Internal_Type::UINT) : output << "unsigned int"; break;
            case (Internal_Type::FLOAT) : output << "float"; break;
            case (Internal_Type::DOUBLE) : output << "double"; break;
            case (Internal_Type::STRING) : output << "Typeless_Vector__"; break;
            case (Internal_Type::CHAR) : output << "char"; break;
            case (Internal_Type::POINTER) : output << "void*"; break;
            case (Internal_Type::OBJECT) : output << "void*"; break;
        }
    }
    else {
        output << "struct type__" << type_def_num;
    }
}

void Codegen::codegen_tu_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::LIST) {
        output << "VoidPtr";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::BOOL) : output << "Bool"; break;
            case (Internal_Type::INT) : output << "Int32"; break;
            case (Internal_Type::UINT) : output << "UInt32"; break;
            case (Internal_Type::FLOAT) : output << "Float"; break;
            case (Internal_Type::DOUBLE) : output << "Double"; break;
            case (Internal_Type::STRING) : output << "VoidPtr"; break;
            case (Internal_Type::CHAR) : output << "Int8"; break;
            case (Internal_Type::POINTER) : output << "VoidPtr"; break;
            case (Internal_Type::OBJECT) : output << "VoidPtr"; break;
        }
    }
    else {
        output << "VoidPtr";
    }
}

void Codegen::codegen_default_value(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];
    if (td->container == Container_Type::LIST) {
        output << "NULL";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : break;
            case (Internal_Type::BOOL) : output << "FALSE"; break;
            case (Internal_Type::INT) : output << "0"; break;
            case (Internal_Type::UINT) : output << "0"; break;
            case (Internal_Type::FLOAT) : output << "0.0"; break;
            case (Internal_Type::DOUBLE) : output << "0.0"; break;
            case (Internal_Type::STRING) : output << "NULL"; break;
            case (Internal_Type::CHAR) : output << "0"; break;
            case (Internal_Type::POINTER) : output << "NULL"; break;
            case (Internal_Type::OBJECT) : output << "NULL"; break;
        }
    }
    else {
        output << "NULL";
    }
}


void Codegen::codegen_block(Program *p, Token *t, std::ostringstream &output) {
    //output << "{" << std::endl;

    for (unsigned int i = 0; i < t->children.size(); ++i) {
        codegen_token(p, t->children[i], output);
        output << ";" << std::endl;
    }

    //output << "}" << std::endl;
}

void Codegen::codegen_fun_call(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *fd = p->funs[t->definition_number];

    if (fd->external_name != "") {
        output << fd->external_name << "(";
        if (t->children.size() > 1) {
            codegen_token(p, t->children[1], output);
        }
        output << ")";
    }
    else {
        output << "fun__" << t->definition_number << "(m__";

        if (t->children.size() > 1) {
            output << ", ";
            codegen_token(p, t->children[1], output);
        }
        output << ")";
    }
}

void Codegen::codegen_method_call(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *fd = p->funs[t->children[1]->definition_number];

    if (fd->is_internal == false) {
        output << "fun__" << t->children[1]->definition_number << "(m__, ";
        codegen_token(p, t->children[0], output);

        if (t->children[1]->children.size() > 1) {
            output << ", ";
            codegen_token(p, t->children[1]->children[1], output);
        }
        output << ")";
    }
    else {
        Token *child = t->children[1];
        Type_Def *td = p->types[t->children[0]->type_def_num];
        if ((child->children[0]->contents == "push") && (td->container == Container_Type::LIST)) {
            output << "push_onto_typeless_vector__(";
            codegen_token(p, t->children[0], output);
            output << ", &";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
    }
}

void Codegen::codegen_id(Program *p, Token *t, std::ostringstream &output) {
    output << "var__" << t->definition_number;
}

void Codegen::codegen_action_call(Program *p, Token *t, std::ostringstream &output) {
    output << "{";
    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
    output << "msg__->act_data.task = fun__" << t->children[1]->definition_number  << ";" << std::endl;
    output << "msg__->recipient = ";
    codegen_token(p, t->children[0], output);
    output << ";" << std::endl;
    output << "msg__->message_type = MESSAGE_TYPE_ACTION;" << std::endl;

    Function_Def *fd = p->funs[t->children[1]->definition_number];
    unsigned int num_args = fd->arg_def_nums.size();
    Token *arg = t->children[1]->children[1];
    if (num_args > 0) {
        for (int i = (num_args - 1); i >= 0; --i) {
            if (i > 0) {
                output << "msg__->args[" << i << "].";
                codegen_tu_typesig(p, arg->children[1]->type_def_num, output);
                output << " = ";
                codegen_token(p, arg->children[1], output);
                arg = arg->children[0];
                output << ";" << std::endl;
            }
            else if (i == 0) {
                output << "msg__->args[" << i << "].";
                codegen_tu_typesig(p, arg->type_def_num, output);
                output << " = ";
                codegen_token(p, arg, output);
                output << ";" << std::endl;
            }
        }
    }
    output << "mail_to_actor__(msg__, (Actor__*)(msg__->recipient));}" << std::endl;
}

void Codegen::codegen_symbol(Program *p, Token *t, std::ostringstream &output) {
    if (t->contents == ",") {
        codegen_token(p, t->children[0], output);
        output << t->contents;
        codegen_token(p, t->children[1], output);
    }
    else if (t->contents == "=") {
        int string_id = p->global->local_types["string"];

        if ((t->children[0]->type_def_num == string_id) && (t->children[0]->type != Token_Type::VAR_DECL)) {
            output << "safe_eq__((void**)&";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ", " << t->children[0]->type_def_num << ")";
        }
        else {
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
        }
    }
    else if (t->contents == ":") {
        codegen_token(p, t->children[0], output);
    }
    else if (t->contents == "**") {
        output << "pow(";
        codegen_token(p, t->children[0], output);
        output << ",";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else if (t->contents == "<+") {
        if (t->children[0]->type_def_num == (signed)p->global->local_types["object"]) {
            if (t->children[0]->type == Token_Type::VAR_DECL) {
                codegen_token(p, t->children[0], output);
                output << " = add_primary_feature__((Object_Feature__*)";
                output << "var__" << t->children[0]->children[0]->definition_number;
                output << ", (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";

            }
            else {
                output << "add_primary_feature__((Object_Feature__*)(";
                codegen_token(p, t->children[0], output);
                output << "), (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";
            }
        }
        else {
            output << "add_child_feature__((Object_Feature__*)(";
            codegen_token(p, t->children[0], output);
            output << "), (Object_Feature__ *)";
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == "+>") {
        output << "remove_feature__((Object_Feature__*)";
        codegen_token(p, t->children[0], output);
        output << ", (Object_Feature__ *)";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else {
        output << "(";
        codegen_token(p, t->children[0], output);
        output << t->contents;
        codegen_token(p, t->children[1], output);
        output << ")";
    }
}

void Codegen::codegen_concatenate(Program *p, Token *t, std::ostringstream &output) {
    int string_id = p->global->local_types["string"];
    int char_id = p->global->local_types["char"];
    if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
        output << "concatenate_new_char_string__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else if ((t->children[0]->type_def_num == char_id) && (t->children[1]->type_def_num == string_id)) {
        output << "insert_into_new_char_string__(";
        codegen_token(p, t->children[1], output);
        output << ", ";
        codegen_token(p, t->children[0], output);
        output << ", 0)";
    }
    else if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == char_id)) {
        output << "push_onto_new_char_string__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else {
        output << "(";
        codegen_token(p, t->children[0], output);
        output << t->contents;
        codegen_token(p, t->children[1], output);
        output << ")";
    }
}

void Codegen::codegen_if(Program *p, Token *t, std::ostringstream &output) {
    unsigned int block_start = this->temp_num++;
    unsigned int block_skip = this->temp_num++;
    unsigned int block_end = this->temp_num++;

    if (t->children[1]->children.size() > 1) {
        for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
            codegen_token(p, t->children[1]->children[i], output);
            output << ";" << std::endl;
        }
        output << "if (";
        codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
        output << ") goto ifjmp" << block_start << "; else goto ifjmp" << block_skip << ";" << std::endl;
    }
    else {
        output << "if (";
        codegen_token(p, t->children[1]->children[0], output);
        output << ") goto ifjmp" << block_start << "; else goto ifjmp" << block_skip << ";" << std::endl;
    }

    output << "ifjmp" << block_start << ":" << std::endl;

    if (t->children.size() > 2) {
        //output << "{" << std::endl;

        codegen_block(p, t->children[2], output);
        output << "goto ifjmp" << block_end << ";" << std::endl;
        output << "ifjmp" << block_skip << ":" << std::endl;
        //output << "}" << std::endl;
        for (unsigned int i = 3; i < t->children.size(); ++i) {
            if (t->children[i]->type == Token_Type::ELSEIF_BLOCK) {
                Token *child = t->children[i];

                unsigned int ei_block_start = this->temp_num++;
                unsigned int ei_block_skip = this->temp_num++;

                if (child->children[1]->children.size() > 1) {
                    for (unsigned int j = 0; j < child->children[1]->children.size() - 1; ++j) {
                        codegen_token(p, child->children[1]->children[j], output);
                        output << ";" << std::endl;
                    }
                    output << "if (";
                    codegen_token(p, child->children[1]->children[child->children.size() - 1], output);
                    output << ") goto ifjmp" << ei_block_start << "; else goto ifjmp" << ei_block_skip << ";" << std::endl;
                }
                else {
                    output << "if (";
                    codegen_token(p, child->children[1]->children[0], output);
                    output << ") goto ifjmp" << ei_block_start << "; else goto ifjmp" << ei_block_skip << ";" << std::endl;
                }
                output << "ifjmp" << ei_block_start << ":" << std::endl;
                if (child->children.size() > 2) {
                    codegen_block(p, child->children[2], output);
                }
                output << "goto ifjmp" << block_end << ";" << std::endl;
                output << "ifjmp" << ei_block_skip << ":" << std::endl;
            }
            else if (t->children[i]->type == Token_Type::ELSE_BLOCK) {
                Token *child = t->children[i];

                if (child->children.size() > 1) {
                    codegen_block(p, child->children[1], output);
                }
                output << "goto ifjmp" << block_end << ";" << std::endl;
            }
        }
    }
    output << "ifjmp" << block_end << ":" << std::endl;
}

void Codegen::codegen_while(Program *p, Token *t, std::ostringstream &output) {
    unsigned int top = this->temp_num++;
    unsigned int block_start = this->temp_num++;
    unsigned int block_end = this->temp_num++;

    output << "whilejmp" << top << ":" << std::endl;
    if (t->children[1]->children.size() > 1) {
        for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
            codegen_token(p, t->children[1]->children[i], output);
            output << ";" << std::endl;
        }
        output << "if (";
        codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
        output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
    }
    else {
        output << "if (";
        codegen_token(p, t->children[1]->children[0], output);
        output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
    }

    output << "whilejmp" << block_start << ":" << std::endl;
    codegen_block(p, t->children[2], output);
    output << "goto whilejmp" << top << ";" << std::endl;
    output << "whilejmp" << block_end << ":" << std::endl;
}

void Codegen::codegen_return(Program *p, Token *t, std::ostringstream &output) {
    output << "return";
    if (t->children.size() > 1) {
        output << "(";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
}

void Codegen::codegen_array_call(Program *p, Token *t, std::ostringstream &output) {
    Var_Def *vd = p->vars[t->children[0]->definition_number];
    Type_Def *td = p->types[vd->type_def_num];

    output << "INDEX_AT__(";
    codegen_token(p, t->children[0], output);
    output << ", ";
    codegen_token(p, t->children[1], output);
    output << ", ";
    codegen_typesig(p, td->contained_type_def_num, output);
    output << ")";
}

void Codegen::codegen_new(Program *p, Token *t, std::ostringstream &output) {
    Type_Def *td = p->types[t->type_def_num];

    if (td->container == Container_Type::SCALAR) {
        output << "ctr__" << t->children[1]->definition_number << "(m__";
        if (t->children[1]->contents != ".") {
            if (t->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1], output);
            }
        }
        else {
            if (t->children[1]->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1]->children[1], output);
            }
        }
        output << ")";
    }
    else if (td->container == Container_Type::LIST) {
        output << "create_typeless_vector__(sizeof(";
        codegen_typesig(p, td->contained_type_def_num, output);
        output << "), 0)";
    }
}

void Codegen::codegen_spawn(Program *p, Token *t, std::ostringstream &output) {
    Type_Def *td = p->types[t->type_def_num];

    if (td->container == Container_Type::SCALAR) {
        output << "ctr__" << t->children[1]->definition_number << "(m__";
        if (t->children[1]->contents != ".") {
            if (t->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1], output);
            }
        }
        else {
            if (t->children[1]->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1]->children[1], output);
            }
        }
        output << ")";
    }
}

void Codegen::codegen_reference_feature(Program *p, Token *t, std::ostringstream &output) {
    if (t->children[0]->type_def_num == (signed)p->global->local_types["object"]) {
        output << "(";
        codegen_typesig(p, t->type_def_num, output);
        output << ")find_primary_feature__((Object_Feature__*)(";
        codegen_token(p, t->children[0], output);
        output << "), " << t->type_def_num << ")";
    }
    else {
        output << "(";
        codegen_typesig(p, t->type_def_num, output);
        output << ")find_feature__((Object_Feature__*)(";
        codegen_token(p, t->children[0], output);
        output << "), " << t->type_def_num << ")";
    }
}

void Codegen::codegen_continuation_site(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *owner = this->current_fun;
    ++this->cont_id;
    output << "case(" << this->cont_id << "):" << std::endl;

    if ((t->children.size() > 0) && (t->children[0]->type != Token_Type::DELETION_SITE)) {
        output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;
        codegen_token(p, t->children[0], output);
        output << ";" << std::endl;

        //Deletion if it's there
        if (t->children.size() > 1) {
            codegen_token(p, t->children[1], output);
        }

        output << "timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
        output << "if (timeslice__ == 0) {" << std::endl;

        if (t->definition_number != -1) {
            for (unsigned int i = 0; i < p->var_sites[t->definition_number].size(); ++i) {
                output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &var__" << p->var_sites[t->definition_number][i] << ");" << std::endl;
            }
        }
        output << "cont_id__ = " << this->cont_id << ";" << std::endl;
        output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
        if (owner->token->type == Token_Type::ACTION_DEF) {
            output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_ACTIVE__;" << std::endl;
            output << "return TRUE; } " << std::endl;

        }
        else {
            output << "return ";
            codegen_default_value(p, owner->return_type_def_num, output);
            output << "; } " << std::endl;
        }

    }
    else {
        if ((t->children.size() > 0) && (t->children[0]->type == Token_Type::DELETION_SITE)) {
            codegen_token(p, t->children[0], output);
        }
        output << "if (--timeslice__ == 0) {" << std::endl;
        output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;
        if (t->definition_number != -1) {
            for (unsigned int i = 0; i < p->var_sites[t->definition_number].size(); ++i) {
                output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &var__" << p->var_sites[t->definition_number][i] << ");" << std::endl;
            }
        }
        output << "cont_id__ = " << this->cont_id << ";" << std::endl;
        output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
        if (owner->token->type == Token_Type::ACTION_DEF) {
            output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_ACTIVE__;" << std::endl;
            output << "return TRUE; } " << std::endl;
        }
        else {
            output << "return ";
            codegen_default_value(p, owner->return_type_def_num, output);
            output << "; } " << std::endl;
        }
    }

}

void Codegen::codegen_deletion_site(Program *p, Token *t, std::ostringstream &output) {
    if (t->definition_number != -1) {
        std::vector<int> vars = p->var_sites[t->definition_number];

        for (unsigned int i = 0; i < vars.size(); ++i) {
            Var_Def *vd = p->vars[vars[i]];
            output << "delete__(var__" << vars[i] << ", " << vd->type_def_num << ");" << std::endl;
        }
    }
}

void Codegen::codegen_token(Program *p, Token *t, std::ostringstream &output) {

    switch (t->type) {
        case (Token_Type::BLOCK) : codegen_block(p, t, output); break;
        case (Token_Type::FUN_CALL) : codegen_fun_call(p, t, output); break;
        case (Token_Type::METHOD_CALL) : codegen_method_call(p, t, output); break;
        case (Token_Type::VAR_DECL) :
        case (Token_Type::VAR_CALL) : codegen_id(p, t, output); break;
        case (Token_Type::ARRAY_CALL) : codegen_array_call(p, t, output); break;
        case (Token_Type::FLOAT) :
        case (Token_Type::INT) :
        case (Token_Type::QUOTED_STRING) : output << t->contents; break;
        case (Token_Type::BOOL) : if (t->contents == "true") { output << "TRUE"; } else { output << "FALSE"; }; break;
        case (Token_Type::SINGLE_QUOTED_STRING) : output << "'" << t->contents << "'"; break;
        case (Token_Type::QUOTED_STRING_CONST) : output << "create_char_string_from_char_ptr__(\"" << t->contents << "\")"; break;
        case (Token_Type::ATTRIBUTE_CALL) : {
            codegen_token(p, t->children[0], output);
            output << "->";
            codegen_token(p, t->children[1], output);
        }
        break;
        case (Token_Type::COPY) : {
            output << "copy__(";
            codegen_token(p, t->children[0], output);
            output << ", " << t->children[0]->type_def_num << ")";
        }
        break;
        case (Token_Type::DELETE) : {
            output << "delete__(";
            codegen_token(p, t->children[0], output);
            output << ", " << t->children[0]->type_def_num << ")";
        }
        break;
        case (Token_Type::DELETION_SITE) : codegen_deletion_site(p, t, output); break;
        case (Token_Type::THIS) : output << "this_ptr__"; break;
        case (Token_Type::CONCATENATE) : codegen_concatenate(p, t, output); break;
        case (Token_Type::SYMBOL) : codegen_symbol(p, t, output); break;
        case (Token_Type::RETURN_CALL) : codegen_return(p, t, output); break;
        case (Token_Type::IF_BLOCK) : codegen_if(p, t, output); break;
        case (Token_Type::WHILE_BLOCK) : codegen_while(p, t, output); break;
        case (Token_Type::NEW_ALLOC) : codegen_new(p, t, output); break;
        case (Token_Type::SPAWN_ACTOR) : codegen_spawn(p, t, output); break;
        case (Token_Type::ACTION_CALL) : codegen_action_call(p, t, output); break;
        case (Token_Type::REFERENCE_FEATURE) : codegen_reference_feature(p, t, output); break;
        case (Token_Type::CONTINUATION_SITE) : codegen_continuation_site(p, t, output); break;
        default: break;
    }
}

void Codegen::codegen_constructor_internal_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal) {
                //Find the constructed type
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(Message__ *m__);" << std::endl;
            }
        }
    }
}

void Codegen::codegen_constructor_not_internal_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal == false) {
                Function_Def *fd = p->funs[i];

                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }
                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ");" << std::endl;
            }
        }
    }
}

void Codegen::codegen_action_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::ACTION_DEF) {
                output << "BOOL fun__" << i << "(Message__ *m__);" << std::endl;
            }
        }
    }
}

void Codegen::codegen_fun_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::FUN_DEF) {
                codegen_typesig(p, fd->return_type_def_num, output);
                output << " fun__" << i << "(";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }

                Scope *scope = fd->token->scope;
                while ((scope != NULL) && (scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }

                if ((scope != NULL) /*&& (scope->owner->type == Token_Type::FEATURE_DEF)*/) {
                    if (argsize == 0) {
                        output << ", ";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__";
                    }
                    else {
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__,";
                    }
                }


                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ");" << std::endl;
            }
        }
        else if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name != "")) {
            Function_Def *fd = p->funs[i];
            output << "extern ";
            codegen_typesig(p, fd->return_type_def_num, output);
            output << " "  << fd->external_name << "(";

            unsigned int argsize = fd->arg_def_nums.size();
            /*
            if (argsize == 0) {
                output << "Message__ *m__";
            }
            else {
                output << "Message__ *m__,";
            }
            */
            for (unsigned int j = 0; j < argsize; ++j) {
                if (j > 0) {
                    output << ", ";
                }
                codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                output << " var__" << fd->arg_def_nums[j];
            }
            output << ");" << std::endl;
        }
    }
}

void Codegen::codegen_constructor_internal_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal) {
                //Find the constructed type
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                //output << "type__" << scope->owner->definition_number << "*";
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(Message__ *m__)" << std::endl <<"{" << std::endl;
                if (scope->owner->type == Token_Type::ACTOR_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    //output << "type__" << scope->owner->definition_number;
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "add_actor_to_sched__((Scheduler__*)m__->sched, (Actor__*)ret_val__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    //output << "type__" << scope->owner->definition_number;
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
                    output << "msg__->message_type = MESSAGE_TYPE_CREATE_ISOLATED_ACTOR;" << std::endl;
                    output << "msg__->args[0].VoidPtr = ret_val__;" << std::endl;
                    output << "msg__->next = NULL;" << std::endl;
                    output << "send_messages__(((Scheduler__*)m__->sched)->outgoing_channel, msg__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::FEATURE_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")malloc(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "initialize_feature__(&ret_val__->base__, " << scope->owner->definition_number << ");" << std::endl;
                }
                output << "return ret_val__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_constructor_not_internal_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];

        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal == false) {
                //Find the constructed type
                Function_Def *fd = p->funs[i];
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                //output << "type__" << scope->owner->definition_number << "*";
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(";
                //output << "type__" << scope->owner->definition_number << "* this_ptr__, ";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }
                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ")" << std::endl << "{" << std::endl;

                output << "unsigned int cont_id__ = 0;" << std::endl;
                output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;

                if (scope->owner->type == Token_Type::ACTOR_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    //output << "type__" << scope->owner->definition_number;
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;

                    output << "add_actor_to_sched__((Scheduler__*)m__->sched, (Actor__*)ret_val__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    //output << "type__" << scope->owner->definition_number;
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;
                    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
                    output << "msg__->message_type = MESSAGE_TYPE_CREATE_ISOLATED_ACTOR;" << std::endl;
                    output << "msg__->args[0].VoidPtr = ret_val__;" << std::endl;
                    output << "msg__->next = NULL;" << std::endl;
                    output << "send_messages__(((Scheduler__*)m__->sched)->outgoing_channel, msg__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::FEATURE_DEF) {
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")malloc(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "initialize_feature__(&ret_val__->base__, " << scope->owner->definition_number << ");" << std::endl;

                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;
                }
                output << "return ret_val__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_action_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];

            if (fd->token->type == Token_Type::ACTION_DEF) {
                output << "BOOL fun__" << i << "(Message__ *m__)" << std::endl;
                output << "{" << std::endl;
                output << "unsigned int cont_id__ = 0;" << std::endl;
                output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;

                if (fd->token->children.size() > 2) {
                    Scope *scope_var = fd->token->children[2]->scope;
                    if (scope_var != NULL) {
                        for (std::map<std::string, unsigned int>::iterator iter = scope_var->local_vars.begin(),
                                end = scope_var->local_vars.end(); iter != end; ++iter)
                        {
                            Var_Def *vd = p->vars[iter->second];
                            codegen_typesig(p, vd->type_def_num, output);
                            output << " var__" << iter->second << ";" << std::endl;
                        }
                    }
                }

                unsigned int argsize = fd->arg_def_nums.size();
                for (unsigned int j = 0; j < argsize; ++j) {
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j] << ";" << std::endl;;
                }

                //find "this" ptr
                bool has_actor = false;
                Scope *scope = fd->token->scope;
                while (scope != NULL) {
                    if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF) || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF))) {
                        has_actor = true;
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__ = (";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << ")m__->recipient;" << std::endl;
                    }
                    scope = scope->parent;
                }

                if (fd->continuation_sites.size() > 1) {

                    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
                    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
                    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                    output << "  switch(cont_id__) {" << std::endl;

                    for (unsigned int i = 0; i < fd->continuation_sites.size(); ++i) {
                        if (fd->continuation_sites[i] != -1) {
                            if (p->var_sites[fd->continuation_sites[i]].size() > 0) {
                                output << "  case(" << i << ") : " << std::endl;
                            }
                            for (unsigned int j = 0; j < p->var_sites[fd->continuation_sites[i]].size(); ++j) {
                                Var_Def *vd = p->vars[p->var_sites[fd->continuation_sites[i]][j]];
                                output << "  var__" << p->var_sites[fd->continuation_sites[i]][j] << " = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack,";
                                output << "    ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, ";
                                codegen_typesig(p, vd->type_def_num, output);
                                output << ");" << std::endl;
                                output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                                output << "  break;" << std::endl;
                            }
                        }
                    }

                    output << "} }" << std::endl;
                    output << "else {" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        for (unsigned int j = 0; j < argsize; ++j) {
                            output << " var__" << fd->arg_def_nums[j] << " = ";
                            output << "(";
                            codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ")m__->args[" << j << "].";
                            codegen_tu_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ";" << std::endl;
                        }

                    }
                    output << "}" << std::endl;

                    this->cont_id = 0;
                    output << "switch(cont_id__) {" << std::endl;
                    output << "case(0):" << std::endl;
                    codegen_block(p, fd->token->children[2], output);
                    output << "break; }" << std::endl;
                }
                else {
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        for (unsigned int j = 0; j < argsize; ++j) {
                            output << " var__" << fd->arg_def_nums[j] << " = ";
                            output << "(";
                            codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ")m__->args[" << j << "].";
                            codegen_tu_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        codegen_block(p, fd->token->children[2], output);
                    }
                }

                if (has_actor) {
                    output << "this_ptr__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
                }
                else {
                    output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
                }
                output << "return FALSE;" << std::endl << "}" << std::endl;

            }
        }
    }
}

void Codegen::codegen_fun_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {

            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::FUN_DEF) {
                codegen_typesig(p, fd->return_type_def_num, output);
                output << " fun__" << i << "(";

                /*
                if (p->funs[i]->is_constructor) {
                    Scope *scope = p->funs[i]->token->scope;
                    while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                            (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                        scope = scope->parent;
                    }

                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " this_ptr__, ";
                }
                */

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }

                Scope *scope = fd->token->scope;
                while ((scope != NULL) && (scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }

                if ((scope != NULL) /*&& (scope->owner->type == Token_Type::FEATURE_DEF)*/) {
                    if (argsize == 0) {
                        output << ", ";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__";
                    }
                    else {
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__,";
                    }
                }


                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ")" << std::endl;
                output << "{" << std::endl;
                output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
                output << "unsigned int cont_id__ = 0;" << std::endl;

                if (fd->token->children.size() > 2) {
                    for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                            end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter)
                    {
                        Var_Def *vd = p->vars[iter->second];
                        codegen_typesig(p, vd->type_def_num, output);
                        output << " var__" << iter->second << ";" << std::endl;
                    }
                }

                if (fd->continuation_sites.size() > 1) {
                    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
                    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
                    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                    output << "  switch(cont_id__) {" << std::endl;

                    for (unsigned int i = 0; i < fd->continuation_sites.size(); ++i) {
                        if (fd->continuation_sites[i] != -1) {
                            if (p->var_sites[fd->continuation_sites[i]].size() > 0) {
                                output << "  case(" << i << ") : " << std::endl;
                            }
                            for (unsigned int j = 0; j < p->var_sites[fd->continuation_sites[i]].size(); ++j) {
                                Var_Def *vd = p->vars[p->var_sites[fd->continuation_sites[i]][j]];
                                output << "  var__" << p->var_sites[fd->continuation_sites[i]][j] << " = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack,";
                                output << "    ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, ";
                                codegen_typesig(p, vd->type_def_num, output);
                                output << ");" << std::endl;
                                output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                                output << "  break;" << std::endl;
                            }
                        }
                    }

                    output << "} }" << std::endl;
                    output << "else {" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                    }
                    output << "}" << std::endl;

                    this->cont_id = 0;
                    output << "switch(cont_id__) {" << std::endl;
                    output << "case(0):" << std::endl;

                    codegen_block(p, fd->token->children[2], output);
                    output << "}" << std::endl;
                }
                else {
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        codegen_block(p, fd->token->children[2], output);
                    }
                }
                output << "((Actor__*)m__->recipient)->timeslice_remaining = --timeslice__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_copy_predecl(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    output << "void *copy__(void *v__, unsigned int t__);" << std::endl;
}

void Codegen::codegen_delete_predecl(Program *p, std::ostringstream &output) {
    output << "void delete__(void *v__, unsigned int t__);" << std::endl;
}

void Codegen::codegen_copy_decl(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    unsigned int obj_id = p->global->local_types["object"];
    unsigned int string_id = p->global->local_types["string"];
    output << "void *copy__(void *v__, unsigned int t__)" << std::endl << "{" << std::endl;
    output << "if (v__ == NULL) return NULL;" << std::endl;

    output << "switch(t__)" << "{" << std::endl;
    output << "  case(" << string_id << "): {" << std::endl << "  ";
    codegen_typesig(p, string_id, output);
    output << "  ret_val__;" << std::endl;
    output << "  ret_val__ = create_char_string__(0);" << std::endl;
    output << "  concatenate_char_string__(ret_val__, (";
    codegen_typesig(p, string_id, output);
    output << ")v__);" << std::endl;
    output << "  return ret_val__;" << std::endl;
    output << "  }; break;" << std::endl;
    for (unsigned int i = obj_id; i < p->types.size(); ++i) {
        output << "  case(" << i << "): {" << std::endl;
        Type_Def *td = p->types[i];

        if (i == obj_id) {
            output << "  Object_Feature__ *ret_val__ = (Object_Feature__*)copy__(v__, ((Object_Feature__*)v__)->feature_id);" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        else if (td->container == Container_Type::LIST) {
            output << "  unsigned int i;" << std::endl;
            output << "  Typeless_Vector__ *ret_val__ = create_typeless_vector__(((Typeless_Vector__ *)v__)->elem_size, ((Typeless_Vector__ *)v__)->current_size);" << std::endl;
            output << "  for (i = 0; i < ((Typeless_Vector__ *)v__)->current_size; ++i)" << std::endl << "  {" << std::endl;
            output << "    INDEX_AT__(ret_val__, i, ";
            codegen_typesig(p, td->contained_type_def_num, output);
            output << ") = ";
            if ((td->contained_type_def_num >= (signed)obj_id) || (td->contained_type_def_num == (signed)string_id)) {
                output << "(";
                codegen_typesig(p, td->contained_type_def_num, output);
                output << ")copy__(INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_num, output);
                output << "), " << td->contained_type_def_num << ");" << std::endl;
            }
            else {
                output << "INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_num, output);
                output << ");" << std::endl;
            }
            output << "  }" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        else if ((td->token->type == Token_Type::ACTOR_DEF) || (td->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            //do nothing
        }
        else if (td->token->type == Token_Type::FEATURE_DEF) {
            output << "  Object_Feature__ *ret_val__ = (Object_Feature__ *)create_feature__(sizeof(";
            codegen_typesig_no_tail(p, i, output);
            output << "), " << i << ");" << std::endl;

            std::ostringstream copy_fn;
            copy_fn << "copy__" << i;
            if (td->token->scope->local_funs.find(copy_fn.str()) != td->token->scope->local_funs.end()) {
                //Function_Def *delete_fd = p->funs[td->token->scope->local_funs["delete"]];
                output << "  fun__" << td->token->scope->local_funs[copy_fn.str()] << "( (";
                codegen_typesig(p, i, output);
                output << ")ret_val__, (";
                codegen_typesig(p, i, output);
                output << ")v__);" << std::endl;
            }
            else {
                for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_vars.begin(),
                    end = td->token->scope->local_vars.end(); iter != end; ++iter) {
                    Var_Def *vd = p->vars[iter->second];
                    if ((vd->type_def_num >= obj_id) || (vd->type_def_num == string_id)) {
                        output << "  ((";
                        codegen_typesig(p, i, output);
                        output << ")ret_val__)->var__" << iter->second << " = (";
                        codegen_typesig(p, vd->type_def_num, output);
                        output << ")copy__(((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__"
                            << iter->second << ", " << vd->type_def_num << ");" << std::endl;
                    }
                    else {
                        output << "  ((";
                        codegen_typesig(p, i, output);
                        output << ")ret_val__)->var__" << iter->second << " = ((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__" << iter->second << ";" << std::endl;
                    }
                }
            }
            output << "  if (((Object_Feature__*)v__)->next != NULL) {" << std::endl;
            output << "    ret_val__->next = (Object_Feature__*)copy__( ((Object_Feature__*)v__)->next, ((Object_Feature__*)(((Object_Feature__*)v__)->next))->feature_id);" << std::endl;
            output << "  }" << std::endl;
            output << "  else {" << std::endl;
            output << "    ret_val__->next = NULL;" << std::endl;
            output << "  }" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        output << "  }; break;" << std::endl;
    }

    output << "}" << std::endl;

    output << "}" << std::endl;
}

void Codegen::codegen_delete_decl(Program *p, std::ostringstream &output) {
    unsigned int obj_id = p->global->local_types["object"];
    unsigned int string_id = p->global->local_types["string"];

    output << "void delete__(void *v__, unsigned int t__)" << std::endl << "{" << std::endl;
    output << "if (v__ == NULL) return;" << std::endl;
    output << "switch(t__)" << "{" << std::endl;
    output << "  case(" << string_id << "): {" << std::endl;
    output << "  delete_char_string__ ((";
    codegen_typesig(p, string_id, output);
    output << ")v__);" << std::endl;
    output << "  }; break;" << std::endl;

    for (unsigned int i = obj_id; i < p->types.size(); ++i) {
        output << "  case(" << i << "): {" << std::endl;
        Type_Def *td = p->types[i];

        if (i == obj_id) {
            output << "  delete__(v__, ((Object_Feature__*)v__)->feature_id);" << std::endl;
        }
        else if (td->container == Container_Type::LIST) {
            output << "  unsigned int i;" << std::endl;
            //Type_Def *contained = p->types[td->contained_type_def_num];
            //if ((contained->token->type != Token_Type::ACTOR_DEF) && (contained->token->type != Token_Type::ISOLATED_ACTOR_DEF)) {
            if ((td->contained_type_def_num >= (signed)obj_id) || (td->contained_type_def_num == (signed)string_id)) {
                output << "  for (i = 0; i < ((Typeless_Vector__ *)v__)->current_size; ++i)" << std::endl << "  {" << std::endl;
                output << "    delete__(INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_num, output);
                output << "), " << td->contained_type_def_num << ");" << std::endl;
                output << "  }" << std::endl;
            }
            output << "  delete_typeless_vector__((Typeless_Vector__ *)v__);" << std::endl;
        }
        else if ((td->token->type == Token_Type::ACTOR_DEF) || (td->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            //do nothing
        }
        else if (td->token->type == Token_Type::FEATURE_DEF) {
            if (td->token->scope->local_funs.find("delete") != td->token->scope->local_funs.end()) {
                //Function_Def *delete_fd = p->funs[td->token->scope->local_funs["delete"]];
                output << "  fun__" << td->token->scope->local_funs["delete"] << "(NULL,  (";
                codegen_typesig(p, i, output);
                output << ")v__);" << std::endl;
            }
            else {
                for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_vars.begin(),
                        end = td->token->scope->local_vars.end(); iter != end; ++iter) {
                    Var_Def *vd = p->vars[iter->second];
                    if ((vd->type_def_num >= obj_id) || (vd->type_def_num == string_id)) {
                        output << "  delete__(((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__"
                            << iter->second << ", " << vd->type_def_num << ");" << std::endl;
                    }
                }
            }
            output << "  if (((Object_Feature__*)v__)->next != NULL) {" << std::endl;
            output << "    delete__( ((Object_Feature__*)v__)->next, ((Object_Feature__*)(((Object_Feature__*)v__)->next))->feature_id);" << std::endl;
            output << "  }" << std::endl;
            output << "  free(v__);" << std::endl;
        }
        output << "  }; break;" << std::endl;
    }
    output << "}" << std::endl;

    output << "}" << std::endl;
}

void Codegen::codegen_safe_eq_predecl(Program *p, Token *token, std::ostringstream &output) {
    output << "void *safe_eq__(void **lhs__, void *rhs__, unsigned int t__);" << std::endl;
}

void Codegen::codegen_safe_eq_decl(Program *p, Token *token, std::ostringstream &output) {
    output << "void *safe_eq__(void **lhs__, void *rhs__, unsigned int t__) {" << std::endl;
    output << "  if (*lhs__ != NULL) {" << std::endl;
    output << "    delete__(*lhs__, t__);" << std::endl;
    output << "  }" << std::endl;
    output << "  *lhs__ = rhs__;" << std::endl;
    output << "}" << std::endl;
}

void Codegen::codegen_class_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->types.size(); ++i) {
        if ((p->types[i]->is_internal == false) && (p->types[i]->container == Container_Type::SCALAR)) {
            output << "struct type__" << i << ";" << std::endl;
        }
    }
}

void Codegen::codegen_class_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->types.size(); ++i) {

        if ((p->types[i]->is_internal == false) && (p->types[i]->container == Container_Type::SCALAR)) {
            output << "struct type__" << i << "{" << std::endl;
            if ((p->types[i]->token->type == Token_Type::ACTOR_DEF) || (p->types[i]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                output << "Actor__ base__;" << std::endl;
            }
            else if (p->types[i]->token->type == Token_Type::FEATURE_DEF) {
                output << "Object_Feature__ base__;" << std::endl;
            }
            for (std::map<std::string, unsigned int>::iterator iter = p->types[i]->token->scope->local_vars.begin(),
                    end = p->types[i]->token->scope->local_vars.end(); iter != end; ++iter)
            {
                Var_Def *vd = p->vars[iter->second];
                codegen_typesig(p, vd->type_def_num, output);
                output << " var__" << iter->second << ";" << std::endl;
            }
            output << "};" << std::endl;
        }
    }
}

void Codegen::codegen_main_action(Program *p, std::ostringstream &output) {
    std::map<std::string, unsigned int>::iterator iter = p->global->local_funs.find("main");
    bool use_cmdline = false;

    if (iter == p->global->local_funs.end()) {
        Position pos;
        pos.line = 0;

        //if the blank one isn't find, look for the commandline one
        std::ostringstream typename_t;
        typename_t << "Con___" << p->global->local_types["string"];
        std::map<std::string, unsigned int>::iterator iter_t = p->global->local_types.find(typename_t.str());

        if (iter_t == p->global->local_types.end()) {
            throw Compiler_Exception("Main action not found or incorrect main action", pos);
        }

        std::ostringstream mainname;
        mainname << "main__" << iter_t->second;
        iter = p->global->local_funs.find(mainname.str());

        if (iter == p->global->local_funs.end()) {
            throw Compiler_Exception("Main action not found or incorrect main action", pos);
        }
        else {
            use_cmdline = true;
        }
    }
    Function_Def *main_action = p->funs[iter->second];
    if (main_action->token->type != Token_Type::ACTION_DEF) {
        throw Compiler_Exception("Main action not found.  Use 'action main' to define a starting point in your application",
                main_action->token->start_pos);
    }


    output << "int main(int argc, char *argv[]) {" << std::endl;
    if (use_cmdline == false) {
        output << "aquarium_main__(argc, argv, fun__" << iter->second << ", FALSE);" << std::endl;
    }
    else {
        output << "aquarium_main__(argc, argv, fun__" << iter->second << ", TRUE);" << std::endl;
    }
    output << "}" << std::endl;
}

void Codegen::codegen(Program *p, Token *t, std::ostringstream &output) {
    internal_type_map[p->global->local_types["void"]] = Internal_Type::VOID;
    internal_type_map[p->global->local_types["int"]] = Internal_Type::INT;
    internal_type_map[p->global->local_types["bool"]] = Internal_Type::BOOL;
    internal_type_map[p->global->local_types["uint"]] = Internal_Type::UINT;
    internal_type_map[p->global->local_types["string"]] = Internal_Type::STRING;
    internal_type_map[p->global->local_types["float"]] = Internal_Type::FLOAT;
    internal_type_map[p->global->local_types["double"]] = Internal_Type::DOUBLE;
    internal_type_map[p->global->local_types["char"]] = Internal_Type::CHAR;
    internal_type_map[p->global->local_types["pointer"]] = Internal_Type::POINTER;
    internal_type_map[p->global->local_types["object"]] = Internal_Type::OBJECT;

    output << "#include <Aquarium.hpp>" << std::endl;
    output << "#include <math.h>" << std::endl;

    codegen_class_predecl(p, t, output);
    codegen_constructor_internal_predecl(p, t, output);
    codegen_constructor_not_internal_predecl(p, t, output);
    codegen_fun_predecl(p, t, output);
    codegen_action_predecl(p, t, output);
    codegen_copy_predecl(p, 0, output);
    codegen_delete_predecl(p, output);
    codegen_safe_eq_predecl(p, t, output);

    codegen_class_decl(p, t, output);
    codegen_constructor_internal_decl(p, t, output);
    codegen_constructor_not_internal_decl(p, t, output);
    codegen_fun_decl(p, t, output);
    codegen_action_decl(p, t, output);
    codegen_copy_decl(p, 0, output);
    codegen_delete_decl(p, output);
    codegen_safe_eq_decl(p, t, output);

    codegen_main_action(p, output);


}
