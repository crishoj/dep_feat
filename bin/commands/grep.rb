
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