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

command :single do |c|
  c.syntax = 'corpus annotate [options] FILE'
  c.description = 'Annotate corpus'
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

command :directory do |c|
  c.syntax = 'annotate directory [options] DIR'
  c.description = 'Make a copy of DIR and annotate each of its corpus files'
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
          say "      [skip] #{source}"
          next
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