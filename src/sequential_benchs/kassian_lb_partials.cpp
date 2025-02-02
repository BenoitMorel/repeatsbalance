#include <iostream>
#include "common.h"
#include <time.h>

double get_unique_pattern_ratio(LikelihoodEngine &engine)
{
  double total_patterns = 0;
  double total_sites = 0;
  std::vector<Partition*> &partitions = engine.get_partitions();
  for (unsigned int p = 0; p < partitions.size(); ++p) {
    
    Partition *partition = partitions[p];
    std::cout << partition << std::endl;
    total_sites += partition->get_sites_number();
    total_patterns += partition->get_unique_repeats_pattern_ratio() * double(partition->get_sites_number());
  }
  double res = total_patterns / total_sites;
  std::cout << "total sites " << total_sites << std::endl;
  std::cout << "ratio " << res << std::endl;
  std::cout << "Sites * ratio = " << total_patterns << std::endl;
  return res;
}


void kassian_lb_partials(int argc, char *params[])
{
  if (argc != 10) {
    std::cerr << "Error : syntax is" << std::endl;
    std::cerr 
      << "newick sequence partitions states use_repeats update_repeats repeats_lookup_size iterations cores randomized" 
      << std::endl;
    return ;
  }
  unsigned int i = 0;
  const char *newick = params[i++];
  const char *seq = params[i++];
  const char *partition_file = params[i++];
  unsigned int states_number = atoi(params[i++]);
  unsigned int use_repeats = atoi(params[i++]);
  unsigned int update_repeats = atoi(params[i++]);
  unsigned int repeats_lookup_size = atoi(params[i++]);
  unsigned int iterations = atoi(params[i++]);
  unsigned int cores = atoi(params[i++]);
  unsigned int randomized = atoi(params[i++]);


  long worst_time = 0;
  srand(time(NULL)); 
  std::cout << "parse" << std::endl;
  MSA *full_msa = new MSA(seq, states_number); 
  Tree tree(full_msa, newick);
  //Tree tree(&full_msa);
  std::vector<PartitionIntervals> initial_partitionning;
  PartitionIntervals::parse(partition_file, initial_partitionning);
  std::vector<MSA *> msas;
  std::vector<WeightedMSA> weighted_msas;
  LoadBalancer balancer;
  std::cout << "create submsas" << std::endl;
  for (unsigned int i = 0; i < initial_partitionning.size(); ++i) {
    msas.push_back(new MSA(full_msa, initial_partitionning[i], i));
    msas[i]->compress();
  }
  std::cout << "parse" << std::endl;
 
  if (!randomized) {
    for (unsigned int i = 0; i < initial_partitionning.size(); ++i) {
      weighted_msas.push_back(WeightedMSA(msas[i], 1.0));
    }
  } else {
  Timer lb_timer;
    balancer.compute_weighted_msa(msas, weighted_msas, PLL_ATTRIB_SITES_REPEATS | PLL_ATTRIB_ARCH_AVX);
  std::cout << "LB time " << lb_timer.get_time() << "ms" << std::endl;
  }
  std::vector<CoreAssignment> assignments;
  balancer.kassian_load_balance(cores, weighted_msas, assignments);
  unsigned int attribute = Partition::compute_attribute(use_repeats, 
		  0, 
		  "avx");

  std::cout << "GO" << std::endl;
  for (unsigned int core = 0; core < assignments.size(); ++core) {
    LikelihoodEngine engine(&tree, msas, assignments[core], attribute, states_number, 4, repeats_lookup_size);
    engine.update_operations();
    engine.update_matrices();
    engine.update_partials();
    Timer timer;
    for (unsigned int i = 0; i < iterations; ++i) {
      engine.update_partials(update_repeats);
    }
    get_unique_pattern_ratio(engine);
    worst_time = std::max(worst_time, timer.get_time());
    std::cout << timer.get_time() << " ms" << std::endl;
  }

  std::cout << "WORST TIME " << worst_time << "ms" <<  std::endl;

}
