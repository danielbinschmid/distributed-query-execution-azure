# Managing Shared State for Distributed Query Execution

In this project, we implement a distributed query execution with shared state. 
We now process a query that needs to share state between workers. 
As a query, the goal is to calculate how often each domain appeared (not only a specific domain as in the last
assignment) in a list of domains, and report the result for the top 25 domains.

For each input partition, we build multiple (partial) aggregates, one for each domain, which we then merge.
After the initial aggregation and partitioning, we distribute the work of merging the partial aggregate partitions, and
send the merged results to the coordinator.

## Azure Cloud

The code is written to run on Azure cloud (can be run locally as well). For step-by-step instructions on how to run on Azure, follow the [Azure Tutorial](./AZURE_TUTORIAL.md).

## Benchmarking

We provide 3 scripts for evaluation and benchmarking. They can be run via:

```s
$ ./testCorrectness.sh
$ ./testElasticity.sh
$ ./testResilience.sh
```

## Documentation

Find our report [here](./report.pdf).
