
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