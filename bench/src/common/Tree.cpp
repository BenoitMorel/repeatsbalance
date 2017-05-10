#include "Tree.hpp"
#include <iostream>

#define PLLMOD_TREE_DEFAULT_BRANCH_LENGTH 0.1


Tree::Tree(const char *newick_file)
{
  if (!newick_file) {
    std::cerr << "Tree::Tree null newick file" << std::endl;
    return;
  }
  tips_number = 0;
  pll_utree = pll_utree_parse_newick(newick_file, &tips_number);
  if (!pll_utree) {
    std::cerr << "Error: Tree::tree null pll_utree" << std::endl;
  }
  init();
}

void Tree::init() 
{
  set_missing_branch_length(pll_utree, 0.000001);

  innernodes_number = tips_number - 2;
  nodes_number = innernodes_number + tips_number;
  branches_number = nodes_number - 1;
  
  branch_lengths.resize(branches_number);
  matrix_indices.resize(branches_number);
}

int pllmod_utree_connect_nodes(pll_utree_t * parent,
    pll_utree_t * child,
    double length)
{
  if(!(parent && child))
    return PLL_FAILURE;

  parent->back = child;
  child->back = parent;
  parent->length = parent->back->length = length;
  /* PMatrix index is set to parent node */
  child->pmatrix_index = parent->pmatrix_index;

  return PLL_SUCCESS;
}

pll_utree_t * pllmod_utree_create_node(unsigned int clv_index,
    int scaler_index,
    char * label,
    void * data)
{
  pll_utree_t * new_node = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
  new_node->next         = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
  new_node->next->next   = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));

  new_node->next->next->next = new_node;
  new_node->label = label;
  new_node->next->label =
    new_node->next->next->label =
    new_node->label;
  new_node->next->data =
    new_node->next->next->data =
    new_node->data = data;
  new_node->next->length =
    new_node->next->next->length =
    new_node->length = 0;
  new_node->next->clv_index =
    new_node->next->next->clv_index =
    new_node->clv_index = clv_index;
  new_node->next->scaler_index =
    new_node->next->next->scaler_index =
    new_node->scaler_index = scaler_index;
  new_node->back =
    new_node->next->back =
    new_node->next->next->back = NULL;
  return new_node;
}

