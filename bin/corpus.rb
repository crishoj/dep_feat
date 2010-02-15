#!/usr/bin/env ruby
# encoding: UTF-8

require 'rubygems'
require 'active_support'
require 'commander/import'
require 'terminal-table/import'

program :version, '0.1'
program :description, 'Process a corpus in different ways'

# Autoloading
ActiveSupport::Dependencies.load_paths << File.join(File.dirname(File.dirname(__FILE__)), 'lib')

class String
  include ActiveSupport::CoreExtensions::String::Inflections
end

global_option '--kind ANNOTATION', String, 'Kind of annotation to perform' 

GREP_OPTIONS = [
  '--pos TAG',
  '--cpos TAG',
  '--form WORD',
  '--form-re REGEX',
  '--feat FEAT',
  '--deprel DEP'
]

def find_corpus_file(path, kind)
  Dir.glob("#{path}/#{kind}/*.conll").first or raise "No #{kind} corpus found at #{path}"
end

def make_directory(dir)
  if File.exists?(dir)
    say "    [exists] #{dir}"
  else
    Dir.mkdir(dir)
    say "     [mkdir] #{dir}"
  end
end

def measure(what, corpus, gold, categories)
  puts
  puts "[#{what}]"
  puts sprintf("  %-15s %9s %8s %9s %8s %9s %8s %9s %8s", "",
    "sentences", "ratio",
    "tokens",    "ratio",
    "head OK",   "ratio",
    "both OK",   "ratio")
  metrics = corpus.evaluate(gold, categories)
  metrics.each do |category, results|
    puts sprintf("  %-15s %9d %.6f %9d %.6f %9d %.6f %9d %.6f", category,
      results[:sentences],    results[:sentences].to_f / metrics[:total][:sentences].to_f,
      results[:tokens],       results[:tokens].to_f / metrics[:total][:tokens].to_f,
      results[:head_correct], results[:head_correct].to_f / results[:tokens].to_f,
      results[:both_correct], results[:both_correct].to_f / results[:tokens].to_f
    )
  end

end

def extract_grep_args(options)
  args = {}
  GREP_OPTIONS.collect {
    |opt| opt.split(/ /).first.gsub(/^--/, '').gsub(/-/, '_')
  }.each { |name|
    if (val = eval("options.#{name}"))
      args[name.to_sym] = (name =~ /-re$/) ? Regexp.compile(val) : val
    end
  }
  args
end

Dir.glob(File.dirname(__FILE__)+'/commands/*.rb').each do |command_file|
  require command_file
end
