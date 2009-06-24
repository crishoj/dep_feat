#!/bin/sh
$MSTPARSER_DIR/bin/mst_parse.sh \
  train train-file:data/danish/ddt/train/danish_ddt_train.conll model-name:dep.model \
  test test-file:data/danish/ddt/test/danish_ddt_test.conll output-file:out.txt \
  eval gold-file:data/danish/ddt/test/danish_ddt_test.conll
