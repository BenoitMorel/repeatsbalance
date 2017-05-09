#include "LikelihoodEngine.hpp"  

#include "Tree.hpp"
#include "Partition.hpp"



/* a callback function for performing a full traversal */
static int traverser_full(pll_utree_t * node)
{
  node->data = (void *)~0;
  return 1;
}

LikelihoodEngine::LikelihoodEngine(const char *newick_file,
    const char *phy_file,
    unsigned int attribute_flag, 
    unsigned int states_number,
    unsigned int rate_categories_number,
    unsigned int repeats_lookup_size): 
  tree(newick_file)
{
  partitions.push_back(new Partition(phy_file,
        tree,
        attribute_flag,
        states_number,
        rate_categories_number,
        repeats_lookup_size));
}

LikelihoodEngine::LikelihoodEngine(const char *newick_file,
    const char *phy_file,
    const char *part_file,
    unsigned int attribute_flag, 
    unsigned int states_number,
    unsigned int rate_categories_number,
    unsigned int repeats_lookup_size): 
  tree(newick_file)
{
  if (part_file) {
    std::vector<PartitionIntervals> partition_intervals;
    PartitionIntervals::parse(part_file, partition_intervals);
    MSA msa(phy_file, states_number);
    for (unsigned int i = 0; i < partition_intervals.size(); ++i) {
      MSA submsa(&msa, partition_intervals[i], i);
      submsa.compress();
      partitions.push_back(new Partition(&submsa,
          tree,
          attribute_flag,
          states_number,
          rate_categories_number,
          repeats_lookup_size));
    }
  } else {
    partitions.push_back(new Partition(phy_file,
          tree,
          attribute_flag,
          states_number,
          rate_categories_number,
          repeats_lookup_size));
  }
}

LikelihoodEngine::LikelihoodEngine(const char *newick_file,
    const std::vector<MSA *> &msas,
    const CoreAssignment &assignment,
    unsigned int attribute_flag, 
    unsigned int states_number,
    unsigned int rate_categories_number,
    unsigned int repeats_lookup_size): tree(newick_file)
{
  const std::vector<PartitionIntervals> &partition_intervals = assignment.get_assignments();
  for (unsigned int i = 0; i < partition_intervals.size(); ++i) {
    const PartitionIntervals &intervals = partition_intervals[i];
    partitions.push_back(new Partition(msas[intervals.get_partition_id()],
        intervals,
        tree,
        attribute_flag,
        states_number,
        rate_categories_number,
        repeats_lookup_size));
  } 
}


 
LikelihoodEngine::~LikelihoodEngine()
{
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    delete partitions[i];
  }
}


void LikelihoodEngine::update_operations()
{
  tree.update_operations(traverser_full);
}
  
void LikelihoodEngine::update_matrices()
{
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    partitions[i]->update_matrices(tree);
  }
}
  
void LikelihoodEngine::update_partials()
{
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    partitions[i]->update_partials(tree);
  }
}

double LikelihoodEngine::compute_likelihood()
{
  double ll = 0;
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    ll += partitions[i]->compute_likelihood(tree);
  }
  return ll;
}

void LikelihoodEngine::update_sumtable()
{
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    partitions[i]->update_sumtable(tree);
  }
}

void LikelihoodEngine::compute_derivatives(double *d_f, double *dd_f) 
{	
  *d_f = *dd_f = 0;
  for (unsigned int i = 0; i < partitions.size(); ++i) {
    double d = 0;
    double dd = 0;
    partitions[i]->compute_derivatives(&d, &dd);
    *d_f += d;
    *dd_f += dd;
  }
}


