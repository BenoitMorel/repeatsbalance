#include <iostream>
#include "common.h"

/*
 *  For each node, print : left and right children number, and clv_computation time
 */
void pernode(int argc, char *params[])
{
  if (argc != 7 && argc != 2) {
    std::cerr << "Error : syntax is" << std::endl;
    std::cerr 
      << "newick sequence use_repeats update_repeats repeats_lookup_size iterations arch" 
      << std::endl;
    std::cerr 
      << "newick sequence" 
      << std::endl;
    return ;
  }
  unsigned int i = 0;
  const char *newick = params[i++];
  const char *seq = params[i++];

  // just print nodes stats
  if (argc == 2) {
    PLLHelper helper(newick, seq, 0); 
    // a bit overkill to fill operations :)
    helper.update_all_partials();
    for (unsigned int op = 0; op < helper.ops_count; ++op) {
      Timer t;
      std::cout << op << " ";
      helper.print_op_stats(helper.operations[op]);
    }
    return;
  }

  unsigned int use_repeats = atoi(params[i++]);
  unsigned int update_repeats = atoi(params[i++]);
  unsigned int repeats_lookup_size = atoi(params[i++]);
  unsigned int iterations = atoi(params[i++]);
  const char *arch = params[i++];


  unsigned int attribute = PLLHelper::compute_attribute(use_repeats, arch);
  if (INVALID_ATTRIBUTE == attribute) {
    return;
  }
  PLLHelper helper(newick, seq, attribute); 
  helper.set_srlookup_size(repeats_lookup_size);
  helper.update_all_partials();
  
  for (unsigned int op = 0; op < helper.ops_count; ++op) {
    Timer t;
    helper.update_partial(helper.operations[op], iterations, update_repeats);
    std::cout << t.get_time() << "ms" << std::endl; 
  }
}



