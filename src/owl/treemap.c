#include "owl/treemap.h"

static tree_node empty;

void treemap_init(void)
{
	empty.child[0] = &empty;
	empty.child[1] = &empty;
}

bool treemap_is_null(tree_node *n)
{
	return n == &empty;
}

void treemap_new(treemap *t, node_memmgr *nmm, size_t value_size)
{
	t->nmm = nmm;
	t->node_size = sizeof(tree_node) + value_size;
	t->size = 0;
	t->root = &empty;
}

inline static void treemap_init_node(tree_node *n, const lkey *key)
{
	n->key = *key;
	n->level = 1;
	n->child[0] = &empty;
	n->child[1] = &empty;
}

static void treemap_destroy_rec(node_memmgr *nmm, tree_node *node)
{
	if (node != &empty) {
		treemap_destroy_rec(nmm, node->child[0]);
		treemap_destroy_rec(nmm, node->child[1]);
		nmm->release(node);
	}
}

void treemap_destroy(treemap *t)
{
	treemap_destroy_rec(t->nmm, t->root);
}

typedef struct {
	treemap *tree;
	const lkey *key;
	tree_node *res;
} put_st;

static tree_node *treemap_put_rec(put_st *state, tree_node *node)
{
	if (node == &empty) {
		tree_node *p = state->tree->nmm->allocatez(state->tree->node_size);
		treemap_init_node(p, state->key);
		state->tree->size++;
		state->res = p;
		return p;
	}

	int d = lkey_compare(state->key, &node->key);
	if (d == 0) {
		state->res = node;
		return node;
	}

	int branch = d > 0;
	tree_node *p = node;
	tree_node *current_child = p->child[branch];
	tree_node *c = (p->child[branch] = treemap_put_rec(state, current_child));
	if (c == current_child) {
		// Fast return: do not touch other nodes (possibly not in cache)
		return node;
	}

	if (branch) {
		// Right child: test if we need to increment level first
		if (p->level < c->level) {
			p->level++;
		}
	} else if (p->level == c->level) {
		// Rotate right
		p->child[0] = c->child[1];
		c->child[1] = p;
		p = c;
	}

	// Check if we have 3 nodes of the same level of the right
	if (p->level == p->child[1]->child[1]->level) {
		c = p->child[1];
		p->child[1] = c->child[0];
		c->child[0] = p;
		c->level++;
		p = c;
	}

	return p;
}

void *treemap_put(treemap *t, lkey key)
{
	put_st state = {.tree = t, .key = &key};
	t->root = treemap_put_rec(&state, t->root);
	return state.res->value;
}

void *treemap_get(treemap *t, lkey key)
{
	tree_node *node = t->root;
	while (node != &empty) {
		int d = lkey_compare(&key, &node->key);
		if (d == 0) {
			return node->value;
		}
		node = node->child[d > 0];
	}
	return NULL;
}

typedef struct {
	treemap *tree;
	const lkey *key;
} del_st;

static tree_node *treemap_del_rec(del_st *state, tree_node *node)
{
	if (node == &empty) {
		return node;
	}

	int d = lkey_compare(state->key, &node->key);
	if (d == 0) {
		state->tree->size--;
		tree_node *r;
		if (node->child[0] == &empty) {
			r = node->child[1];
		} else {
			r = node;
		}
		return r;
	}

	int branch = d > 0;
	tree_node *p = node;
	tree_node *current_child = p->child[branch];
	tree_node *c = (p->child[branch] = treemap_del_rec(state, current_child));

	return c;
}

bool treemap_del(treemap *t, lkey key)
{
	size_t s = t->size;
	del_st state = {.tree = t, .key = &key};
	t->root = treemap_del_rec(&state, t->root);
	return t->size < s;
}
