
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
