#ifndef _RB_LOADBALANCING_H_
#define _RB_LOADBALANCING_H_

#include "Bin.hpp"
#include "Partition.hpp"
#include <vector>


typedef std::vector<Partition> CPUAssignment;
typedef std::vector<CPUAssignment> Assignment;

class LoadBalancing {
  public:
    LoadBalancing(const Partitions & partitions, unsigned int Bin_number) : 
      _partitions(partitions),
      _bins(Bin_number)
    {
      
    }

    void compute_naive();

    void compute_kassian();

    void compute_kassian_weighted();

    const std::vector<Bin> &load_balancing() const {
      return _bins;
    }

    void build_assignment(Assignment &assignment);


    // the follozing mehtods are only used to test and make statistics
    bool is_consistent() const;
    bool is_sites_balanced() const;
    unsigned int max_partitions_difference() const; // different between the max and the min number of partitions in the bins
  private:
    const Partitions & _partitions;
    std::vector<Bin> _bins;

};

#endif
