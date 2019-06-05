#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use File::Copy;
use Getopt::Long;
use Cwd 'abs_path';
use File::Basename;
use lib abs_path(dirname(__FILE__));
use File::Path qw(make_path);
use easyjson;
no warnings 'experimental::smartmatch';

my $usage = <<EOF;
Usage: $0 [OPTION] [JSON_FILE(s)]
Generate operator defination code from operator description JSON.
Read the JSON string from standard input if JSON_FILE(s) are not given.
Print the output code to standard output if --dir and --root are omited.

Options:
  -h, --help              print this message
  -d, --dir=DIRECTORY     save operator defination file(s) in DIRECTORY
  -r, --root=ROOT         set project root directory; this option will save
                          operator defination file(s) in ROOT/src/op/auto, and
                          add operator declarations and such to
                          ROOT/src/arch/ln_arch_*.c
Author: Zhao Zhixu
EOF

my $INDENT_OFFSET = 4;
my $INDENT_SPACE = " "x$INDENT_OFFSET;

my $root = '';
my $dir = '';
GetOptions(
           'help' => sub {&exit_msg(0, $usage)},
           'dir=s' => \$dir,
           'root=s' => \$root,
          ) or &exit_msg(1, $usage);

my @json_files = @ARGV;
if (@json_files == 0) {
    my $json_text = join '', <STDIN>;
    &parse_and_generate($json_text);
} else {
    foreach my $json_file (@json_files) {
        open JSON_FILE, '<', $json_file or die "Cannot open $json_file: $!";
        my $json_text = join '', <JSON_FILE>;
        close JSON_FILE;
        &parse_and_generate($json_text, $json_file);
    }
}

sub parse_and_generate {
    my $json_text = shift;
    my $json_file = shift;
    $json_text = easyjson::easy_to_json($json_text);
    my $json_obj = JSON->new->relaxed();
    my $json = $json_obj->decode($json_text);
    if (exists $json->{ops}) {
        foreach my $op (@{$json->{ops}}) {
            next if exists $_->{autogen} and not $_->{autogen};
            &gen_code($op, $json_file);
        }
    } elsif (exists $json->{optype}) {
        return if exists $json->{autogen} and not $json->{autogen};
        &gen_code($json, $json_file);
    } else {
        err_exit("JSON doesn't contain an 'ops' or 'optype' field");
    }
}

sub gen_code {
    my $op = shift;
    my $file = shift;
    &err_exit("needs a `optype`") unless exists $op->{optype};
    my $optype = $op->{optype};
    &err_exit("'${optype}' needs a `tensors_in`") unless exists $op->{tensors_in};
    my $tensors_in = $op->{tensors_in};
    &err_exit("'${optype}' needs a `tensors_out`") unless exists $op->{tensors_out};
    my $tensors_out = $op->{tensors_out};
    &err_exit("'${optype}' needs a `params`") unless exists $op->{params};
    my $params = $op->{params};

    &err_exit("'${optype}' needs an `arch`") unless exists $op->{arch};
    my $arch = $op->{arch};
    if ($arch ne "none" and $arch ne "cpu" and $arch ne "cuda" and
        $arch ne "cudnn" and $arch ne "tensorrt") {
        &err_exit("'${optype}' has unsupported `arch` '${arch}'");
    }
    if ($arch ne "none" and not $optype =~ /\w+_$arch$/) {
        &err_exit("'${optype}' doesn't match `arch` '${arch}'");
    }

    my @blocks = ();
    push @blocks, &gen_head_block($op, $file);
    push @blocks, &gen_struct_def($op);
    push @blocks, &gen_pre_run($op);
    push @blocks, &gen_static_run($op) if exists $op->{static_run};
    push @blocks, &gen_run($op) if exists $op->{run};
    push @blocks, &gen_post_run($op);
    push @blocks, &gen_calc_offset($op) if exists $op->{calc_offset};
    push @blocks, &gen_op_arg($op);
    push @blocks, &gen_op_impl($op);

    my $code_str = join "\n", @blocks;
    if (not $dir and not $root) {
        print $code_str;
    }
    if ($dir) {
        unless (-d $dir) {
            make_path($dir, {mode => 0755})
                or die "Cannot create directory $dir: $!";
        }
        my $dir_file = "${dir}/ln_opimpl_${optype}.c";
        &backup_write($dir_file, $code_str);
    }
    if ($root) {
        unless (-d "${root}/src/op/auto") {
            make_path("${root}/src/op/auto", {mode => 0755})
                or die "Cannot create directory ${root}/src/op/auto: $!";
        }
        my $src_file = "${root}/src/op/auto/ln_opimpl_${optype}.c";
        &backup_write($src_file, $code_str);
        my $arch_file = "${root}/src/arch/ln_archimpl_${arch}.c";
        &add_to_arch_file($arch_file, $optype, $arch);
    }
}

