
command :annotate do |c|
  c.syntax = 'corpus annotate [options] DIR'
  c.description = 'Make a copy of DIR and augment each of its corpus files with the specified KIND of feature.'
  c.option '--kinds ANNOTATOR1+ANNOTATOR2', 'Apply multiple annotators'
  c.when_called do |args, options|
    kinds = options.kinds ? options.kinds.split(/\+/) : []
    kinds << options.kind if options.kind
    designation = kinds.join('+')
    if args.size == 0
      say "Missing corpus directory"
    else
      for dir in args
        if File.directory?(dir)
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
              annotator = Annotation::find_annotator(kind).new
              corpus = annotator.annotate(corpus)
            end
            File.open(target, 'w') do |f|
              puts "      [save] #{source} => #{target}"
              f << corpus
            end
          end
        else
          say "Invalid corpus directory: #{dir}"
        end
      end
    end
  end
end