pll_utree_t *Tree::create_random(unsigned int taxa_count,
    const char * const* names)
{
  /* code taken from pllmodule */
  unsigned int i;
  unsigned int tip_node_count        = taxa_count;
  unsigned int inner_node_count      = taxa_count - 2;
  unsigned int node_count            = tip_node_count + inner_node_count;
  unsigned int max_branches          = 2 * tip_node_count - 3;
  unsigned int placed_branches_count = 0;

  pll_utree_t ** nodes    = (pll_utree_t **) calloc(node_count,
      sizeof(pll_utree_t *));
  pll_utree_t ** branches = (pll_utree_t **) calloc(max_branches,
      sizeof(pll_utree_t *));

  pll_utree_t * next_tip;
  pll_utree_t * next_inner;
  pll_utree_t * next_branch;
  pll_utree_t * new_tree;

  unsigned int next_branch_id = taxa_count;
  unsigned int rand_branch_id;
  unsigned int node_id = 0;

  /* allocate tips */
  for (i=0; i<taxa_count; ++i)
  {
    nodes[i] = (pll_utree_t *)calloc(1, sizeof(pll_utree_t));
    nodes[i]->clv_index = i;
    nodes[i]->scaler_index = PLL_SCALE_BUFFER_NONE;
    nodes[i]->pmatrix_index = i;
    nodes[i]->node_index = node_id++;

    if (names)
    {
      nodes[i]->label = (char *) malloc( strlen(names[i]) + 1 );
      strcpy(nodes[i]->label, names[i]);
    }
    else
    {
      nodes[i]->label = NULL;
    }
  }

  /* allocate inner */
  for (i=taxa_count; i<node_count; ++i)
  {
    nodes[i] = pllmod_utree_create_node(i, (int)i, NULL, NULL);
    nodes[i]->scaler_index -= taxa_count;
    nodes[i]->next->scaler_index -= taxa_count;
    nodes[i]->next->next->scaler_index -= taxa_count;

    nodes[i]->node_index = node_id++;
    nodes[i]->next->node_index = node_id++;
    nodes[i]->next->next->node_index = node_id++;
  }
  assert(node_id == tip_node_count + inner_node_count * 3);

  /* set an inner node as return value */
  new_tree = nodes[taxa_count];

  /* build minimal tree with 3 tips and 1 inner node */
  pllmod_utree_connect_nodes(nodes[0], nodes[taxa_count],
      PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);
  branches[placed_branches_count++] = nodes[taxa_count];
  pllmod_utree_connect_nodes(nodes[1], nodes[taxa_count]->next,
      PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);
  branches[placed_branches_count++] = nodes[taxa_count]->next;
  pllmod_utree_connect_nodes(nodes[2], nodes[taxa_count]->next->next,
      PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);
  branches[placed_branches_count++] = nodes[taxa_count]->next->next;

  for (i=3; i<taxa_count; ++i)
  {
    /* take tips iteratively */
    next_tip = nodes[i];
    next_inner = nodes[tip_node_count + i - 2];

    /* select random branch from the tree */
    rand_branch_id = (unsigned int) rand() % placed_branches_count;
    next_branch = branches[rand_branch_id];

    /* connect tip to selected branch */
    pllmod_utree_connect_nodes(next_branch->back, next_inner,
        PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);
    pllmod_utree_connect_nodes(next_branch, next_inner->next,
        PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);
    pllmod_utree_connect_nodes(next_tip, next_inner->next->next,
        PLLMOD_TREE_DEFAULT_BRANCH_LENGTH);

    if (next_inner->back->next == NULL)
    {
      next_inner->next->pmatrix_index = next_inner->next->back->pmatrix_index =
        next_branch_id++;
    }
    else
    {
      next_inner->pmatrix_index = next_inner->back->pmatrix_index =
        next_branch_id++;
    }

    /* store branches */
    branches[placed_branches_count++] = next_inner;
    branches[placed_branches_count++] = next_inner->next->next;
  }
  assert(placed_branches_count == max_branches);

  /* clean */
  free (nodes);
  free (branches);

  return (new_tree);
}

Tree::Tree(unsigned int tips, const char * const* names)
{
  pll_utree = create_random(tips, names);
  tips_number = tips;
  init();
}

Tree::~Tree()
{

}

void Tree::update_operations(int (*traverse)(pll_utree_t *))
{
  unsigned int traverse_size = 0;
  traverse_buffer.resize(nodes_number);
  pll_utree_traverse(pll_utree,
      traverse,
      &traverse_buffer[0],
      &traverse_size);
  traverse_buffer.resize(traverse_size);

  operations.resize(branches_number);
  matrix_indices.resize(branches_number);

  unsigned int matrices_number = 0;
  unsigned int operations_number = 0;
  pll_utree_create_operations(&traverse_buffer[0],
      traverse_size,
      &branch_lengths[0],
      &matrix_indices[0],
      &operations[0],
      &matrices_number,
      &operations_number);

  matrix_indices.resize(matrices_number);
  operations.resize(operations_number);
}

void Tree::set_missing_branch_length_recursive(pll_utree_t * tree, double length)
{
  if (tree) {
    /* set branch length to default if not set */
    if (!tree->length)
      tree->length = length;

    if (tree->next) {   
      if (!tree->next->length)
        tree->next->length = length;

      if (!tree->next->next->length)
        tree->next->next->length = length;

      set_missing_branch_length_recursive(tree->next->back, length);
      set_missing_branch_length_recursive(tree->next->next->back, length);
    }   
  }
}

void Tree::set_missing_branch_length(pll_utree_t * tree, double length)
{
  set_missing_branch_length_recursive(tree, length);
  set_missing_branch_length_recursive(tree->back, length);
}

void Tree::randomize_pll_utree(const MSA *msa) {
  pll_utree_destroy(pll_utree, free);
  pll_utree = create_random(msa->get_pll_msa()->count, msa->get_pll_msa()->label);
  std::cout << "got new pll utree" << std::endl;
  init();
}


