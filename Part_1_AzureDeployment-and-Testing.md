# Results of Part 1

1. After you have finished with the Azure tutorial, measure the time it takes for the Assignment 3 query to run on Azure. What do you notice?
- There is an overhead for creating the container instances compared to running it on the local machine. The overhead increases the time before the containers can start processing the query.
- A single worker runs on a single CPU, hence has less computation power compared to typical multi-core CPUs.


1. Go to the Azure monitoring panel for your containers: explain what is the bottleneck that increases query execution time. Include screenshots if needed.
- The network download rate is only 10 mbit/s for the worker. Since the task's query execution time strongly depends on the download rate, the low download rate becomes a bottleneck. A system with a faster network finishes the task significantly faster.


1. Let’s get faster: Pre-upload the data partitions and fileList inside Azure blob storage. Adapt your solution to read from there. What is the speedup you observe? How is it explained? 
- To be expected would be an increased download rate compared to the 10mbit/s mentioned above. The faster download rate would be explained by the fact, that the Microsoft Azure backbone can be used which may optimize the network between the containers and the blob storage. However, after adapting the solution to read from the blob storage instead of downloading files via http requests, the download time is not reduced. This may be explained by a bad network setup inside of Microsoft Azure. A solution to resolve this problem would be to upload the data partitions and fileList to Azure filesystem instead of blob storage. With that it would be possible to mount the filesystem inside of the container instances and would provide fast write and reading from files in scale of a local SSD.