# DUET

This folder contains source codes for the paper DUET: A Generic Framework for Finding Specific Secondary Elements in Data Streams.

You can find the source codes of DUET used for each application in path '/HH', '/TH', and '/PP'. 

The data sets used in the paper can be found in [CAIDA1](https://www.caida.org/data/passive/passive_2016_dataset.xml), [CAIDA2](https://www.caida.org/data/passive/passive_2016_dataset.xml), [Stack Overflow Temporal Network](http://snap.stanford.edu/data/sx-stackoverflow.html), and [Reddit Hyperlink Network](http://snap.stanford.edu/data/soc-RedditHyperlinks.html).

We provide CAIDA1 (/data/125910.rar) in this repsitory. You can unzip and use it to run examples. 

# How to run:
    Build
    cd build
    cmake ..
    make
    ./mainXX (e.g., ./mainHH to test the application HH)


# Experiments

We use [Space Saving](https://github.com/papergitkeeper/heavy-keeper-project), [On-Off sketch](https://github.com/Sketch-Data-Stream/On-Off-Sketch), and [Small-Space](https://github.com/Sketch-Data-Stream/On-Off-Sketch/tree/master/FPI) for comparison in our experiments.