sub backup_write {
    my $file = shift;
    my $str = shift;
    if (-e $file) {
        &warn_msg("${file} exists, backuped with subfix .bak");
        copy($file, "${file}.bak")
            or die "Cannot backup file ${file}.bak: $!";
    }
    open FILE, '>', $file or die "Cannot open $file: $!";
    print FILE $str;
    close FILE;
}

sub add_to_arch_file {
    my $arch_file = shift;
    my $optype = shift;
    my $arch = shift;

    my $declare = "extern ln_op ln_opimpl_${optype};";
    my $item = "&ln_opimpl_${optype},";

    copy($arch_file, "${arch_file}.bak")
        or die "Cannot backup file ${arch_file}: $!";
    open ARCH_FILE_BAK, '<', "${arch_file}.bak"
        or die "Cannot open ${arch_file}.bak: $!";
    open ARCH_FILE, '>', $arch_file
        or die "Cannot open ${arch_file}: $!";

    my $declared = 0;
    my $inited = 0;
    while (<ARCH_FILE_BAK>) {
        $declared = 1 if /$declare$/;
        s|/\* end of declare $arch ops \*/|$declare\n/* end of declare $arch ops */|
            unless $declared;
        $inited = 1 if /$item$/;
        s|/\* end of init $arch ops \*/|    $item\n/* end of init $arch ops */|
            unless $inited;
        print ARCH_FILE;
    }

    close ARCH_FILE;
    close ARCH_FILE_BAK;
}

sub gen_head_block {
    my $op = shift;
    my $file = shift;
    &err_exit("'$op->{optype}' needs an `author`") unless exists $op->{author};
    my $author = $op->{author};
    my @headers = ();
    push @headers, "#include <assert.h>";
    push @headers, "#include \"ln_op.h\"";
    push @headers, "#include \"ln_arch.h\"";
    if ($op->{arch} eq "cuda") {
        push @headers, "#include \"ln_cuda.h\"";
      }
    if ($op->{arch} eq "cudnn") {
      push @headers, "#include \"ln_cudnn.h\"";
    }
    if ($op->{arch} eq "tensorrt") {
        push @headers, "#include \"ln_tensorrt.h\"";
    }
    my $headers_str = join "\n", @headers;
    my $head_block_tpl = <<EOF;
/*
 * Copyright (c) 2018-2019 $author
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* NOTE: this file is automatically generated by $file
   using $0 */

$headers_str
EOF
}

sub gen_struct_def {
    my $op = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @defs = ();
    foreach (@$tensors_in) {
        &err_exit("'$op->{optype}' needs an `arg_name` in one of the  `tensors_in`")
            unless exists $_->{arg_name};
        push @defs, "ln_tensor_entry *$_->{arg_name}_entry;";
    }
    foreach (@$tensors_out) {
        &err_exit("'$op->{optype}' needs an `arg_name` in one of the `tensors_out`")
            unless exists $_->{arg_name};
        push @defs, "ln_tensor_entry *$_->{arg_name}_entry;";
    }
    if (exists $op->{extra_privs}) {
        my $extra_privs = $op->{extra_privs};
        foreach (@$extra_privs) {
            &err_exit("'$op->{optype}' needs an `type` and `name` in one of the `extra_privs`")
                unless exists $_->{type} and exists $_->{name};
            push @defs, "$_->{type} $_->{name};";
        }
    }
    &gen_params($op, 1, 0, \@defs);
    &make_defs_neat(\@defs);
    &indent_lines($INDENT_OFFSET, \@defs);

    my $defs_str = join "\n", @defs;
    my $struct_def_tpl = <<EOF;
struct priv_s {
${defs_str}
};
EOF
}

sub gen_pre_run {
    my $op = shift;
    my $pre_run_local_vars = &gen_pre_run_local_vars($op);
    my $pre_run_checks = &gen_pre_run_checks($op);
    my $output_tensor_def = &gen_output_tensor_def($op);
    my $priv_assigns = &gen_priv_assigns($op);

    my $pre_run_str = <<EOF;
/* This function should do the parameter checking and tensor shape inference. */
static void $op->{optype}_pre_run(ln_op_arg *op_arg)
{
${pre_run_local_vars}
    /* check tensors and parameters */
${pre_run_checks}
    /* define output tensor shape, tensor data should be NULL */
${output_tensor_def}
    /* use op_arg->priv to store private data to be used in other functions */
${priv_assigns}
}
EOF
}

