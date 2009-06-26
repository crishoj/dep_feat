#!/bin/sh
CORPUS_DIR=$1
MODEL_DIR=models/$CORPUS_DIR
SYSTEM_DIR=$CORPUS_DIR/system
mkdir -p $MODEL_DIR
mkdir -p $SYSTEM_DIR
$MSTPARSER_DIR/bin/mst_parse.sh \
  train train-file:$(echo $CORPUS_DIR/train/*.conll) model-name:$MODEL_DIR/dep.model \
  test test-file:$(echo $CORPUS_DIR/test/*.conll) output-file:$SYSTEM_DIR/out.conll \
  eval gold-file:$(echo $CORPUS_DIR/test/*.conll)
