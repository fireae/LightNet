/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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

#include "ln_dfg.h"

static void ln_graph_node_free_wrapper(void *p)
{
    ln_graph_node_free(p);
}

static ln_hash *table_create(void)
{
    return ln_hash_create(ln_str_hash, ln_str_cmp, ln_free,
                          ln_graph_node_free_wrapper);
}

static void table_free(ln_hash *table)
{
    ln_hash_free(table);
}

static int table_insert(ln_hash *table, const char *key, void *value)
{
    char *key_copy;

    key_copy = ln_strdup(key);
    return ln_hash_insert(table, key_copy, value);
}

static int table_remove(ln_hash *table, const char *key)
{
    return ln_hash_remove(table, (void *)key);
}

static void *table_find(ln_hash *table, const char *key)
{
    return ln_hash_find(table, (char *)key);
}

static int en_cmp(const void *p1, const void *p2)
{
    const ln_graph_edge_node *en1 = p1;
    const ln_graph_edge_node *en2 = p2;

    if (strcmp(en1->edge_data, en2->edge_data) == 0 &&
        en1->node == en2->node)
        return 0;
    return 1;
}

static int en_cmp_edge(const void *p1, const void *p2)
{
    const ln_graph_edge_node *en1 = p1;
    const ln_graph_edge_node *en2 = p2;

    return strcmp(en1->edge_data, en2->edge_data);
}

static void en_free(void *p)
{
    ln_graph_edge_node *en = p;

    ln_free(en->edge_data);
    ln_graph_edge_node_free(en);
}

static void add_dangling(ln_list **danglings, const char *tname,
                         ln_graph_node *node)
{
    ln_graph_edge_node *en;
    char *tname_copy;

    tname_copy = ln_strdup(tname);
    en = ln_graph_edge_node_create(tname_copy, node, ln_str_cmp);
    *danglings = ln_list_prepend(*danglings, en);
}

/* if node == NULL, remove all danglings that have tname as edge*/
static void remove_dangling(ln_list **danglings, const char *tname,
                            ln_graph_node *node)
{
    ln_graph_edge_node en_hint;

    if (node) {
        en_hint.edge_data = (char *)tname;
        en_hint.node = node;
        *danglings = ln_list_remove_custom_deep(*danglings, &en_hint,
                                                en_cmp, en_free);
    } else {
        en_hint.edge_data = (char *)tname;
        *danglings = ln_list_remove_all_custom_deep(*danglings, &en_hint,
                                                    en_cmp_edge, en_free);
    }
}

static void free_danglings(ln_list *danglings)
{
    ln_list_free_deep(danglings, en_free);
}

ln_dfg *ln_dfg_create(void)
{
    ln_dfg *dfg;

    dfg = ln_alloc(sizeof(ln_dfg));
    dfg->graph = ln_graph_create(ln_direct_cmp, ln_str_cmp);
    dfg->node_table = table_create();
    dfg->dangling_outs = NULL;
    dfg->dangling_ins = NULL;

    return dfg;
}

void ln_dfg_free(ln_dfg *dfg)
{
    if (!dfg)
        return;
    ln_graph_free(dfg->graph);
    table_free(dfg->node_table);
    free_danglings(dfg->dangling_ins);
    free_danglings(dfg->dangling_outs);
    ln_free(dfg);
}

void ln_dfg_link(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname)
{
    ln_graph_node *node1;
    ln_graph_node *node2;
    char *tname_copy;

    if (!ln_tensor_list_find_by_name(op1->op_arg->tensors_out, (char *)tname) ||
        !ln_tensor_list_find_by_name(op2->op_arg->tensors_in, (char *)tname))
        return;

    node1 = ln_hash_find(dfg->node_table, op1->op_arg->name);
    if (!node1)
        return;
    node2 = ln_hash_find(dfg->node_table, op2->op_arg->name);
    if (!node2)
        return;

    tname_copy = ln_strdup(tname);
    ln_graph_link_node(dfg->graph, node1, node2, tname_copy);
}

void ln_dfg_unlink(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname)
{
    ln_graph_node *node1;
    ln_graph_node *node2;
    char *origin_tname;

    if (!ln_tensor_list_find_by_name(op1->op_arg->tensors_out, (char *)tname) ||
        !ln_tensor_list_find_by_name(op2->op_arg->tensors_in, (char *)tname))
        return;

    node1 = ln_hash_find(dfg->node_table, op1->op_arg->name);
    if (!node1)
        return;
    node2 = ln_hash_find(dfg->node_table, op2->op_arg->name);
    if (!node2)
        return;

    origin_tname = ln_graph_unlink_node(node1, node2, (char *)tname);
    ln_free(origin_tname);
}