sub gen_pre_run_local_vars {
    my $op = shift;
    my $code_str = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @vars = ();
    foreach (@$tensors_in) {
        push @vars, "char *$_->{arg_name}_name;";
        push @vars, "ln_tensor_list_entry *$_->{arg_name}_list_entry;";
        push @vars, "ln_tensor_entry *$_->{arg_name}_entry;";
        push @vars, "tl_tensor *$_->{arg_name};";
    }
    foreach (@$tensors_out) {
        push @vars, "char *$_->{arg_name}_name;";
        push @vars, "ln_tensor_list_entry *$_->{arg_name}_list_entry;";
        push @vars, "ln_tensor_entry *$_->{arg_name}_entry;";
        push @vars, "tl_tensor *$_->{arg_name};";
        push @vars, "int $_->{arg_name}_ndim;";
        push @vars, "int *$_->{arg_name}_dims;";
        push @vars, "tl_dtype $_->{arg_name}_dtype;";
    }
    &gen_params($op, 1, 1, \@vars);

    push @vars, "int tensors_in_n;";
    push @vars, "int tensors_out_n;";
    push @vars, "int params_n;";
    push @vars, "struct priv_s *priv;";
    &make_defs_neat(\@vars);
    &indent_lines($INDENT_OFFSET, \@vars);
    push @vars, "";

    join "\n", @vars;
}

sub get_param_type {
    my $param = shift;

    &err_exit("'$param->{arg_name}' needs a `ptype`")
        unless exists $param->{ptype};
    my $realtype;
    given ($param->{ptype}) {
        when ("LN_PARAM_NULL") {
            $realtype = "void *";
        }
        when ("LN_PARAM_STRING") {
            if (exists $param->{realtype}) {
                $realtype = $param->{realtype};
            } else {
                $realtype = "char *";
            }
        }
        when ("LN_PARAM_NUMBER") {
            &err_exit("$param->{arg_name} needs a `realtype`")
                unless exists $param->{realtype};
            if ($param->{realtype} eq "float" ||
                $param->{realtype} eq "double"||
                $param->{realtype} eq "int") {
                $realtype = $param->{realtype};
            } else {
                &err_exit("unsupported `realtype`: '$param->{realtype}'");
            }
        }
        when ("LN_PARAM_BOOL") {
            $realtype = "tl_bool_t";
        }
        when ("LN_PARAM_ARRAY_STRING") {
            if (exists $param->{realtype}) {
                $realtype = "$param->{realtype} *";
            } else {
                $realtype = "char **";
            }
        }
        when ("LN_PARAM_ARRAY_NUMBER") {
            &err_exit("$param->{arg_name} needs a `realtype`")
                unless exists $param->{realtype};
            if ($param->{realtype} eq "float" ||
                $param->{realtype} eq "double"||
                $param->{realtype} eq "int") {
                $realtype = "$param->{realtype} *";
            } else {
                &err_exit("$param->{arg_name} has unsupported `realtype`: '$param->{realtype}'");
            }
        }
        when ("LN_PARAM_ARRAY_BOOL") {
            $realtype = "tl_bool_t *";
        }
        default {
            &err_exit("$param->{arg_name} has unsupported `ptype`: '$param->{ptype}'");
        }
    }
    $realtype .= " " unless ($realtype =~ /\*$/);
    $realtype;
}

sub gen_params {
    my $op = shift;
    my $params = $op->{params};
    my $do_gen_entry = shift;
    my $do_gen_param = shift;
    my $defs = shift;
    foreach my $param (@$params) {
        &err_exit("'$op->{optype}' needs an `arg_name` in one of the `params`")
            unless exists $param->{arg_name};
        if ($do_gen_param) {
            my $type = &get_param_type($param);
            push @$defs, "${type}$param->{arg_name};";
        }
        if ($do_gen_entry) {
            push @$defs, "ln_param_entry *$param->{arg_name}_entry;";
        }
    }
}

