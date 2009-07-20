#!/bin/bash

svm-train -s 0 -t 2 -g 0.5 -c 8.0 Swedish-data10_all.features data10_all.features.model
svm-predict Danish-data10_all.features data10_all.features.model data10_all_danish.results

