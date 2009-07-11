#!/usr/bin/env ruby

require 'lib/conll/corpus'
require 'lib/annotator'
require 'lib/annotation/null'
require 'rubygems'
require 'commander'
require 'activesupport'

program :description, 'Process a corpus in different ways'
program :version, '0.1'

class String
  include ActiveSupport::CoreExtensions::String::Inflections
end

$annotator_class = Annotation::Null

global_option '--kind ANNOTATION', String, 'Kind of annotation to perform' do |kind|
  filename = "lib/annotation/#{kind}"
  require filename
  $annotator_class = Annotation.const_get(kind.classify)
end

command :count do |c|
  c.syntax = 'corpus count [options] DIR'
  c.description = 'Count tokens/sentences, optionally restricted by occurrence of POS tags or word FORMs'
  c.option '--pos TAG'
  c.option '--cpos TAG'
  c.option '--form WORD'
  c.option '--form-re REGEX'
  c.option '--feat FEAT'
  c.option '--recursive', 'Also look for corpus files in subdirs'
  c.option '--print', 'Print matched sentences'
  c.when_called do |args, options|
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      form_re = Regexp.compile(options.form_re) if options.form_re
      files = Dir.glob("#{args[0]}/*/*.conll") 
      files += Dir.glob("#{args[0]}/*/*/*.conll") if options.recursive
      files.each do |file|
        corpus = Conll::Corpus.parse(file)
        tc = sc = tt = ts = 0
        for sentence in corpus.sentences
          ts += 1
          matched = false
          for token in sentence.tokens
            tt += 1
            next if options.pos  and not token.pos == options.pos
            next if options.cpos and not token.cpos == options.cpos
            next if options.form and not token.form == options.form
            next if options.feat and not token.features.include? options.feat
            next if options.form_re and not token.form =~ form_re
            matched = true
            tc += 1
          end
          if matched
            sc += 1
            puts "#{sentence}\n\n" if options.print
          end
        end
        puts sprintf("%-50s: %d/%d sentences (%d%%), %d/%d tokens (%d%%)",
          file, sc, ts, sc.fdiv(ts)*100, tc, tt, tc.fdiv(tt)*100)
      end
    else
      say "Invalid corpus directory"
    end
  end
end

command :truncate do |c|
  c.syntax = 'corpus truncate --max N DIR'
  c.description = 'Truncate corpus files to maximum N sentences'
  c.when_called do |args, options|
    options.default :max => 1000
    source_dir = args[0]
    target_dir = "#{source_dir}-#{options.max}"
    make_directory(target_dir)
    Dir.glob("#{source_dir}/*/*.conll").each do |source|
      target = source.gsub(source_dir, target_dir)
      target_subdir = File.dirname(target)
      make_directory(target_subdir)
      File.open(target, 'w') do |f|
        corpus = Conll::Corpus.parse(source)
        corpus.sentences[0..options.max].each do |sent|
          sent.tokens.each do |tok|
            f.puts tok.to_s
          end
          f.puts
        end
        puts "  [truncate] #{source} => #{target}"
      end
    end
  end
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
      measure 'gold',     gold,     gold
      measure 'baseline', baseline, gold
      measure 'system',   system,   gold
    else
      say "Invalid corpus directory"
    end
  end
end

command :categorize do |c|
  c.syntax = 'corpus categorize --kind ANNOTATOR DIR'
  c.description = 'Split corpus into several parts according to the presence of the feature introduced by the specified annotator. This is useful for evaluating the effect of introducing the feature.'
  c.when_called do |args, options|
    if not options.kind
      say "Missing annotator class"
    elsif args.size == 0
      say "Missing corpus directory"
    elsif not File.directory?(args[0])
      say "Invalid corpus directory"
    else
      dir  = args[0]
      gold_file = Dir.glob("#{dir}/test/*.conll").first
      say "      [gold] #{gold_file}"
      gold = Conll::Corpus.parse gold_file
      annotator = $annotator_class.new(gold)
      %w{baseline test system}.each do |section|
        filename = Dir.glob("#{dir}/#{section}/*.conll").first
        corpus = Conll::Corpus.parse filename
        say "[categorize] #{filename}"
        annotator.categorize(corpus).each_pair do |category, part|
          category_filename = filename.sub(/#{section}/, "#{section}/#{category}")
          make_directory File.dirname(category_filename)
          say "      [part] #{category} => #{category_filename}"
          File.open(category_filename, 'w') do |f|
            f << part
          end
        end
      end
    end
  end
end

command :compare do |c|
  c.syntax = 'corpus compare DIR CATEGORY'
  c.description = 'Use the CoNLL eval script to compare a baseline against the system output, in order to determine the effect of the introduced feature.'
  c.when_called do |args, options|
    if args.size < 1
      say "Missing corpus directory"
    else
      dir = args[0]
      cat = args[1] || ''
      gold = Dir.glob("#{dir}/test/#{cat}/*.conll").first
      say "[gold] #{gold}"
      contenders = %w{baseline system}.collect { |name|
        Dir.glob("#{dir}/#{name}/#{cat}/*.conll").first
      }
      evalbs = []
      for file in contenders
        say "[eval] #{file}"
        puts `bin/eval.pl -q -g #{gold} -s #{file} 2> /dev/null`
      end
      evalbs = contenders.collect { |file|
        evalb = File.dirname(file) + "/evalb.out"
        say "[evalb] #{file} => #{evalb}"
        `bin/eval.pl -b -q -g #{gold} -s #{file} > #{evalb} 2> /dev/null`
        evalb
      }
      say "[compare] #{evalbs[0]} #{evalbs[1]}"
      puts `bin/compare.pl #{evalbs[0]} #{evalbs[1]} 2> /dev/null`
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
  c.description = <<DESC
Make a copy of DIR and augment each of its corpus files with the
  specified KIND of feature.
DESC

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
        if target_subdir =~ /baseline$/
          next
        elsif target_subdir =~ /system$/
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

def measure(what, corpus, gold)
  puts
  puts "[#{what}]"
  puts sprintf("  %-15s %9s %6s %9s %6s %9s %6s", "",
    "tokens",  "in %",
    "head OK", "in %",
    "both OK", "in %")
  metrics = $annotator_class.new(corpus).evaluate(gold)
  total_tokens = metrics[:total][:tokens]
  metrics.each do |category, results|
    puts sprintf("  %-15s %9d %6.2f %9d %6.2f %9d %6.2f", category,
      results[:tokens],       results[:tokens].to_f / total_tokens.to_f * 100.0,
      results[:head_correct], results[:head_correct].to_f / results[:tokens].to_f * 100.0,
      results[:both_correct], results[:both_correct].to_f / results[:tokens].to_f * 100.0
    )
  end

end