sub gen_pre_run_checks {
    my $op = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @states = ();
    my $tensors_in_n = @$tensors_in;
    push @states, "tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);";
    push @states, "ln_opck_tensors_in_len_eq(tensors_in_n, ${tensors_in_n});";
    push @states, "";
    foreach my $tensor (@$tensors_in) {
        my $arg_name = $tensor->{arg_name};
        push @states, "${arg_name}_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, \"${arg_name}\");";
        push @states, "ln_opck_tensor_in_exist(${arg_name}_list_entry, \"${arg_name}\");";
        push @states, "${arg_name}_name = ${arg_name}_list_entry->name;";
        push @states, "${arg_name}_entry = ln_tensor_table_find(op_arg->tensor_table, ${arg_name}_name);";
        push @states, "ln_opck_tensor_defined(${arg_name}_entry, ${arg_name}_name);";
        push @states, "${arg_name} = ${arg_name}_entry->tensor;";
        # to subpress -Wunused-but-set-variable
        push @states, "${arg_name} = ${arg_name};";
        &err_exit("'$tensor->{arg_name}' needs a `mtype`") unless exists $tensor->{mtype};
        # maybe not so strict in none arch
        push @states, "ln_opck_tensor_mtype_eq(${arg_name}_entry, $tensor->{mtype});" unless $tensor->{mtype} eq "LN_MEM_NONE";
        if (exists $tensor->{dtype}) {
            push @states, "ln_opck_tensor_dtype_eq(${arg_name}_entry, $tensor->{dtype});";
        }
        if (exists $tensor->{ndim}) {
            push @states, "ln_opck_tensor_ndim(${arg_name}_entry, $tensor->{ndim});";
        }
        if (exists $tensor->{len}) {
            push @states, "ln_opck_tensor_len(${arg_name}_entry, $tensor->{len});";
        }
        if (exists $tensor->{sametype}) {
            push @states, "ln_opck_tensor_issametype(${arg_name}_entry, $tensor->{sametype}_entry);";
        }
        if (exists $tensor->{sameshape}) {
            push @states, "ln_opck_tensor_issameshape(${arg_name}_entry, $tensor->{sameshape}_entry);";
        }
        if (exists $tensor->{static}) {
            if ($tensor->{static}) {
                push @states, "ln_opck_tensor_isstatic(${arg_name}_entry);";
            } else {
                push @states, "ln_opck_tensor_isnotstatic(${arg_name}_entry);";
            }
        }
        if (exists $tensor->{check}) {
            if ($tensor->{check} =~ /,/) {
                push @states, "ln_opck_satisfy_msg($tensor->{check});";
            } else {
                push @states, "ln_opck_satisfy($tensor->{check});";
            }
        }
        if (exists $tensor->{checks}) {
            my $checks = $tensor->{checks};
            foreach (@$checks) {
                if (exists $_->{check}) {
                    if ($_->{check} =~ /,/) {
                        push @states, "ln_opck_satisfy_msg($_->{check});";
                    } else {
                        push @states, "ln_opck_satisfy($_->{check});";
                    }
                } else {
                    &err_exit("tensor '${arg_name}' expects a `check` in `checks`");
                }
            }
        }
        if (exists $tensor->{custom}) {
            &add_custom_block($tensor->{custom}, \@states);
        }
        push @states, "";
    }

    my $tensors_out_n = @$tensors_out;
    push @states, "tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);";
    push @states, "ln_opck_tensors_out_len_eq(tensors_out_n, ${tensors_out_n});";
    push @states, "";
    foreach my $tensor (@$tensors_out) {
        my $arg_name = $tensor->{arg_name};
        push @states, "${arg_name}_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, \"${arg_name}\");";
        push @states, "ln_opck_tensor_out_exist(${arg_name}_list_entry, \"${arg_name}\");";
        push @states, "${arg_name}_name = ${arg_name}_list_entry->name;";
        push @states, "${arg_name}_entry = ln_tensor_table_find(op_arg->tensor_table, ${arg_name}_name);";
        push @states, "ln_opck_tensor_not_defined(${arg_name}_entry, ${arg_name}_name);";
        push @states, "";
    }

    my $params_n = @$params;
    push @states, "params_n = ln_param_list_length(op_arg->params);";
    push @states, "ln_opck_params_len_eq(params_n, ${params_n});";
    push @states, "";
    foreach my $param (@$params) {
        my $arg_name = $param->{arg_name};
        push @states, "${arg_name}_entry = ln_param_list_find(op_arg->params, \"${arg_name}\");";
        push @states, "ln_opck_param_exist(${arg_name}_entry, \"${arg_name}\");";
        push @states, "ln_opck_param_type(${arg_name}_entry, $param->{ptype});";
        given ($param->{ptype}) {
            when ("LN_PARAM_NULL") {
                push @states, "${arg_name} = NULL;";
            }
            when ("LN_PARAM_STRING") {
                if (exists $param->{realtype}) {
                    if (exists $param->{from_func}) {
                        push @states, "${arg_name} = $param->{from_func}(${arg_name}_entry->value_string);";
                        push @states, "${arg_name}_entry->value_$param->{realtype} = ${arg_name};";
                    } else {
                        &err_exit("$param->{arg_name} needs a `from_func` to convert '${arg_name}'");
                    }
                } else {
                    push @states, "${arg_name} = ${arg_name}_entry->value_string;";
                }
            }
            when ("LN_PARAM_NUMBER") {
                if ($param->{realtype} eq "float" ||
                    $param->{realtype} eq "double"||
                    $param->{realtype} eq "int") {
                    push @states, "${arg_name} = ${arg_name}_entry->value_$param->{realtype};";
                    if (exists $param->{eq}) {
                        push @states, "ln_opck_param_$param->{realtype}_eq(${arg_name}_entry, $param->{eq});";
                    }
                    if (exists $param->{gt}) {
                        push @states, "ln_opck_param_$param->{realtype}_gt(${arg_name}_entry, $param->{gt});";
                    }
                    if (exists $param->{ge}) {
                        push @states, "ln_opck_param_$param->{realtype}_ge(${arg_name}_entry, $param->{ge});";
                    }
                    if (exists $param->{lt}) {
                        push @states, "ln_opck_param_$param->{realtype}_lt(${arg_name}_entry, $param->{lt});";
                    }
                    if (exists $param->{le}) {
                        push @states, "ln_opck_param_$param->{realtype}_le(${arg_name}_entry, $param->{le});";
                    }
                    if (exists $param->{ne}) {
                        push @states, "ln_opck_param_$param->{realtype}_ne(${arg_name}_entry, $param->{ne});";
                    }
                } else {
                    &err_exit("$param->{arg_name} has unsupported `realtype`: '$param->{realtype}'");
                }
            }
            when ("LN_PARAM_BOOL") {
                push @states, "${arg_name} = ${arg_name}_entry->value_bool;";
            }
            when (/^LN_PARAM_ARRAY/) {
                if (exists $param->{len}) {
                    push @states, "ln_opck_param_array_len_eq(${arg_name}_entry, $param->{len});";
                } continue ;
            }
            when ("LN_PARAM_ARRAY_STRING") {
                if (exists $param->{realtype}) {
                    if (exists $param->{from_func}) {
                        push @states, "${arg_name} = ln_alloc(sizeof($param->{realtype})*${arg_name}_entry->array_len);";
                        push @states, "${arg_name}_entry->value_array_$param->{realtype} = ${arg_name}";
                        push @states, "for (int i = 0; i < ${arg_name}_entry->array_len; i++)";
                        push @states, &indent_line($INDENT_OFFSET, "${arg_name}[i] = $param->{from_func}(${arg_name}_entry->value_array_string[i]);");
                    } else {
                        &err_exit("$param->{arg_name} needs a `from_func` to convert '${arg_name}'");
                    }
                } else {
                    push @states, "${arg_name} = ${arg_name}_entry->value_array_string;";
                }
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
                if ($param->{realtype} eq "float" ||
                    $param->{realtype} eq "double"||
                    $param->{realtype} eq "int") {
                    push @states, "${arg_name} = ${arg_name}_entry->value_array_$param->{realtype};";
                    if (exists $param->{eq}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_eq(${arg_name}_entry, $param->{eq});";
                    }
                    if (exists $param->{gt}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_gt(${arg_name}_entry, $param->{gt});";
                    }
                    if (exists $param->{ge}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_ge(${arg_name}_entry, $param->{ge});";
                    }
                    if (exists $param->{lt}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_lt(${arg_name}_entry, $param->{lt});";
                    }
                    if (exists $param->{le}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_le(${arg_name}_entry, $param->{le});";
                    }
                    if (exists $param->{ne}) {
                        push @states, "ln_opck_param_array_$param->{realtype}_ne(${arg_name}_entry, $param->{ne});";
                    }
                } else {
                    &err_exit("$param->{arg_name} has unsupported `realtype`: '$param->{realtype}'");
                }
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                push @states, "${arg_name} = ${arg_name}_entry->value_array_bool;";
            }
            default {
                &err_exit("$param->{arg_name} has unsupported `ptype`: '$param->{ptype}'");
            }
        }
        # to subpress -Wunused-but-set-variable
        push @states, "${arg_name} = ${arg_name};";
        if (exists $param->{check}) {
            if ($param->{check} =~ /,/) {
                push @states, "ln_opck_satisfy_msg($param->{check});";
            } else {
                push @states, "ln_opck_satisfy($param->{check});";
            }
        }
        if (exists $param->{checks}) {
            my $checks = $param->{checks};
            foreach (@$checks) {
                if (exists $_->{check}) {
                    if ($_->{check} =~ /,/) {
                        push @states, "ln_opck_satisfy_msg($_->{check});";
                    } else {
                        push @states, "ln_opck_satisfy($_->{check});";
                    }
                } else {
                    &err_exit("$param->{arg_name} expects a `check` in `checks`");
                }
            }
        }
        if (exists $param->{custom}) {
            &add_custom_block($param->{custom}, \@states);
        }
        push @states, "";
    }

    if (exists $op->{checks}) {
        my $checks = $op->{checks};
        foreach (@$checks) {
            if (exists $_->{check}) {
                if ($_->{check} =~ /,/) {
                    push @states, "ln_opck_satisfy_msg($_->{check});";
                } else {
                    push @states, "ln_opck_satisfy($_->{check});";
                }
            }
        }
        push @states, "";
    }
    if (exists $op->{custom}) {
        &add_custom_block($op->{custom}, \@states);
        push @states, "";
    }

    &indent_lines($INDENT_OFFSET, \@states);
    join "\n", @states;
}

