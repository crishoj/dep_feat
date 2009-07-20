#!/usr/bin/env ruby

require 'rubygems'
require 'commander'
require 'activesupport'

program :description, 'Process a corpus in different ways'
program :version, '0.1'

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

command :count do |c|
  c.syntax = 'corpus count [options] DIR'
  c.description = 'Count tokens/sentences, optionally restricted by occurrence of POS tags or word FORMs'
  c.option '--recursive', 'Also look for corpus files in subdirs'
  c.option '--print', 'Print matched sentences'
  GREP_OPTIONS.each { |o| c.option o }
  c.when_called do |args, options|
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      grep_args = extract_grep_args(options)
      files = Dir.glob("#{args[0]}/*/*.conll") 
      files += Dir.glob("#{args[0]}/*/*/*.conll") if options.recursive
      files.each do |file|
        corpus = Conll::Corpus.parse(file)
        ts = corpus.sentences.size
        next if ts == 0
        sc = 0
        corpus.grep(grep_args) do |sentence|
          sc += 1
          puts "#{sentence}\n\n" if options.print
        end
        puts sprintf("%-50s: %d/%d sentences (%d%%)", file, sc, ts, sc.fdiv(ts)*100)
      end
    else
      say "Invalid corpus directory"
    end
  end
end

command :survey do |c|
  c.when_called do |args, options|
    categories = {
      'quotes'      => { :form => '"' },
      'parenthesis' => { :form => '(' },
      'colon'       => { :form => ':' },
      'hypen'       => { :form => '-' }
    }
    Dir.glob("#{args[0]}/test/*.conll").each do |file|
      gold   = Conll::Corpus.parse(file)
      system = Conll::Corpus.parse(file.sub(/test\/.+$/, 'system/out.conll'))
      measure(file, system, gold, categories)
    end
  end
end

command :grep do |c|
  c.syntax = 'corpus grep [options] DIR'
  c.description = 'Grep for sentences'
  c.option '--recursive', 'Also look for corpus files in subdirs'
  c.option '--show',      'Show matched sentences'
  c.option '--save NAME', 'Save in subdir constructed from NAME'
  GREP_OPTIONS.each { |o| c.option o }
  c.when_called do |args, options|
    if options.save
      target_dir = args[0]+"-only-#{options.save}"
      puts "   [save to] #{target_dir}"
      make_directory(target_dir)
    end
    files = Dir.glob("#{args[0]}/*/*.conll")
    files += Dir.glob("#{args[0]}/*/*/*.conll") if options.recursive
    files.each do |file|
      if options.save
        target_file = file.sub(args[0], target_dir)
        make_directory(File.dirname(target_file))
        target = File.new(target_file, 'w')
      end
      corpus = Conll::Corpus.parse(file)
      ts = corpus.sentences.size
      sc = 0
      corpus.grep(options) do |sentence|
        sc += 1
        target << "#{sentence}\n\n" if options.save
        puts sentence.forms.join(' ') if options.show
      end
      puts sprintf("%-50s: %d/%d sentences (%d%%)", file, sc, ts, sc.fdiv(ts)*100)
      if options.save
        puts "     [saved] #{target_file}"
        target.close
      end
    end
  end
end

command :truncate do |c|
  c.syntax = 'corpus truncate [options] DIR'
  c.description = 'Truncate corpus files to maximum N sentences'
  c.option '--take N'
  c.option '--skip N'
  c.when_called do |args, options|
    options.default :take => 2000, :skip => 0
    source_dir = args[0]
    target_dir = "#{source_dir}-#{options.take}"
    make_directory(target_dir)
    Dir.glob("#{source_dir}/*/*.conll").each do |source|
      target = source.gsub(source_dir, target_dir)
      target_subdir = File.dirname(target)
      make_directory(target_subdir)
      File.open(target, 'w') do |f|
        corpus = Conll::Corpus.parse(source)
        from = options.skip.to_i
        to   = from + options.take.to_i
        (corpus.sentences[from .. to] || []).each do |sent|
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
      annotator = find_annotator(options.kind).new(gold)
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

command :eval do |c|
  c.syntax = 'corpus eval DIR'
  c.description = 'Run the CoNLL eval script on the given corpus'
  c.when_called do |args, options|
    system = Dir.glob("#{args[0]}/*.conll").first
    gold   = Dir.glob("#{File.dirname(args[0])}/test/*.conll").first
    say `bin/eval.pl -g #{gold} -s #{system}`
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
  c.description = 'Annotate single corpus file'
  c.when_called do |args, options|
    options.default :kind => 'null'
    if args.size > 0 and File.exists?(args[0])
      corpus = Conll::Corpus.parse(args[0])
      annotator = find_annotator(options.kind).new(corpus)
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
  c.description = 'Make a copy of DIR and augment each of its corpus files with the specified KIND of feature.'
  c.option '--kinds ANNOTATOR1+ANNOTATOR2', 'Apply multiple annotators'
  c.when_called do |args, options|
    options.default :kinds => []
    kinds = options.kinds.split(/\+/)
    kinds << options.kind if options.kind
    designation = kinds.join('+')
    if args.size == 0
      say "Missing corpus directory"
    elsif File.directory?(args[0])
      dir = args[0]
      target_dir = "#{dir}-#{designation}"
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
        corpus = Conll::Corpus.parse(source)
        kinds.each do |kind|
          puts "  [annotate] #{source} (#{kind})"
          annotator = find_annotator(kind).new(corpus)
          corpus = annotator.run
        end
        File.open(target, 'w') do |f|
          puts "      [save] #{source} => #{target}"
          f << corpus
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

def find_annotator(kind)
  Annotation.const_get(kind.camelize)
end