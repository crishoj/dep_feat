#!/usr/bin/env ruby

require 'lib/conll/corpus'
require 'lib/annotator'
require 'lib/annotation/null'
require 'rubygems'
require 'commander'

program :description, 'Process a corpus in different ways'
program :version, '0.1'

$annotator_class = Annotation::Null

global_option '--kind ANNOTATION', String, 'Kind of annotation to perform' do |kind|
  filename = "lib/annotation/#{kind}"
  require filename
  classname = kind[0].upcase + kind[1..-1]
  $annotator_class = Annotation.const_get(classname)  
end

command :evaluate do |c|
  c.syntax = 'corpus evaluate [options] DIR'
  c.description = 'Gather performance metrics related to the specified KIND of augmented feature'
  c.when_called do |args, options|
    options.default :kind => 'null'
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      dir  = args[0]
      gold     = Conll::Corpus.parse Dir.glob("#{dir}/test/*.conll").first
      baseline = Conll::Corpus.parse Dir.glob("#{dir}/baseline/*.conll").first
      system   = Conll::Corpus.parse Dir.glob("#{dir}/system/*.conll").first
      present_metrics 'baseline', $annotator_class.new(baseline).evaluate(gold)
      present_metrics 'system',   $annotator_class.new(system).evaluate(gold)
    else
      say "Invalid corpus directory"
    end
  end
end

command :annotate_single do |c|
  c.syntax = 'corpus annotate [options] FILE'
  c.description = 'Annotate singel corpus file'
  c.when_called do |args, options|
    options.default :kind => 'null'
    if args.size > 0 and File.exists?(args[0])
      corpus = Conll::Corpus.parse(args[0])
      annotator = $annotator_class.new(corpus)
      annotator.run do |line|
        puts line
      end
    else
      say "Missing corpus file"
    end
  end
end

command :annotate do |c|
  c.syntax = 'corpus annotate [options] DIR'
  c.description = 'Make a copy of DIR and augment each of its corpus files with the specified KIND of feature'
  c.when_called do |args, options|
    options.default :kind => 'null'
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      dir = args[0]
      target_dir = "#{dir}-#{options.kind}"
      make_directory(target_dir)
      Dir.glob("#{dir}/*/*.conll").each do |source|
        target = source.gsub(dir, target_dir)
        target_subdir = File.dirname(target)
        if target_subdir =~ /system$/
          # Annotate a copy of the baseline system output (for evaluation)
          say "  [baseline] #{source}"
          target_subdir.gsub! /system/, 'baseline'
          target.gsub!        /system/, 'baseline'
        end
        make_directory(target_subdir)
        File.open(target, 'w') do |f|
          corpus = Conll::Corpus.parse(source)
          annotator = $annotator_class.new(corpus)
          annotator.run do |line|
            f.puts(line)
          end
          puts "  [annotate] #{source} => #{target}"
        end
      end
    else
      say "Invalid corpus directory"
    end
  end
end

def make_directory(dir)
  if File.exists?(dir)
    say "    [exists] #{dir}"
  else
    Dir.mkdir(dir)
    say "     [mkdir] #{dir}"
  end
end

def present_metrics(what, metrics)
  puts
  puts "[#{what}]"
  puts sprintf("  %-10s %9s %6s %9s %6s %9s %6s", "",
    "tokens",  "in %",
    "head OK", "in %",
    "both OK", "in %")
  total_tokens = metrics[:total][:tokens]
  metrics.each do |category, results|
    puts sprintf("  %-10s %9d %6.2f %9d %6.2f %9d %6.2f", category,
      results[:tokens],       results[:tokens].to_f / total_tokens.to_f * 100.0,
      results[:head_correct], results[:head_correct].to_f / results[:tokens].to_f * 100.0,
      results[:both_correct], results[:both_correct].to_f / results[:tokens].to_f * 100.0
    )
  end

end