sub gen_output_tensor_def {
    my $op = shift;
    my $tensors_out = $op->{tensors_out};

    my @states = ();
    foreach my $tensor (@$tensors_out) {
        my $arg_name = $tensor->{arg_name};
        if (exists $tensor->{ndim}) {
            push @states, "${arg_name}_ndim = $tensor->{ndim};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `ndim` or `custom` to give the defination of '${arg_name}_ndim'");
        }
        if (exists $tensor->{dims}) {
            push @states, "${arg_name}_dims = $tensor->{dims};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `dims` or `custom` to give the defination of '${arg_name}_dims'");
        }
        if (exists $tensor->{dtype}) {
            push @states, "${arg_name}_dtype = $tensor->{dtype};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `dtype` or `custom` to give the defination of '${arg_name}_dtype'");
        }
        if (exists $tensor->{custom}) {
            &add_custom_block($tensor->{custom}, \@states);
        }
        push @states, "${arg_name} = tl_tensor_create(NULL, ${arg_name}_ndim, ${arg_name}_dims, ${arg_name}_dtype);";
        push @states, "${arg_name}_entry = ln_tensor_entry_create(${arg_name}_name, ${arg_name});";
        push @states, "${arg_name}_entry->offset = ${arg_name}_list_entry->offset;";
        push @states, "ln_tensor_entry_set_creater(${arg_name}_entry, op_arg->name);";
        if (exists $tensor->{owner}) {
            push @states, "ln_tensor_entry_set_owner(${arg_name}_entry, op_arg->tensor_table, $tensor->{owner});";
        }
        if (exists $tensor->{static}) {
            push @states, "${arg_name}_entry->isstatic = 1;";
        }
        if (exists $tensor->{mtype}) {
            push @states, "${arg_name}_entry->mtype = $tensor->{mtype};";
        } else {
            &err_exit("${arg_name} needs a `mtype`");
        }
        push @states, "ln_tensor_table_insert(op_arg->tensor_table, ${arg_name}_entry);";
        if (exists $tensor->{cleanup}) {
            &add_custom_block($tensor->{cleanup}, \@states);
        }
        push @states, "";
    }

    &indent_lines($INDENT_OFFSET, \@states);
    join "\n", @states;
}