void ln_dfg_add(ln_dfg *dfg, ln_op *op)
{
    ln_graph_node *node;
    ln_graph_edge_node *en;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    int ret, refered;

    node = ln_graph_add(dfg->graph, op);
    ret = table_insert(dfg->node_table, op->op_arg->name, node);
    if (!ret)
        ln_msg_inter_error("duplicated op name '%s'",  op->op_arg->name);

    LN_LIST_FOREACH(tle, op->op_arg->tensors_in) {
        te = ln_tensor_table_find(op->op_arg->tensor_table, tle->name);
        assert(te);
        node = table_find(dfg->node_table, te->creater);
        if (!node) {
            node = table_find(dfg->node_table, op->op_arg->name);
            add_dangling(&dfg->dangling_ins, te->name, node);
            continue;
        }
        ln_dfg_link(dfg, node->data, op, te->name);
        remove_dangling(&dfg->dangling_outs, te->name, node);
    }

    LN_LIST_FOREACH(tle, op->op_arg->tensors_out) {
        te = ln_tensor_table_find(op->op_arg->tensor_table, tle->name);
        assert(te);
        refered = 0;
        LN_LIST_FOREACH(en, dfg->dangling_ins) {
            if (ln_streq(en->edge_data, te->name)) {
                ln_dfg_link(dfg, op, en->node->data, te->name);
                refered = 1;
            }
        }
        if (!refered) {
            node = table_find(dfg->node_table, op->op_arg->name);
            add_dangling(&dfg->dangling_outs, te->name, node);
        } else {
            remove_dangling(&dfg->dangling_ins, te->name, NULL);
        }
    }
}

void ln_dfg_remove(ln_dfg *dfg, ln_op *op)
{
    ln_graph_node *node;
    ln_graph_edge_node *en;
    ln_tensor_list_entry *tle;
    ln_list *l;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    if (!node)
        return;

    for (l = node->in_edge_nodes; l;) {
        en = l->data;
        l = l->next;
        if (!ln_list_find_custom(dfg->dangling_outs, en, en_cmp_edge))
            add_dangling(&dfg->dangling_outs, en->edge_data, en->node);
        ln_dfg_unlink(dfg, en->node->data, op, en->edge_data);
    }
    LN_LIST_FOREACH(tle, op->op_arg->tensors_in) {
        remove_dangling(&dfg->dangling_ins, tle->name, node);
    }

    for (l = node->out_edge_nodes; l;) {
        en = l->data;
        l = l->next;
        add_dangling(&dfg->dangling_ins, en->edge_data, en->node);
        ln_dfg_unlink(dfg, op, en->node->data, en->edge_data);
    }

    LN_LIST_FOREACH(tle, op->op_arg->tensors_out) {
        remove_dangling(&dfg->dangling_outs, tle->name, NULL);
    }

    table_remove(dfg->node_table, op->op_arg->name);
    dfg->graph->nodes = ln_list_remove(dfg->graph->nodes, node);
}

ln_op *ln_dfg_next(const ln_dfg *dfg, const ln_op *op, const char *tname)
{
    ln_graph_node *node;
    ln_graph_edge_node edge_node;
    ln_graph_edge_node *res;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    edge_node.edge_data = (char *)tname;
    res = ln_list_find_custom(node->out_edge_nodes, &edge_node, en_cmp_edge);
    return res ? res->node->data : NULL;
}

/* res needs to be freed by caller */
ln_list *ln_dfg_nexts(const ln_dfg *dfg, const ln_op *op, const char *tname)
{
    ln_graph_node *node;
    ln_graph_edge_node edge_node;
    ln_graph_edge_node *en;
    ln_list *res_ens;
    ln_list *res_ops = NULL;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    edge_node.edge_data = (char *)tname;
    res_ens = ln_list_find_all_custom(node->out_edge_nodes,
                                      &edge_node, en_cmp_edge);
    if (res_ens) {
        LN_LIST_FOREACH(en, res_ens) {
            res_ops = ln_list_append(res_ops, en->node->data);
        }
        ln_list_free(res_ens);
    }
    return res_ops;
}

ln_op *ln_dfg_prev(const ln_dfg *dfg, const ln_op *op, const char *tname)
{
    ln_graph_node *node;
    ln_graph_edge_node edge_node;
    ln_graph_edge_node *res;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    edge_node.edge_data = (char *)tname;
    res = ln_list_find_custom(node->in_edge_nodes, &edge_node, en_cmp_edge);
    return res ? res->node->data : NULL;
}

int ln_dfg_check(const ln_dfg *dfg)
{
    ln_graph_edge_node *en;
    ln_op *op;
    char *tname;

    LN_LIST_FOREACH(en, dfg->dangling_ins) {
        op = en->node->data;
        tname = en->edge_data;
        ln_msg_inter_error("unresolved input tensor '%s' of op '%s' (%s)",
                       tname, op->op_arg->name, op->op_arg->optype);
    }

    LN_LIST_FOREACH(en, dfg->dangling_outs) {
        op = en->node->data;
        tname = en->edge_data;
        /* ln_msg_inter_warn("unused output tensor '%s' of op '%s' (%s)", */
        /*                   tname, op->op_arg->name, op->op_arg->optype); */
    }
    return 1;
}

static void fprint_node(FILE *fp, const void *p)
{
    const ln_op *op = p;
    fprintf(fp, "%s(%s)", op->op_arg->name, op->op_arg->optype);
}

static void fprint_edge(FILE *fp, const void *p)
{
    const char *s = p;
    fprintf(fp, "%s", s);
}

void ln_dfg_fprint(FILE *fp, const ln_dfg *dfg)
{
    ln_graph_fprint(fp, dfg->graph, fprint_node, fprint_edge);
}

void ln_dfg_print(const ln_dfg *dfg)
{
    ln_dfg_fprint(stdout, dfg);
}