sub gen_priv_assigns {
    my $op = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @states = ();
    push @states, "priv = ln_alloc(sizeof(struct priv_s));";
    foreach (@$tensors_in) {
        push @states, "priv->$_->{arg_name}_entry = $_->{arg_name}_entry;";
    }
    foreach (@$tensors_out) {
        push @states, "priv->$_->{arg_name}_entry = $_->{arg_name}_entry;";
    }
    foreach (@$params) {
        push @states, "priv->$_->{arg_name}_entry = $_->{arg_name}_entry;";
    }
    push @states, "op_arg->priv = priv;";

    &indent_lines($INDENT_OFFSET, \@states);
    join "\n", @states;
}

sub have_variable {
    my $code_str = shift;
    my $variable = shift;
    my $ret;

    if ($code_str =~ /((?<!->)|(?<!.)|(?<!\w))(?=\w)$variable\b/) {
        $ret = 1;
    } else {
        $ret = 0;
    }
}

sub gen_param_decl_from_priv {
    my $param = shift;

    my $type = &get_param_type($param);
    my $arg_name = $param->{arg_name};
    my $decl;
    given ($param->{ptype}) {
        when ("LN_PARAM_NULL") {
            $decl = "${type}${arg_name} = NULL;";
        }
        when ("LN_PARAM_STRING") {
            if (exists $param->{realtype}) {
                    $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_$param->{realtype};";
            } else {
                $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_string;";
            }
        }
        when ("LN_PARAM_NUMBER") {
            $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_$param->{realtype};";
        }
        when ("LN_PARAM_BOOL") {
            $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_bool;";
        }
        when ("LN_PARAM_ARRAY_STRING") {
            if (exists $param->{realtype}) {
                $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_array_$param->{realtype};";
            } else {
                $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_array_string;";
            }
        }
        when ("LN_PARAM_ARRAY_NUMBER") {
            $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_array_$param->{realtype};";
        }
        when ("LN_PARAM_ARRAY_BOOL") {
            $decl = "${type}${arg_name} = priv->${arg_name}_entry->value_array_bool;";
        }
        default {
            &err_exit("$param->${arg_name} has unsupported `ptype`: '$param->{ptype}'");
        }
    }
}

sub add_dynamic_decs_from_priv {
    my $op = shift;
    my $code_str = shift;
    my $states = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    return unless $code_str;
    foreach (@$tensors_in) {
        my $arg_name = $_->{arg_name};
        if (&have_variable($code_str, "$_->{arg_name}_entry")) {
            push @$states, "ln_tensor_entry *$_->{arg_name}_entry = priv->${arg_name}_entry;";
        }
        if (&have_variable($code_str, "$_->{arg_name}")) {
            push @$states, "tl_tensor *$_->{arg_name} = priv->${arg_name}_entry->tensor;";
        }
        if (&have_variable($code_str, "$_->{arg_name}_name")) {
            push @$states, "tl_tensor *$_->{arg_name} = priv->${arg_name}_entry->name;";
        }
    }
    foreach (@$tensors_out) {
        my $arg_name = $_->{arg_name};
        if (&have_variable($code_str, "$_->{arg_name}_entry")) {
            push @$states, "ln_tensor_entry *$_->{arg_name}_entry = priv->${arg_name}_entry;";
        }
        if (&have_variable($code_str, "$_->{arg_name}")) {
            push @$states, "tl_tensor *$_->{arg_name} = priv->${arg_name}_entry->tensor;";
        }
        if (&have_variable($code_str, "$_->{arg_name}_name")) {
            push @$states, "tl_tensor *$_->{arg_name} = priv->${arg_name}_entry->name;";
        }
    }
    foreach (@$params) {
        my $arg_name = $_->{arg_name};
        if (&have_variable($code_str, "$_->{arg_name}_entry")) {
            push @$states, "ln_param_entry *$_->{arg_name}_entry = priv->${arg_name}_entry;";
        }
        if (&have_variable($code_str, "$_->{arg_name}")) {
            push @$states, &gen_param_decl_from_priv($_);
        }
    }
    # if (exists $op->{extra_privs}) {
    #     my $extra_privs = $op->{extra_privs};
    #     foreach (@$extra_privs) {
    #         my $name = $_->{name};
    #         if (&have_variable($code_str, "$_->{name}")) {
    #             push @$states, "$_->{type} $_->{name} = priv->${name};";
    #         }
    #     }
    # }
}

sub gen_calc_offset {
    my $op = shift;

    my @states = ();
    push @states, "struct priv_s *priv = op_arg->priv;";
    &add_dynamic_decs_from_priv($op, $op->{calc_offset}, \@states);
    &make_defs_neat(\@states);
    push @states, "";
    &add_custom_block($op->{calc_offset}, \@states);
    &indent_lines($INDENT_OFFSET, \@states);
    my $states_str = join "\n", @states;

    my $calc_offset_tpl = <<EOF;
/* This function is used to manually set the tensor's offset address. */
static size_t $op->{optype}_calc_offset(ln_op_arg *op_arg, ln_tensor_entry *te)
{
${states_str}
}
EOF
}

sub gen_static_run {
    my $op = shift;

    my @states = ();
    push @states, "struct priv_s *priv = op_arg->priv;";
    &add_dynamic_decs_from_priv($op, $op->{static_run}, \@states);
    &make_defs_neat(\@states);
    push @states, "";
    &add_custom_block($op->{static_run}, \@states);
    &indent_lines($INDENT_OFFSET, \@states);
    my $states_str = join "\n", @states;

    my $static_run_tpl = <<EOF;
/* This function runs only once per instance right after memory allocation. */
static void $op->{optype}_static_run(ln_op_arg *op_arg)
{
${states_str}
}
EOF
}

sub gen_run {
    my $op = shift;

    my @states = ();
    push @states, "struct priv_s *priv = op_arg->priv;";
    &add_dynamic_decs_from_priv($op, $op->{run}, \@states);
    &make_defs_neat(\@states);
    push @states, "";
    &add_custom_block($op->{run}, \@states);
    &indent_lines($INDENT_OFFSET, \@states);
    my $states_str = join "\n", @states;

    my $static_run_tpl = <<EOF;
/* This function should only do the calculations. */
static void $op->{optype}_run(ln_op_arg *op_arg)
{
${states_str}
}
EOF
}

sub gen_post_run {
    my $op = shift;
    my $tensors_out = $op->{tensors_out};

    my @states = ();
    push @states, "struct priv_s *priv = op_arg->priv;";
    &add_dynamic_decs_from_priv($op, $op->{post_run}, \@states);
    &make_defs_neat(\@states);
    push @states, "";
    &add_custom_block($op->{post_run}, \@states) if exists $op->{post_run};
    foreach (@$tensors_out) {
        push @states, "ln_tensor_table_remove(op_arg->tensor_table, priv->$_->{arg_name}_entry->name);";
    }
    push @states, "ln_free(priv);";

    &indent_lines($INDENT_OFFSET, \@states);
    my $states_str = join "\n", @states;

    my $static_run_tpl = <<EOF;
/* This function should free all the memory allocated by other *_run()s. */
static void $op->{optype}_post_run(ln_op_arg *op_arg)
{
${states_str}
}
EOF
}

sub gen_op_arg {
    my $op = shift;
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @in_declares = ();
    foreach (@$tensors_in) {
        push @in_declares, "\"$_->{arg_name}\",";
    }
    push @in_declares, "NULL";
    &indent_lines($INDENT_OFFSET, \@in_declares);
    my $in_str = join "\n", @in_declares;

    my @out_declares = ();
    foreach (@$tensors_out) {
        push @out_declares, "\"$_->{arg_name}\",";
    }
    push @out_declares, "NULL";
    &indent_lines($INDENT_OFFSET, \@out_declares);
    my $out_str = join "\n", @out_declares;

    my @param_declares = ();
    foreach (@$params) {
        push @param_declares, "\"$_->{arg_name}\",";
    }
    push @param_declares, "NULL";
    &indent_lines($INDENT_OFFSET, \@param_declares);
    my $param_str = join "\n", @param_declares;

    my @param_ptypes = ();
    foreach (@$params) {
        push @param_ptypes, "$_->{ptype},";
    }
    &indent_lines($INDENT_OFFSET, \@param_ptypes);
    my $ptype_str = join "\n", @param_ptypes;

    my $op_arg_tpl = <<EOF;
static const char *in_arg_names[] = {
$in_str
};

static const char *out_arg_names[] = {
$out_str
};

static const char *param_arg_names[] = {
$param_str
};

static const ln_param_type param_ptypes[] = {
$ptype_str
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_$op->{optype} = {
    .optype = "$op->{optype}",
    .arch = "$op->{arch}",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};
EOF
}

sub gen_op_impl {
    my $op = shift;
    my $calc_offset_func = exists $op->{calc_offset} ? "$op->{optype}_calc_offset" : "NULL";
    my $static_run_func = exists $op->{static_run} ? "$op->{optype}_static_run" : "NULL";
    my $run_func = exists $op->{run} ? "$op->{optype}_run" : "NULL";

    my $op_impl_tpl = <<EOF;
/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_$op->{optype} = {
    .op_arg = &op_arg_$op->{optype},
    .pre_run = $op->{optype}_pre_run,
    .static_run = ${static_run_func},
    .run = ${run_func},
    .post_run = $op->{optype}_post_run,
    .calc_offset = ${calc_offset_func},
};
EOF
}

sub add_custom_block {
    my $custom_str = shift;
    my $states = shift;
    my @customs = split "\n", $custom_str;
    &indent_lines($INDENT_OFFSET, \@customs);
    push @$states, "{";
    push @$states, @customs;
    push @$states, "}";
}

sub make_defs_neat {
    my $defs = shift;
    my $max_offset = 0;
    foreach (@$defs) {
        if (/( |\*)(\w+(;$| *=.+;$))/) {
            my $offset = index($_, $2);
            $max_offset = $max_offset < $offset ? $offset : $max_offset;
        } else {
            &err_exit("make_defs_neat: wrong format: $_");
        }
    }
    foreach (@$defs) {
        my $type = $1 if /^(struct +\w+|\w+)/;
        my $nstars = 0;
        $nstars = length $1 if /^$type *(\*+)/;
        my $rest = $2 if /( |\*)(\w+(;$| *=.+;$))/;
        $_ = sprintf("%-${max_offset}s", $type);
        my $re = " "x$nstars;
        my $stars = "*"x$nstars;
        s/$re$/$stars$rest/;
    }
}

sub indent_lines {
    my $nspaces = shift;
    my $states = shift;
    foreach (@$states) {
        $_ = " "x$nspaces.$_ unless /^\s*$/;
    }
}

sub indent_line {
    my $nspaces = shift;
    my $state = shift;
    $state = " "x$nspaces.$state;
}

sub err_exit {
    my $msg = $_[0];
    print STDERR "ERROR: $msg\n";
    exit 1;
}

sub warn_msg {
    my $msg = $_[0];
    print STDERR "WARNING: $msg\n";
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}
